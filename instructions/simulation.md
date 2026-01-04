# Simulation

## Simulation objetctive:

Before manufacturing the robot, the design was validated in a virtual environment in order to evaluate its stability, center of mass, range of motion, and torque demand in the main joints. In addition, the simulation was used to verify that the robot physics were coherent, that the overall size and proportions were reasonable, and that the robot was able to maneuver correctly while supporting an external load.

The simulation was mainly used to detect and correct possible design errors, with the goal of obtaining a robot that is as robust as possible.

The simulation is not intended to be a perfect digital twin, but rather a tool to detect major issues that could compromise the future development of the robot, such as incorrect mass distribution, insufficient servo torque, or poor leg geometry.

## URDF model:

The URDF model describes all the links, joints, and meshes that make up the robot (chassis, legs, servos, battery, top box, etc.). This allows not only to define the geometric and kinematic structure of the robot, but also to assign mass and inertia to each element, which is essential to correctly compute the center of mass and the forces that the actuators must withstand during the simulation.


#### Mass distribution by links

In order to achieve a realistic dynamic behavior, specific links were created for heavy components (battery, servos, etc.), and mass and inertia were assigned to them in the URDF.

The additional load required for the final exercise was directly included as an increased mass in the top box link (up_box_link), reflecting its real position on the robot and its effect on stability and torque demand.

The rest of the robot components were also adjusted so that their mass matches the real components as closely as possible, in order to obtain a simulation that is as realistic as possible given the modeling limitations.

#### Parallel linkage mechanism

The real robot legs are based on a parallel linkage mechanism (parallelogram mechanism). This means that each leg is formed by a set of bars connected by joints that create a parallelogram, so that when the mechanism moves, the two opposite sides remain parallel. This allows the end effector (the robot foot) to keep an almost constant orientation while it moves.

This type of mechanism is very difficult to simulate accurately in physics engines such as PyBullet, since it forms a closed kinematic chain. For this reason, one of the bars of the real mechanism was removed in the URDF model, and the feet orientation was actively controlled in the simulation so that they remain approximately perpendicular to the ground, which is the functional effect produced by the real parallel linkage.

This approximation slightly reduces the effective pushing force of the legs during locomotion. However, the simulation remains sufficiently realistic and functional to extract the required information. Due to the absence of the real closed-chain mechanism, the simulated motion is not as smooth as in the physical robot.

## Simulation in Pybullet

To validate the dynamic behavior of the design before physical construction, the URDF model was loaded into the PyBullet physics engine and locomotion and load tests were executed under realistic gravity conditions.

To avoid unrealistic sliding or slipping effects, the friction parameters of both the ground and the robot feet were explicitly tuned:

- lateralFriction

- rollingFriction

- spinningFriction

- frictionAnchor = true

This ensures that during the support phase the legs transmit forces to the ground in a physically consistent way.

#### Joint control

The leg servomotors were modeled using position control (POSITION_CONTROL), which emulates the typical behavior of a real servo: receiving a target angle and generating the required torque to reach it.

Each joint was configured with a maximum angular speed and a maximum torque corresponding to the real servos used in the robot (600°/s).

#### Torque logging and analysis

To evaluate whether the selected servos are capable of supporting the motion and load of the robot, force and torque sensors were enabled on the main joints (elbow and bottom of each leg). This allowed the applied motor torque to be recorded in real time and stored in a CSV file (logs/torques.csv).

This log makes it possible to:

- Identify torque peaks during the support phase.

- Compare the required torque with the servo limits.

- Monitor the vertical position of the battery with respect to the ground.

This procedure constitutes the main validation that the selected servomotors are compatible with the robot weight and the additional load defined in the URDF model.