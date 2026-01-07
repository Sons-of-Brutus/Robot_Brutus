# Cable Selection

The wiring of the robot must be designed to safely deliver power and signals to all components without excessive voltage drop, overheating or signal degradation. Different cable gauges are used depending on the current level and the type of connection (power or signal).

The robot uses four main wiring categories: battery power, regulated power distribution, actuator power, and signal connections.

## Battery to DC-DC Buck Converter

- 2 × 16 AWG (VIN and GND)

- Connection: Battery → Buck converter

These wires must support up to 10 A peak current, which is why a thick 16 AWG cable is used to minimize voltage drop and heating.

## Buck Converter to Power Distribution Terminal

- 2 × 20 AWG (V+ and GND)

- Connection: Buck → Power terminal block

This carries the full 5 V system current to the power distribution point for the servos and electronics.

## Power Terminal to ESP32

- 2 × 26 AWG (VIN and GND)

- Connection: Power terminal → ESP32

The ESP32 consumes relatively low current, so a thinner cable is sufficient.

## Power Terminal to PCA9685

- 2 × 20 AWG (V+ and GND)

- Connection: Power terminal → PCA9685

The PCA9685 feeds all servos, so this line must carry several amperes during motion.

## Signal and control wiring

Thin flexible wires are preferred for ease of routing and reliability.

- ESP32 to PCA9685 (control bus)

- 3 × 28 AWG (cut wires)

- Connection: ESP32 ↔ PCA9685
(I²C bus and control lines)

## ESP32 to RGB LED

- 3 × 28 AWG (cut wires)

- Connection: ESP32 ↔ RGB LED

These wires carry only a few milliamps and are purely signal/control lines.

## ESP32 to Ultrasonic Sensors (Echo & Trigger)

- 2 × (3-wire) female jumpers (cut)

- Connection: ESP32 ↔ HC-SR04
(Echo and Trigger signals for three sensors)

## Power Terminal to Ultrasonic Sensors (VCC & GND)

- 2 × (3-wire) female jumpers (cut)

- Connection: Power terminal ↔ HC-SR04

These provide 5 V and ground to the ultrasonic sensors.