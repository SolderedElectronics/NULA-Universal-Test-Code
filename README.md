# Universal NULA Test Code 🟣

Welcome to the **Universal NULA Test Code** repository! This sketch is designed to test various functionalities of NULA devices after production, ensuring everything works as expected before deployment.

---

## 🔧 Features

This sketch performs the following tests:

| **Feature**      | **Description**                                                                                                                                   |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Push Button**  | Prompts the user to press a button by blinking the onboard LED very fast (200ms intervals). |
| **EasyC Port**   | Configures the device as an I2C master and communicates with a follower device at address `0x30` to ensure no transmission errors.                |
| **WiFi**  | The final test, tries to connect to the local network, if it succeeds, data is written to the EEPROM to ensure the device isnt tested again |
---

## ⚙️ Configuration

The sketch automatically detects and compiles for the selected board in the Arduino IDE, so there is no need for manual configuration. Supported boards include:

| **Board**                | **Microcontroller**       |
| ------------------------ | ------------------------- |
| **NULA Mini**    | ESP32 C6 Mini                |

---

## 🚀 Usage Instructions

1. Open the sketch in the Arduino IDE.
2. Select the appropriate NULA board from the **Tools > Board** menu.
3. Connect NULA and connect the easyC device and insert SD card (if applicable).
4. Upload the sketch.
5. To complete tests push the button when the LED is blinking - **the code will not reach the blinking LED step unless all other features were tested OK**
6. The test completed successfully if the onboard LED slowly fades on and off
7. The test failed if the onboard LED blinks on/off in 1 second intervals
8. Repeat steps 3-5 for testing devices in Series

> **Note:** The test failed if the onboard LED blinks on/off in 1 second intervals, you will be able to see some Serial output at a baud rate of 115200.

---

That's about it, feel free to ask any questions so that this README can be updated. 

Happy Testing! ✨