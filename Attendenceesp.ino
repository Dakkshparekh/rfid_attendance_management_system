// Include necessary libraries
#include <WiFi.h> // For Wi-Fi connection
#include <HTTPClient.h> // For sending data over HTTP
#include <ArduinoJson.h> // For creating JSON messages
#include <MFRC522.h> // For the RFID reader
#include <LiquidCrystal_I2C.h> // For the I2C LCD display
#include <Wire.h> // For I2C communication (needed for custom I2C pins)

// --- Wi-Fi Configuration ---
const char* ssid = "net na hai"; // Your Wi-Fi network name
const char* password = "rechargekarle"; // Your Wi-Fi password

// --- Backend Server Configuration ---
// IMPORTANT: Updated to your computer's Wi-Fi IPv4 Address from ipconfig output.
const char* serverUrl = "http://192.168.227.250:3000/attendance"; // <--- THIS IS THE CRITICAL CHANGE!

// --- RFID Reader Pins ---
#define SS_PIN 21 // SDA (SS) pin for RC522 (connected to GPIO 21 in circuit)
#define RST_PIN 22 // RST pin for RC522 (connected to GPIO 22 in circuit)

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance

// --- LCD Display Configuration ---
// Find your LCD's I2C address. Common ones are 0x27 or 0x3F.
// You can run an I2C scanner sketch (search "Arduino I2C scanner") to find it.
// LCD SDA is connected to GPIO 14, SCL to GPIO 27 as per circuit diagram.
// We need to initialize Wire with these custom pins.
#define LCD_SDA_PIN 14
#define LCD_SCL_PIN 27

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD I2C address, 16 columns and 2 rows

// --- Function to connect to Wi-Fi ---
void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  lcd.clear();
  lcd.print("Connecting WiFi...");
  WiFi.begin(ssid, password); // Start connecting to Wi-Fi

  // Keep trying until connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); // Wait a bit
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Show the ESP32's IP address
  lcd.clear();
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000); // Show for 2 seconds
}

// --- Setup function (runs once when ESP32 starts) ---
void setup() {
  Serial.begin(115200); // Start serial communication for debugging

  // Initialize I2C (Wire) with custom pins for LCD before initializing LCD
  // This must match your physical wiring for SDA (data) and SCL (clock)
  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN); // Initialize I2C with specified SDA and SCL pins

  // Initialize LCD
  // If you continue to get I2C errors, double-check the LCD's I2C address.
  // You might need to change 0x27 to 0x3F or another address found by an I2C scanner.
  lcd.init(); // Initialize the LCD
  lcd.backlight(); // Turn on the backlight
  lcd.print("Hello Teacher!");
  delay(1000);

  // Initialize RFID reader
  // SPI communication uses default pins for ESP32 unless explicitly changed:
  // SCK (Clock): GPIO 18
  // MISO (Master In Slave Out): GPIO 19
  // MOSI (Master Out Slave In): GPIO 23
  // SS_PIN (Slave Select): GPIO 21 (defined above)
  // RST_PIN (Reset): GPIO 22 (defined above)
  SPI.begin(); // Start SPI communication
  mfrc522.PCD_Init(); // Initialize MFRC522
  Serial.println("RFID Reader Ready!");
  lcd.clear();
  lcd.print("Scan your card!");

  connectToWiFi(); // Connect to Wi-Fi
}

// --- Loop function (runs over and over again) ---
void loop() {
  // Check if a new card is present
  if (mfrc522.PICC_IsNewCardPresent()) {
    // Select the card
    if (mfrc522.PICC_ReadCardSerial()) {
      // Get the UID (Unique ID) of the card
      String uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""); // Add leading zero if needed
        uid += String(mfrc522.uid.uidByte[i], HEX); // Convert byte to hexadecimal string
      }
      uid.toUpperCase(); // Convert to uppercase for consistency

      Serial.print("Card UID: ");
      Serial.println(uid);

      // Display on LCD
      lcd.clear();
      lcd.print("Card Scanned!");
      lcd.setCursor(0, 1);
      lcd.print(uid);
      delay(2000);

      // --- Send data to Node.js backend ---
      if (WiFi.status() == WL_CONNECTED) { // Only send if Wi-Fi is connected
        HTTPClient http; // Create an HTTPClient object

        // Prepare JSON payload
        StaticJsonDocument<200> doc; // Create a JSON document (200 bytes is usually enough)
        doc["cardId"] = uid; // Add the card UID
        doc["timestamp"] = millis(); // Add a timestamp (you can use NTP for real time)

        String jsonPayload;
        serializeJson(doc, jsonPayload); // Convert JSON document to a string

        Serial.print("Sending JSON: ");
        Serial.println(jsonPayload);

        http.begin(serverUrl); // Specify the URL
        http.addHeader("Content-Type", "application/json"); // Tell the server we're sending JSON

        // Send the HTTP POST request
        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
          Serial.print("HTTP Response Code: ");
          Serial.println(httpResponseCode);
          String response = http.getString(); // Get the response from the server
          Serial.print("Server Response: ");
          Serial.println(response);

          lcd.clear();
          lcd.print("Sent to Server!");
          if (httpResponseCode == 200) {
            lcd.setCursor(0, 1);
            lcd.print("Success!");
          } else {
            lcd.setCursor(0, 1);
            lcd.print("Error: " + String(httpResponseCode));
          }
        } else {
          // This is where you're seeing the -1 error.
          Serial.print("Error on sending POST: ");
          Serial.println(httpResponseCode);
          lcd.clear();
          lcd.print("Send Failed!");
          lcd.setCursor(0, 1);
          lcd.print("Check server.");
        }
        http.end(); // Free resources
      } else {
        Serial.println("WiFi not connected. Cannot send data.");
        lcd.clear();
        lcd.print("WiFi Disconnected!");
        lcd.setCursor(0, 1);
        lcd.print("Reconnecting...");
        connectToWiFi(); // Try to reconnect
      }

      // Halt PICC (card) to avoid multiple reads of the same card
      mfrc522.PICC_HaltA();
      // Stop encryption on PCD (reader)
      mfrc522.PCD_StopCrypto1();
    }
  }
  delay(50); // Small delay to prevent continuous polling
}
