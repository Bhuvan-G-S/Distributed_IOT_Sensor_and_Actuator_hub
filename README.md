STM32 & ESP32 Distributed Sensor Hub

This project is a robust, dual-microcontroller IoT sensor monitoring system. An STM32F401RE acts as a dedicated real-time data acquisition "hub," collecting data from multiple sensors. It then transmits this data via SPI to an ESP32 co-processor, which handles all Wi-Fi connectivity and forwards the data to a cloud endpoint using an HTTP POST request.

This architecture is a common pattern in commercial IoT, separating time-critical sensing tasks from complex, non-real-time connectivity tasks.

## Key Features

    Dual-Processor Architecture: Uses an STM32 as a dedicated SPI Master for reliable, real-time sensor polling, while the ESP32 acts as an SPI Slave and communications co-processor.

    Multi-Sensor Input: Gathers data from three different sensor types simultaneously:

        Analog: Soil Moisture Sensor (via ADC)

        Digital: IR Proximity Sensor (via GPIO)

        Parallel: 8-bit DIP switch array (via GPIO)

    Hardware-to-Hardware Communication: Employs SPI for fast, reliable, and synchronous data transfer between the two microcontrollers.

    Web Connectivity: The ESP32 sends the final sensor payload as a JSON object to a cloud endpoint using an HTTP POST request, a universally compatible method that bypasses port-blocking issues often found with MQTT.

## System Architecture

The data flows in one direction, from the sensors to the cloud, with each device handling a specific task.

-> STM32 -> (SPI) -> ESP32 -> (Wi-Fi/HTTP) -> Cloud]

    STM32 (Master): The while(1) loop continuously:

        Reads the analog value from the soil sensor.

        Reads the digital state of the IR sensor.

        Reads the 8-bit value from the DIP switches.

        Formats all this data into a JSON string (e.g., {"soil": 1850, "ir_detected": 1, "switches": 5}).

        Sends this string over SPI to the ESP32.
        <img width="582" height="216" alt="st32 pic" src="https://github.com/user-attachments/assets/e81c126f-7c5a-478c-bcf8-86684d2f167a" />


    ESP32 (Slave): The loop() function continuously:

        Listens for an incoming SPI transaction from the STM32.

        Upon receiving the data, it connects to the local Wi-Fi.

        It then makes an HTTP POST request, sending the JSON string as the payload to a public webhook URL.

        <img width="602" height="203" alt="webhookpic" src="https://github.com/user-attachments/assets/87bff358-494d-482f-b258-a0a8ae65eb52" />


## Hardware Required

    STM32 Nucleo-F401RE (or similar)

    ESP32 DevKitC V1 (or similar)

    Soil Moisture Sensor (Analog)

    IR Proximity Sensor (Digital)

    2x 4-position DIP Switches (or 1x 8-position)

    Breadboard and Jumper Wires

## Software & Toolchain

    STM32: STM32CubeIDE

    ESP32: Arduino IDE

        Libraries: HTTPClient

    Testing: Webhook.site (or any other HTTP request inspector)

## Repository Structure

This repository contains the firmware for both microcontrollers, separated into two main folders:

/
├── stm32_firmware/   (Contains the complete STM32CubeIDE project)


├── esp32_firmware/   (Contains the Arduino .ino sketch for the ESP32)


└── README.md         (You are here)

## How to Test

    Open the stm32_firmware project in STM32CubeIDE and flash it to your Nucleo board.

    Open the esp32_firmware sketch in the Arduino IDE.

        Fill in your Wi-Fi SSID and password.

        Go to Webhook.site, get your unique URL, and paste it into the webhook_url variable.

        Flash the sketch to your ESP32.

    Power both boards.

    Watch the Webhook.site page. You will see your sensor data arriving in real-time.
