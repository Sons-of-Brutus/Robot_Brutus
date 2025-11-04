import pybullet as p
import pybullet_data
import time
import os
import math

import numpy as np
import csv
from datetime import datetime

brutus_id_global = None

BRUTUS_PATH = os.path.abspath("./model/urdf/brutus.urdf")

GRAVITY = -9.8

FORCE = 70  # par máximo, ajusta 30–80 según tu modelo
MAX_TORQUE = 0.22

START_POS = (0,0,0.08)
START_ORIENTATION = p.getQuaternionFromEuler([0, 0, -math.pi/2])

SIM_TIME_STEP = 1/240

TEST_ANGLE = math.radians(30)

# JOINTS
BACK_RIGHT_JOINTS = {'elbow':20, 'bottom':24, 'foot':26}
BACK_LEFT_JOINTS = {'elbow':29, 'bottom':33, 'foot':35}
FRONT_LEFT_JOINTS = {'elbow':11, 'bottom':15, 'foot':17}
FRONT_RIGHT_JOINTS = {'elbow':2, 'bottom':6, 'foot':8}

LOG_JOINT_IDXS = [
    FRONT_RIGHT_JOINTS["elbow"], FRONT_LEFT_JOINTS["elbow"],
    BACK_RIGHT_JOINTS["elbow"],  BACK_LEFT_JOINTS["elbow"],
    FRONT_RIGHT_JOINTS["bottom"], FRONT_LEFT_JOINTS["bottom"],
    BACK_RIGHT_JOINTS["bottom"],  BACK_LEFT_JOINTS["bottom"],
]

LOG_JOINT_NAMES = [
    "fr_elbow","fl_elbow","br_elbow","bl_elbow",
    "fr_bottom","fl_bottom","br_bottom","bl_bottom"
]



ELBOWS_INIT_POSITIONS = {'br':(-math.radians(20)), 'bl':(math.radians(20)), 'fl':(-math.radians(20)), 'fr':(math.radians(20))}

# 600°/s = 600*(180/π) ​= 10.4719755 rad/s
MAX_SERVOS_SPEED = 10.47

WAIT_MOV = 0.05


# JOINTD ANGLES

r40 = math.radians(40)
r15 = math.radians(15)
r30 = math.radians(30)

# FORWARD
FRONT_RIGHT_JOINT_FORWARD = r40
BACK_RIGHT_JOINT_FORWARD  = -math.radians(20)
FRONT_LEFT_JOINT_FORWARD  = -r40
BACK_LEFT_JOINT_FORWARD   = math.radians(20)

# BOTTOM_UP
BACK_RIGHT_JOINT_BOTTOM_UP  = r30
FRONT_LEFT_JOINT_BOTTOM_UP  = r30
BACK_LEFT_JOINT_BOTTOM_UP   = -r30
FRONT_RIGHT_JOINT_BOTTOM_UP = -r30

# MID_F (mitad de 40°)
FRONT_RIGHT_JOINT_MID_F = r40 / 2
BACK_RIGHT_JOINT_MID_F  = r40 / 2
FRONT_LEFT_JOINT_MID_F  = -r40 / 2
BACK_LEFT_JOINT_MID_F   = -r40 / 2

# BACKWARD
FRONT_RIGHT_JOINT_BACKWARD = 0.0
FRONT_LEFT_JOINT_BACKWARD  = 0.0
BACK_RIGHT_JOINT_BACKWARD  = -math.radians(50)
BACK_LEFT_JOINT_BACKWARD   =  math.radians(50)

POS_ERROR = 0.05

#state machine
UP = 0
FORWARD = 1
RETURN = 2

robot_state = 0


# ---- CONFIG PIES PLANOS ----
FOOT_OFFSETS = {
    "fr": 0.0,  # ajusta si el pie no queda exactamente plano
    "fl": 0.0,
    "br": 0.0,
    "bl": 0.0,
}

# Si algún eje está invertido por la kinemática local, cambia los signos aquí.
SIGNS = {
    # cómo suma el codo y el bottom para cada pata (ajusta si ves que “se pasa” o va al revés)
    "fr": {"elbow": +1.0, "bottom": -1.0, "foot": -1.0},
    "fl": {"elbow": -1.0, "bottom": +1.0, "foot": -1.0},
    "br": {"elbow": -1.0, "bottom": -1.0, "foot": +1.0},
    "bl": {"elbow": +1.0, "bottom": +1.0, "foot": +1.0},
}

LEGS = {
    "fr": FRONT_RIGHT_JOINTS,
    "fl": FRONT_LEFT_JOINTS,
    "br": BACK_RIGHT_JOINTS,
    "bl": BACK_LEFT_JOINTS,
}

# Mapas de conveniencia por pata
BOTTOM_UP = {
    "fr": FRONT_RIGHT_JOINT_BOTTOM_UP,
    "fl": FRONT_LEFT_JOINT_BOTTOM_UP,
    "br": BACK_RIGHT_JOINT_BOTTOM_UP,
    "bl": BACK_LEFT_JOINT_BOTTOM_UP,
}

ELBOW_FWD = {
    "fr": FRONT_RIGHT_JOINT_FORWARD,
    "fl": FRONT_LEFT_JOINT_FORWARD,
    "br": BACK_RIGHT_JOINT_FORWARD,
    "bl": BACK_LEFT_JOINT_FORWARD,
}

ELBOW_BACK = {
    "fr": FRONT_RIGHT_JOINT_BACKWARD,
    "fl": FRONT_LEFT_JOINT_BACKWARD,
    "br": BACK_RIGHT_JOINT_BACKWARD,
    "bl": BACK_LEFT_JOINT_BACKWARD,
}

# Ángulo de apoyo del bottom (tal como lo tenías en WAIT_INIT)
BOTTOM_SUPPORT = {
    "fr":  TEST_ANGLE,
    "fl": -TEST_ANGLE,
    "br": -TEST_ANGLE,
    "bl":  TEST_ANGLE,
}


def compute_flat_foot_target(brutus_id, leg_key):
    joints = LEGS[leg_key]
    s = SIGNS[leg_key]
    q_bottom = p.getJointState(brutus_id, joints["bottom"])[0]

    # objetivo de pie para “anular” la rotación total y quedar plano
    q_foot_des = - (s["bottom"]*q_bottom) + FOOT_OFFSETS[leg_key]
    # si el eje del propio pie está invertido, aplica también el signo:
    q_foot_des *= s["foot"]

    #print(f"({leg_key}) bottom: {q_bottom}, foot desired: {q_foot_des}")

    return q_foot_des

def keep_feet_flat(brutus_id, kp=1.0):
    """Lanza órdenes a los 4 pies para que se mantengan planos (se llama en cada iteración)."""
    for leg_key in ["fr", "fl", "br", "bl"]:
        q_des = compute_flat_foot_target(brutus_id, leg_key)
        p.setJointMotorControl2(
            brutus_id,
            LEGS[leg_key]["foot"],
            p.POSITION_CONTROL,
            targetPosition=q_des,
            force=80,
            maxVelocity=600
        )

def wait_sim(dt, sim_step=SIM_TIME_STEP):
    steps = int(dt/sim_step)
    finished = False

    for i in range(steps):
        if brutus_id_global is not None:
            keep_feet_flat(brutus_id_global)

        p.stepSimulation()
        time.sleep(sim_step)



def is_in_pos(brutus_id, j, q_target):
  theshold = 0.3
  q_now, dq_now, *_ = p.getJointState(brutus_id, j)
  reach_p = abs(q_now - q_target)
  print("reach_p: ",reach_p)

  if(reach_p < theshold):
     return True
  
  return False

def all_joints_in_pos(brutus_id, targets):
    for j, q in targets:
      if not is_in_pos(brutus_id, j, q):
         return False
    
    return True

       

def set_phase(brutus_id, targets, dt=0.03, stagger=0.0):
    """
    targets: lista [(jointIndex, targetPosition), ...]
    - Aplica TODOS los targets de este mini-paso.
    - Si stagger>0, hace una micro-pausa entre joints.
    - Luego espera wait_after antes de seguir.
    """
    
    for j, q in targets:
        p.setJointMotorControl2(
            brutus_id, j, p.POSITION_CONTROL,
            targetPosition=q, maxVelocity=MAX_SERVOS_SPEED, force=MAX_TORQUE
        )

        if stagger > 0.0:
            wait_sim(stagger)  # micro-pausa entre joints del mismo paso
    
    #while not all_joints_in_pos(brutus_id, targets):
    wait_sim(dt)      # pausa entre mini-pasos



def movement_1(brutus_id):
  print("MOV 1")

  # Fase 1: levantar y adelantar FR a la vez (bottom + elbow) y plantar un poco el pie
  set_phase(brutus_id, [
      (FRONT_RIGHT_JOINTS["bottom"], FRONT_RIGHT_JOINT_BOTTOM_UP),
      (FRONT_RIGHT_JOINTS["elbow"],  FRONT_RIGHT_JOINT_FORWARD),

      (BACK_LEFT_JOINTS["bottom"], BACK_LEFT_JOINT_BOTTOM_UP),
      (BACK_LEFT_JOINTS["elbow"],  BACK_LEFT_JOINT_FORWARD)


      #(FRONT_RIGHT_JOINTS["bottom"], TEST_ANGLE)
  ], dt=1, stagger=0.5)

  
def movement_2(brutus_id):
  print("MOV 2")

  # Fase 1: levantar y adelantar FR a la vez (bottom + elbow) y plantar un poco el pie
  set_phase(brutus_id, [
      (BACK_LEFT_JOINTS["bottom"], BACK_LEFT_JOINT_BOTTOM_UP),
      (BACK_LEFT_JOINTS["elbow"],  BACK_LEFT_JOINT_FORWARD)
      #(BACK_LEFT_JOINTS["bottom"], TEST_ANGLE)
  ], dt=1, stagger=0.5)

def movement_3(brutus_id):
  print("MOV 3")

  # Fase 1: levantar y adelantar FR a la vez (bottom + elbow) y plantar un poco el pie
  set_phase(brutus_id, [
      (FRONT_LEFT_JOINTS["bottom"], FRONT_LEFT_JOINT_BOTTOM_UP),
      (FRONT_LEFT_JOINTS["elbow"],  FRONT_LEFT_JOINT_FORWARD)
      #(FRONT_LEFT_JOINTS["bottom"], -TEST_ANGLE)
  ], dt=1, stagger=0.5)


def movement_4(brutus_id):
  print("MOV 4")

  # Fase 1: levantar y adelantar FR a la vez (bottom + elbow) y plantar un poco el pie
  set_phase(brutus_id, [
      (BACK_RIGHT_JOINTS["bottom"], BACK_RIGHT_JOINT_BOTTOM_UP),
      (BACK_RIGHT_JOINTS["elbow"],  BACK_RIGHT_JOINT_FORWARD)
      #(BACK_RIGHT_JOINTS["bottom"], -TEST_ANGLE)
  ], dt=1, stagger=0.5)




'''
def trot_pair_step(brutus_id, swing=("fr","bl"), dt_lift=0.10, dt_lower=0.10, dt_push=0.12):
  """Un paso de trote:
      - swing: pareja que se levanta y avanza (por defecto FR+BL)
      - la pareja soporte es la complementaria (FL+BR) que empuja después
  """
  support = tuple({"fr","fl","br","bl"} - set(swing))

  # 1) Levantar bottoms y adelantar elbows de ambas patas swing (a la vez)
  lift_targets = []
  for leg in swing:
      lift_targets.append((LEGS[leg]["bottom"], BOTTOM_UP[leg]))
      lift_targets.append((LEGS[leg]["elbow"],  ELBOW_FWD[leg]))
  set_phase(brutus_id, lift_targets, dt=dt_lift, stagger=0.0)

  # 2) Bajar esas dos patas a su ángulo de apoyo (a la vez)
  lower_targets = []
  for leg in swing:
      lower_targets.append((LEGS[leg]["bottom"], BOTTOM_SUPPORT[leg]))
  set_phase(brutus_id, lower_targets, dt=dt_lower, stagger=0.0)

  # 3) Empujar con la pareja de soporte (llevar sus elbows a backward)
  push_targets = []
  for leg in support:
      push_targets.append((LEGS[leg]["elbow"], ELBOW_BACK[leg]))
      push_targets.append((LEGS[leg]["bottom"], BOTTOM_SUPPORT[leg]))
  set_phase(brutus_id, push_targets, dt=dt_push, stagger=0.0)
'''

def trot_pair_step(brutus_id, swing=("fr","bl"), dt_lift=0.10, dt_lower=0.10, dt_push=0.12):
  support = tuple({"fr","fl","br","bl"} - set(swing))

  # 1) Levantar bottoms
  lift_targets = []
  for leg in swing:
      lift_targets.append((LEGS[leg]["bottom"], BOTTOM_UP[leg]))
  set_phase(brutus_id, lift_targets, dt=dt_lift, stagger=0.0)

  # 2) Adelantar elbows de ambas patas swing (a la vez) y empujar con la pareja de soporte (llevar sus elbows a backward)
  advance_targets = []
  for leg in swing:
      advance_targets.append((LEGS[leg]["elbow"],  ELBOW_FWD[leg]))

  for leg in support:
      advance_targets.append((LEGS[leg]["elbow"], ELBOW_BACK[leg]))
      advance_targets.append((LEGS[leg]["bottom"], BOTTOM_SUPPORT[leg]))
  set_phase(brutus_id, advance_targets, dt=dt_lift, stagger=0.0)

  ## 3) Empujar con la pareja de soporte (llevar sus elbows a backward)
  #push_targets = []
  #for leg in support:
  #    push_targets.append((LEGS[leg]["elbow"], ELBOW_BACK[leg]))
  #    push_targets.append((LEGS[leg]["bottom"], BOTTOM_SUPPORT[leg]))
  #set_phase(brutus_id, push_targets, dt=dt_push, stagger=0.0)

  # 4) Bajar esas dos patas a su ángulo de apoyo (a la vez)
  lower_targets = []
  for leg in swing:
      lower_targets.append((LEGS[leg]["bottom"], BOTTOM_SUPPORT[leg]))
  set_phase(brutus_id, lower_targets, dt=dt_lower, stagger=0.0)

def read_elbows_bottom_torques(brutus_id):
    # lista de indices de elbows y bottoms
    idxs = [
        FRONT_RIGHT_JOINTS["elbow"], FRONT_LEFT_JOINTS["elbow"],
        BACK_RIGHT_JOINTS["elbow"],  BACK_LEFT_JOINTS["elbow"],
        FRONT_RIGHT_JOINTS["bottom"], FRONT_LEFT_JOINTS["bottom"],
        BACK_RIGHT_JOINTS["bottom"],  BACK_LEFT_JOINTS["bottom"],
    ]

    states = p.getJointStates(brutus_id, idxs)
    # states[i] = (position, velocity, reactionForces[6], appliedJointMotorTorque)

    # Mapea a un dict legible
    names = [
        "fr_elbow","fl_elbow","br_elbow","bl_elbow",
        "fr_bottom","fl_bottom","br_bottom","bl_bottom"
    ]

    data = {}
    for name, st in zip(names, states):
        q, dq, react, tau_motor = st
        # tau_motor: par aplicado por el actuador en el último paso
        # react[3:6] = (Mx, My, Mz) momento de reacción en marco mundial
        data[name] = {
            "tau_motor": tau_motor,
            "Mx_react": react[3],
            "My_react": react[4],
            "Mz_react": react[5],
        }
    return data

def enable_force_sensors_for_logged_joints(brutus_id):
    for j in LOG_JOINT_IDXS:
        p.enableJointForceTorqueSensor(brutus_id, j, enableSensor=1)

def read_logged_joint_torques(brutus_id):
    """Devuelve dict nombre -> {tau, Mx, My, Mz}"""
    states = p.getJointStates(brutus_id, LOG_JOINT_IDXS)
    out = {}
    for name, st in zip(LOG_JOINT_NAMES, states):
        q, dq, react, tau = st  # tau = appliedJointMotorTorque
        out[name] = {
            "tau": abs(tau),
            "Mx": react[3],
            "My": react[4],
            "Mz": react[5],
        }
    return out

def init_csv_logger():
    # Siempre el mismo fichero
    fname = "logs/torques.csv"
    os.makedirs(os.path.dirname(fname), exist_ok=True)

    # "w" = sobrescribe en cada ejecución
    f = open(fname, "w", newline="")   # newline="" evita líneas en blanco extra en Windows
    writer = csv.writer(f)

    # Cabecera fija
    header = ["t_seconds"]
    for n in LOG_JOINT_NAMES:
        header.append(f"{n}_tau")
        # Si algún día guardas Mx/My/Mz, añade aquí las columnas como antes
    writer.writerow(header)

    print(f"[logger] Escribiendo torques en {fname} (sobrescribiendo)")
    return f, writer, fname


def write_csv_row(writer, t, data_dict):
    row = [t]
    for n in LOG_JOINT_NAMES:
        row.append(data_dict[n]["tau"])
        # si activaste Mx/My/Mz en cabecera, añade también:
        # row.extend([data_dict[n]["Mx"], data_dict[n]["My"], data_dict[n]["Mz"]])
    writer.writerow(row)

  


  


last_print = time.time()

def main():
  physics_client = p.connect(p.GUI)
  p.setAdditionalSearchPath(pybullet_data.getDataPath())
  p.setGravity(0,0, GRAVITY)

  plane_id = p.loadURDF("plane.urdf")
  brutus_id = p.loadURDF(BRUTUS_PATH,
                         basePosition=START_POS,
                         baseOrientation=START_ORIENTATION,
                         #useFixedBase=True
                        )
  
  for j in [
    FRONT_RIGHT_JOINTS["elbow"], FRONT_LEFT_JOINTS["elbow"],
    BACK_RIGHT_JOINTS["elbow"],  BACK_LEFT_JOINTS["elbow"],
    FRONT_RIGHT_JOINTS["bottom"], FRONT_LEFT_JOINTS["bottom"],
    BACK_RIGHT_JOINTS["bottom"],  BACK_LEFT_JOINTS["bottom"],
  ]:
    p.enableJointForceTorqueSensor(brutus_id, j, enableSensor=1)

  
  p.changeDynamics(
    bodyUniqueId=plane_id,
    linkIndex=-1,              # el plano no tiene links: usa -1
    lateralFriction=0.6,       # prueba 3–6
    rollingFriction=0.005,     # opcional, reduce deslizamiento por rodadura
    spinningFriction=0.005     # opcional, evita giros sobre sí mismo
)
  
  enable_force_sensors_for_logged_joints(brutus_id)

  csv_file, csv_writer, csv_path = init_csv_logger()
  print(f"[logger] Escribiendo torques en {csv_path}")

  # tras: csv_file, csv_writer, csv_path = init_csv_logger()
  LOG_DT = 0.02           # frecuencia de log (50 Hz). Sube a 0.05 si quieres menos filas
  last_log_t = time.time()
  t0_log = last_log_t
  rows_written = 0
  FLUSH_EVERY = 50        # fuerza flush cada 50 filas (ajústalo a gusto)


  global brutus_id_global
  brutus_id_global = brutus_id
  
  # tras cargar el plano y el robot:
  p.changeDynamics(plane_id, -1, lateralFriction=5)

  

  # aplica a cada pie; usa los linkIndex de los pies (en PyBullet suele ser el mismo índice del joint del link)
  for foot_idx in [FRONT_RIGHT_JOINTS["foot"], FRONT_LEFT_JOINTS["foot"],
                  BACK_RIGHT_JOINTS["foot"],  BACK_LEFT_JOINTS["foot"],]:
      p.changeDynamics(
          brutus_id, foot_idx,
          lateralFriction=4.0,      # prueba entre 1.5 y 3.0
          rollingFriction=0.003,    # pequeña resistencia al rodar
          spinningFriction=0.003,   # evita “girar sobre sí” sin grip
          frictionAnchor=1,         # “ancla” de fricción -> menos deslizamiento lateral
    )


  numJoints = p.getNumJoints(brutus_id)

  total_mass = 0
  for i in range(-1, numJoints):
    mass = p.getDynamicsInfo(brutus_id, i)[0]
    total_mass += mass

  print(f"Total mass: {total_mass:.6f} kg")

  masses = []
  positions = []

  # Incluir también el cuerpo base (link -1)
  link_indices = [-1] + list(range(numJoints))

  for link_index in link_indices:
      # Obtener masa
      mass = p.getDynamicsInfo(brutus_id, link_index)[0]
      if mass == 0:
          continue  # ignorar links sin masa

      # Obtener posición del centro de masa en coordenadas globales
      if link_index == -1:
          pos, _ = p.getBasePositionAndOrientation(brutus_id)
      else:
          pos = p.getLinkState(brutus_id, link_index, computeForwardKinematics=True)[0]

      masses.append(mass)
      positions.append(np.array(pos))

  # Calcular centro de masas global
  total_mass = np.sum(masses)
  com_global = np.sum([m * r for m, r in zip(masses, positions)], axis=0) / total_mass

  print("Masa total:", total_mass, "kg")
  print("Centro de masas global (x, y, z):", com_global)

#  print("------------------------------------")
#  print("JOINTS:", numJoints)
#
#  for i in range(numJoints):
#      info = p.getJointInfo(brutus_id, i)
#      joint_index = info[0]
#      joint_name = info[1].decode("utf-8")
#      joint_type = info[2]
#      link_index = info[12]
#
#      print("Joint", joint_index)
#      print("  Name -", joint_name)
#      print("  Link -", link_index)
#
#  print("-----------------------------------------")

  # Initialize joint positions
  p.resetJointState(brutus_id, BACK_RIGHT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["br"])
  p.resetJointState(brutus_id, BACK_LEFT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["bl"])
  p.resetJointState(brutus_id, FRONT_RIGHT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["fr"])
  p.resetJointState(brutus_id, FRONT_LEFT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["fl"])

  p.setTimeStep(SIM_TIME_STEP)

  start_time = time.time()
  state = "WAIT_INIT"        # estados: WAIT_INIT -> MOVE1 -> PAUSE -> MOVE2 -> PAUSE -> (repite)
  pause_start = None
  MOVE_DELAY = 0.5

  try:
    while True:
      now = time.time()
      elapsed = now - start_time

      if state == "WAIT_INIT":
        # Elbow
        p.setJointMotorControl2(brutus_id,
          FRONT_RIGHT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["fr"],
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          FRONT_LEFT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["fl"],
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          BACK_RIGHT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["br"],
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          BACK_LEFT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["bl"],
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)

        # Bottom
        p.setJointMotorControl2(brutus_id,
          FRONT_RIGHT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          FRONT_LEFT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          BACK_RIGHT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          BACK_LEFT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        # Foot
        p.setJointMotorControl2(brutus_id,
          FRONT_RIGHT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          FRONT_LEFT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          BACK_RIGHT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)
        
        p.setJointMotorControl2(brutus_id,
          BACK_LEFT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED,
          force=MAX_TORQUE)

        if now - start_time > 3:
          p.setJointMotorControl2(brutus_id,
                                  FRONT_RIGHT_JOINTS["bottom"],
                                  p.POSITION_CONTROL,
                                  targetPosition=BOTTOM_UP["fr"],
                                  maxVelocity=MAX_SERVOS_SPEED,
                                  force=MAX_TORQUE)
          
          p.setJointMotorControl2(brutus_id,
                                  BACK_LEFT_JOINTS["bottom"],
                                  p.POSITION_CONTROL,
                                  targetPosition=BOTTOM_UP["bl"],
                                  maxVelocity=MAX_SERVOS_SPEED,
                                  force=MAX_TORQUE)

          state = "TROT_A"
       

      elif state == "TROT_A":
          # Pareja 1: FR + BL
          trot_pair_step(brutus_id, swing=("fr","bl"))
          pause_start = now
          state = "PAUSE_A"

      elif state == "PAUSE_A":
          if (now - pause_start) >= MOVE_DELAY:
              state = "TROT_B"

      elif state == "TROT_B":
          # Pareja 2: FL + BR
          trot_pair_step(brutus_id, swing=("fl","br"))
          pause_start = now
          state = "PAUSE_B"

      elif state == "PAUSE_B":
          if (now - pause_start) >= MOVE_DELAY:
              state = "TROT_A"
        
      #print("----------")
      #print("BR BOTTOM:", p.getJointState(brutus_id, BACK_RIGHT_JOINTS["bottom"])[0])
      #print("BR FOOT:", p.getJointState(brutus_id, BACK_RIGHT_JOINTS["foot"])[0])

      p.stepSimulation()
      time.sleep(SIM_TIME_STEP)

      # ---- LOG CSV ----
      now_log = time.time()
      if (now_log - last_log_t) >= LOG_DT:
          torques = read_logged_joint_torques(brutus_id)
          write_csv_row(csv_writer, now_log - t0_log, torques)
          rows_written += 1
          if rows_written % FLUSH_EVERY == 0:
              csv_file.flush()  # asegura datos en disco periódicamente
          last_log_t = now_log

  except KeyboardInterrupt:
    pass

  p.disconnect()

if __name__ == '__main__':
  main()