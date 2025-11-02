/*
The IoT Health Monitoring System is a comprehensive 
biomedical monitoring solution that collects and analyzes 
vital signs in real-time using multiple medical-grade sensors. 
The system provides healthcare professionals with 
a complete patient monitoring platform 
with cloud storage and web-based visualization.

Component	            Quantity	   Specification	        Purpose
ESP8266 NodeMCU	          1	      CP2102, 4MB Flash	    Main Controller
AD8232 ECG Module	      1	      3.3V, Analog Output	Heart Electrical Activity
MAX30100 Sensor	          1	      I2C, 3.3V	            Pulse Oximetry
DS18B20 Sensor	          1	      Waterproof, 3.3V	    Body Temperature
ECG Electrodes	          3+	  Disposable, Ag/AgCl	Patient Interface
Breadboard	              1	      400 points	        Circuit Assembly
Jumper Wires	          15+	  Male-to-Male	        Connections
4.7kΩ Resistor	          1	      1/4W	                DS18B20 Pull-up
Micro USB Cable	          1	      5V/2A	                Power Supply

Created By - Anurag Panda
Dated - 02/11/2025
*/

//webserver related header files
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

//sensor based header files
#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "MAX30100_PulseOximeter.h"

//sensor pin Dfinations
#define ECG_LO_PLUS D1
#define ECG_LO_MINUS D2
#define ECG_OUTPUT A0
#define ONE_WIRE_BUS D3
#define SDA_PIN D4
#define SCL_PIN D5

//Sensor objects
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);
PulseOximeter pox;

//WiFi credentials
const char* ssid = "MY_SSID"; //wifi username
const char* password = "anuragini1234"; //wifi password

ESP8266WebServer server(80);
WiFiClientSecure client;

const char* serverURL = "https://iot-health-api-production.up.railway.app/upload-patient-data"; //fastAPI endpoint


//Data variables
struct PatientData {
  //Personal Information
  String name = "";
  int age = 0;
  String sex = "";
  String bloodGroup = "";
  int height = 0; //in cm
  float weight = 0.0; //in kg
  String  diseases = "";

  //vital signs
  int heartRate = 0;
  float spo2 = 0.0;
  float temperature = 0.0;

  // //calcualated Metrics
  // float bmi = 0.0
};

PatientData patient;
bool isScanning = false;
unsigned long scanStartTime = 0;


const int ECG_DURATION_MS = 20000;
const int  ECG_INTERVAL_MS = 100;

/*
// HRV Calculation
const int HRV_SAMPLES = 10;
unsigned long rrIntervals[HRV_SAMPLES];
int rrIndex = 0;
unsigned long lastBeatTime = 0;
*/

void setup() {
  Serial.begin(115200);

  //initialize pins
  pinMode(ECG_LO_PLUS, INPUT);
  pinMode(ECG_LO_MINUS, INPUT);
  pinMode(ECG_OUTPUT, INPUT);

  //initialize sensoers
  initializeSensors();

  //WiFi server start
  WiFi.begin(ssid, password);
  
  Serial.print("Conneting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  client.setInsecure(); //skip ssl check for testing

  //setup routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/start", HTTP_POST, handleStartScan);
  server.on("/upload", HTTP_POST, handleUploadData);
  server.on("/data", HTTP_GET, handleGetData);
  server.begin();
  Serial.println("Webserver Started");
}


void loop() {
  server.handleClient();

  // if (isScanning) {
  //   readSensors();
  // }

  pox.update();
}


void initializeSensors() {
  // Initialize temperature sensor
  tempSensor.begin();
  
  // Initialize pulse oximeter
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!pox.begin()) {
    Serial.println("MAX30100 initialization failed!");
  } else {
    Serial.println("MAX30100 initialized successfully!");
    // Set up beat detection callback
    // pox.setOnBeatDetectedCallback(onBeatDetected);
  }
  
  // Set pulse oximeter configuration
  pox.setIRLedCurrent(MAX30100_LED_CURR_7_6MA);
  
  Serial.println("All sensors initialized!");
}

/*
void onBeatDetected() {
  unsigned long currentTime = millis();
  if (lastBeatTime > 0) {
    unsigned long rrInterval = currentTime - lastBeatTime;
    rrIntervals[rrIndex] = rrInterval;
    rrIndex = (rrIndex + 1) % HRV_SAMPLES;
  }
  lastBeatTime = currentTime;
}
*/

// "/"
void handleRoot() {
    String html = createWebPage();
    server.send(200, "text/html", html);
}

// "/start"
void handleStartScan() {
  //get patinet details from form
  patient.name = server.arg("name");
  patient.age = server.arg("age").toInt();
  patient.sex = server.arg("sex");
  patient.bloodGroup = server.arg("bloodGroup");
  patient.height = server.arg("height").toInt();
  patient.weight = server.arg("weight").toFloat();
  patient.diseases = server.arg("diseases");

  // Reset sensor data
  resetSensorData();

  // Start scanning
  isScanning = true;
  scanStartTime = millis();

  Serial.println("Scan started for patient: " + patient.name);
  
  server.send(200, "text/plain", "Scan started!");
}

// "/upload"
void handleUploadData() {
  if (readAndUploadData()) {
    server.send(200, "text/plain", "Data uploaded Successfully!");
  } else {
    server.send(500, "text/plain", "Upload failed!");
  }
}

// "/data"
void handleGetData() {
  StaticJsonDocument<512> jsonDoc;
  jsonDoc["hr"] = patient.heartRate;
  jsonDoc["spo2"] = patient.spo2;
  jsonDoc["temp"] = patient.temperature;
  jsonDoc["scanning"] = isScanning;
  
  String jsonString;
  serializeJson(jsonDoc, jsonString);
  
  server.send(200, "application/json", jsonString);
}

/*
void readSensors() {
  // Read ECG
  readECG();
  
  // Read pulse oximeter data (handled in loop via update())
  patient.heartRate = pox.getHeartRate();
  patient.spo2 = pox.getSpO2();
  
  // Read temperature
  tempSensor.requestTemperatures();
  patient.temperature = tempSensor.getTempCByIndex(0);
}

void readECG() {
  if (digitalRead(ECG_LO_PLUS) == HIGH || digitalRead(ECG_LO_MINUS) == HIGH) {
    Serial.println("Electrode contact issue detected!");
    return;
  }
  
  int ecgRaw = analogRead(ECG_OUTPUT);
  float ecgVoltage = (ecgRaw * 3.3) / 1024.0;
}
*/

int  ecgSampleIndex = 0;
int  rrIndex = 0;
int  lastBeatTime = 0;

void resetSensorData() {
  patient.heartRate = 0;
  patient.spo2 = 0.0;
  patient.temperature = 0.0;
  ecgSampleIndex = 0;
  rrIndex = 0;
  lastBeatTime = 0;
}

bool readAndUploadData() {
  Serial.println("Collecting Data...");

  //Read Temperature
  tempSensor.requestTemperatures();
  patient.temperature = tempSensor.getTempCByIndex(0);

  // Update MAX30100
  for (int i = 0; i < 100; i++) {
    pox.update();
    delay(10);
  }
  patient.heartRate = pox.getHeartRate();
  patient.spo2 = pox.getSpO2();

  // ECG Sampling
  DynamicJsonDocument ecgArr(4096);
  JsonArray ecgData = ecgArr.to<JsonArray>();
  unsigned long start = millis();
  while (millis() - start < ECG_DURATION_MS) {
    int ecgVal = analogRead(ECG_OUTPUT);
    float voltage = ecgVal * (3.3 / 1023.0);
    unsigned long ts = millis();
    JsonObject obj = ecgData.createNestedObject();
    obj["timestamp"] = ts;
    obj["voltage"] = voltage;
    delay(ECG_INTERVAL_MS);
  }

  // Build full JSON payload
  DynamicJsonDocument doc(8192);
  doc["name"] = patient.name;
  doc["age"] = patient.age;
  doc["sex"] = patient.sex;
  doc["blood_group"] = patient.bloodGroup;
  doc["height"] = patient.height;
  doc["weight"] = patient.weight;
  doc["previous_conditions"] = patient.diseases;
  doc["temperature"] = patient.temperature;
  doc["heart_rate"] = patient.heartRate;
  doc["spo2"] = patient.spo2;
  doc["ecg_data"] = ecgArr;

  String jsonData;
  serializeJson(doc, jsonData);
  Serial.println("\n--- JSON DATA ---");
  Serial.println(jsonData);

   // Send HTTPS POST
  HTTPClient https;
  https.begin(client, serverURL);
  https.addHeader("Content-Type", "application/json");
  int code = https.POST(jsonData);

  // Response page with Back button
  String page = "<!DOCTYPE html><html><head><style>"
                "body{font-family:Arial;text-align:center;margin-top:50px;background:#f0f8ff;}"
                ".btn{background-color:#007bff;color:white;padding:10px 20px;border:none;border-radius:5px;text-decoration:none;}"
                ".btn:hover{background-color:#0056b3;}"
                "</style></head><body>";

  if (code == 200) {
    String payload = https.getString();
    page += "<h2>Data sent successfully!</h2>";
    page += "<p>Patient <b>" + patient.name + "</b> has been registered.</p>";
    Serial.println("POST Response");
    Serial.println(payload);
    return true;
  } else {
    String payload = https.getString();
    page += "<h2>⚠ Failed to send data!</h2>";
    page += String("<p>Error code: ") + code + "</p>";
    page += "<p>Response: " + payload + "</p>";
    Serial.println("POST Response");
    Serial.println(payload);
    return false;
  }

  page += "<br><a href='/' class='btn'>⬅ Back to Home</a>";
  page += "</body></html>";

  server.send(200, "text/html", page);

  https.end();
}


String createWebPage() {
String html = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>IoT Health Monitoring Dashboard</title>
    <style>
        :root {
            --primary-color: #2196F3;
            --secondary-color: #03A9F4;
            --success-color: #4CAF50;
            --danger-color: #f44336;
            --background-color: #f5f5f5;
            --card-background: #ffffff;
        }

        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }

        body {
            background-color: var(--background-color);
            line-height: 1.6;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
            padding: 20px;
        }

        header {
            background: var(--primary-color);
            color: white;
            padding: 1rem;
            text-align: center;
            margin-bottom: 2rem;
            border-radius: 0 0 10px 10px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
        }

        .dashboard-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 2rem;
        }

        .ecg-section {
            margin-bottom: 2rem;
        }

        .ecg-container {
            width: 100%;
            height: 200px;
            background-color: #fff;
            border-radius: 4px;
            padding: 10px;
        }

        .metrics-section {
            background: var(--card-background);
        }

        .metrics-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 1.5rem;
            padding: 1rem 0;
        }

        .metric-item {
            display: flex;
            flex-direction: column;
            gap: 0.5rem;
        }

        .metric-item label {
            color: #666;
            font-size: 0.9rem;
            font-weight: 500;
        }

        .metric-value {
            background: #f5f5f5;
            padding: 0.75rem;
            border-radius: 4px;
            font-size: 1.1rem;
            font-weight: 600;
            display: flex;
            justify-content: space-between;
            align-items: center;
        }

        .unit {
            color: #666;
            font-size: 0.9rem;
            font-weight: normal;
        }

        .card {
            background: var(--card-background);
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);
            transition: transform 0.3s ease;
        }

        .card:hover {
            transform: translateY(-5px);
        }

        .patient-info {
            margin-bottom: 2rem;
        }

        .form-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 1rem;
            margin-bottom: 1.5rem;
        }

        .form-group {
            display: flex;
            flex-direction: column;
        }

        .form-group.full-width {
            grid-column: 1 / -1;
        }

        .form-group label {
            margin-bottom: 0.5rem;
            color: #666;
            font-weight: 500;
        }

        .form-group input,
        .form-group select {
            padding: 8px 12px;
            border: 1px solid #ddd;
            border-radius: 4px;
            font-size: 1rem;
        }

        .form-group input:focus,
        .form-group select:focus {
            border-color: var(--primary-color);
            outline: none;
            box-shadow: 0 0 0 2px rgba(33, 150, 243, 0.1);
        }

        .button-group {
            display: flex;
            gap: 1rem;
            margin-bottom: 1rem;
        }

        .primary-button {
            background-color: #2196F3;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            flex: 1;
        }

        .secondary-button {
            background-color: #4CAF50;
            color: white;
            padding: 10px 20px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
            flex: 1;
        }

        .system-status {
            background-color: #E0F7FA;
            color: #00838F;
            padding: 10px;
            border-radius: 4px;
            text-align: center;
            margin-top: 1rem;
        }

        .card h2 {
            color: var(--primary-color);
            margin-bottom: 15px;
            font-size: 1.5rem;
        }

        .sensor-value {
            font-size: 2rem;
            font-weight: bold;
            color: #333;
            margin: 10px 0;
        }

        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 8px;
        }

        .status-normal {
            background-color: var(--success-color);
        }

        .status-warning {
            background-color: #FFC107;
        }

        .status-alert {
            background-color: var(--danger-color);
        }

        .chart-container {
            width: 100%;
            height: 200px;
            margin-top: 15px;
        }

        .controls {
            display: flex;
            gap: 10px;
            margin-top: 15px;
        }

        button {
            background-color: var(--primary-color);
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            cursor: pointer;
            transition: background-color 0.3s ease;
        }

        button:hover {
            background-color: var(--secondary-color);
        }

        .refresh-button {
            position: fixed;
            bottom: 20px;
            right: 20px;
            background-color: var(--primary-color);
            color: white;
            width: 50px;
            height: 50px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            cursor: pointer;
            box-shadow: 0 2px 5px rgba(0, 0, 0, 0.2);
        }

        @media (max-width: 768px) {
            .container {
                padding: 10px;
            }

            .dashboard-grid {
                grid-template-columns: 1fr;
            }

            .card {
                margin-bottom: 15px;
            }

            .sensor-value {
                font-size: 1.5rem;
            }
        }
    </style>
</head>

<body>
    <header>
        <h1>IoT Health Monitoring System</h1>
    </header>

    <div class="container">
        <!-- Patient Demographics Section -->
        <div class="card patient-info">
            <h2>Patient Demographics</h2>
            <div class="form-grid">
                <div class="form-group">
                    <label for="fullName">Full Name:</label>
                    <input type="text" id="fullName" placeholder="Enter patient name">
                </div>
                <div class="form-group">
                    <label for="age">Age:</label>
                    <input type="number" id="age" placeholder="Years">
                </div>
                <div class="form-group">
                    <label for="sex">Sex:</label>
                    <select id="sex">
                        <option value="">Select Gender</option>
                        <option value="male">Male</option>
                        <option value="female">Female</option>
                        <option value="other">Other</option>
                    </select>
                </div>
                <div class="form-group">
                    <label for="bloodGroup">Blood Group:</label>
                    <select id="bloodGroup">
                        <option value="">Select Blood Group</option>
                        <option value="A+">A+</option>
                        <option value="A-">A-</option>
                        <option value="B+">B+</option>
                        <option value="B-">B-</option>
                        <option value="AB+">AB+</option>
                        <option value="AB-">AB-</option>
                        <option value="O+">O+</option>
                        <option value="O-">O-</option>
                    </select>
                </div>
                <div class="form-group">
                    <label for="height">Height (cm):</label>
                    <input type="number" id="height" placeholder="Centimeters">
                </div>
                <div class="form-group">
                    <label for="weight">Weight (kg):</label>
                    <input type="number" id="weight" placeholder="Kilograms">
                </div>
                <div class="form-group full-width">
                    <label for="conditions">Known Diseases/Conditions:</label>
                    <input type="text" id="conditions" placeholder="Hypertension, Diabetes, Asthma, etc.">
                </div>
            </div>
            <div class="button-group">
                <button class="primary-button" onclick="startHealthScan()">Start Health Scan</button>
                <button class="secondary-button" onclick="uploadPatientData()">Upload Patient Data</button>
            </div>
            <div id="systemStatus" class="system-status">
                System Ready - Enter patient details to begin
            </div>
        </div>

        <!-- ECG Graph Section -->
        <div class="card ecg-section">
            <div class="ecg-container">
                <canvas id="ecgChart"></canvas>
            </div>
        </div>

        <!-- Vital Signs & Health Metrics Section -->
        <div class="card metrics-section">
            <h2>Vital Signs & Health Metrics</h2>
            <div class="metrics-grid">
                <div class="metric-item">
                    <label>ECG Value:</label>
                    <div class="metric-value">
                        <span id="ecgValue">--</span>
                        <span class="unit">mV</span>
                    </div>
                </div>
                <div class="metric-item">
                    <label>Heart Rate:</label>
                    <div class="metric-value">
                        <span id="heartRate">--</span>
                        <span class="unit">bpm</span>
                    </div>
                </div>
                <div class="metric-item">
                    <label>SpO2 Level:</label>
                    <div class="metric-value">
                        <span id="spo2">--</span>
                        <span class="unit">%</span>
                    </div>
                </div>
                <div class="metric-item">
                    <label>Body Temperature:</label>
                    <div class="metric-value">
                        <span id="temperature">--</span>
                        <span class="unit">C</span>
                    </div>
                </div>
                <!-- <div class="metric-item">
                      <label>Heart Rate Variability:</label>
                      <div class="metric-value">
                        <span id="hrv">--</span>
                        <span class="unit">ms</span>
                      </div>
                  </div> -->
                <div class="metric-item">
                    <label>BMI (Body Mass Index):</label>
                    <div class="metric-value">
                        <span id="bmi">--</span>
                    </div>
                </div>
            </div>
        </div>
    </div>
    </div>

    <div class="refresh-button" onclick="refreshData()">
        refresh
    </div>

    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
        // Initialize ECG data arrays
        let ecgTimeStamps = Array(100).fill('');
        let ecgValues = Array(100).fill(0);

        // Initialize ECG Chart
        const ecgChart = new Chart(
            document.getElementById('ecgChart'),
            {
                type: 'line',
                data: {
                    labels: ecgTimeStamps,
                    datasets: [{
                        label: 'ECG',
                        data: ecgValues,
                        borderColor: '#FF4081',
                        borderWidth: 1.5,
                        fill: false,
                        tension: 0,
                        pointRadius: 0
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    interaction: {
                        intersect: false
                    },
                    scales: {
                        x: {
                            display: true,
                            grid: {
                                display: true,
                                color: '#eee'
                            }
                        },
                        y: {
                            display: true,
                            min: -2,
                            max: 2,
                            grid: {
                                display: true,
                                color: '#eee'
                            }
                        }
                    },
                    animation: false,
                    plugins: {
                        legend: {
                            display: false
                        }
                    }
                }
            }
        );

        // Sensor thresholds
        const thresholds = {
            temperature: {
                min: 35,
                max: 38
            },
            heartRate: {
                min: 60,
                max: 100
            },
            spo2: {
                min: 95,
                max: 100
            }
        };

        let monitoring = false;
        let updateInterval;

        // Function to update sensor status indicators
        function updateStatus(value, type) {
            const element = document.querySelector(`#${type}`).parentElement.querySelector('.status-indicator');
            if (value < thresholds[type].min || value > thresholds[type].max) {
                element.className = 'status-indicator status-alert';
            } else if (value === thresholds[type].min || value === thresholds[type].max) {
                element.className = 'status-indicator status-warning';
            } else {
                element.className = 'status-indicator status-normal';
            }
        }

        // Function to update charts
        function updateChart(chart, value) {
            const now = new Date();
            const timeStr = now.toLocaleTimeString();

            chart.data.labels.push(timeStr);
            chart.data.datasets[0].data.push(value);

            if (chart.data.labels.length > 10) {
                chart.data.labels.shift();
                chart.data.datasets[0].data.shift();
            }

            chart.update();
        }

        // Function to fetch sensor data from ESP8266
        async function fetchSensorData() {
            try {
                // Replace with your ESP8266's IP address or domain
                const response = await fetch('/data');
                const data = await response.json();

                // Handle ECG data (expecting: { ecg: { value: number, timestamp: string } })
                if (data.ecg && data.ecg.value !== undefined) {
                    document.getElementById('ecgValue').textContent = data.ecg.value.toFixed(2);
                    updateECGChart(data.ecg);
                }

                // Update vital signs
                document.getElementById('heartRate').textContent = data.heartRate;
                document.getElementById('spo2').textContent = data.spo2;
                document.getElementById('temperature').textContent = data.temperature.toFixed(1);
                document.getElementById('hrv').textContent = data.hrv || '--';

                // Update BMI if height and weight are available
                const patientData = JSON.parse(localStorage.getItem('patientData') || '{}');
                if (patientData.height && patientData.weight) {
                    const heightInMeters = patientData.height / 100;
                    const bmi = (patientData.weight / (heightInMeters * heightInMeters)).toFixed(1);
                    document.getElementById('bmi').textContent = bmi;
                }

            } catch (error) {
                console.error('Error fetching sensor data:', error);
            }
        }

        // Function to update ECG chart with real-time data
        function updateECGChart(ecgData) {
            // Update data arrays
            ecgValues.push(ecgData.value);
            ecgValues.shift();

            ecgTimeStamps.push(ecgData.timestamp);
            ecgTimeStamps.shift();

            // Update chart data
            ecgChart.data.labels = ecgTimeStamps;
            ecgChart.data.datasets[0].data = ecgValues;

            ecgChart.update('none'); // Update without animation for better performance
        }

        // Function to format timestamp for display
        function formatTimestamp(timestamp) {
            const date = new Date(timestamp);
            return date.toLocaleTimeString('en-US', {
                hour12: false,
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit',
                fractionalSecondDigits: 3
            });
        }

        // Function to start monitoring
        function startMonitoring() {
            if (!monitoring) {
                monitoring = true;
                updateInterval = setInterval(fetchSensorData, 2000); // Update every 2 seconds
                fetchSensorData(); // Initial fetch
            }
        }

        // Function to stop monitoring
        function stopMonitoring() {
            monitoring = false;
            clearInterval(updateInterval);
        }

        // Function to calibrate sensors
        async function calibrateSensors() {
            try {
                const response = await fetch('/calibrate', { method: 'POST' });
                const data = await response.json();
                alert(data.message || 'Calibration complete!');
            } catch (error) {
                console.error('Error calibrating sensors:', error);
                alert('Error calibrating sensors. Please try again.');
            }
        }

        // Function to manually refresh data
        function refreshData() {
            if (!monitoring) {
                fetchSensorData();
            }
        }

        // Function to start health scan
        function startHealthScan() {
            const patientData = {
                fullName: document.getElementById('fullName').value,
                age: document.getElementById('age').value,
                sex: document.getElementById('sex').value,
                bloodGroup: document.getElementById('bloodGroup').value,
                height: document.getElementById('height').value,
                weight: document.getElementById('weight').value,
                conditions: document.getElementById('conditions').value
            };

            // Validate required fields
            if (!patientData.fullName || !patientData.age || !patientData.sex || !patientData.bloodGroup) {
                alert('Please fill in all required fields');
                return;
            }

            // Calculate BMI
            const heightInMeters = patientData.height / 100;
            const bmi = (patientData.weight / (heightInMeters * heightInMeters)).toFixed(1);

            // Store patient data
            localStorage.setItem('patientData', JSON.stringify(patientData));
            localStorage.setItem('bmi', bmi);

            // Start monitoring
            startMonitoring();

            // Update system status
            document.getElementById('systemStatus').textContent = 'Monitoring Started - Collecting vital signs...';
            document.getElementById('systemStatus').style.backgroundColor = '#E8F5E9';
            document.getElementById('systemStatus').style.color = '#2E7D32';
        }

        // Function to upload patient data
        async function uploadPatientData() {
            const patientData = localStorage.getItem('patientData');
            if (!patientData) {
                alert('No patient data available to upload');
                return;
            }

            try {
                // Replace with your server endpoint
                const response = await fetch('/start', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: patientData
                });

                const result = await response.json();
                alert(result.message || 'Data uploaded successfully!');
            } catch (error) {
                console.error('Error uploading patient data:', error);
                alert('Error uploading patient data. Please try again.');
            }
        }

        // Load patient data if available when page loads
        document.addEventListener('DOMContentLoaded', () => {
            const savedPatientData = localStorage.getItem('patientData');
            if (savedPatientData) {
                const data = JSON.parse(savedPatientData);
                document.getElementById('fullName').value = data.fullName;
                document.getElementById('age').value = data.age;
                document.getElementById('sex').value = data.sex;
                document.getElementById('bloodGroup').value = data.bloodGroup;
                document.getElementById('height').value = data.height;
                document.getElementById('weight').value = data.weight;
                document.getElementById('conditions').value = data.conditions;
            }
        });
    </script>
</body>
</html>
)=====";

return html;
}

