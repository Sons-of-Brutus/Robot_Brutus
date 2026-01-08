# Original Design vs New Design

This document presents a comparison between the original robot design and the new design developed in this project. The objective is to clearly describe the main differences between both versions and to justify the modifications introduced in the new design.

## Chasisis
In the chassis design, we have made **two practical changes** and **one aesthetic change**.  

Among the practical changes, the most noticeable, as can be seen in the photos, is that the surface protruding from the base where the servo is mounted has been made taller to fit our servo model. Additionally, in the new chassis, the holes are not pre-made; instead, they are created during assembly by inserting the pins.  

The second practical change is the removal of the four taller columns centered on the chassis. This was done to gain more free space inside the chassis.  

Finally, we have made an aesthetic change to the front part of the chassis to improve its visual appearance.

#### Old chasis
<p align="center">
  <img src="./media/og_design/chasis/old_chasis1.png" width="70%">
</p>

<p align="center">
  <img src="./media/og_design/chasis/old_chasis2.png" width="70%">
</p>

#### New chasis
<p align="center">
  <img src="./media/og_design/chasis/new_chasis1.png" width="70%">
</p>

<p align="center">
  <img src="./media/og_design/chasis/new_chasis2.png" width="70%">
</p>



## Chasis cover

In the chassis cover mesh, we have added the cross visible on the top, with the purpose of attaching the upper module, which I will discuss later. Essentially, this module will be secured using the tabs shown in the photos.

#### Old chasis cover
<p align="center">
  <img src="./media/og_design/chasis_cover/old_chasis_cover1.png" width="70%">
</p>

<p align="center">
  <img src="./media/og_design/chasis_cover/old_chasis_cover2.png" width="70%">
</p>

#### New chasis cover
<p align="center">
  <img src="./media/og_design/chasis_cover/new_chasis_cover1.png" width="70%">
</p>

<p align="center">
  <img src="./media/og_design/chasis_cover/new_chasis_cover2.png" width="70%">
</p>

## Shoulder
This modification has been applied to both the **right shoulder** and the **left shoulder**, although the photos specifically show the **right shoulder**.  

For this mesh, we had to adapt the space for the servo; to do this, we created the two visible slots, which allow the servo to be mounted lower. As with the chassis, the final hole will be made during assembly when screwing in the servo.



#### Old shoulder
<p align="center">
  <img src="./media/og_design/shoulder/old_shoulder1.png" width="70%">
</p>

#### New shoulder
<p align="center">
  <img src="./media/og_design/shoulder/new_shoulder1.png" width="70%">
</p>

## Foot fist
This modification serves both a practical and aesthetic purpose. Knobs have been added to the end of the foot, which not only improves its appearance but also increases the contact surface, providing greater stability.


#### Old foot fist
<p align="center">
  <img src="./media/og_design/foot_fist/old_foot_fist1.png" width="70%">
</p>

#### New foot fist
<p align="center">
  <img src="./media/og_design/foot_fist/new_foot_fist1.png" width="70%">
</p>

## Up module

This module is completely new and has been designed to house both the battery and the circuit with the regulator and ultrasonic sensors

<p align="center">
  <img src="./media/og_design/up_box/up_box_1.png" width="70%">
</p>

<p align="center">
  <img src="./media/og_design/up_box/up_box_2.png" width="70%">
</p>

### How do we attach it to the base?
The box fits into the slots of the previously mentioned modified chasis cover

<p align="center">
  <img src="./media/og_design/up_box/up_box_attach.png" width="70%">
</p>

### How do we attach the ultrasonics?
The ultrasonic sensor is mounted using this model and the slots shown in the photo. Three sensors are used: one at the front and one on each side, in order to meet the exercise requirements and allow the robot to follow a wall while avoiding obstacles.

#### Ultrasonic chasis
<p align="center">
  <img src="./media/og_design/up_box/ultrasonic/ultrasonic1.png" width="70%">
</p>
<p align="center">
  <img src="./media/og_design/up_box/ultrasonic/ultrasonic2.png" width="70%">
</p>

#### Attachment
<p align="center">
  <img src="./media/og_design/up_box/ultrasonic/ultrasonic3.png" width="70%">
</p>

### Battery slot

We have created a compartment for the battery, with an internal opening that allows the battery cables to pass through to the top, and side vents for improved ventilation.
<p align="center">
  <img src="./media/og_design/up_box/up_box_battery1.png" width="70%">
</p>
<p align="center">
  <img src="./media/og_design/up_box/up_box_battery2.png" width="70%">
</p>

### Battery lid
To cover the battery and prevent it from falling out, we designed this lid.
<p align="center">
  <img src="./media/og_design/up_box/up_box_battery_lid.png" width="70%">
</p>

### Voltage regulator

To install the voltage regulator, we created a small recess in the outer casing and two holes to route the regulator's input and output wires.

<p align="center">
  <img src="./media/og_design/up_box/up_box_regulator.png" width="70%">
</p>

### Circuit compartment

To house the entire circuit, we created a compartment that will later be covered with a lid, which will bear the weight. Both the openings for the modulator cables and the openings for the battery cables also connect to this compartment. There are also openings to route the necessary cables outside."

<p align="center">
  <img src="./media/og_design/up_box/up_box_circuit1.png" width="70%">
</p>
<p align="center">
  <img src="./media/og_design/up_box/up_box_circuit2.png" width="70%">
</p>

### Weight case
This box, which will cover the wiring compartment, is designed to support the weight that the robot has to bear.

<p align="center">
  <img src="./media/og_design/up_box/up_box_weight_case1.png" width="70%">
</p>

<p align="center">
  <img src="./media/og_design/up_box/up_box_weight_case2.png" width="70%">
</p>