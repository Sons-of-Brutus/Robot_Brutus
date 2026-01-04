***TODO**: poner bien QUÉ tornillos, tuercas y arandelas son en cada paso.*

# Guide

This assembly instructions are divided in N parts:
1. [Soldering `ESP32` Connections](#soldering-esp32-connections)
2. [Assembling chasis (Part 1)](#assembling-chasis-part-1)
3. [Assembling legs](#assembling-legs)
4. [Connecting servos to `PCA9685`](#connecting-servos-to-pca9685)
5. [Assembling chasis (Part 2)](#assembling-chasis-part-2)
6. [Joining legs with chasis](#joining-legs-with-chasis)
7. [Assembling upper box](#assembling-upper-box)
8. [Joining upper box with chasis](#joining-upper-box-with-chasis)

# Assembly

## Soldering `ESP32` Connections
Following the [electronics schematics image](../electronics/brutus_schematic/brutus_electronic_schematics.svg) (<-- Click here to see it bigger):
<img src="../electronics/brutus_schematic/brutus_electronic_schematics.svg" width="50%">

Solder the following connections:
- `ESP32`

x3 28 AWG (cortados): ESP32 <-> PCA
x3 28 AWG (cortados): ESP32 <-> LED

x(2x3) [echo y trigger] jumper (cortado-hembra): ESP32 <-> HC-SR04
x(2x3) [vcc y gnd] jumper (cortado-hembra): terminal <-> HC-SR04

x2 [vin y gnd] 26 AWG: terminal <-> ESP32

x2 [V+ y GND] 20 AWG : terminal <-> PCA

x2 [vin y gnd] 16 AWG: battery <-> Buck
x2 [vout y gnd] 16 AWG: buck <-> terminal

## Assembling chasis (Part 1)

## Assembling legs

## Connecting servos to `PCA9685`

## Assembling chasis (Part 2)

## Joining legs with chasis

## Assembling upper box

## Joining upper box with chasis