
// Code for NULA Ethernet (W55RP20 - RP2040 + W5500 in one package)
#pragma once

// Include required libs
#include <Arduino.h>
#include <EEPROM.h>
#include <W55RP20lwIP.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>
#include "Wire.h"

// Configure EEPROM parameters
#define EEPROM_SIZE 16
int boardHasBeenConfigured = 111; // This symbolic value
int eepromAddress = 0;            // will be written at this address
// If read back, it will mean that the device has previously been tested
// If required to test again, change the first value to some other number

// The Ethernet object. The SPI bus to the W5500 die is internal to the W55RP20
// package and always uses GPIO 20 (CS), 21 (SCK), 22 (MISO) and 23 (MOSI)
Wiznet55rp20lwIP eth(20);

// The W5500 die's RSTn line, it's wired internally inside the W55RP20 package
// The lwIP driver never touches this line, it only ever does the software reset in
// Wiznet55rp20::begin(), so this is belt and braces. Set it to -1 to skip the hardware
// reset if the schematic says GPIO 25 is not the RSTn line on this board
const int ethResetPin = 25;

// How long to wait for the PHY to report a link before giving up
// Autonegotiation with a 100Mbit switch normally takes about 2 seconds
const unsigned long ethLinkTimeoutMs = 8000;

// Configure WSLED parameters
Adafruit_NeoPixel pixels(1, 1); // WSLED object, the NeoPixel is on IO1
int brightness = 5;             // how bright the LED is
int fadeAmount = 1;             // How much to increment/decrement the brightness when fading
int maxBrightness = 40;         // Maximum brightness level
// RGB components of the color #5b2379 (Soldered purple)
int rBase = 91;  // Red component (5b in hex)
int gBase = 35;  // Green component (23 in hex)
int bBase = 121; // Blue component (79 in hex)

// The USER button on NULA Ethernet
int buttonPin = 8;

// MicroSD card adapter pins - these are on SPI1
const int sdEnablePin = 9; // PMOS gate for the SD card 3V3 rail, LOW enables power
const int sdCsPin = 13;
const int sdMisoPin = 12;
const int sdMosiPin = 11;
const int sdSckPin = 10;

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
 * @brief Pulse the W5500's RSTn line to hard reset the Ethernet controller
 *
 * @note The W5500 datasheet asks for RSTn to be held low for at least 500us and
 * for the PHY to be given time to come back up afterwards
 */
void hardResetEthernet()
{
    if (ethResetPin < 0)
        return; // No reset line available, nothing to do

    pinMode(ethResetPin, OUTPUT);
    digitalWrite(ethResetPin, LOW);
    delay(10); // Hold RSTn low, well over the required 500us
    digitalWrite(ethResetPin, HIGH);
    delay(200); // Let the chip lock its PLL and the PHY finish its internal init
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
    Serial.flush(); // Push everything out over USB before we stop doing anything else
    delay(50);      // Give the USB host time to actually read it

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
 * @brief The setup and test specific for NULA Ethernet (W55RP20)
 *
 * @param easyCaddr the easyC address to test
 * @param buttonPressTimeoutMs How many ms to wait for button press before blocking the code
 * @param wifiTimeoutMs Unused on this board, the Ethernet link timeout is ethLinkTimeoutMs
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
    Serial.println("NULA Ethernet test begin!");
    delay(20);

    // Wait for button press and blink LED
    pinMode(buttonPin, INPUT_PULLUP);

    Serial.println("Press USER button!");
    // Set up timeout for button press
    unsigned long startTime = millis();
    unsigned long lastBlink = millis();
    bool ledState = false;
    // Poll the button continuously - the blinking is non-blocking so no press can be missed
    while (digitalRead(buttonPin) == 1)
    {
        // Blink the LED blue every 100ms without blocking the button polling
        if (millis() - lastBlink >= 100)
        {
            lastBlink = millis();
            ledState = !ledState;
            if (ledState)
                pixels.setPixelColor(0, pixels.Color(0x01, 0x01, 0x23));
            else
                pixels.clear();
            pixels.show();
        }

        // Check for timeout
        if (millis() - startTime > buttonPressTimeoutMs)
        {
            blinkRedAndHalt(); // Call function if timeout occurs
        }

        delay(1); // Short delay to keep the polling loop from hogging the CPU
    }

    // Debounce the press and wait for release so the button state is settled
    delay(30);
    while (digitalRead(buttonPin) == 0)
        ;

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

    // Ethernet test
    Serial.println("Ethernet test start!");

    // This test only checks the hardware: that the W5500 die answers over the internal
    // PIO SPI bus and that its PHY negotiates a link. No IP config is checked, so no DHCP
    // server is needed on the test bench

    // Hard reset the W5500 die once, then bring it up. Call eth.begin() EXACTLY once per
    // boot. It runs lwip_init() unconditionally, and a second lwip_init() on a live stack
    // re-runs memp_init() while lwIP's static next_timeout list still points into the pool
    hardResetEthernet();

    if (!eth.begin())
    {
        Serial.println("No Ethernet hardware detected!");
        Serial.flush();
        blinkRedAndHalt();
    }

    // Wait for the PHY to report a link. This covers the internal SPI bus to the W5500
    // die, the RJ45 jack, the magnetics, the link pairs and the PHY itself
    Serial.print("Waiting for PHY link...");
    startTime = millis();
    while (!eth.isLinked() && millis() - startTime < ethLinkTimeoutMs)
    {
        delay(100);
        Serial.print(".");
    }

    if (!eth.isLinked())
    {
        Serial.println("\nNo PHY link!");
        Serial.println("Is the Ethernet cable connected on both ends?");
        Serial.flush();
        blinkRedAndHalt();
    }

    Serial.println("\nPHY link up!");
    Serial.println("Ethernet test OK!");

    // Enable the SD card 3V3 rail, the PMOS gate is active low
    pinMode(sdEnablePin, OUTPUT);
    digitalWrite(sdEnablePin, LOW);

    // Small delay before initializing the SD card so the rail can settle
    delay(200);

    // SD card test, the adapter is wired to SPI1
    Serial.println("SD card test start!");
    Serial.flush(); // SD.begin() can block for a while, make sure this is visible first

    SPI1.setRX(sdMisoPin);
    SPI1.setTX(sdMosiPin);
    SPI1.setSCK(sdSckPin);

    if (!SD.begin(sdCsPin, SPI1))
    {
        Serial.println("SD card FAIL!");
        Serial.println("Is a MicroSD card inserted and formatted as FAT32?");
        Serial.flush();
        blinkRedAndHalt();
    }

    Serial.println("SD card test OK!");

    // All tests OK!
    Serial.println("Test complete!");

    // Now save into EEPROM that the board has been previously configured
    EEPROM.put(eepromAddress, boardHasBeenConfigured);
    EEPROM.commit();
}

/**
 * @brief Loop function which fades the LED for NULA Ethernet
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
