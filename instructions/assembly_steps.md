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

## Soldering `ESP32` and `PCA9685` Connections
Following the [electronics schematics image](../electronics/brutus_schematic/brutus_electronic_schematics.svg) (<-- Click here to see it bigger):
<img src="../electronics/brutus_schematic/brutus_electronic_schematics.svg" width="50%">

**Important**: Below the table there are important considerations you must take in account while soldering.

Solder the following connections:
| PIN 1 | PIN 2 | Wire | Details |
| :--- | :---: | :---: | ---: |
| `ESP32` Vin or 5V | *Nowhere* | 26 AWG | Leave a relatively large quantity of wire as it will be connected to the `2-Pole Terminal Block` at the upper box. Continued at "[Joining upper box with chasis](#joining-upper-box-with-chasis)" steps. |
| `ESP32` GND | *Nowhere* | 26 AWG | Leave a relatively large quantity of wire as it will be connected to the `2-Pole Terminal Block` at the upper box. Continued at "[Joining upper box with chasis](#joining-upper-box-with-chasis)" steps. |
| `PCA9685` V+ | *Nowhere* | 20 AWG | Leave a relatively large quantity of wire as it will be connected to the `2-Pole Terminal Block` at the upper box. Continued at "[Joining upper box with chasis](#joining-upper-box-with-chasis)" steps. |
| `PCA9685` GND | *Nowhere* | 20 AWG | Leave a relatively large quantity of wire as it will be connected to the `2-Pole Terminal Block` at the upper box. Continued at "[Joining upper box with chasis](#joining-upper-box-with-chasis)" steps. |
| `PCA9685` VCC | `ESP32` 3.3V | 28 AWG | - |
| `PCA9685` SCL | `ESP32` GPIO 22 | 28 AWG | It is **ESSENTIAL** that this connection doesn't change, `PCA9685`'s SCL needs to be connected to `ESP32`'s GPIO 22 |
| `PCA9685` SDA | `ESP32` GPIO 21 | 28 AWG | It is **ESSENTIAL** that this connection doesn't change, `PCA9685`'s SCL needs to be connected to `ESP32`'s GPIO 22 |
| `PCA9685` OE | `ESP32` GPIO 19 | 28 AWG | - |
| `RGB LED` R | `ESP32` GPIO 32 | 28 AWG | - |
| `RGB LED` G | `ESP32` GPIO 25 | 28 AWG | - |
| `RGB LED` B | `ESP32` GPIO 33 | 28 AWG | - |
| `RGB LED` GND | `ESP32` GND | 28 AWG | **VERY IMPORTANT!** This `ESP32` GND pin must not be the same as the one in the 2nd row of this table. |
| `ESP32` GPIO 2 | *Nowhere* | Female-to-female jumper | This wire needs a relatively large quantity of wire as it will be connected to one of the `HC-SR04` at the upper box. Connections at "[Joining upper box with chasis](#joining-upper-box-with-chasis)" steps. |
| `ESP32` GPIO 15 | *Nowhere* | Female-to-female jumper | The same as `ESP32` GPIO 2 |
| `ESP32` GPIO 16 | *Nowhere* | Female-to-female jumper | The same as `ESP32` GPIO 2 |
| `ESP32` GPIO 4 | *Nowhere* | Female-to-female jumper | The same as `ESP32` GPIO 2 |
| `ESP32` GPIO 17 | *Nowhere* | Female-to-female jumper | The same as `ESP32` GPIO 2 |
| `ESP32` GPIO 5 | *Nowhere* | Female-to-female jumper | The same as `ESP32` GPIO 2 |

**Considerations:**
- The wires for the 4 connections between `ESP32` and `PCA9685` should be *long enough* to allow the `chasis cover` to be separated slightly from the `chasis` when it is open, but *short enough* to avoid taking up excess space when closed.
- The 4 wires that connect `RGB LED` and `ESP32` should be long enough to place this two components as explained in "[Assembling chasis (Part 2)](#assembling-chasis-part-2)" steps.
- For each of the 6 wires for `HC-SR04`, you must remove one of the tips of a `Female-to-female jumper`. The removed part is the one soldered to the `ESP32` and the *female* part will be connected to the `HC-SR04`.

## Assembling chasis (Part 1)

## Assembling legs

## Connecting servos to `PCA9685`

## Assembling chasis (Part 2)

## Joining legs with chasis

## Assembling upper box

x(2x3) [vcc y gnd] jumper (cortado-hembra): terminal <-> HC-SR04

x2 [vin y gnd] 16 AWG: battery <-> Buck
x2 [vout y gnd] 16 AWG: buck <-> terminal

## Joining upper box with chasis