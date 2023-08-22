#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <SSD1306.h>
#include "images.h"
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <TimeLib.h>
#include <string>
#include <sstream> // Add this header for the stringstream


#define LED_PIN 2
/* more definition */

#define SCK     5    // GPIO5  -- SX1278's SCK
#define MISO    19   // GPIO19 -- SX1278's MISO
#define MOSI    27   // GPIO27 -- SX1278's MOSI
#define SS      18   // GPIO18 -- SX1278's CS
#define RST     23   // GPIO14 -- SX1278's RESET
#define DI0     26   // GPIO26 -- SX1278's IRQ(Interrupt Request)
#define BAND    436.703E6




/*   ##############FIREBASE AND WIFI SETUP ########################*/
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID ""
#define WIFI_PASSWORD ""

// Insert Firebase project API Key
#define API_KEY ""
// Insert RTDB URL
#define DATABASE_URL ""

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

 /* defining variable */

 unsigned long sendDataPrevMillis = 0;
 SSD1306  display(0x3c, 21, 22);
////String payload = "";
String rssi = "RSSI --";
String snr = "";
String FreqError = "";
//String packet;
int packetSize = 0;
bool signupOK = false;
bool packetReceived = false;
bool wifiConnected = false;
String packSize = "  --";
char buffer[50];
char packet[256];
char payload[256];

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  wifiConnected = true;
  delay(100);
}
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
// Time zone offset in seconds (Helsinki, Finland: UTC+2 during standard time, UTC+3 during daylight saving time)
const int TIMEZONE_OFFSET = 3 * 3600;

String getTimeStamp() {
  timeClient.update();
  time_t currentTime = timeClient.getEpochTime() + TIMEZONE_OFFSET;
  char timestamp[20];
  sprintf(timestamp, "%04d-%02d-%02d %02d:%02d:%02d", year(currentTime), month(currentTime), day(currentTime), hour(currentTime), minute(currentTime), second(currentTime));
  return String(timestamp);
}
void updateDisplay() {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "Message: " +String(packet));
  display.drawString(0, 10, "Length: " + String(packSize) + " bytes");
  display.drawString(0, 20, "RSSI: " + rssi);
  display.drawString(0, 30, "SNR: " + snr);
  display.drawString(0, 40, "Frequency Error: " + FreqError);
  display.display();
}


// ... (your other includes and setup function)

void onLoRaPacketReceived(int packetSize) {
  packSize = String(packetSize,DEC);
  uint8_t ix0 = 0, ix1 = 0;
  memset(packet, 0, 256);
  memset(payload, 0, 256);

  for (int i = 0; i < packetSize; i++) {
    byte data = LoRa.read();
    packet[ix0++] = (char)data;

    // Format payload as hex with leading zeros
    sprintf(payload + ix1, "%02X ", data);
    ix1 += 3; // Move index by 3 (2 digits and a space)
  }

  rssi = String(LoRa.packetRssi(), DEC);
  snr = String(LoRa.packetSnr(), 5);
  FreqError = String(LoRa.packetFrequencyError(), DEC);
  updateDisplay();

  // Set a flag to indicate that a packet has been received and processed
  packetReceived = true;

  // Update the timestamp for the last data upload
  //sendDataPrevMillis = millis();
}

void setup() {
  
   pinMode(LED_PIN, OUTPUT);

  digitalWrite(LED_PIN, LOW);  // turn off the LED initially

  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(16, HIGH);   // while OLED is running, must set GPIO16 in high

  Serial.begin(115200);
  while (!Serial);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(436.703E6)) {


    Serial.println("Starting LoRa failed!");
  while (1);
  }
  //NORBY PARAMETERS//
  LoRa.setSpreadingFactor(10);
  LoRa.setSignalBandwidth(250E3);
  LoRa.setCodingRate4(5);
  LoRa.setSyncWord(0x12);
  LoRa.setPreambleLength(8);
  LoRa.enableCrc();
 
 

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  Serial.println("Init completed.");

  connectToWiFi(); // Connect to Wi-Fi at startup

   /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  timeClient.begin();
  timeClient.update();
 // LoRa.onReceive(onLoRaPacketReceived);
  LoRa.receive();
  delay(100);
  }

void loop() {
  /*if (!wifiConnected) {
    connectToWiFi(); // Reconnect to Wi-Fi if not connected
  }*/

  // Check for incoming LoRa packets
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
     packSize = String(packetSize, DEC);
    // Call the callback function to handle the received packet
    onLoRaPacketReceived(packetSize);

    // Get the elapsed time since the Arduino board started
    String timestamp = getTimeStamp();

    // Check for Firebase and upload available packets (if needed)
    if (Firebase.ready() && signupOK && packetReceived) {
      

      // Upload the payload and values to different paths with timestamps
      if (Firebase.RTDB.set(&fbdo, "test/payload/" + timestamp, payload)) {
        Serial.println("Payload stored successfully with timestamp: " + timestamp);
      } else {
        Serial.println("Failed to store Payload");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.set(&fbdo, "test/packSize/" + timestamp, packSize + "bytes")) {
        Serial.println("Payload stored successfully with packSize: " + timestamp);
      } else {
        Serial.println("Failed to store packSize");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.set(&fbdo, "test/rssi/" + timestamp, rssi)) {
        Serial.println("RSSI stored successfully with timestamp: " + timestamp);
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      } else {
        Serial.println("Failed to store RSSI");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.set(&fbdo, "test/snr/" + timestamp, snr)) {
        Serial.println("SNR stored successfully with timestamp: " + timestamp);
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      } else {
        Serial.println("Failed to store SNR");
        Serial.println("REASON: " + fbdo.errorReason());
      }

      if (Firebase.RTDB.set(&fbdo, "test/freqError/" + timestamp, FreqError)) {
        Serial.println("Frequency Error stored successfully with timestamp: " + timestamp);
        Serial.println("PATH: " + fbdo.dataPath());
        Serial.println("TYPE: " + fbdo.dataType());
      } else {
        Serial.println("Failed to store Frequency Error");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      
      packetReceived = false;
    }
  }

  
  timeClient.update();
  
}
