import pybullet as p
import pybullet_data
import time
import os
import math

BRUTUS_PATH = os.path.abspath("./model/urdf/brutus.urdf")

GRAVITY = -9.8

START_POS = (0,0,0.08)
START_ORIENTATION = p.getQuaternionFromEuler([0, 0, -math.pi/2])

SIM_TIME_STEP = 0.005

TEST_ANGLE = math.radians(30)

# JOINTS
BACK_RIGHT_JOINTS = {'elbow':20, 'bottom':24, 'foot':26}
BACK_LEFT_JOINTS = {'elbow':29, 'bottom':33, 'foot':35}
FRONT_LEFT_JOINTS = {'elbow':11, 'bottom':15, 'foot':17}
FRONT_RIGHT_JOINTS = {'elbow':2, 'bottom':6, 'foot':8}

ELBOWS_INIT_POSITIONS = {'br':(-math.radians(10)), 'bl':(math.radians(10)), 'fl':(-math.radians(10)), 'fr':(math.radians(10))}

MAX_SERVOS_SPEED = 600

def main():
  physics_client = p.connect(p.GUI)
  p.setAdditionalSearchPath(pybullet_data.getDataPath())
  p.setGravity(0,0, GRAVITY)

  plane_id = p.loadURDF("plane.urdf")
  brutus_id = p.loadURDF(BRUTUS_PATH,
                         basePosition=START_POS,
                         baseOrientation=START_ORIENTATION)

  numJoints = p.getNumJoints(brutus_id)

  total_mass = 0
  for i in range(-1, numJoints):
    mass = p.getDynamicsInfo(brutus_id, i)[0]
    total_mass += mass

  print(f"Total mass: {total_mass:.6f} kg")

  #print("------------------------------------")
  #print("JOINTS:", numJoints)
#
  #for i in range(numJoints):
  #    info = p.getJointInfo(brutus_id, i)
  #    joint_index = info[0]
  #    joint_name = info[1].decode("utf-8")
  #    joint_type = info[2]
  #    link_index = info[12]
#
  #    if joint_type == p.JOINT_REVOLUTE:
  #        print("Joint", joint_index)
  #        print("  Name -", joint_name)
  #        print("  Link -", link_index)
#
  #print("-----------------------------------------")

  # Initialize joint positions
  p.resetJointState(brutus_id, BACK_RIGHT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["br"])
  p.resetJointState(brutus_id, BACK_LEFT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["bl"])
  p.resetJointState(brutus_id, FRONT_RIGHT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["fr"])
  p.resetJointState(brutus_id, FRONT_LEFT_JOINTS["elbow"], targetValue=ELBOWS_INIT_POSITIONS["fl"])

  p.setTimeStep(SIM_TIME_STEP)

  start_time = time.time()

  try:
    while True:
      if time.time() - start_time >= 3:
        # Elbow
        p.setJointMotorControl2(brutus_id,
          FRONT_RIGHT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["fr"],
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          FRONT_LEFT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["fl"],
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          BACK_RIGHT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["br"],
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          BACK_LEFT_JOINTS["elbow"],
          p.POSITION_CONTROL,
          targetPosition=ELBOWS_INIT_POSITIONS["bl"],
          maxVelocity=MAX_SERVOS_SPEED)

        # Bottom
        p.setJointMotorControl2(brutus_id,
          FRONT_RIGHT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          FRONT_LEFT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          BACK_RIGHT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          BACK_LEFT_JOINTS["bottom"],
          p.POSITION_CONTROL,
          targetPosition=TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
        
        # Foot
        p.setJointMotorControl2(brutus_id,
          FRONT_RIGHT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          FRONT_LEFT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          BACK_RIGHT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
        
        p.setJointMotorControl2(brutus_id,
          BACK_LEFT_JOINTS["foot"],
          p.POSITION_CONTROL,
          targetPosition=-TEST_ANGLE,
          maxVelocity=MAX_SERVOS_SPEED)
      
      p.stepSimulation()
      time.sleep(SIM_TIME_STEP)
  except KeyboardInterrupt:
    pass

  p.disconnect()

if __name__ == '__main__':
  main()