# rfid_attendance_management_system
Sure! Here’s a professional README you can use on your LinkedIn or GitHub to showcase your RFID attendance system project, highlighting the hardware and software components as well as the full-stack integration with the Node.js backend and MongoDB database.

# RFID Attendance System with ESP32, Node.js & MongoDB

### Overview  
This project implements a complete RFID attendance recording system using an **ESP32 microcontroller**, **MFRC522 RFID module**, and a **LiquidCrystal I2C display**, with real-time data storage and retrieval using a **Node.js server** and **MongoDB** database.

The system enables teachers to track attendance by scanning RFID cards. When a card is scanned, the ESP32 sends the card UID and timestamp as a JSON payload to a backend API for storage. The status is displayed on an LCD screen and logged via serial output.

### Key Features
- **ESP32 based RFID Reader:** Uses MFRC522 module for RFID detection and UID reading.
- **LiquidCrystal I2C Display:** Real-time feedback on card scanning and network status.
- **Wi-Fi Connectivity:** Connects ESP32 to local Wi-Fi to communicate with backend server.
- **HTTP POST Requests:** Sends attendance data securely as JSON to a Node.js REST API.
- **Node.js Server:** Handles incoming attendance records, validates data, and manages storage.
- **MongoDB Integration:** Stores attendance entries with timestamps, enabling persistence and data analysis.
- **Cross-Origin Resource Sharing (CORS):** Enables API to be accessed from different clients securely.
- **Error Handling & Reconnection:** Manages Wi-Fi disconnections and HTTP failures gracefully.

### Hardware Components  
- **ESP32 Development Board**
- **MFRC522 RFID Module** (SPI interface)  
- **LiquidCrystal I2C 16x2 LCD Display**  
- Connecting wires and a power supply  

### Software & Libraries
- **ESP32 Arduino Framework**  
- Libraries:  
  - `WiFi.h` – Wireless networking  
  - `HTTPClient.h` – HTTP client for REST calls  
  - `ArduinoJson.h` – JSON construction and serialization  
  - `MFRC522.h` – RFID reader functions  
  - `LiquidCrystal_I2C.h` & `Wire.h` – LCD display control  

- **Node.js** with:  
  - `express` – Web server framework  
  - `mongoose` – MongoDB object modeling  
  - `body-parser` – Parsing incoming JSON  
  - `cors` – Handling cross-origin requests  

- **MongoDB** – NoSQL database for data persistence

### How It Works
1. **Setup & Wi-Fi Connection:** ESP32 connects to the configured Wi-Fi network.
2. **Waiting for Card Scan:** The MFRC522 RFID reader waits for an RFID card to be presented.
3. **UID Reading & Display:** Reads card UID, displays on LCD, and logs on serial monitor.
4. **Send Attendance Data:** Constructs JSON payload with card UID and timestamp (milliseconds since ESP32 boot) and sends it via HTTP POST to `http://:3000/attendance`.
5. **Node.js Server Processing:** Validates the payload, converts timestamp to proper date, and saves attendance record in MongoDB.
6. **ACK & Feedback:** Node.js responds with success message; feedback shown on the LCD.
7. **Repeat:** Ready for next card scan.

### Code Snippet Examples  
**ESP32 Arduino (Sending attendance):**
```cpp
StaticJsonDocument doc;
doc["cardId"] = uid;
doc["timestamp"] = millis();

String jsonPayload;
serializeJson(doc, jsonPayload);

http.begin(serverUrl);
http.addHeader("Content-Type", "application/json");

int httpResponseCode = http.POST(jsonPayload);
```

**Node.js Server (Handling attendance POST request):**
```js
app.post('/attendance', async (req, res) => {
  const { cardId, timestamp } = req.body;
  if (!cardId) return res.status(400).json({ message: 'Card ID is required.' });

  const newAttendance = new Attendance({
    cardId,
    timestamp: timestamp ? new Date(timestamp) : new Date(),
  });

  await newAttendance.save();
  res.status(200).json({ message: 'Attendance recorded successfully!', attendance: newAttendance });
});
```

### Board Connections  
| Signal                  | ESP32 Pin | MFRC522 Pin | LCD Pin    |
|-------------------------|-----------|-------------|------------|
| SPI CS (SS)             | GPIO 21   | SDA (SS)    | N/A        |
| SPI RST                 | GPIO 22   | RST         | N/A        |
| SPI MOSI                | GPIO 23   | MOSI        | N/A        |
| SPI MISO                | GPIO 19   | MISO        | N/A        |
| SPI SCK                 | GPIO 18   | SCK         | N/A        |
| I2C SDA (for LCD)       | GPIO 14   | N/A         | SDA        |
| I2C SCL (for LCD)       | GPIO 27   | N/A         | SCL        |
| Power (3.3V/5V)         | 3.3V      | VCC         | VCC        |
| Ground                  | GND       | GND         | GND        |

### Future Improvements  
- Add authentication for card readers and secure HTTP communication (HTTPS).
- Implement web dashboard to visualize attendance data in real-time.
- Support multiple RFID readers with different IDs.
- Add timestamp synchronization via NTP on ESP32 for accurate logging.

### Contact  
Feel free to connect with me or ask for further details about this project or similar IoT & Embedded systems work.

Would you like me to help you format it for a GitHub README file or LinkedIn project description specifically?
