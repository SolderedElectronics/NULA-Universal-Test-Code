# Universal NULA Test Code 🟣

Welcome to the **Universal NULA Test Code** repository! This sketch is designed to test various functionalities of NULA devices after production, ensuring everything works as expected before deployment.

---

## 🔧 Features

This sketch performs the following tests:

| **Feature**      | **Description**                                                                                                                                   |
| ---------------- | ------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Onboard LED**  | Tests the onboard LED with a fade in **Soldered purple** (`#5b2379`).                                                                             |
| **Onboard D13 LED**  | Tests the onboard simple LED with blinking (specific to NULA Core and CorePlus).                                                                             |
| **EasyC Port**   | Configures the device as an I2C master and communicates with a follower device at address `0x30` to ensure no transmission errors.                |
| **MicroSD Card** | Writes a file (e.g., `test.txt`) to the SD card and reads it back to confirm the slot is working (specific to NULA CorePlus).                 |
| **Push Button**  | The final step prompts the user to press a button. This writes a value to EEPROM, which disables further testing and goes straight to LED fading. |

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
6. Repeat steps 3-5 for testing devices in Series

> **Note:** If something went wrong with EasyC or SDCard, the LED will blink RED, and you will be able to see some Serial output at a baud rate of 115200.

---

That's about it, feel free to ask any questions so that this README can be updated. 

Happy Testing! ✨