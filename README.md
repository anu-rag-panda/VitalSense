# IoT Health Monitoring System - Complete Documentation

## 📖 Table of Contents
1. [Project Overview](##🏥-project-overview)
2. [Project Structure](##project-structure)
3. [Features](##features)
4. [Prerequisites](##prerequisites)
5. [Setup Instructions](##setup-instructions)
6. [API Documentation](##api-documentation)
7. [Contributing](##contributing)
8. [📄 License and Disclaimer](##📄-license-and-disclaimer)
9. [Authors](##authors)
10. [Support](##support)
11. [Last Updated](##last-updated)

---

## 🏥 Project Overview

### Purpose
The IoT Health Monitoring System is a comprehensive solution for real-time patient vital sign monitoring using multiple biomedical sensors connected to an ESP8266 microcontroller. The system collects ECG, heart rate, SpO2, and temperature data, then transmits it to both cloud database platforms (Supabase) for analysis and storage.

### Key Features
- **Multi-sensor Integration**: ECG, Pulse Oximetry, Temperature monitoring
- **Real-time Web Interface**: Live data visualization and patient management
- **Cloud Storage**: Dual storage to ThingSpeak and MySQL database
- **20-second Scan Sessions**: Controlled data collection periods
- **Responsive Design**: Mobile-friendly web dashboard

### Target Users
- Healthcare professionals
- Remote patient monitoring systems
- Medical research institutions
- Home healthcare applications

## Project Structure

```
├── API/                    # Backend API server
│   ├── main.py            # Main API implementation
│   ├── requirements.txt    # Python dependencies
│   └── procfile           # Deployment configuration
└── VitalSense/            # ESP8266 firmware
    └── VitalSense.ino     # Arduino code for vital signs monitoring
```

## Features

- Real-time vital signs monitoring
- ESP8266-based sensor integration
- RESTful API for data collection and retrieval
- Secure data transmission and storage

## Prerequisites

- Arduino IDE with ESP8266 board support
- Python 3.x
- Required Python packages (listed in requirements.txt)
- ESP8266 development board
- Vital signs sensors (as per hardware requirements)

## Setup Instructions

### API Backend

1. Navigate to the API directory:
   ```bash
   cd API
   ```

2. Create a virtual environment (recommended):
   ```bash
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   ```

3. Install dependencies:
   ```bash
   pip install -r requirements.txt
   ```

4. Set up environment variables:
   - Copy `.env.example` to `.env`
   - Configure the required environment variables

5. Start the API server:
   ```bash
   python main.py
   ```

### ESP8266 Setup

1. Open Arduino IDE
2. Install ESP8266 board support if not already installed
3. Open `VitalSense/VitalSense.ino`
4. Configure WiFi credentials and API endpoint
5. Upload the code to your ESP8266 board

## Usage

1. Power up the ESP8266 device with sensors connected
2. Ensure the API server is running
3. The device will automatically start collecting and transmitting vital signs data
4. Access the API endpoints to retrieve and analyze the data

## API Documentation

### Web Server Endpoints

#### GET /
**Purpose**: Serve main web dashboard
**Response**: HTML page with patient form and data display

#### POST /start
**Purpose**: Begin 20-second data collection
**Parameters**:
- `name` (string): Patient name
- `age` (int): Patient age  
- `sex` (string): Patient gender (M/F/O)
- `diseases` (string): Known medical conditions

**Response**: "Scan started!"

#### POST /upload-patient-data
**Purpose**: Send collected data to cloud services
**Response**: "Data uploaded successfully!" or "Upload failed!"

### Data Formats

#### JSON Payload
```json
{
  "name": "string",
  "age": 0,
  "sex": "string",
  "blood_group": "string",
  "height": 0,
  "weight": 0,
  "known_conditions": "string",
  "temperature": 0,
  "heart_rate": 0,
  "spo2": 0,
  "ecg_data": [
    {
      "voltage": 0.0,
      "timestamp": "string"
    },
    ...
  ]
}
```

For more detailed API documentation, configuration guides, and troubleshooting information, please refer to our detailed documentation sections above.

## Contributing

1. Fork the repository
2. Create a new branch for your feature
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## 📄 License and Disclaimer

### Medical Disclaimer
This system is intended for educational and monitoring purposes only. It is not a medical device and should not be used for diagnostic purposes. Always consult healthcare professionals for medical advice and diagnosis.

### Safety Warning
- Ensure proper electrical isolation when connected to humans
- Do not use with patients connected to mains-powered equipment
- Follow all local safety regulations and guidelines
- Regular maintenance and inspection required

### License
This project is open-source under MIT License. Use responsibly and in accordance with local regulations.

## Authors

Documentation Maintainer: Anurag Panda

## Support

For technical support and contributions, please:
1. Check the documentation first
2. Review troubleshooting guide
3. Open an issue on GitHub
4. Contact the documentation maintainer

## Last Updated
Version: 1.0  

Date: November 2, 2025
