const express = require('express');
const mongoose = require('mongoose');
const bodyParser = require('body-parser');
const cors = require('cors');

const app = express();
const port = 3000;

app.use(cors());
app.use(bodyParser.json());

const mongoDBUri = 'mongodb://localhost:27017';

mongoose.connect(mongoDBUri)
  .then(() => console.log('MongoDB connected successfully!'))
  .catch(err => console.error('MongoDB connection error:', err));

const attendanceSchema = new mongoose.Schema({
  cardId: { type: String, required: true },
  timestamp: { type: Date, default: Date.now },
});

const Attendance = mongoose.model('Attendance', attendanceSchema);

app.post('/attendance', async (req, res) => {
  try {
    const { cardId, timestamp } = req.body;

    if (!cardId) {
      return res.status(400).json({ message: 'Card ID is required.' });
    }

    const newAttendance = new Attendance({
      cardId,
      timestamp: timestamp ? new Date(timestamp) : new Date(),
    });

    await newAttendance.save();

    console.log('Attendance recorded:', newAttendance);
    res.status(200).json({ message: 'Attendance recorded successfully!', attendance: newAttendance });

  } catch (error) {
    console.error('Error recording attendance:', error);
    res.status(500).json({ message: 'Internal server error.' });
  }
});

app.get('/attendance', async (req, res) => {
  try {
    const allAttendances = await Attendance.find({});
    res.status(200).json(allAttendances);
  } catch (error) {
    console.error('Error fetching attendance:', error);
    res.status(500).json({ message: 'Internal server error.' });
  }
});

app.listen(port, () => {
  console.log(`Node.js server listening at http://localhost:${port}`);
  console.log(`Attendance endpoint: http://localhost:${port}/attendance`);
});
