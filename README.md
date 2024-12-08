# IoT-Enabled DC Motor Control System

This project is an **IoT-Enabled DC Motor Control System** that provides precise DC motor speed control and real-time IoT-based monitoring and remote control. The system is built using **C**, the **Tiva-TM4C microcontroller**, the **ESP8266 Wi-Fi module**, and the **GNU-ARM-Toolchain**.

## Features

- **Accurate Motor Control:**
  - Uses **PWM control** and tachometer feedback for precise speed regulation.
  - Implements a **Proportional-Integral (PI) controller** for stable motor performance.

- **IoT Integration:**
  - Employs the **ESP8266 Wi-Fi module** with an **MQTT-based communication system**.
  - Allows **remote motor control** and **real-time monitoring** via a custom web application.

- **Compact Hardware Design:**
  - Custom PCB integrates the Tiva microcontroller, ESP8266 module, and motor control circuitry.
  - Optimized layout enhances signal integrity and reduces the overall design footprint.

---

## Getting Started

### Hardware Setup
1. Assemble the custom PCB with the microcontroller, ESP8266, and motor control components.
2. Connect the DC motor and tachometer to the designated PCB terminals.

### Software Setup
1. Install the **GNU-ARM-Toolchain** on your system.
2. Clone this repository:
   ```bash
   git clone https://github.com/maanavkoladia/IoTRobotCar.git

