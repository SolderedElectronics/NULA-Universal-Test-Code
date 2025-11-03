#pragma once


// Include required libs
#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>
#include "Wire.h"

// Configure EEPROM parameters
#define EEPROM_SIZE 16
int boardHasBeenConfigured = 112; // This symbolic value
int eepromAddress = 0;            // will be written at this address
// If read back, it will mean that the device has previously been tested
// If required to test again, change the first value to some other number

const char* ssid = "Soldered-testingPurposes";
const char* pass = "Testing443";

// Configure WSLED parameters
Adafruit_NeoPixel pixels(1, 26); // WSLED object
int brightness = 5;              // how bright the LED is
int fadeAmount = 1;              // How much to increment/decrement the brightness when fading
int maxBrightness = 40;          // Maximum brightness level
// RGB components of the color #5b2379 (Soldered purple)
int rBase = 91;  // Red component (5b in hex)
int gBase = 35;  // Green component (23 in hex)
int bBase = 121; // Blue component (79 in hex)

/**
 * @brief Function to test if a follower (slave) device is connected over I2C
 *
 * @param easyCaddr the I2C address to test (default is 0x30)
 * @return true if it's successful and the device responded
 * @return false if it failed and the device is not connected
 */
bool scanI2CDevice(uint8_t easyCaddr)
{
    Wire.begin(); // Initialize the I2C bus
    Wire.beginTransmission(easyCaddr);
    uint8_t error = Wire.endTransmission();

    if (error == 0)
        // Device found
        return true;
    else
        // No device found
        return false;
}

/**
 * @brief Function to blink the onboard LED red if there's an error
 *
 * @note This function doesn't return!
 *
 */
void blinkRedAndHalt()
{
    Serial.println("ERROR! Test FAILED!");
    // Blink LED red infinitely - something's wrong
    while (true)
    {
        pixels.setPixelColor(0, pixels.Color(0x20, 0, 0)); // Set the color to red
        pixels.show();
        delay(400);
        pixels.clear();
        pixels.show();
        delay(400);
    }
}

/**
 * @brief The setup and test specific for ESP8266 (Dasduino Connect)
 *
 * @param easyCaddr the easyC address to test
 * @param buttonPressTimeoutMs How many ms to wait for button press before blocking the code
 */
void boardSpecificSetup(uint8_t easyCaddr, unsigned long buttonPressTimeoutMs, unsigned long wifiTimeoutMs)
{
    pixels.begin(); // Init NeoPixel
    
    // Check if value has previously been entered in EEPROM
    EEPROM.begin(EEPROM_SIZE);
    int eepromReadValue;
    EEPROM.get(eepromAddress, eepromReadValue);
    if (eepromReadValue == boardHasBeenConfigured)
    {
        // This device was tested before
        // Go straight to loop
        return;
    }

    // Set BLUE LED to signify test begin
    pixels.setPixelColor(0, pixels.Color(0x01, 0x01, 0x23));
    pixels.show();
    
    // We are now entering first-time test code
    // So, Serial can init from this point on as the user won't see this
    Serial.begin(115200); // Init Serial for debugging
    while (!Serial)
        ; // Wait until Serial is available

    // Print debug messages
    Serial.println("NULA RP2350 test begin!");
    delay(20);

    // Wait for button press and blink LED
    int buttonPin = 27;
    pinMode(buttonPin, INPUT_PULLUP);

    Serial.println("Press USER button!");
    // Set up timeout for button press
    unsigned long startTime = millis();
    while (digitalRead(buttonPin) == 1)
    {
        // Blink the LED blue
        pixels.setPixelColor(0, pixels.Color(0x01, 0x01, 0x23));
        pixels.show();
        delay(100);
        pixels.clear();
        pixels.show();
        delay(100);

        // Check for timeout
        if (millis() - startTime > buttonPressTimeoutMs)
        {
            blinkRedAndHalt(); // Call function if timeout occurs
        }
    }
    Serial.println("Button pressed!");

    // Set BLUE LED
    pixels.setPixelColor(0, pixels.Color(0x01, 0x01, 0x23));
    pixels.show();

    Serial.println("Qwiic test start!");
    // Test I2C
    if (!scanI2CDevice(easyCaddr))
    {
        // I2C failed
        Serial.println("Qwiic test failed!");

        char addrBuffer[60];
        // Format the address as a hexadecimal string
        sprintf(addrBuffer, "The test address is: 0x%02X", easyCaddr);
        Serial.println(addrBuffer);

        sprintf(addrBuffer, "Is an I2C device using address 0x%02X connected over easyC?", easyCaddr);
        Serial.println(addrBuffer);

        blinkRedAndHalt();
    }

    Serial.println("Qwiic test OK!");

    Serial.print("Connecting to WiFi...");

    WiFi.begin(ssid, pass);

    // Set up timeout for wifi connection
    startTime = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        if (millis() - startTime > wifiTimeoutMs)
        {
            blinkRedAndHalt(); // Call function if timeout occurs
        }
    }
    Serial.println("\nWiFi Test Passed!");

    WiFi.disconnect();

    // Set SDEnable pin to low to enable SD Card
    pinMode(4, OUTPUT);
    digitalWrite(4,LOW);

    // SD card test
    bool sdInitialized = false;


    SPI.setRX(0);
    SPI.setTX(3);
    SPI.setSCK(2);
    sdInitialized = SD.begin(1);

    if (!sdInitialized) {
        Serial.println("SD card FAIL!");
        blinkRedAndHalt();
    } else {
        Serial.println("SD card test ok!");

    }

    // All tests OK!
    

    Serial.println("Test complete!");

    // Now save into EEPROM that the board has been previously configured
    EEPROM.put(eepromAddress, boardHasBeenConfigured);
    EEPROM.commit();
}

/**
 * @brief Loop function which fades the LED for ESP8266
 */
void boardSpecificLoop()
{
    // Calculate scaled RGB values based on brightness
    int rScaled = (rBase * brightness) / maxBrightness;
    int gScaled = (gBase * brightness) / maxBrightness;
    int bScaled = (bBase * brightness) / maxBrightness;

    // Set the color of the WS2812 LED
    pixels.setPixelColor(0, pixels.Color(rScaled, gScaled, bScaled));
    pixels.show();

    // Adjust brightness for fading effect
    brightness += fadeAmount;

    // Reverse direction at the ends of the fade
    if (brightness <= 0 || brightness >= maxBrightness)
    {
        fadeAmount = -fadeAmount;
    }

    delay(30); // Wait to see the dimming effect
}
