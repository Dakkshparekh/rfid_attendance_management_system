#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

const char* ssid = "net na hai";
const char* password = "recharge karle";

const char* serverUrl = "http://192.168.227.250:3000/attendance";

#define SS_PIN 21
#define RST_PIN 22

MFRC522 mfrc522(SS_PIN, RST_PIN);

#define LCD_SDA_PIN 14
#define LCD_SCL_PIN 27

LiquidCrystal_I2C lcd(0x27, 16, 2);

void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  lcd.clear();
  lcd.print("Connecting WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.print("WiFi Connected!");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(2000);
}

void setup() {
  Serial.begin(115200);

  Wire.begin(LCD_SDA_PIN, LCD_SCL_PIN);

  lcd.init();
  lcd.backlight();
  lcd.print("Hello Teacher!");
  delay(1000);

  SPI.begin();
  mfrc522.PCD_Init();
  Serial.println("RFID Reader Ready!");
  lcd.clear();
  lcd.print("Scan your card!");

  connectToWiFi();
}

void loop() {
  if (mfrc522.PICC_IsNewCardPresent()) {
    if (mfrc522.PICC_ReadCardSerial()) {
      String uid = "";
      for (byte i = 0; i < mfrc522.uid.size; i++) {
        uid += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
        uid += String(mfrc522.uid.uidByte[i], HEX);
      }
      uid.toUpperCase();

      Serial.print("Card UID: ");
      Serial.println(uid);

      lcd.clear();
      lcd.print("Card Scanned!");
      lcd.setCursor(0, 1);
      lcd.print(uid);
      delay(2000);

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        StaticJsonDocument<200> doc;
        doc["cardId"] = uid;
        doc["timestamp"] = millis();

        String jsonPayload;
        serializeJson(doc, jsonPayload);

        Serial.print("Sending JSON: ");
        Serial.println(jsonPayload);

        http.begin(serverUrl);
        http.addHeader("Content-Type", "application/json");

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
          Serial.print("HTTP Response Code: ");
          Serial.println(httpResponseCode);
          String response = http.getString();
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
          Serial.print("Error on sending POST: ");
          Serial.println(httpResponseCode);
          lcd.clear();
          lcd.print("Send Failed!");
          lcd.setCursor(0, 1);
          lcd.print("Check server.");
        }
        http.end();
      } else {
        Serial.println("WiFi not connected. Cannot send data.");
        lcd.clear();
        lcd.print("WiFi Disconnected!");
        lcd.setCursor(0, 1);
        lcd.print("Reconnecting...");
        connectToWiFi();
      }

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
  }
  delay(50);
}
