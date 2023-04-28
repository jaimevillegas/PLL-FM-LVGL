# PLL-FM-LVGL

## About the project
**PLL FM LVGL** is a system that monitors FM Transmitter devices. It has the functionallity to monitor temperature, Power, Audio signals, Antenna status and FM Frequency of the device.

## Built with

### Hardware
- ESP32
- TFT 3.5" SPI Display
- TSA5511 IC
- Custom PCB with circuits to interface sensors with the microcontroller

## Software
- PlatformIO Arduino framework for ESP32
- LovyanGFX to interface with the TFT Display
- LittleVGL as a library to manage graphical elements on the display
- SquareLine Studio as a UI Design software

## Screenshots
![image](https://user-images.githubusercontent.com/5252636/235233701-cbb16249-8196-4086-8f1f-abefa7751b10.png)
![image](https://user-images.githubusercontent.com/5252636/235233767-fd1c98bc-075a-48a4-96ed-b8f30183ebb1.png)


## I/O pins

| NAME       | PIN | DESCRIPTION              |
|------------|-----|--------------------------|
| potDir_out | 25  | Salida Potencia Directa  |
| fan_out    | 15  | Salida Ventilador        |
| buzzer_out | 4   | Salida Buzzer Alarma     |
| modL_in    | 14  | Entrada mod L            |
| modR_in    | 26  | Entrada mod R            |
| modMPX_in  | 13  | Entrada mod MPX          |
| temp_in    | 36  | Entrada temperatura      |
| potDir_in  | 39  | Entrada Potencia Directa |
| potRef_in  | 34  | Entrada Potencia Refleja |
