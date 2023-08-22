# LEO-Satellite-Receiver-LoRaEsp32

LoRa Satellite Packet Receiver and Data Uploader - README

This repository contains an Arduino code for a LoRa satellite packet receiver. The receiver listens for LoRa packets transmitted by satellites and displays the received data, along with RSSI, SNR, and frequency error information, on an OLED screen. The received data is then uploaded to the Firebase Realtime Database (RTDB) for storage and analysis.
Table of Contents

    Introduction
    Components
    Wiring
    Setup
    Firebase Setup
    Data Uploading
    Time Synchronization
    License

Introduction

This Arduino project demonstrates how to receive LoRa packets using SX1278 modules, display packet information on an OLED screen, and upload the received data to Firebase RTDB. The code is designed to work with LoRa satellite packets transmitted on a specific frequency.
Components

To use this code, you'll need the following components:

    Arduino board (e.g., Arduino Uno, Arduino Nano)
    SX1278 LoRa module
    SSD1306 OLED display
    LED (for indication)
    WiFi connection (for Firebase upload)

Wiring

Ensure the wiring is set up as follows:

    SCK: Connect to GPIO5 on the LoRa module
    MISO: Connect to GPIO19 on the LoRa module
    MOSI: Connect to GPIO27 on the LoRa module
    SS (CS): Connect to GPIO18 on the LoRa module
    RST: Connect to GPIO23 on the LoRa module
    DI0 (IRQ): Connect to GPIO26 on the LoRa module
    SDA: Connect to SDA on the OLED display
    SCL: Connect to SCL on the OLED display
    LED Pin: Connect to GPIO2 for indication
    WiFi Connection: Ensure your device is connected to WiFi for Firebase upload

Setup

The setup() function initializes various components and services:

    Initializes OLED display, LoRa module, and LED pin.
    Connects to WiFi network.
    Initializes Firebase authentication and configuration.
    Initializes the NTPClient for time synchronization.
    Sets LoRa parameters such as spreading factor, signal bandwidth, coding rate, sync word, and preamble length.
    Sets up CRC (Cyclic Redundancy Check) for packet verification.
    Displays "Init completed." on the OLED display.

Firebase Setup

This code uses the Firebase ESP8266 Client library to communicate with Firebase RTDB. The library requires authentication information:

    Insert your network credentials in WIFI_SSID and WIFI_PASSWORD.
    Insert your Firebase project API key in API_KEY.
    Insert your RTDB URL in DATABASE_URL.

Data Uploading

When a LoRa packet is received, the onLoRaPacketReceived() function is called. It extracts the packet's content, calculates RSSI, SNR, and frequency error, and updates the OLED display with this information. If a WiFi connection is established and Firebase is ready, the data is uploaded to the RTDB:

    The payload, packet size, RSSI, SNR, and frequency error are uploaded to different paths with timestamps.
    The getTimeStamp() function returns the current timestamp adjusted for the local time zone.

Time Synchronization

The NTPClient is used to synchronize the device's time with an NTP server. The time synchronization is used to timestamp the data when uploading to the Firebase RTDB.
Usage Notes

    Configure the LoRa parameters (BAND, spreading factor, bandwidth, etc.) as per your satellite's specifications.
    Adjust other settings (display content, paths, etc.) based on your requirements.

License

This code is provided under the MIT License.
