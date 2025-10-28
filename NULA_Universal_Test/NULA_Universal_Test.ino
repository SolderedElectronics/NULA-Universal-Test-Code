/**
 **************************************************
 *
 * @file        NULA_Universal_Test.ino
 * @brief       A final test sketch for all NULA boards. This test tests the button press, easyC, WiFi and
 *on board LED. When the test is complete and successful, the onboard LED will fade in purple and from then on,
 *whenever the device is powered on it will skip straight to the LED fading.
 *
 * @note        To correctly compile all the user needs to do is select the correct board in Arduino board manager. The
 *supported boards are:
 *              - Soldered NULA Mini ESP32C6
 *
 * @note        A follower easyC device with an address 0x30 needs to be connected to the easyC port for the test to
 *succeed! This address can be modified below if needed.
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Robert Sorić for soldered.com
 ***************************************************/

// The I2C follower address being tested over easyC
const uint8_t easyCaddr = 0x30;

// Timeout for pressing the button upon finished test can be set here
const unsigned long buttonPressTimeoutMs = 10000; // 10 seconds by default

// Timeout for connecting to the WiFi
const unsigned long wifiTimeoutMs = 20000; // 20 seconds by default

// Include the appropriate header file based on the board
#ifdef ARDUINO_SOLDERED_NULA_MINI_ESP32C6
#include "MiniESP32C6.hpp"
#elif defined(ARDUINO_ESP32S3_DEV)
#include"DeepSleepESP32-S3.hpp"
#else
#error "Unsupported board!"
#endif

void setup()
{
    // Call the board-specific setup function
    boardSpecificSetup(easyCaddr, buttonPressTimeoutMs, wifiTimeoutMs);
}

void loop()
{
    // Call the board-specific loop function
    boardSpecificLoop();
}
