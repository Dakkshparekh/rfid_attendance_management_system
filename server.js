// Import necessary packages
const express = require('express'); // Our web server framework
const mongoose = require('mongoose'); // For talking to MongoDB
const bodyParser = require('body-parser'); // To understand incoming data
const cors = require('cors'); // To allow communication from our ESP32 device

const app = express(); // Create our Express app
const port = 3000; // The port our server will listen on

// --- Middleware ---
app.use(cors()); // Allow requests from any origin (important for ESP32)
app.use(bodyParser.json()); // Tell Express to understand JSON data

// --- MongoDB Connection ---
// Replace 'YOUR_MONGODB_CONNECTION_STRING' with your actual MongoDB connection string.
// For local MongoDB: 'mongodb://localhost:27017/attendanceDB'
// For MongoDB Atlas: Go to your Atlas cluster, click 'Connect', then 'Connect your application'.
const mongoDBUri = 'mongodb://localhost:27017';

mongoose.connect(mongoDBUri)
  .then(() => console.log('MongoDB connected successfully!'))
  .catch(err => console.error('MongoDB connection error:', err));

// --- Define a Schema for Attendance Data ---
// This is like telling MongoDB what kind of information we want to store for each attendance record.
const attendanceSchema = new mongoose.Schema({
  cardId: { type: String, required: true }, // The unique ID from the RFID card
  timestamp: { type: Date, default: Date.now }, // The time the attendance was recorded
  // You could add more fields here, like 'teacherName', 'status' etc.
});

// Create a Model from the Schema. This is what we'll use to interact with the 'attendances' collection.
const Attendance = mongoose.model('Attendance', attendanceSchema);

// --- API Endpoint to Receive Attendance Data ---
// This is the special address our ESP32 will send data to.
app.post('/attendance', async (req, res) => {
  try {
    const { cardId, timestamp } = req.body; // Get the data sent from the ESP32
    console.log(req.body)

    // Basic validation (optional but good practice)
    if (!cardId) {
      return res.status(400).json({ message: 'Card ID is required.' });
    }

    // Create a new attendance record
    const newAttendance = new Attendance({
      cardId,
      timestamp: timestamp ? new Date(timestamp) : new Date(), // Use provided timestamp or current time
    });

    // Save the record to the database
    await newAttendance.save();

    console.log('Attendance recorded:', newAttendance);
    res.status(200).json({ message: 'Attendance recorded successfully!', attendance: newAttendance });

  } catch (error) {
    console.error('Error recording attendance:', error);
    res.status(500).json({ message: 'Internal server error.' });
  }
});

// --- Optional: API Endpoint to Get All Attendance Data ---
// app.get('/attendance', async (req, res) => {
//   try {
//     const allAttendances = await Attendance.find({}); // Find all attendance records
//     res.status(200).json(allAttendances);
//   } catch (error) {
//     console.error('Error fetching attendance:', error);
//     res.status(500).json({ message: 'Internal server error.' });
//   }
// });


// --- Start the Server ---
app.listen(port, () => {
  console.log(`Node.js server listening at http://localhost:${port}`);
  console.log(`Attendance endpoint: http://localhost:${port}/attendance`);
});