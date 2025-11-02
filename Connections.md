## 🏗️ System Architecture

### Block Diagram
```
[ Sensors ] → [ ESP8266 ] → [ Web Server ] → [ Cloud Storage ]
   │            │               │              │
 ECG        WiFi Module    Patient Dashboard  ThingSpeak
 Pulse Ox                  Data Processing    MySQL Database
 Temperature
```

### Data Flow
1. **Sensor Data Acquisition**
   - AD8232: ECG waveform and heart rate
   - MAX30100: Heart rate variability and SpO2
   - DS18B20: Body temperature

2. **Local Processing**
   - ESP8266 processes and filters data
   - 20-second averaging window
   - Web server hosting for real-time display

3. **Cloud Integration**
   - ThingSpeak: Real-time data streaming and visualization
   - MySQL: Permanent patient record storage

---

## 🔌 Hardware Setup

### Components List
| Component | Quantity | Purpose |
|-----------|----------|---------|
| ESP8266 NodeMCU | 1 | Main microcontroller & WiFi |
| AD8232 ECG Sensor | 1 | Heart electrical activity |
| MAX30100 Pulse Oximeter | 1 | Heart rate & oxygen saturation |
| DS18B20 Temperature Sensor | 1 | Body temperature |
| Breadboard | 1 | Circuit assembly |
| Jumper Wires | Multiple | Connections |
| 4.7kΩ Resistor | 1 | DS18B20 pull-up |
| ECG Electrodes | 3 | Patient connection |

### Pin Connections Table
| ESP8266 Pin | Sensor | Connection | Notes |
|-------------|---------|------------|-------|
| D1 | AD8232 | LO+ | Lead-off detection |
| D2 | AD8232 | LO- | Lead-off detection |
| A0 | AD8232 | OUTPUT | ECG analog output |
| D4 | MAX30100 | SDA | I2C data |
| D5 | MAX30100 | SCL | I2C clock |
| D3 | DS18B20 | DATA | OneWire data |
| 3.3V | All | VCC | Power supply |
| GND | All | GND | Ground |

### Circuit Diagram
```
ESP8266 NodeMCU
┌─────────────┐
│          3V3│───┬───[All Sensors VCC]
│          GND│───┬───[All Sensors GND]
│           D1│───┼───[AD8232 LO+]
│           D2│───┼───[AD8232 LO-]
│           A0│───┼───[AD8232 OUTPUT]
│           D4│───┼───[MAX30100 SDA]
│           D5│───┼───[MAX30100 SCL]
│           D3│───┼───[DS18B20 DATA]───4.7kΩ───3V3
└─────────────┘
```

### Sensor Placement Guidelines

#### AD8232 ECG Electrodes
```
Right Arm (RA) ─── White wire
Left Arm (LA) ─── Black wire  
Right Leg (RL) ─── Red wire (ground reference)
```

#### MAX30100 Placement
- Finger clip sensor on index finger
- Ensure good contact without excessive pressure
- Avoid ambient light interference

#### DS18B20 Placement
- Axillary (armpit) for body temperature
- Ensure good skin contact
- Can be insulated for accurate reading

---

## 💻 Software Installation

### Arduino IDE Setup

#### 1. Install Required Boards
```arduino
File → Preferences → Additional Board Manager URLs:
http://arduino.esp8266.com/stable/package_esp8266com_index.json
```

#### 2. Install Libraries
```arduino
Tools → Manage Libraries → Search and Install:
- ESP8266WiFi
- ESP8266WebServer
- OneWire
- DallasTemperature
- MAX30100 by OXullo Intersecans
- ArduinoJson by Benoit Blanchon
```

#### 3. Board Configuration
```arduino
Tools → Board → NodeMCU 1.0 (ESP-12E Module)
Tools → Flash Size → 4M (3M SPIFFS)
Tools → CPU Frequency → 80 MHz
Tools → Upload Speed → 115200
Tools → Port → [Your COM Port]
```
