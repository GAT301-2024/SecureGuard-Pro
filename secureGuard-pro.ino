#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <TimeLib.h>


// === Sensor & Peripheral Pins ===
#define SOUND_SENSOR_PIN 13      // Replacing PIR_LEFT_PIN
#define TEMP_SENSOR_PIN 12       // Replacing PIR_RIGHT_PIN (analog input)
#define LDR_PIN 27               // Replacing WATER_SENSOR_PIN (analog input)
#define BUZZER_PIN 14
#define RGB_R_PIN 25
#define RGB_G_PIN 26
#define RGB_B_PIN 33

// 7-Segment 5161AS Dual Digit Display Pins
#define SEG_A_PIN 22
#define SEG_B_PIN 21
#define SEG_C_PIN 23
#define SEG_D_PIN 18
#define SEG_E_PIN 19
#define SEG_F_PIN 5
#define SEG_G_PIN 17
#define SEG_DP_PIN 16
#define DIGIT1_PIN 4
#define DIGIT2_PIN 2

const byte digits[10] = {
    // GFEDCBA
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

// === Globals ===
AsyncWebServer server(80);
const char *ssid = "SecureGuard-Pro";
const char *password = "secure@123";
const char *username = "Emma";
const char *userpass = "Em.ma.45";

volatile bool soundDetected = false;
bool buzzerMuted = false;
unsigned long lastSoundTime = 0;
unsigned long lastSensorUpdate = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastLoginTime = 0;
bool showTemperature = true;     // Toggle between temp and light

// User preferences
String currentTheme = "default";
String currentLanguage = "en";
String currentTimezone = "UTC+3";
String currentUnits = "metric";


// === Common Styles for All Pages ===
const char commonStyles[] PROGMEM = R"rawliteral(
:root {
  --primary: #2c3e50;
  --secondary: #3498db;
  --accent: #e74c3c;
  --light: #ecf0f1;
  --dark: #2c3e50;
  --success: #2ecc71;
  --info: #3498db;
  --warning: #f39c12;
  --danger: #e74c3c;
  --text-light: #ffffff;
  --text-dark: #2c3e50;
}

* {
  box-sizing: border-box;
  margin: 0;
  padding: 0;
  font-family: 'Roboto', sans-serif;
}

body {
  background-color: #f5f5f5;
  color: var(--dark);
  line-height: 1.6;
  background-image: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
}

.container {
  max-width: 1200px;
  margin: 0 auto;
  padding: 20px;
}

.btn {
  display: inline-block;
  background-color: var(--secondary);
  color: white;
  border: none;
  padding: 12px 24px;
  border-radius: 6px;
  cursor: pointer;
  font-size: 16px;
  font-weight: 500;
  text-decoration: none;
  text-align: center;
  transition: all 0.3s ease;
  margin: 5px 0;
  min-width: 120px;
}

.btn:hover {
  background-color: var(--primary);
  transform: translateY(-2px);
  box-shadow: 0 4px 8px rgba(0,0,0,0.1);
}

.btn-danger {
  background-color: var(--danger);
}

.btn-danger:hover {
  background-color: #c0392b;
}

input[type="text"], 
input[type="password"], 
input[type="email"], 
input[type="number"],
select, textarea {
  width: 100%;
  padding: 12px;
  margin: 8px 0;
  border: 1px solid #ddd;
  border-radius: 6px;
  font-size: 16px;
  transition: all 0.3s;
}

input:focus, select:focus, textarea:focus {
  border-color: var(--secondary);
  box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.2);
  outline: none;
}

.card {
  background: white;
  border-radius: 12px;
  box-shadow: 0 6px 15px rgba(0,0,0,0.1);
  padding: 25px;
  margin-bottom: 20px;
}

.header {
  background-color: var(--primary);
  color: white;
  padding: 15px 25px;
  margin-bottom: 30px;
  border-radius: 12px;
  box-shadow: 0 4px 12px rgba(0,0,0,0.1);
  display: flex;
  justify-content: space-between;
  align-items: center;
}

.nav-link {
  color: white;
  text-decoration: none;
  padding: 8px 12px;
  border-radius: 6px;
  transition: all 0.3s;
  margin: 0 5px;
}

.nav-link:hover {
  background-color: rgba(255,255,255,0.1);
}

.status-indicator {
  display: inline-block;
  width: 12px;
  height: 12px;
  border-radius: 50%;
  margin-right: 10px;
}

.status-active { background-color: var(--success); }
.status-warning { background-color: var(--warning); }
.status-danger { background-color: var(--danger); }

@media (max-width: 768px) {
  .container {
    padding: 10px;
  }
  
  .btn {
    padding: 10px 15px;
    font-size: 14px;
    min-width: 100px;
  }
  
  .header {
    flex-direction: column;
    text-align: center;
    padding: 15px;
  }
  
  .nav-link {
    margin: 5px;
    display: inline-block;
  }
}
)rawliteral";

// === Login Page ===
const char loginPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="%LANG%">
<head>
  <title>SecureGuard-Pro | Login</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@400;700&family=Roboto:wght@300;400;500&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
  <style>
    %COMMON_STYLES%
    
    .login-container {
      background: white;
      border-radius: 16px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.1);
      max-width: 450px;
      margin: 40px auto;
      padding: 40px;
      text-align: center;
    }
    
    .logo {
      width: 120px;
      height: auto;
      margin-bottom: 20px;
    }
    
    h2 {
      color: var(--primary);
      margin-bottom: 25px;
      font-family: 'Montserrat', sans-serif;
    }
    
    .forgot-password {
      text-align: right;
      margin: 15px 0;
    }
    
    .forgot-password a {
      color: var(--secondary);
      text-decoration: none;
      font-size: 14px;
    }
    
    .forgot-password a:hover {
      text-decoration: underline;
    }
    
    .language-selector {
      margin: 20px 0;
    }
    
    .language-selector select {
      padding: 8px 15px;
      border-radius: 20px;
      border: 1px solid #ddd;
      background-color: white;
    }
    
    .social-icons {
      margin: 25px 0;
      display: flex;
      justify-content: center;
      gap: 15px;
    }
    
    .social-icons a {
      display: inline-flex;
      align-items: center;
      justify-content: center;
      width: 40px;
      height: 40px;
      background: white;
      border-radius: 50%;
      color: var(--primary);
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      transition: all 0.3s;
    }
    
    .social-icons a:hover {
      background: var(--secondary);
      color: white;
      transform: translateY(-3px);
    }
    
    .version-info {
      font-size: 12px;
      color: #95a5a6;
      margin-top: 20px;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="login-container">
      <img src="https://i.imgur.com/JQZ1wzP.png" alt="Logo" class="logo">
      <h2>SecureGuard-Pro Login</h2>
      
      <form action="/login" method="POST">
        <input type="text" name="user" placeholder="Username" required>
        <input type="password" name="pass" placeholder="Password" required>
        
        <div class="forgot-password">
          <a href="/reset-password">Forgot password?</a>
        </div>
        
        <button type="submit" class="btn">Login</button>
      </form>
      
      <div class="language-selector">
        <select onchange="location.href='/setlang?lang='+this.value">
          <option value="en" %EN_SELECTED%>English</option>
          <option value="es" %ES_SELECTED%>Español</option>
          <option value="fr" %FR_SELECTED%>Français</option>
          <option value="de" %DE_SELECTED%>Deutsch</option>
        </select>
      </div>
      
      <div class="social-icons">
        <a href="https://github.com/Fthnath" title="GitHub"><i class="fab fa-github"></i></a>
        <a href="https://www.linkedin.com/in/emmanuel-olara-6aa993374/" title="LinkedIn"><i class="fab fa-linkedin"></i></a>
        <a href="https://x.com/eolara68" title="Twitter"><i class="fab fa-twitter"></i></a>
      </div>
      
      <div class="version-info">
        SecureGuard-Pro v2.4.1 | © 2023 All rights reserved
      </div>
    </div>
  </div>
</body>
</html>
)rawliteral";

// === Dashboard Page ===
const char dashboardPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="%LANG%">
<head>
  <title>SecureGuard-Pro | Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@400;700&family=Roboto:wght@300;400;500&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
  <style>
    %COMMON_STYLES%
    
    .dashboard-grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
      gap: 20px;
      margin-bottom: 30px;
    }
    
    .card h3 {
      color: var(--primary);
      border-bottom: 2px solid var(--secondary);
      padding-bottom: 10px;
      margin-bottom: 20px;
      font-family: 'Montserrat', sans-serif;
      display: flex;
      align-items: center;
      gap: 10px;
    }
    
    .card h3 i {
      color: var(--secondary);
    }
    
    .sensor-status {
      margin: 15px 0;
      padding: 10px;
      background: #f8f9fa;
      border-radius: 8px;
    }
    
    .sensor-status:hover {
      background: #e9ecef;
    }
    
    .sensor-label {
      font-weight: 500;
      margin-bottom: 5px;
    }
    
    .sensor-value {
      font-size: 18px;
      font-weight: 700;
    }
    
    .sensor-time {
      font-size: 12px;
      color: #7f8c8d;
    }
    
    .quick-actions {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 10px;
      margin-top: 20px;
    }
    
    .quick-action {
      padding: 15px;
      background: #f8f9fa;
      border-radius: 8px;
      text-align: center;
      transition: all 0.3s;
      text-decoration: none;
      color: var(--dark);
    }
    
    .quick-action:hover {
      background: #e9ecef;
      transform: translateY(-3px);
    }
    
    .quick-action i {
      font-size: 24px;
      margin-bottom: 10px;
      color: var(--secondary);
    }
    
    .quick-action-label {
      font-weight: 500;
    }
    
    footer {
      text-align: center;
      padding: 20px;
      color: #7f8c8d;
      font-size: 14px;
      background: white;
      border-radius: 12px;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="logo-container">
        <img src="https://i.imgur.com/JQZ1wzP.png" alt="Logo" style="width:40px;height:40px;">
        <span style="font-family: 'Montserrat', sans-serif; font-weight:700;">SecureGuard-Pro</span>
      </div>
      <nav>
        <a href="/dashboard" class="nav-link"><i class="fas fa-home"></i> Dashboard</a>
        <a href="/profile" class="nav-link"><i class="fas fa-user"></i> Profile</a>
        <a href="/settings" class="nav-link"><i class="fas fa-cog"></i> Settings</a>
      </nav>
      <div class="user-menu">
        <a href="/logout" class="nav-link"><i class="fas fa-sign-out-alt"></i></a>
      </div>
    </div>
    
    <div class="dashboard-grid">
      <div class="card" style="border-top: 4px solid var(--danger);">
        <h3><i class="fas fa-shield-alt"></i> Security Status</h3>
        
        <div class="sensor-status">
          <div class="sensor-label">Sound Detection</div>
          <div class="sensor-value"><span class="status-indicator %SOUND_CLASS%"></span> %SOUND%</div>
          <div class="sensor-time">Last updated: just now</div>
        </div>
        
        <button class="btn %BTN_CLASS%" onclick="window.location.href='/mute'">
          <i class="fas %BTN_ICON%"></i> %BTN_TEXT%
        </button>
      </div>
      
      <div class="card" style="border-top: 4px solid var(--success);">
        <h3><i class="fas fa-thermometer-half"></i> Environmental Data</h3>
        
        <div class="sensor-status">
          <div class="sensor-label">Temperature</div>
          <div class="sensor-value">%TEMPC% °C</div>
          <div class="sensor-time">Updated: just now</div>
        </div>
        
        <div class="sensor-status">
          <div class="sensor-label">Light Level</div>
          <div class="sensor-value">%LIGHT%</div>
          <div class="sensor-time">Updated: just now</div>
        </div>
      </div>
      
      <div class="card" style="border-top: 4px solid var(--info);">
        <h3><i class="fas fa-bolt"></i> Quick Actions</h3>
        
        <div class="quick-actions">
          <a href="/settings" class="quick-action">
            <i class="fas fa-cog"></i>
            <div class="quick-action-label">Settings</div>
          </a>
          <a href="/profile" class="quick-action">
            <i class="fas fa-user"></i>
            <div class="quick-action-label">Profile</div>
          </a>
          <a href="/help" class="quick-action">
            <i class="fas fa-question-circle"></i>
            <div class="quick-action-label">Help</div>
          </a>
          <a href="/logout" class="quick-action">
            <i class="fas fa-sign-out-alt"></i>
            <div class="quick-action-label">Logout</div>
          </a>
        </div>
      </div>
    </div>
    
    <footer>
      <div class="social-icons" style="justify-content: center; margin:15px 0;">
        <a href="https://github.com/Fthnath" title="GitHub"><i class="fab fa-github"></i></a>
        <a href="https://www.linkedin.com/in/emmanuel-olara-6aa993374/" title="LinkedIn"><i class="fab fa-linkedin"></i></a>
        <a href="https://x.com/eolara68" title="Twitter"><i class="fab fa-twitter"></i></a>
      </div>
      <p>SecureGuard-Pro v2.4.1 | © 2023 All rights reserved</p>
    </footer>
  </div>
  
  <script>
    // Update time display
    function updateTime() {
      const now = new Date();
      document.getElementById('timeDisplay').textContent = now.toLocaleString();
    }
    setInterval(updateTime, 1000);
    updateTime();
  </script>
</body>
</html>
)rawliteral";

// === Settings Page ===
const char settingsPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="%LANG%">
<head>
  <title>SecureGuard-Pro | Settings</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@400;700&family=Roboto:wght@300;400;500&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
  <style>
    %COMMON_STYLES%
    
    .settings-tabs {
      display: flex;
      border-bottom: 1px solid #eee;
      margin-bottom: 25px;
    }
    
    .tab {
      padding: 12px 20px;
      cursor: pointer;
      font-weight: 500;
      border-bottom: 3px solid transparent;
      transition: all 0.3s;
    }
    
    .tab.active {
      border-bottom-color: var(--secondary);
      color: var(--secondary);
    }
    
    .tab:hover:not(.active) {
      background-color: #f9f9f9;
    }
    
    .tab-content {
      display: none;
    }
    
    .tab-content.active {
      display: block;
    }
    
    .form-group {
      margin-bottom: 20px;
    }
    
    label {
      display: block;
      margin-bottom: 8px;
      font-weight: 500;
    }
    
    .checkbox-group {
      margin-bottom: 15px;
    }
    
    .checkbox-label {
      display: flex;
      align-items: center;
      cursor: pointer;
    }
    
    .checkbox-label input {
      width: auto;
      margin-right: 10px;
    }
    
    .range-slider {
      width: 100%;
      margin: 15px 0;
    }
    
    .range-value {
      text-align: center;
      font-weight: 500;
      margin-top: 5px;
    }
    
    .system-status {
      display: flex;
      align-items: center;
      margin-bottom: 20px;
    }
    
    .progress-bar {
      height: 6px;
      background-color: #e9ecef;
      border-radius: 3px;
      margin-top: 8px;
      overflow: hidden;
    }
    
    .progress {
      height: 100%;
      background-color: var(--secondary);
      width: 0%;
      transition: width 0.3s;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="logo-container">
        <img src="https://i.imgur.com/JQZ1wzP.png" alt="Logo" style="width:40px;height:40px;">
        <span style="font-family: 'Montserrat', sans-serif; font-weight:700;">SecureGuard-Pro</span>
      </div>
      <nav>
        <a href="/dashboard" class="nav-link"><i class="fas fa-home"></i> Dashboard</a>
        <a href="/profile" class="nav-link"><i class="fas fa-user"></i> Profile</a>
        <a href="/settings" class="nav-link"><i class="fas fa-cog"></i> Settings</a>
      </nav>
      <div class="user-menu">
        <a href="/logout" class="nav-link"><i class="fas fa-sign-out-alt"></i></a>
      </div>
    </div>

    <div class="card">
      <h1 style="color:var(--primary); font-family:'Montserrat'; margin-bottom:20px;">
        <i class="fas fa-cog"></i> System Settings
      </h1>
      
      <div class="settings-tabs">
        <div class="tab active" data-tab="general">General</div>
        <div class="tab" data-tab="security">Security</div>
        <div class="tab" data-tab="network">Network</div>
      </div>
      
      <div id="general" class="tab-content active">
        <h2><i class="fas fa-wrench"></i> General Settings</h2>
        
        <div class="form-group">
          <label for="theme">Theme</label>
          <select id="theme">
            <option>Default</option>
            <option>Dark Mode</option>
            <option>Light Mode</option>
          </select>
        </div>
        
        <div class="form-group">
          <label for="language">Language</label>
          <select id="language">
            <option value="en" %EN_SELECTED%>English</option>
            <option value="es" %ES_SELECTED%>Español</option>
            <option value="fr" %FR_SELECTED%>Français</option>
          </select>
        </div>
        
        <button class="btn" onclick="saveSettings()">
          <i class="fas fa-save"></i> Save Settings
        </button>
      </div>
      
      <div id="security" class="tab-content">
        <h2><i class="fas fa-shield-alt"></i> Security Settings</h2>
        
        <div class="form-group">
          <label>Alarm Volume</label>
          <input type="range" id="alarm-volume" min="0" max="100" value="80" class="range-slider">
          <div class="range-value" id="alarm-volume-value">80%</div>
        </div>
        
        <div class="checkbox-group">
          <label class="checkbox-label">
            <input type="checkbox" id="auto-alarm" checked> Enable automatic alarm
          </label>
        </div>
        
        <button class="btn" onclick="saveSecuritySettings()">
          <i class="fas fa-save"></i> Save Security Settings
        </button>
      </div>
      
      <div id="network" class="tab-content">
        <h2><i class="fas fa-network-wired"></i> Network Settings</h2>
        
        <div class="form-group">
          <label for="wifi-ssid">WiFi SSID</label>
          <input type="text" id="wifi-ssid" value="SecureGuard-Pro">
        </div>
        
        <div class="form-group">
          <label for="wifi-password">WiFi Password</label>
          <input type="password" id="wifi-password" value="secure@123">
        </div>
        
        <button class="btn" onclick="saveNetworkSettings()">
          <i class="fas fa-save"></i> Save Network Settings
        </button>
      </div>
    </div>
    
    <footer style="margin-top:30px;">
      <p>SecureGuard-Pro v2.4.1 | © 2023 All rights reserved</p>
    </footer>
  </div>
  
  <script>
    // Tab switching
    document.querySelectorAll('.tab').forEach(tab => {
      tab.addEventListener('click', function() {
        // Remove active class from all tabs and content
        document.querySelectorAll('.tab').forEach(t => t.classList.remove('active'));
        document.querySelectorAll('.tab-content').forEach(c => c.classList.remove('active'));
        
        // Add active class to clicked tab and corresponding content
        this.classList.add('active');
        const tabId = this.getAttribute('data-tab');
        document.getElementById(tabId).classList.add('active');
      });
    });
    
    // Range slider update
    document.getElementById('alarm-volume').addEventListener('input', function() {
      document.getElementById('alarm-volume-value').textContent = this.value + '%';
    });
    
    function saveSettings() {
      alert('Settings saved successfully!');
      // In a real app, this would send to the server
    }
    
    function saveSecuritySettings() {
      alert('Security settings saved!');
    }
    
    function saveNetworkSettings() {
      alert('Network settings saved!');
    }
  </script>
</body>
</html>
)rawliteral";

// === Help Page ===
const char helpPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="%LANG%">
<head>
  <title>SecureGuard-Pro | Help Center</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@400;700&family=Roboto:wght@300;400;500&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
  <style>
    %COMMON_STYLES%
    
    .faq-item {
      margin-bottom: 25px;
      border-bottom: 1px solid #eee;
      padding-bottom: 20px;
    }
    
    .faq-question {
      font-weight: 600;
      color: var(--primary);
      cursor: pointer;
      display: flex;
      justify-content: space-between;
      align-items: center;
    }
    
    .faq-question:hover {
      color: var(--secondary);
    }
    
    .faq-answer {
      margin-top: 10px;
      padding-left: 20px;
      border-left: 3px solid var(--secondary);
      display: none;
    }
    
    .faq-answer.show {
      display: block;
    }
    
    .contact-methods {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
      gap: 20px;
      margin-top: 30px;
    }
    
    .contact-card {
      background-color: #f9f9f9;
      padding: 20px;
      border-radius: 8px;
      text-align: center;
      transition: all 0.3s;
      border-top: 4px solid var(--secondary);
    }
    
    .contact-card:hover {
      transform: translateY(-5px);
      box-shadow: 0 6px 12px rgba(0,0,0,0.1);
    }
    
    .contact-icon {
      width: 60px;
      height: 60px;
      background-color: var(--secondary);
      color: white;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      margin: 0 auto 15px;
      font-size: 24px;
    }
    
    .contact-card h3 {
      margin-bottom: 10px;
      color: var(--primary);
    }
    
    .form-response {
      margin-top: 20px;
      padding: 15px;
      border-radius: 6px;
      display: none;
    }
    
    .form-response.success {
      background-color: #d4edda;
      color: #155724;
      display: block;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="logo-container">
        <img src="https://i.imgur.com/JQZ1wzP.png" alt="Logo" style="width:40px;height:40px;">
        <span style="font-family: 'Montserrat', sans-serif; font-weight:700;">SecureGuard-Pro</span>
      </div>
      <nav>
        <a href="/dashboard" class="nav-link"><i class="fas fa-home"></i> Dashboard</a>
        <a href="/profile" class="nav-link"><i class="fas fa-user"></i> Profile</a>
        <a href="/help" class="nav-link"><i class="fas fa-question-circle"></i> Help</a>
      </nav>
      <div class="user-menu">
        <a href="/logout" class="nav-link"><i class="fas fa-sign-out-alt"></i></a>
      </div>
    </div>

    <div class="card">
      <h1 style="color:var(--primary); font-family:'Montserrat'; margin-bottom:20px;">
        <i class="fas fa-question-circle"></i> Help Center
      </h1>
      
      <h2>Frequently Asked Questions</h2>
      
      <div class="faq-item">
        <div class="faq-question" onclick="toggleAnswer(this)">
          <span>How do I reset my password?</span>
          <i class="fas fa-chevron-down"></i>
        </div>
        <div class="faq-answer">
          <p>You can reset your password from the login page by clicking "Forgot password" link.</p>
        </div>
      </div>
      
      <div class="faq-item">
        <div class="faq-question" onclick="toggleAnswer(this)">
          <span>How to adjust alarm sensitivity?</span>
          <i class="fas fa-chevron-down"></i>
        </div>
        <div class="faq-answer">
          <p>Go to Settings > Security and adjust the alarm volume slider.</p>
        </div>
      </div>
      
      <h2 style="margin-top:40px;">Contact Support</h2>
      
      <div class="contact-methods">
        <div class="contact-card">
          <div class="contact-icon">
            <i class="fas fa-envelope"></i>
          </div>
          <h3>Email Support</h3>
          <p>support@secureguardpro.com</p>
          <p>Response within 24 hours</p>
        </div>
        
        <div class="contact-card">
          <div class="contact-icon">
            <i class="fas fa-phone"></i>
          </div>
          <h3>Phone Support</h3>
          <p>+256 770 680938</p>
          <p>8AM - 6PM EAT</p>
        </div>
      </div>
      
      <div class="contact-form" style="margin-top:30px;">
        <h3><i class="fas fa-paper-plane"></i> Send us a message</h3>
        <form id="supportForm">
          <div class="form-group">
            <input type="text" placeholder="Your Name" required>
          </div>
          <div class="form-group">
            <input type="email" placeholder="Your Email" required>
          </div>
          <div class="form-group">
            <textarea placeholder="Your Message" rows="5" required></textarea>
          </div>
          <button type="submit" class="btn">
            <i class="fas fa-paper-plane"></i> Send Message
          </button>
        </form>
        <div id="formResponse" class="form-response"></div>
      </div>
    </div>
    
    <footer style="margin-top:30px;">
      <p>SecureGuard-Pro v2.4.1 | © 2023 All rights reserved</p>
    </footer>
  </div>
  
  <script>
    // FAQ toggle functionality
    function toggleAnswer(element) {
      const answer = element.nextElementSibling;
      const icon = element.querySelector('i');
      
      answer.classList.toggle('show');
      
      if (answer.classList.contains('show')) {
        icon.classList.remove('fa-chevron-down');
        icon.classList.add('fa-chevron-up');
      } else {
        icon.classList.remove('fa-chevron-up');
        icon.classList.add('fa-chevron-down');
      }
    }
    
    // Form submission
    document.getElementById('supportForm').addEventListener('submit', function(e) {
      e.preventDefault();
      const response = document.getElementById('formResponse');
      response.className = 'form-response success';
      response.innerHTML = '<p><i class="fas fa-check-circle"></i> Thank you for your message! We will respond soon.</p>';
      this.reset();
    });
  </script>
</body>
</html>
)rawliteral";

// === Profile Page ===
const char profilePage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="%LANG%">
<head>
  <title>SecureGuard-Pro | User Profile</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link href="https://fonts.googleapis.com/css2?family=Montserrat:wght@400;700&family=Roboto:wght@300;400;500&display=swap" rel="stylesheet">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
  <style>
    %COMMON_STYLES%
    
    .profile-header {
      display: flex;
      align-items: center;
      margin-bottom: 30px;
    }
    
    .profile-pic {
      width: 100px;
      height: 100px;
      border-radius: 50%;
      object-fit: cover;
      border: 4px solid var(--secondary);
      margin-right: 20px;
    }
    
    .profile-info h2 {
      color: var(--primary);
      margin-bottom: 5px;
      font-family: 'Montserrat', sans-serif;
    }
    
    .profile-info p {
      color: #7f8c8d;
    }
    
    .badge {
      display: inline-block;
      padding: 3px 8px;
      border-radius: 12px;
      font-size: 12px;
      font-weight: 600;
      background-color: #d4edda;
      color: #155724;
    }
    
    .info-item {
      margin-bottom: 15px;
      padding-bottom: 15px;
      border-bottom: 1px solid #eee;
    }
    
    .info-label {
      font-weight: 500;
      color: #7f8c8d;
      margin-bottom: 5px;
    }
    
    .info-value {
      font-size: 16px;
    }
    
    .social-links {
      margin-top: 30px;
    }
    
    .social-icons {
      display: flex;
      gap: 15px;
      margin-top: 15px;
    }
    
    .social-icons a {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background-color: #f8f9fa;
      color: var(--primary);
      display: flex;
      align-items: center;
      justify-content: center;
      transition: all 0.3s;
    }
    
    .social-icons a:hover {
      background-color: var(--secondary);
      color: white;
    }
  </style>
</head>
<body>
  <div class="container">
    <div class="header">
      <div class="logo-container">
        <img src="https://i.imgur.com/JQZ1wzP.png" alt="Logo" style="width:40px;height:40px;">
        <span style="font-family: 'Montserrat', sans-serif; font-weight:700;">SecureGuard-Pro</span>
      </div>
      <nav>
        <a href="/dashboard" class="nav-link"><i class="fas fa-home"></i> Dashboard</a>
        <a href="/profile" class="nav-link"><i class="fas fa-user"></i> Profile</a>
        <a href="/settings" class="nav-link"><i class="fas fa-cog"></i> Settings</a>
      </nav>
      <div class="user-menu">
        <a href="/logout" class="nav-link"><i class="fas fa-sign-out-alt"></i></a>
      </div>
    </div>

    <div class="card">
      <div class="profile-header">
        <img src="https://i.imgur.com/JQZ1wzP.png" class="profile-pic">
        <div class="profile-info">
          <h2>Emma Olara</h2>
          <p>Administrator <span class="badge">Verified</span></p>
        </div>
      </div>
      
      <div class="info-item">
        <div class="info-label">Username</div>
        <div class="info-value">Emma</div>
      </div>
      
      <div class="info-item">
        <div class="info-label">Email</div>
        <div class="info-value">eolara68@gmail.com</div>
      </div>
      
      <div class="info-item">
        <div class="info-label">Phone</div>
        <div class="info-value">+256 770 680938</div>
      </div>
      
      <div class="info-item">
        <div class="info-label">Last Login</div>
        <div class="info-value">Today, 14:30</div>
      </div>
      
      <h3 style="margin-top:30px;">Update Profile</h3>
      
      <form>
        <div class="form-group">
          <label>Full Name</label>
          <input type="text" value="Emma Olara">
        </div>
        
        <div class="form-group">
          <label>Email Address</label>
          <input type="email" value="eolara68@gmail.com">
        </div>
        
        <button class="btn" type="button" onclick="updateProfile()">
          <i class="fas fa-save"></i> Save Changes
        </button>
      </form>
      
      <div class="social-links">
        <h3>Connect With Me</h3>
        <div class="social-icons">
          <a href="https://github.com/Fthnath" title="GitHub"><i class="fab fa-github"></i></a>
          <a href="https://www.linkedin.com/in/emmanuel-olara-6aa993374/" title="LinkedIn"><i class="fab fa-linkedin"></i></a>
          <a href="https://x.com/eolara68" title="Twitter"><i class="fab fa-twitter"></i></a>
        </div>
      </div>
    </div>
    
    <footer style="margin-top:30px;">
      <p>SecureGuard-Pro v2.4.1 | © 2023 All rights reserved</p>
    </footer>
  </div>
  
  <script>
    function updateProfile() {
      alert('Profile updated successfully!');
      // In a real app, this would send to the server
    }
  </script>
</body>
</html>
)rawliteral";

void handleDashboard(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", dashboardPage);
}

void handleSettings(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", settingsPage);
}

void handleHelp(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", helpPage);
}

void handleProfile(AsyncWebServerRequest *request) {
  request->send_P(200, "text/html", profilePage);
}

// === Enhanced Processor Function ===
String processor(const String &var)
{
    if (var == "TEMPC") {
        float temp = analogRead(TEMP_SENSOR_PIN) * 0.322265625; // Convert ADC reading to °C (LM36DZ)
        return String(temp);
    }
    if (var == "LIGHT") {
        int light = analogRead(LDR_PIN);
        return String(map(light, 0, 4095, 0, 100)); // Convert to percentage
    }
    if (var == "SOUND")
        return soundDetected ? "Detected" : "None";
    if (var == "SOUND_SENSOR")
        return digitalRead(SOUND_SENSOR_PIN) ? "Active" : "Inactive";
    if (var == "LIGHT_SENSOR") {
        int light = analogRead(LDR_PIN);
        return String(light);
    }
    if (var == "BUZZER_STATE")
        return buzzerMuted ? "MUTED" : "ACTIVE";
    if (var == "SOUND_CLASS")
        return soundDetected ? "danger" : "active";
    if (var == "BUZZER_CLASS")
        return buzzerMuted ? "warning" : "active";
    if (var == "BTN_TEXT")
        return buzzerMuted ? "UNMUTE ALARM" : "MUTE ALARM";
    if (var == "BTN_CLASS")
        return buzzerMuted ? "btn-danger" : "btn";
    if (var == "BTN_ICON")
        return buzzerMuted ? "fa-bell" : "fa-bell-slash";
    if (var == "LANG")
        return currentLanguage;

    // Handle language selection
    if (var == "EN_SELECTED")
        return currentLanguage == "en" ? "selected" : "";
    if (var == "ES_SELECTED")
        return currentLanguage == "es" ? "selected" : "";
    if (var == "FR_SELECTED")
        return currentLanguage == "fr" ? "selected" : "";
    if (var == "DE_SELECTED")
        return currentLanguage == "de" ? "selected" : "";
    if (var == "SW_SELECTED")
        return currentLanguage == "sw" ? "selected" : "";

    // Theme colors
    if (var == "PRIMARY_COLOR")
    {
        if (currentTheme == "green")
            return "#2ecc71";
        if (currentTheme == "purple")
            return "#9b59b6";
        if (currentTheme == "blue")
            return "#3498db";
        return "#2c3e50"; // default
    }
    if (var == "SECONDARY_COLOR")
    {
        if (currentTheme == "green")
            return "#27ae60";
        if (currentTheme == "purple")
            return "#8e44ad";
        if (currentTheme == "blue")
            return "#2980b9";
        return "#3498db"; // default
    }
    if (var == "ACCENT_COLOR")
    {
        if (currentTheme == "green")
            return "#e74c3c";
        if (currentTheme == "purple")
            return "#e91e63";
        if (currentTheme == "blue")
            return "#e67e22";
        return "#e74c3c"; // default
    }

    return String();
}

void setupWiFi()
{
    WiFi.softAP(ssid, password);
    Serial.print("Access Point IP: ");
    Serial.println(WiFi.softAPIP());
}

void displayDigit(byte digit, bool isFirst)
{
    byte segs = digits[digit];
    digitalWrite(SEG_A_PIN, segs & 0b00000001);
    digitalWrite(SEG_B_PIN, segs & 0b00000010);
    digitalWrite(SEG_C_PIN, segs & 0b00000100);
    digitalWrite(SEG_D_PIN, segs & 0b00001000);
    digitalWrite(SEG_E_PIN, segs & 0b00010000);
    digitalWrite(SEG_F_PIN, segs & 0b00100000);
    digitalWrite(SEG_G_PIN, segs & 0b01000000);
    digitalWrite(SEG_DP_PIN, LOW);
    digitalWrite(DIGIT1_PIN, isFirst ? LOW : HIGH);
    digitalWrite(DIGIT2_PIN, isFirst ? HIGH : LOW);
    delay(5);
}

void displayNumber(int number)
{
    number = constrain(number, 0, 99); // Ensure 2-digit number
    int tens = number / 10;
    int ones = number % 10;
    displayDigit(tens, true);
    displayDigit(ones, false);
}

void updateDisplay()
{
    if (millis() - lastDisplayUpdate < 2000)
        return; // Update every 2 seconds

    lastDisplayUpdate = millis();

    if (showTemperature)
    {
        float t = analogRead(TEMP_SENSOR_PIN) * 0.322265625; // LM36DZ conversion
        if (!isnan(t))
        {
            displayNumber(t);
        }
    }
    else
    {
        int l = analogRead(LDR_PIN);
        displayNumber(map(l, 0, 4095, 0, 100)); // Show light percentage
    }

    showTemperature = !showTemperature; // Toggle between temp and light
}

void setupWebServer()
{
    // Login page and authentication
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = loginPage;
        html.replace("%LANG%", currentLanguage);
        request->send(200, "text/html", html);
    });

    server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!request->hasParam("user", true) || !request->hasParam("pass", true)) {
            request->send(400, "text/plain", "Missing Credentials");
            return;
        }
        String user = request->getParam("user", true)->value();
        String pass = request->getParam("pass", true)->value();
        if (user == username && pass == userpass) {
            lastLoginTime = now();
            String html = dashboardPage;
            html.replace("%LANG%", currentLanguage);
            request->send(200, "text/html", html);
        } else {
            request->send(401, "text/plain", "Unauthorized");
        }
    });

    // Add these route handlers for all your pages
    server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = dashboardPage;
        html.replace("%LANG%", currentLanguage);
        request->send(200, "text/html", html);
    });

    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = settingsPage;
        html.replace("%LANG%", currentLanguage);
        request->send(200, "text/html", html);
    });

    server.on("/help", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = helpPage;
        html.replace("%LANG%", currentLanguage);
        request->send(200, "text/html", html);
    });

    server.on("/profile", HTTP_GET, [](AsyncWebServerRequest *request) {
        String html = profilePage;
        html.replace("%LANG%", currentLanguage);
        request->send(200, "text/html", html);
    });

    // Other existing handlers
    server.on("/setlang", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("lang")) {
            currentLanguage = request->getParam("lang")->value();
            request->send(200, "text/plain", "Language set to " + currentLanguage);
        } else {
            request->send(400, "text/plain", "Missing language parameter");
        }
    });

    server.on("/settheme", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (request->hasParam("theme")) {
            currentTheme = request->getParam("theme")->value();
            request->send(200, "text/plain", "Theme set to " + currentTheme);
        } else {
            request->send(400, "text/plain", "Missing theme parameter");
        }
    });

    server.on("/mute", HTTP_GET, [](AsyncWebServerRequest *request) {
        buzzerMuted = !buzzerMuted;
        request->send(200, "text/plain", buzzerMuted ? "Alarm muted" : "Alarm active");
    });

    server.begin();
}

void setup()
{
    Serial.begin(115200);

    // Initialize sensors
    pinMode(SOUND_SENSOR_PIN, INPUT);
    pinMode(TEMP_SENSOR_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(RGB_R_PIN, OUTPUT);
    pinMode(RGB_G_PIN, OUTPUT);
    pinMode(RGB_B_PIN, OUTPUT);

    // Initialize 7-segment display
    int segmentPins[] = {SEG_A_PIN, SEG_B_PIN, SEG_C_PIN, SEG_D_PIN,
                         SEG_E_PIN, SEG_F_PIN, SEG_G_PIN, SEG_DP_PIN,
                         DIGIT1_PIN, DIGIT2_PIN};
    for (int pin : segmentPins)
    {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
    }

    setTime(8, 0, 0, 25, 7, 2024); // Set initial time

    setupWiFi();
    setupWebServer();
}

void loop()
{
    // Update temperature display
    float temp = analogRead(TEMP_SENSOR_PIN) * 0.322265625; // LM36DZ conversion
    if (!isnan(temp))
    {
        int t = int(temp);
        for (int i = 0; i < 50; i++)
            displayNumber(t);
    }

    // Check sensors and control buzzer
    bool soundDetectedNow = digitalRead(SOUND_SENSOR_PIN);
    int lightLevel = analogRead(LDR_PIN);

    soundDetected = soundDetectedNow;

    if (soundDetected && !buzzerMuted)
    {
        digitalWrite(BUZZER_PIN, HIGH);
        lastSoundTime = millis();
    }
    else
    {
        digitalWrite(BUZZER_PIN, LOW);
    }

    // Control RGB based on light level
    int brightness = map(lightLevel, 0, 4095, 0, 255);
    analogWrite(RGB_R_PIN, brightness);
    analogWrite(RGB_G_PIN, brightness);
    analogWrite(RGB_B_PIN, brightness);

    updateDisplay();
    delay(10); // Small delay to prevent watchdog timer issues
}