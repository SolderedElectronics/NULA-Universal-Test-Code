// Code for Soldered NULA Mini
#pragma once

// Include required libs
#include <Arduino.h>
#include <EEPROM.h>
#include <WiFi.h>
#include "Wire.h"

// Configure EEPROM parameters
#define EEPROM_SIZE 16
int boardHasBeenConfigured = 111; // This symbolic value
int eepromAddress = 0;            // will be written at this address
// If read back, it will mean that the device has previously been tested
// If required to test again, change the first value to some other number

const char* ssid = "Soldered-testingPurposes";
const char* pass = "Testing443";

int ledPin = 23;

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
        digitalWrite(ledPin, LOW);
        delay(400);
        digitalWrite(ledPin, HIGH);
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
    
    // We are now entering first-time test code
    // So, Serial can init from this point on as the user won't see this
    Serial.begin(115200); // Init Serial for debugging
    while (!Serial)
        ; // Wait until Serial is available

    // Print debug messages
    Serial.println("NULA ESP32C6 Mini test begin!");
    delay(20);

    // Test I2C
    if (!scanI2CDevice(easyCaddr))
    {
        // I2C failed
        Serial.println("EasyC test failed!");

        char addrBuffer[60];
        // Format the address as a hexadecimal string
        sprintf(addrBuffer, "The test address is: 0x%02X", easyCaddr);
        Serial.println(addrBuffer);

        sprintf(addrBuffer, "Is an I2C device using address 0x%02X connected over easyC?", easyCaddr);
        Serial.println(addrBuffer);

        blinkRedAndHalt();
    }

    Serial.println("EasyC test OK!");

    // All tests OK!
    // Wait for button press and blink LED
    int buttonPin = 9;
    pinMode(buttonPin, INPUT);

    Serial.println("Press USER button!");
    // Set up timeout for button press
    unsigned long startTime = millis();
    while (digitalRead(buttonPin) == 1)
    {
        digitalWrite(ledPin, HIGH);

        // Check for timeout
        if (millis() - startTime > buttonPressTimeoutMs)
        {
            blinkRedAndHalt(); // Call function if timeout occurs
        }
    }

    Serial.println("Button pressed!");

    Serial.print("Connecting to WiFi...");

    // Connect to the WiFi network.
    WiFi.mode(WIFI_MODE_STA);
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
    digitalWrite(ledPin, HIGH);

    delay(30); // Wait to see the dimming effect
}

