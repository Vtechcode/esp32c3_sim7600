#include <Arduino.h>
#include <HardwareSerial.h>
#include <TinyGsmClient.h>
#include <Firebase_ESP_Client.h>

// Firebase project configuration
#define FIREBASE_HOST "your-project-id.firebaseio.com" //Replace with your Firebase project ID
#define FIREBASE_AUTH "your-database-secret" //Replace with your Firebase database secret

// SIM card and APN configuration
#define APN "your-apn" //Replace with your SIM card APN
#define GPRS_USER "" // GPRS User, set to "" if not required
#define GPRS_PASS "" // GPRS Password, set to "" if not required

// ESP32-C3 hardware serial for SIM7600
HardwareSerial simSerial(1); // Use UART1

// TinyGSM client for SIM7600
TinyGsm modem(simSerial);
TinyGsmClient client(modem);

// Firebase data object
FirebaseData fbdo;

void setup() {
  Serial.begin(115200);
  simSerial.begin(115200, SERIAL_8N1, 20, 21); // RX, TX for ESP32-C3

  Serial.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  Serial.println("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" failed to connect to network");
    while (true);
  }

  Serial.println("Connecting to GPRS...");
  if (!modem.gprsConnect(APN, GPRS_USER, GPRS_PASS)) {
    Serial.println(" failed to connect to GPRS");
    while (true);
  }

  Serial.println("GPRS connected");

  // Enable GPS
  modem.sendAT("+CGPS=1");
  delay(1000);

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH, &client);
  Firebase.reconnectWiFi(true);
}

void loop() {
  float lat, lon;
  if (modem.getGPS(&lat, &lon)) {
    Serial.print("Latitude: ");
    Serial.print(lat, 6);
    Serial.print("\tLongitude: ");
    Serial.println(lon, 6);

    if (Firebase.setFloat(fbdo, "/gps/lat", lat) && Firebase.setFloat(fbdo, "/gps/lon", lon)) {
      Serial.println("GPS data sent to Firebase");
    } else {
      Serial.println("Failed to send GPS data to Firebase");
      Serial.println(fbdo.errorReason());
    }
  } else {
    Serial.println("Waiting for GPS fix...");
  }

  delay(30000); // Update every 30 seconds
}


