# Arduino-Multi-Sensor-Comfort-Index-Syste
This project reads gas, temperature, humidity, crowd, noise, and vibration sensors via Arduino, computes comfort index, and displays results on OLED.  
Uses DHT11, MQ135, ultrasonics, sound, MPU6050 sensors.  
Apply exponential smoothing, normalize readings, and visualize real-time indexes for air comfort monitoring.

## Hardware Required
- Arduino Uno/Nano
- DHT11 temperature/humidity sensor
- MQ135 gas sensor
- Ultrasonic sensor (HC-SR04)
- Sound sensor
- MPU6050 IMU sensor
- 128x64 OLED display (SSD1306)
- Connecting wires

## Features
- Reads all sensors every second
- Exponential smoothing for precise values
- Custom normalization per environment
- Weighted comfort index calculation
- Real-time graphical display on OLED

## Getting Started
Wire all components as per pin mapping in the code.  
Install required libraries: `DHT`, `Adafruit_SSD1306`, `NewPing`, `MPU6050` via Arduino IDE Library Manager.  
Upload the provided code to Arduino.

