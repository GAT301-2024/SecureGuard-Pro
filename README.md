# SecureGuard-Pro - ESP32 Smart Security System

![Project Banner](https://via.placeholder.com/800x300?text=SecureGuard-Pro+Banner)  
*Intelligent environmental monitoring and security system with web-based control*

## 📌 Table of Contents
- [Features](#-features)
- [Hardware Components](#-hardware-components)
- [Installation](#-installation)
- [Web Interface](#-web-interface)
- [API Endpoints](#-api-endpoints)
- [Troubleshooting](#-troubleshooting)
- [Contributing](#-contributing)
- [License](#-license)

## 🌟 Features

### 🛡️ Security Monitoring
- Real-time temperature tracking with dual threshold alerts
- Ambient noise detection with configurable sensitivity
- Light level analysis for environmental awareness

### 🚨 Alert System
- Tri-color RGB LED for visual status indication
  - Red: Critical temperature
  - Purple: Sound alert
  - Blue: Low light condition
  - Green: Normal operation
- Piezo buzzer with programmable patterns

### 🌐 Web Dashboard
- Responsive design works on mobile/desktop
- Real-time sensor data visualization
- Interactive configuration panels
- User authentication system

### ⚙️ Configuration
```cpp
struct Config {
  bool alarmEnabled = true;
  float tempWarning = 30.0;    // °C
  float tempCritical = 35.0;   // °C
  int soundThreshold = 2000;   // 0-4095
  int lightThreshold = 2000;   // 0-4095
  int updateInterval = 3;      // seconds
  String deviceName = "SecureGuard Pro";
};
