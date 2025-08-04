#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Pins
#define RED_PIN 16
#define GREEN_PIN 17
#define BLUE_PIN 18
#define BUZZER_PIN 19
#define SOUND_SENSOR_PIN 34
#define LDR_PIN 35
#define TEMP_SENSOR_PIN 32

// System Configuration
struct Config {
  bool alarmEnabled = true;
  float tempWarning = 30.0;
  float tempCritical = 35.0;
  int soundThreshold = 2000;
  int lightThreshold = 2000;
  int updateInterval = 3;
  String deviceName = "SecureGuard Pro";
  String location = "Living Room";
  String adminEmail = "eolara68@gmail.com";
  String adminPhone = "256770680938";
};

Config config;
AsyncWebServer server(80);

float readTemperature() {
  int rawValue = analogRead(TEMP_SENSOR_PIN);
  float voltage = (rawValue / 4095.0) * 3.3;
  return voltage * 100;
}

const char *navigation = R"rawliteral(
<ul class="nav-menu">
    <li class="nav-item">
        <a href="/dashboard" class="nav-link %DASHBOARD_ACTIVE%">
            <i class="fas fa-tachometer-alt"></i>
            <span>Dashboard</span>
        </a>
    </li>
    <li class="nav-item">
        <a href="/help" class="nav-link %HELP_ACTIVE%">
            <i class="fas fa-question-circle"></i>
            <span>Help</span>
        </a>
    </li>
    <li class="nav-item">
        <a href="/settings" class="nav-link %SETTINGS_ACTIVE%">
            <i class="fas fa-cog"></i>
            <span>Settings</span>
        </a>
    </li>
    <li class="nav-item">
        <a href="/alerts" class="nav-link %ALERTS_ACTIVE%">
            <i class="fas fa-bell"></i>
            <span>Alerts</span>
        </a>
    </li>
    <li class="nav-item">
        <a href="/logout" class="nav-link">
            <i class="fas fa-sign-out-alt"></i>
            <span>Logout</span>
        </a>
    </li>
</ul>
)rawliteral";

const char *loginPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SecureGuard-Pro Login</title>
    <style>
        :root {
            --primary: #3498db;
            --primary-dark: #2980b9;
            --secondary: #2ecc71;
            --danger: #e74c3c;
            --warning: #f39c12;
            --dark: #2c3e50;
            --dark-light: #34495e;
            --light: #ecf0f1;
            --light-dark: #bdc3c7;
            --gray: #95a5a6;
            --white: #ffffff;
            --shadow: 0 4px 20px rgba(0, 0, 0, 0.15);
            --transition: all 0.3s ease;
            --border-radius: 12px;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Roboto', 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background: linear-gradient(135deg, #2c3e50 0%, #4ca1af 100%);
            color: var(--light);
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            padding: 1rem;
        }
        
        .login-container {
            background-color: rgba(255, 255, 255, 0.1);
            backdrop-filter: blur(10px);
            -webkit-backdrop-filter: blur(10px);
            border: 1px solid rgba(255, 255, 255, 0.1);
            padding: 2.5rem;
            border-radius: var(--border-radius);
            width: 100%;
            max-width: 420px;
            box-shadow: var(--shadow);
            text-align: center;
            transition: var(--transition);
            animation: fadeIn 0.5s ease-out;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; transform: translateY(20px); }
            to { opacity: 1; transform: translateY(0); }
        }
        
        .logo {
            width: 100px;
            height: 100px;
            margin: 0 auto 1.5rem;
            display: block;
            border-radius: 50%;
            object-fit: cover;
            border: 3px solid rgba(255, 255, 255, 0.1);
            padding: 0.5rem;
            background-color: rgba(255, 255, 255, 0.05);
        }
        
        h1 {
            margin-bottom: 1.5rem;
            color: var(--white);
            font-weight: 500;
            font-size: 1.8rem;
            letter-spacing: 0.5px;
        }
        
        .input-group {
            margin-bottom: 1.5rem;
            text-align: left;
            position: relative;
        }
        
        label {
            display: block;
            margin-bottom: 0.5rem;
            color: var(--light);
            font-weight: 400;
            font-size: 0.95rem;
        }
        
        .input-wrapper {
            position: relative;
        }
        
        input {
            width: 100%;
            padding: 0.9rem 1rem;
            border: none;
            border-radius: 8px;
            background-color: rgba(255, 255, 255, 0.15);
            color: var(--white);
            font-size: 1rem;
            transition: var(--transition);
            padding-right: 2.5rem;
        }
        
        input::placeholder {
            color: rgba(255, 255, 255, 0.6);
        }
        
        input:focus {
            outline: none;
            background-color: rgba(255, 255, 255, 0.25);
            box-shadow: 0 0 0 2px rgba(52, 152, 219, 0.5);
        }
        
        .toggle-password {
            position: absolute;
            right: 12px;
            top: 50%;
            transform: translateY(-50%);
            color: rgba(255, 255, 255, 0.7);
            cursor: pointer;
            transition: var(--transition);
        }
        
        .toggle-password:hover {
            color: var(--white);
        }
        
        .forgot-password {
            display: block;
            text-align: right;
            margin-top: 0.5rem;
            color: rgba(255, 255, 255, 0.7);
            font-size: 0.85rem;
            text-decoration: none;
            transition: var(--transition);
        }
        
        .forgot-password:hover {
            color: var(--primary);
            text-decoration: underline;
        }
        
        button {
            width: 100%;
            padding: 1rem;
            border: none;
            border-radius: 8px;
            background-color: var(--primary);
            color: var(--white);
            font-size: 1rem;
            font-weight: 500;
            cursor: pointer;
            transition: var(--transition);
            margin-top: 1rem;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }
        
        button:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(0, 0, 0, 0.2);
        }
        
        button:active {
            transform: translateY(0);
        }
        
        .error-message {
            color: var(--danger);
            margin: 1rem 0;
            padding: 0.75rem;
            background-color: rgba(231, 76, 60, 0.1);
            border-radius: 6px;
            display: none;
            font-size: 0.9rem;
            animation: shake 0.5s;
        }
        
        @keyframes shake {
            0%, 100% { transform: translateX(0); }
            20%, 60% { transform: translateX(-5px); }
            40%, 80% { transform: translateX(5px); }
        }
        
        .footer {
            margin-top: 2rem;
            font-size: 0.85rem;
            color: rgba(255, 255, 255, 0.6);
            line-height: 1.5;
        }
        
        /* Modal Styles */
        .modal {
            display: none;
            position: fixed;
            top: 0;
            left: 0;
            width: 100%;
            height: 100%;
            background-color: rgba(0, 0, 0, 0.7);
            z-index: 1000;
            justify-content: center;
            align-items: center;
            animation: fadeIn 0.3s;
        }
        
        .modal-content {
            background-color: var(--dark);
            padding: 2rem;
            border-radius: var(--border-radius);
            width: 90%;
            max-width: 400px;
            box-shadow: var(--shadow);
            position: relative;
        }
        
        .close-modal {
            position: absolute;
            top: 1rem;
            right: 1rem;
            font-size: 1.5rem;
            color: var(--light);
            cursor: pointer;
            transition: var(--transition);
        }
        
        .close-modal:hover {
            color: var(--danger);
        }
        
        .modal-title {
            margin-bottom: 1.5rem;
            color: var(--white);
        }
        
        .modal-message {
            margin-bottom: 1.5rem;
            color: var(--light);
        }
        
        .modal-input {
            width: 100%;
            padding: 0.9rem 1rem;
            margin-bottom: 1rem;
            border: none;
            border-radius: 6px;
            background-color: rgba(255, 255, 255, 0.1);
            color: var(--white);
        }
        
        .modal-button {
            background-color: var(--secondary);
        }
        
        .modal-button:hover {
            background-color: #27ae60;
        }
        
        /* Responsive Styles */
        @media (max-width: 480px) {
            .login-container {
                padding: 1.5rem;
            }
            
            h1 {
                font-size: 1.5rem;
            }
        }
    </style>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@300;400;500;700&display=swap" rel="stylesheet">
</head>
<body>
    <div class="login-container">
        <img src="https://via.placeholder.com/100x100?text=SG" alt="SecureGuard Logo" class="logo">
        <h1>SecureGuard-Pro Login</h1>
        
        <div class="input-group">
            <label for="username">Username</label>
            <div class="input-wrapper">
                <input type="text" id="username" placeholder="Enter your username">
            </div>
        </div>
        
        <div class="input-group">
            <label for="password">Password</label>
            <div class="input-wrapper">
                <input type="password" id="password" placeholder="Enter your password">
                <i class="fas fa-eye toggle-password" id="togglePassword"></i>
            </div>
            <a href="#" class="forgot-password" id="forgotPassword">Forgot password?</a>
        </div>
        
        <button id="loginButton">
            <i class="fas fa-sign-in-alt"></i> Login
        </button>
        
        <div class="error-message" id="errorMessage">
            <i class="fas fa-exclamation-circle"></i> Invalid username or password
        </div>
        
        <div class="footer">
            <p>SecureGuard-Pro Security System</p>
            <p>© 2025 All Rights Reserved</p>
        </div>
    </div>
    
    <!-- Forgot Password Modal -->
    <div class="modal" id="forgotPasswordModal">
        <div class="modal-content">
            <span class="close-modal" id="closeModal">&times;</span>
            <h2 class="modal-title">Reset Password</h2>
            <p class="modal-message">Enter your username to receive password reset instructions.</p>
            <input type="text" id="resetUsername" class="modal-input" placeholder="Your username">
            <button class="modal-button" id="submitReset">
                <i class="fas fa-paper-plane"></i> Submit Request
            </button>
        </div>
    </div>
    
    <script>
        // Toggle password visibility
        document.getElementById('togglePassword').addEventListener('click', function() {
            const password = document.getElementById('password');
            const type = password.getAttribute('type') === 'password' ? 'text' : 'password';
            password.setAttribute('type', type);
            this.classList.toggle('fa-eye');
            this.classList.toggle('fa-eye-slash');
        });
        
        // Login function
        document.getElementById('loginButton').addEventListener('click', function() {
            const username = document.getElementById('username').value;
            const password = document.getElementById('password').value;
            
            fetch('/login', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `username=${encodeURIComponent(username)}&password=${encodeURIComponent(password)}`
            })
            .then(response => {
                if(response.redirected) {
                    window.location.href = response.url;
                } else {
                    return response.text();
                }
            })
            .then(text => {
                if(text) {
                    const errorMessage = document.getElementById('errorMessage');
                    errorMessage.textContent = text;
                    errorMessage.style.display = 'block';
                    setTimeout(() => {
                        errorMessage.style.display = 'none';
                    }, 3000);
                }
            });
        });
        
        // Forgot password modal
        document.getElementById('forgotPassword').addEventListener('click', function(e) {
            e.preventDefault();
            document.getElementById('forgotPasswordModal').style.display = 'flex';
        });
        
        document.getElementById('closeModal').addEventListener('click', function() {
            document.getElementById('forgotPasswordModal').style.display = 'none';
        });
        
        // Submit password reset
        document.getElementById('submitReset').addEventListener('click', function() {
            const username = document.getElementById('resetUsername').value;
            
            fetch('/reset-password', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/x-www-form-urlencoded',
                },
                body: `username=${encodeURIComponent(username)}`
            })
            .then(response => response.text())
            .then(text => {
                alert(text);
                document.getElementById('forgotPasswordModal').style.display = 'none';
            });
        });
        
        // Allow login on Enter key press
        document.getElementById('password').addEventListener('keyup', function(event) {
            if (event.key === 'Enter') {
                document.getElementById('loginButton').click();
            }
        });
        
        // Close modal when clicking outside
        window.addEventListener('click', function(event) {
            if (event.target === document.getElementById('forgotPasswordModal')) {
                document.getElementById('forgotPasswordModal').style.display = 'none';
            }
        });
    </script>
</body>
</html>
)rawliteral";

const char* dashboardPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SecureGuard-Pro Dashboard</title>
    <style>
        :root {
            --primary: #3498db;
            --primary-dark: #2980b9;
            --secondary: #2ecc71;
            --secondary-dark: #27ae60;
            --danger: #e74c3c;
            --danger-dark: #c0392b;
            --warning: #f39c12;
            --warning-dark: #d35400;
            --dark: #2c3e50;
            --dark-light: #34495e;
            --light: #ecf0f1;
            --light-dark: #bdc3c7;
            --gray: #95a5a6;
            --gray-dark: #7f8c8d;
            --white: #ffffff;
            --shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            --shadow-hover: 0 10px 15px rgba(0, 0, 0, 0.15);
            --transition: all 0.3s ease;
            --border-radius: 10px;
            --border-radius-sm: 5px;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
        }
        
        body {
            background-color: #f8fafc;
            color: var(--dark);
            line-height: 1.6;
        }
        
        .container {
            display: grid;
            grid-template-columns: 280px 1fr;
            min-height: 100vh;
        }
        
        /* Sidebar Styles */
        .sidebar {
            background-color: var(--dark);
            color: var(--light);
            padding: 1.5rem;
            box-shadow: 2px 0 10px rgba(0,0,0,0.1);
            position: sticky;
            top: 0;
            height: 100vh;
        }
        
        .logo {
            display: flex;
            align-items: center;
            margin-bottom: 2.5rem;
            padding-bottom: 1.5rem;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        .logo img {
            width: 42px;
            height: 42px;
            margin-right: 12px;
            border-radius: 50%;
            object-fit: cover;
        }
        
        .logo h2 {
            font-size: 1.3rem;
            font-weight: 600;
            color: var(--white);
        }
        
        .nav-menu {
            list-style: none;
            margin-top: 1rem;
        }
        
        .nav-item {
            margin-bottom: 0.75rem;
        }
        
        .nav-link {
            display: flex;
            align-items: center;
            padding: 0.9rem 1.2rem;
            color: var(--light);
            text-decoration: none;
            border-radius: var(--border-radius-sm);
            transition: var(--transition);
            font-weight: 500;
        }
        
        .nav-link:hover {
            background-color: rgba(255,255,255,0.08);
            color: var(--white);
            transform: translateX(5px);
        }
        
        .nav-link.active {
            background-color: var(--primary);
            color: var(--white);
            box-shadow: 0 2px 5px rgba(0,0,0,0.2);
        }
        
        .nav-link i {
            margin-right: 12px;
            font-size: 1.25rem;
            width: 24px;
            text-align: center;
        }
        
        /* Main Content Styles */
        .main-content {
            padding: 2.5rem;
            overflow-y: auto;
            background-color: #f8fafc;
        }
        
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2.5rem;
            padding-bottom: 1.5rem;
            border-bottom: 1px solid #e2e8f0;
        }
        
        .page-title h1 {
            font-size: 2rem;
            font-weight: 600;
            color: var(--dark);
            margin-bottom: 0.5rem;
        }
        
        .page-title p {
            color: var(--gray-dark);
            font-size: 0.95rem;
        }
        
        .user-profile {
            display: flex;
            align-items: center;
            background-color: var(--white);
            padding: 0.5rem 1rem;
            border-radius: 50px;
            box-shadow: var(--shadow);
            transition: var(--transition);
        }
        
        .user-profile:hover {
            transform: translateY(-2px);
            box-shadow: var(--shadow-hover);
        }
        
        .user-profile img {
            width: 42px;
            height: 42px;
            border-radius: 50%;
            margin-right: 12px;
            object-fit: cover;
            border: 2px solid var(--primary);
        }
        
        .user-profile span {
            font-weight: 500;
            color: var(--dark);
        }
        
        /* Cards Grid */
        .cards-grid {
            display: grid;
            grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
            gap: 1.75rem;
            margin-bottom: 2.5rem;
        }
        
        .card {
            background-color: var(--white);
            border-radius: var(--border-radius);
            padding: 1.75rem;
            box-shadow: var(--shadow);
            transition: var(--transition);
            position: relative;
            overflow: hidden;
        }
        
        .card:hover {
            transform: translateY(-5px);
            box-shadow: var(--shadow-hover);
        }
        
        .card::before {
            content: '';
            position: absolute;
            top: 0;
            left: 0;
            width: 4px;
            height: 100%;
            background-color: var(--gray);
        }
        
        .card.temperature::before {
            background-color: var(--danger);
        }
        
        .card.sound::before {
            background-color: var(--warning);
        }
        
        .card.light::before {
            background-color: var(--secondary);
        }
        
        .card.alarm::before {
            background-color: var(--primary);
        }
        
        .card-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1.5rem;
        }
        
        .card-title {
            font-size: 1.15rem;
            font-weight: 600;
            color: var(--dark);
        }
        
        .card-icon {
            width: 44px;
            height: 44px;
            border-radius: 50%;
            display: flex;
            align-items: center;
            justify-content: center;
            color: var(--white);
            font-size: 1.25rem;
            box-shadow: 0 2px 5px rgba(0,0,0,0.1);
        }
        
        .card-icon.temperature {
            background-color: var(--danger);
        }
        
        .card-icon.sound {
            background-color: var(--warning);
        }
        
        .card-icon.light {
            background-color: var(--secondary);
        }
        
        .card-icon.alarm {
            background-color: var(--primary);
        }
        
        .card-value {
            font-size: 2.25rem;
            font-weight: 700;
            margin-bottom: 0.5rem;
            color: var(--dark);
            font-family: 'Roboto', sans-serif;
        }
        
        .card-unit {
            color: var(--gray-dark);
            font-size: 0.95rem;
            margin-bottom: 1rem;
            display: block;
        }
        
        .card-status {
            display: inline-block;
            padding: 0.4rem 1rem;
            border-radius: 50px;
            font-size: 0.85rem;
            font-weight: 600;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }
        
        .status-normal {
            background-color: rgba(46, 204, 113, 0.1);
            color: var(--secondary-dark);
        }
        
        .status-warning {
            background-color: rgba(243, 156, 18, 0.1);
            color: var(--warning-dark);
        }
        
        .status-danger {
            background-color: rgba(231, 76, 60, 0.1);
            color: var(--danger-dark);
        }
        
        /* Charts Section */
        .charts-section {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 1.75rem;
            margin-bottom: 2.5rem;
        }
        
        .chart-container {
            background-color: var(--white);
            border-radius: var(--border-radius);
            padding: 1.75rem;
            box-shadow: var(--shadow);
            transition: var(--transition);
        }
        
        .chart-container:hover {
            transform: translateY(-3px);
            box-shadow: var(--shadow-hover);
        }
        
        .chart-header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 1.5rem;
        }
        
        .chart-title {
            font-size: 1.15rem;
            font-weight: 600;
            color: var(--dark);
        }
        
        .chart-header select {
            padding: 0.5rem 1rem;
            border-radius: var(--border-radius-sm);
            border: 1px solid #e2e8f0;
            background-color: var(--white);
            color: var(--dark);
            font-size: 0.9rem;
            transition: var(--transition);
        }
        
        .chart-header select:focus {
            outline: none;
            border-color: var(--primary);
            box-shadow: 0 0 0 2px rgba(52, 152, 219, 0.2);
        }
        
        /* Controls Section */
        .controls-section {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(400px, 1fr));
            gap: 1.75rem;
            margin-bottom: 2rem;
        }
        
        .control-panel {
            background-color: var(--white);
            border-radius: var(--border-radius);
            padding: 1.75rem;
            box-shadow: var(--shadow);
            transition: var(--transition);
        }
        
        .control-panel:hover {
            transform: translateY(-3px);
            box-shadow: var(--shadow-hover);
        }
        
        .control-title {
            font-size: 1.15rem;
            font-weight: 600;
            color: var(--dark);
            margin-bottom: 1.5rem;
            padding-bottom: 0.75rem;
            border-bottom: 1px solid #e2e8f0;
        }
        
        .control-group {
            margin-bottom: 1.5rem;
        }
        
        .control-label {
            display: block;
            margin-bottom: 0.75rem;
            font-weight: 500;
            color: var(--dark-light);
            font-size: 0.95rem;
        }
        
        .slider-container {
            display: flex;
            align-items: center;
            gap: 1rem;
        }
        
        .slider {
            flex-grow: 1;
            -webkit-appearance: none;
            height: 8px;
            border-radius: 4px;
            background: #e2e8f0;
            outline: none;
            transition: var(--transition);
        }
        
        .slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 20px;
            height: 20px;
            border-radius: 50%;
            background: var(--primary);
            cursor: pointer;
            transition: var(--transition);
        }
        
        .slider::-webkit-slider-thumb:hover {
            transform: scale(1.1);
            box-shadow: 0 0 0 4px rgba(52, 152, 219, 0.2);
        }
        
        .slider-value {
            width: 60px;
            text-align: center;
            padding: 0.4rem 0.6rem;
            background-color: #f1f5f9;
            border-radius: var(--border-radius-sm);
            font-weight: 600;
            font-size: 0.9rem;
            color: var(--dark);
        }
        
        .toggle-container {
            display: flex;
            align-items: center;
            gap: 1rem;
        }
        
        .toggle-switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        
        .toggle-switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .toggle-slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #e2e8f0;
            transition: var(--transition);
            border-radius: 34px;
        }
        
        .toggle-slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: var(--white);
            transition: var(--transition);
            border-radius: 50%;
        }
        
        input:checked + .toggle-slider {
            background-color: var(--primary);
        }
        
        input:checked + .toggle-slider:before {
            transform: translateX(26px);
        }
        
        .toggle-label {
            font-weight: 500;
            color: var(--dark-light);
            font-size: 0.95rem;
        }
        
        /* Button Styles */
        .btn {
            padding: 0.8rem 1.5rem;
            border: none;
            border-radius: var(--border-radius-sm);
            background-color: var(--primary);
            color: var(--white);
            font-weight: 500;
            cursor: pointer;
            transition: var(--transition);
            font-size: 0.95rem;
            display: inline-flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
        }
        
        .btn:hover {
            background-color: var(--primary-dark);
            transform: translateY(-2px);
            box-shadow: 0 4px 8px rgba(0,0,0,0.1);
        }
        
        .btn:active {
            transform: translateY(0);
        }
        
        .btn-secondary {
            background-color: var(--gray);
        }
        
        .btn-secondary:hover {
            background-color: var(--gray-dark);
        }
        
        .btn-danger {
            background-color: var(--danger);
        }
        
        .btn-danger:hover {
            background-color: var(--danger-dark);
        }
        
        /* Responsive Styles */
        @media (max-width: 1200px) {
            .container {
                grid-template-columns: 240px 1fr;
            }
            
            .main-content {
                padding: 1.5rem;
            }
        }
        
        @media (max-width: 992px) {
            .container {
                grid-template-columns: 1fr;
            }
            
            .sidebar {
                display: none;
            }
            
            .charts-section, .controls-section {
                grid-template-columns: 1fr;
            }
        }
        
        @media (max-width: 768px) {
            .header {
                flex-direction: column;
                align-items: flex-start;
                gap: 1rem;
            }
            
            .user-profile {
                align-self: flex-end;
            }
            
            .cards-grid {
                grid-template-columns: 1fr;
            }
        }
    </style>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
    <link href="https://fonts.googleapis.com/css2?family=Roboto:wght@400;500;700&display=swap" rel="stylesheet">
</head>
<body>
    <div class="container">
        <!-- Sidebar -->
        <div class="sidebar">
            <div class="logo">
                <img src="https://via.placeholder.com/42x42?text=SG" alt="SecureGuard Logo">
                <h2>SecureGuard-Pro</h2>
            </div>
            
            %NAVIGATION%
            
        </div>
        
        <!-- Main Content -->
        <div class="main-content">
            <div class="header">
                <div class="page-title">
                    <h1>System Dashboard</h1>
                    <p>Monitor and control your SecureGuard-Pro system</p>
                </div>
                
                <div class="user-profile">
                    <img src="https://via.placeholder.com/42x42?text=EM" alt="User Profile">
                    <span>Emma</span>
                </div>
            </div>
            
            <!-- Sensor Cards -->
            <div class="cards-grid">
                <div class="card temperature">
                    <div class="card-header">
                        <h3 class="card-title">Temperature</h3>
                        <div class="card-icon temperature">
                            <i class="fas fa-thermometer-half"></i>
                        </div>
                    </div>
                    <div class="card-value" id="temperature-value">--</div>
                    <div class="card-unit">°C</div>
                    <span class="card-status status-normal" id="temperature-status">Normal</span>
                </div>
                
                <div class="card sound">
                    <div class="card-header">
                        <h3 class="card-title">Sound Level</h3>
                        <div class="card-icon sound">
                            <i class="fas fa-volume-up"></i>
                        </div>
                    </div>
                    <div class="card-value" id="sound-value">--</div>
                    <div class="card-unit">dB</div>
                    <span class="card-status status-normal" id="sound-status">Normal</span>
                </div>
                
                <div class="card light">
                    <div class="card-header">
                        <h3 class="card-title">Light Level</h3>
                        <div class="card-icon light">
                            <i class="fas fa-lightbulb"></i>
                        </div>
                    </div>
                    <div class="card-value" id="light-value">--</div>
                    <div class="card-unit">Lux</div>
                    <span class="card-status status-normal" id="light-status">Normal</span>
                </div>
                
                <div class="card alarm">
                    <div class="card-header">
                        <h3 class="card-title">Alarm Status</h3>
                        <div class="card-icon alarm">
                            <i class="fas fa-shield-alt"></i>
                        </div>
                    </div>
                    <div class="card-value" id="alarm-status">Armed</div>
                    <button class="btn btn-danger" id="toggle-alarm" style="margin-top: 1rem;">
                        <i class="fas fa-power-off"></i> Disarm
                    </button>
                </div>
            </div>
            
            <!-- Charts Section -->
            <div class="charts-section">
                <div class="chart-container">
                    <div class="chart-header">
                        <h3 class="chart-title">Temperature Trend</h3>
                        <select id="time-range">
                            <option value="1h">Last Hour</option>
                            <option value="6h">Last 6 Hours</option>
                            <option value="24h">Last 24 Hours</option>
                        </select>
                    </div>
                    <canvas id="temp-chart" height="200"></canvas>
                </div>
                
                <div class="chart-container">
                    <div class="chart-header">
                        <h3 class="chart-title">Sensor Activity</h3>
                    </div>
                    <canvas id="sensor-chart" height="200"></canvas>
                </div>
            </div>
            
            <!-- Controls Section -->
            <div class="controls-section">
                <div class="control-panel">
                    <h3 class="control-title">Security Settings</h3>
                    
                    <div class="control-group">
                        <label class="control-label">Alarm System</label>
                        <div class="toggle-container">
                            <label class="toggle-switch">
                                <input type="checkbox" id="alarm-toggle" checked>
                                <span class="toggle-slider"></span>
                            </label>
                            <span class="toggle-label" id="alarm-label">Armed</span>
                        </div>
                    </div>
                    
                    <div class="control-group">
                        <label class="control-label">Sound Threshold</label>
                        <div class="slider-container">
                            <input type="range" min="0" max="4095" value="2000" class="slider" id="sound-threshold">
                            <span class="slider-value" id="sound-threshold-value">2000</span>
                        </div>
                    </div>
                    
                    <button class="btn" id="test-buzzer">
                        <i class="fas fa-bell"></i> Test Buzzer
                    </button>
                </div>
                
                <div class="control-panel">
                    <h3 class="control-title">Environment Settings</h3>
                    
                    <div class="control-group">
                        <label class="control-label">Temperature Warning</label>
                        <div class="slider-container">
                            <input type="range" min="0" max="50" step="0.5" value="30" class="slider" id="temp-warning">
                            <span class="slider-value" id="temp-warning-value">30°C</span>
                        </div>
                    </div>
                    
                    <div class="control-group">
                        <label class="control-label">Temperature Critical</label>
                        <div class="slider-container">
                            <input type="range" min="0" max="50" step="0.5" value="35" class="slider" id="temp-critical">
                            <span class="slider-value" id="temp-critical-value">35°C</span>
                        </div>
                    </div>
                    
                    <div class="control-group">
                        <label class="control-label">Light Threshold</label>
                        <div class="slider-container">
                            <input type="range" min="0" max="4095" value="2000" class="slider" id="light-threshold">
                            <span class="slider-value" id="light-threshold-value">2000</span>
                        </div>
                    </div>
                    
                    <button class="btn" id="save-settings">
                        <i class="fas fa-save"></i> Save Settings
                    </button>
                </div>
            </div>
        </div>
    </div>
    
    <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
    <script>
        function confirmLogout(event) {
        event.preventDefault();
        window.location.href = '/logout';
    }
        // Global variables
        let tempChart, sensorChart;
        let sensorData = {
            temperature: [],
            sound: [],
            light: [],
            timestamps: []
        };
        
        // DOM Elements
        const temperatureValue = document.getElementById('temperature-value');
        const soundValue = document.getElementById('sound-value');
        const lightValue = document.getElementById('light-value');
        const temperatureStatus = document.getElementById('temperature-status');
        const soundStatus = document.getElementById('sound-status');
        const lightStatus = document.getElementById('light-status');
        const alarmStatus = document.getElementById('alarm-status');
        const toggleAlarmBtn = document.getElementById('toggle-alarm');
        const alarmToggle = document.getElementById('alarm-toggle');
        const alarmLabel = document.getElementById('alarm-label');
        const testBuzzerBtn = document.getElementById('test-buzzer');
        const soundThreshold = document.getElementById('sound-threshold');
        const soundThresholdValue = document.getElementById('sound-threshold-value');
        const tempWarning = document.getElementById('temp-warning');
        const tempWarningValue = document.getElementById('temp-warning-value');
        const tempCritical = document.getElementById('temp-critical');
        const tempCriticalValue = document.getElementById('temp-critical-value');
        const lightThreshold = document.getElementById('light-threshold');
        const lightThresholdValue = document.getElementById('light-threshold-value');
        const saveSettingsBtn = document.getElementById('save-settings');
        
        // Initialize charts
        function initCharts() {
            const tempCtx = document.getElementById('temp-chart').getContext('2d');
            const sensorCtx = document.getElementById('sensor-chart').getContext('2d');
            
            tempChart = new Chart(tempCtx, {
                type: 'line',
                data: {
                    labels: [],
                    datasets: [{
                        label: 'Temperature (°C)',
                        data: [],
                        borderColor: '#e74c3c',
                        backgroundColor: 'rgba(231, 76, 60, 0.1)',
                        borderWidth: 2,
                        tension: 0.1,
                        fill: true,
                        pointBackgroundColor: '#fff',
                        pointBorderColor: '#e74c3c',
                        pointBorderWidth: 2,
                        pointRadius: 4,
                        pointHoverRadius: 6
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: {
                            display: false
                        }
                    },
                    scales: {
                        y: {
                            beginAtZero: false,
                            grid: {
                                color: 'rgba(0, 0, 0, 0.05)'
                            },
                            ticks: {
                                color: '#64748b'
                            }
                        },
                        x: {
                            grid: {
                                display: false
                            },
                            ticks: {
                                color: '#64748b'
                            }
                        }
                    },
                    interaction: {
                        intersect: false,
                        mode: 'index'
                    }
                }
            });
            
            sensorChart = new Chart(sensorCtx, {
                type: 'bar',
                data: {
                    labels: ['Sound', 'Light'],
                    datasets: [{
                        label: 'Sensor Values',
                        data: [0, 0],
                        backgroundColor: [
                            'rgba(243, 156, 18, 0.7)',
                            'rgba(46, 204, 113, 0.7)'
                        ],
                        borderColor: [
                            'rgba(243, 156, 18, 1)',
                            'rgba(46, 204, 113, 1)'
                        ],
                        borderWidth: 1,
                        borderRadius: 4
                    }]
                },
                options: {
                    responsive: true,
                    maintainAspectRatio: false,
                    plugins: {
                        legend: {
                            display: false
                        }
                    },
                    scales: {
                        y: {
                            beginAtZero: true,
                            max: 4095,
                            grid: {
                                color: 'rgba(0, 0, 0, 0.05)'
                            },
                            ticks: {
                                color: '#64748b'
                            }
                        },
                        x: {
                            grid: {
                                display: false
                            },
                            ticks: {
                                color: '#64748b'
                            }
                        }
                    }
                }
            });
        }
        
        // Update sensor data
        function updateSensorData() {
            fetch('/api/sensor')
                .then(response => response.json())
                .then(data => {
                    // Update card values
                    temperatureValue.textContent = data.temperature.toFixed(1);
                    soundValue.textContent = data.soundLevel;
                    lightValue.textContent = data.lightLevel;
                    
                    // Update status indicators
                    updateStatusIndicators(data);
                    
                    // Update charts
                    updateCharts(data);
                    
                    // Store data for history
                    storeSensorData(data);
                })
                .catch(error => console.error('Error fetching sensor data:', error));
        }
        
        // Update status indicators
        function updateStatusIndicators(data) {
            // Temperature status
            if (data.temperature >= data.tempCritical) {
                temperatureStatus.className = 'card-status status-danger';
                temperatureStatus.textContent = 'Critical';
            } else if (data.temperature >= data.tempWarning) {
                temperatureStatus.className = 'card-status status-warning';
                temperatureStatus.textContent = 'Warning';
            } else {
                temperatureStatus.className = 'card-status status-normal';
                temperatureStatus.textContent = 'Normal';
            }
            
            // Sound status
            if (data.soundLevel > data.soundThreshold) {
                soundStatus.className = 'card-status status-danger';
                soundStatus.textContent = 'Alert';
            } else {
                soundStatus.className = 'card-status status-normal';
                soundStatus.textContent = 'Normal';
            }
            
            // Light status
            if (data.lightLevel < data.lightThreshold) {
                lightStatus.className = 'card-status status-warning';
                lightStatus.textContent = 'Low';
            } else {
                lightStatus.className = 'card-status status-normal';
                lightStatus.textContent = 'Normal';
            }
            
            // Alarm status
            alarmStatus.textContent = data.alarmEnabled ? 'Armed' : 'Disarmed';
            alarmToggle.checked = data.alarmEnabled;
            alarmLabel.textContent = data.alarmEnabled ? 'Armed' : 'Disarmed';
            toggleAlarmBtn.textContent = data.alarmEnabled ? 'Disarm' : 'Arm';
            toggleAlarmBtn.className = data.alarmEnabled ? 'btn btn-danger' : 'btn';
            toggleAlarmBtn.innerHTML = data.alarmEnabled 
                ? '<i class="fas fa-power-off"></i> Disarm' 
                : '<i class="fas fa-power-off"></i> Arm';
        }
        
        // Update charts
        function updateCharts(data) {
            // Update sensor chart
            sensorChart.data.datasets[0].data = [data.soundLevel, data.lightLevel];
            sensorChart.update();
            
            // Update temperature chart (add new data point)
            const now = new Date();
            const timeLabel = now.getHours() + ':' + now.getMinutes().toString().padStart(2, '0');
            
            if (tempChart.data.labels.length > 20) {
                tempChart.data.labels.shift();
                tempChart.data.datasets[0].data.shift();
            }
            
            tempChart.data.labels.push(timeLabel);
            tempChart.data.datasets[0].data.push(data.temperature);
            tempChart.update();
        }
        
        // Store sensor data for history
        function storeSensorData(data) {
            const now = new Date();
            
            sensorData.temperature.push(data.temperature);
            sensorData.sound.push(data.soundLevel);
            sensorData.light.push(data.lightLevel);
            sensorData.timestamps.push(now.getTime());
            
            // Keep only the last 100 data points
            if (sensorData.temperature.length > 100) {
                sensorData.temperature.shift();
                sensorData.sound.shift();
                sensorData.light.shift();
                sensorData.timestamps.shift();
            }
        }
        
        // Event listeners
        toggleAlarmBtn.addEventListener('click', () => {
            fetch('/api/alarm/toggle', {
                method: 'POST'
            })
            .then(response => response.json())
            .then(data => {
                alarmStatus.textContent = data.alarmEnabled ? 'Armed' : 'Disarmed';
                toggleAlarmBtn.textContent = data.alarmEnabled ? 'Disarm' : 'Arm';
                toggleAlarmBtn.className = data.alarmEnabled ? 'btn btn-danger' : 'btn';
                toggleAlarmBtn.innerHTML = data.alarmEnabled 
                    ? '<i class="fas fa-power-off"></i> Disarm' 
                    : '<i class="fas fa-power-off"></i> Arm';
                alarmToggle.checked = data.alarmEnabled;
                alarmLabel.textContent = data.alarmEnabled ? 'Armed' : 'Disarmed';
            });
        });
        
        alarmToggle.addEventListener('change', () => {
            fetch('/api/alarm/toggle', {
                method: 'POST'
            })
            .then(response => response.json())
            .then(data => {
                alarmStatus.textContent = data.alarmEnabled ? 'Armed' : 'Disarmed';
                toggleAlarmBtn.textContent = data.alarmEnabled ? 'Disarm' : 'Arm';
                toggleAlarmBtn.className = data.alarmEnabled ? 'btn btn-danger' : 'btn';
                toggleAlarmBtn.innerHTML = data.alarmEnabled 
                    ? '<i class="fas fa-power-off"></i> Disarm' 
                    : '<i class="fas fa-power-off"></i> Arm';
                alarmLabel.textContent = data.alarmEnabled ? 'Armed' : 'Disarmed';
            });
        });
        
        testBuzzerBtn.addEventListener('click', () => {
            fetch('/api/buzzer', {
                method: 'POST'
            });
        });
        
        soundThreshold.addEventListener('input', () => {
            soundThresholdValue.textContent = soundThreshold.value;
        });
        
        tempWarning.addEventListener('input', () => {
            tempWarningValue.textContent = tempWarning.value + '°C';
        });
        
        tempCritical.addEventListener('input', () => {
            tempCriticalValue.textContent = tempCritical.value + '°C';
            // Ensure critical is always higher than warning
            if (parseFloat(tempCritical.value) <= parseFloat(tempWarning.value)) {
                tempWarning.value = (parseFloat(tempCritical.value) - 0.5).toFixed(1);
                tempWarningValue.textContent = tempWarning.value + '°C';
            }
        });
        
        lightThreshold.addEventListener('input', () => {
            lightThresholdValue.textContent = lightThreshold.value;
        });
        
        saveSettingsBtn.addEventListener('click', () => {
            const securitySettings = {
                alarmEnabled: alarmToggle.checked,
                soundThreshold: parseInt(soundThreshold.value)
            };
            
            const tempSettings = {
                tempWarning: parseFloat(tempWarning.value),
                tempCritical: parseFloat(tempCritical.value),
                lightThreshold: parseInt(lightThreshold.value)
            };
            
            // Save security settings
            fetch('/api/settings/security', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(securitySettings)
            });
            
            // Save temperature settings
            fetch('/api/settings/temperature', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(tempSettings)
            });
            
            // Show success feedback
            saveSettingsBtn.innerHTML = '<i class="fas fa-check"></i> Saved!';
            setTimeout(() => {
                saveSettingsBtn.innerHTML = '<i class="fas fa-save"></i> Save Settings';
            }, 2000);
        });
        
        // Initialize the dashboard
        document.addEventListener('DOMContentLoaded', () => {
            initCharts();
            updateSensorData();
            
            // Update sensor data every 3 seconds
            setInterval(updateSensorData, 3000);
            
            // Load initial settings
            fetch('/api/sensor')
                .then(response => response.json())
                .then(data => {
                    soundThreshold.value = data.soundThreshold;
                    soundThresholdValue.textContent = data.soundThreshold;
                    tempWarning.value = data.tempWarning;
                    tempWarningValue.textContent = data.tempWarning + '°C';
                    tempCritical.value = data.tempCritical;
                    tempCriticalValue.textContent = data.tempCritical + '°C';
                    lightThreshold.value = data.lightThreshold;
                    lightThresholdValue.textContent = data.lightThreshold;
                });
        });
    </script>
</body>
</html>
)rawliteral"; 
const char *helpPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SecureGuard-Pro Help</title>
    <style>
        :root {
            --primary: #3498db;
            --secondary: #2ecc71;
            --danger: #e74c3c;
            --warning: #f39c12;
            --dark: #2c3e50;
            --light: #ecf0f1;
            --gray: #95a5a6;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background-color: #f5f7fa;
            color: var(--dark);
        }
        
        .container {
            display: grid;
            grid-template-columns: 250px 1fr;
            min-height: 100vh;
        }
        
        /* Sidebar Styles */
        .sidebar {
            background-color: var(--dark);
            color: var(--light);
            padding: 1.5rem;
            box-shadow: 2px 0 10px rgba(0,0,0,0.1);
        }
        
        .logo {
            display: flex;
            align-items: center;
            margin-bottom: 2rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        .logo img {
            width: 40px;
            margin-right: 10px;
        }
        
        .logo h2 {
            font-size: 1.2rem;
            font-weight: 500;
        }
        
        .nav-menu {
            list-style: none;
        }
        
        .nav-item {
            margin-bottom: 0.5rem;
        }
        
        .nav-link {
            display: flex;
            align-items: center;
            padding: 0.8rem 1rem;
            color: var(--light);
            text-decoration: none;
            border-radius: 5px;
            transition: all 0.3s;
        }
        
        .nav-link:hover, .nav-link.active {
            background-color: rgba(255,255,255,0.1);
            color: white;
        }
        
        .nav-link i {
            margin-right: 10px;
            font-size: 1.2rem;
        }
        
        /* Main Content Styles */
        .main-content {
            padding: 2rem;
            overflow-y: auto;
        }
        
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2rem;
        }
        
        .page-title h1 {
            font-size: 1.8rem;
            font-weight: 500;
            color: var(--dark);
        }
        
        .page-title p {
            color: var(--gray);
            font-size: 0.9rem;
        }
        
        .user-profile {
            display: flex;
            align-items: center;
        }
        
        .user-profile img {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            margin-right: 10px;
        }
        
        /* Help Tabs */
        .help-tabs {
            display: flex;
            border-bottom: 1px solid #ddd;
            margin-bottom: 2rem;
        }
        
        .tab-btn {
            padding: 0.8rem 1.5rem;
            background: none;
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            color: var(--gray);
            position: relative;
            transition: all 0.3s;
        }
        
        .tab-btn.active {
            color: var(--dark);
        }
        
        .tab-btn.active::after {
            content: '';
            position: absolute;
            bottom: -1px;
            left: 0;
            width: 100%;
            height: 3px;
        }
        
        .tab-btn.green.active::after {
            background-color: var(--secondary);
        }
        
        .tab-btn.blue.active::after {
            background-color: var(--primary);
        }
        
        .tab-btn.yellow.active::after {
            background-color: var(--warning);
        }
        
        .tab-btn.red.active::after {
            background-color: var(--danger);
        }
        
        .tab-content {
            display: none;
        }
        
        .tab-content.active {
            display: block;
            animation: fadeIn 0.5s;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        
        /* Color-Coded Sections */
        .section-green {
            border-left: 4px solid var(--secondary);
            padding-left: 1rem;
            margin-bottom: 2rem;
        }
        
        .section-blue {
            border-left: 4px solid var(--primary);
            padding-left: 1rem;
            margin-bottom: 2rem;
        }
        
        .section-yellow {
            border-left: 4px solid var(--warning);
            padding-left: 1rem;
            margin-bottom: 2rem;
        }
        
        .section-red {
            border-left: 4px solid var(--danger);
            padding-left: 1rem;
            margin-bottom: 2rem;
        }
        
        /* Help Content */
        .help-content {
            background-color: white;
            border-radius: 10px;
            padding: 2rem;
            box-shadow: 0 4px 6px rgba(0,0,0,0.05);
        }
        
        .help-section h2 {
            margin-bottom: 1rem;
            padding-bottom: 0.5rem;
            border-bottom: 1px solid #eee;
        }
        
        .help-section h3 {
            margin: 1.5rem 0 1rem;
        }
        
        .help-section p {
            margin-bottom: 1rem;
            line-height: 1.6;
        }
        
        .help-section ul, .help-section ol {
            margin-bottom: 1rem;
            padding-left: 2rem;
        }
        
        .help-section li {
            margin-bottom: 0.5rem;
        }
        
        /* FAQ Styles */
        .faq-item {
            margin-bottom: 1.5rem;
            padding: 1rem;
            background-color: #f8f9fa;
            border-radius: 8px;
        }
        
        .faq-question {
            font-weight: 600;
            color: var(--dark);
            margin-bottom: 0.5rem;
            display: flex;
            justify-content: space-between;
            align-items: center;
            cursor: pointer;
        }
        
        .faq-question::after {
            content: '+';
            font-size: 1.2rem;
        }
        
        .faq-question.active::after {
            content: '-';
        }
        
        .faq-answer {
            display: none;
            padding-top: 0.5rem;
        }
        
        .faq-answer.active {
            display: block;
        }
        
        /* Tables */
        table {
            width: 100%;
            border-collapse: collapse;
            margin: 1rem 0;
        }
        
        th, td {
            padding: 0.8rem;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        
        th {
            background-color: #f2f2f2;
            font-weight: 500;
        }
        
        tr:hover {
            background-color: #f5f5f5;
        }
        
        /* Status Indicators */
        .status-indicator {
            display: inline-block;
            width: 12px;
            height: 12px;
            border-radius: 50%;
            margin-right: 0.5rem;
        }
        
        .status-green {
            background-color: var(--secondary);
        }
        
        .status-blue {
            background-color: var(--primary);
        }
        
        .status-yellow {
            background-color: var(--warning);
        }
        
        .status-red {
            background-color: var(--danger);
        }
        
        /* Responsive Styles */
        @media (max-width: 992px) {
            .container {
                grid-template-columns: 1fr;
            }
            
            .sidebar {
                display: none;
            }
            
            .help-tabs {
                flex-wrap: wrap;
            }
            
            .tab-btn {
                flex: 1;
                min-width: 120px;
                text-align: center;
            }
        }
    </style>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
</head>
<body>
    <div class="container">
        <!-- Sidebar -->
        <div class="sidebar">
            <div class="logo">
                <img src="https://via.placeholder.com/40x40?text=SG" alt="SecureGuard Logo">
                <h2>SecureGuard-Pro</h2>
            </div>
            
            %NAVIGATION%
            
        </div>
        
        <!-- Main Content -->
        <div class="main-content">
            <div class="header">
                <div class="page-title">
                    <h1>Help & Support Center</h1>
                    <p>Find answers, guides, and troubleshooting tips</p>
                </div>
                
                <div class="user-profile">
                    <img src="https://via.placeholder.com/40x40?text=EM" alt="User Profile">
                    <span>Emma</span>
                </div>
            </div>
            
            <div class="help-content">
                <!-- Help Tabs Navigation -->
                <div class="help-tabs">
                    <button class="tab-btn green active" onclick="openTab('getting-started')">
                        <i class="fas fa-play-circle"></i> Getting Started
                    </button>
                    <button class="tab-btn blue" onclick="openTab('user-guide')">
                        <i class="fas fa-book"></i> User Guide
                    </button>
                    <button class="tab-btn yellow" onclick="openTab('faq')">
                        <i class="fas fa-question-circle"></i> FAQ
                    </button>
                    <button class="tab-btn red" onclick="openTab('troubleshooting')">
                        <i class="fas fa-tools"></i> Troubleshooting
                    </button>
                </div>
                
                <!-- Getting Started Tab -->
                <div id="getting-started" class="tab-content active">
                    <div class="section-green">
                        <h2><i class="fas fa-play-circle"></i> Quick Start Guide</h2>
                        <p>Get your SecureGuard-Pro system up and running in minutes with this quick start guide.</p>
                        
                        <h3>Initial Setup Process</h3>
                        <ol>
                            <li>Unbox your SecureGuard-Pro device and connect it to power</li>
                            <li>Connect to the SecureGuard-Pro WiFi network (password: Em.ma.45)</li>
                            <li>Open a web browser and navigate to the device IP (usually 192.168.4.1)</li>
                            <li>Log in using the default credentials (username: Emma, password: Em.ma.45)</li>
                            <li>Follow the on-screen setup wizard to configure basic settings</li>
                        </ol>
                        
                        <h3>First-Time Configuration</h3>
                        <table>
                            <tr>
                                <th>Setting</th>
                                <th>Recommended Value</th>
                                <th>Description</th>
                            </tr>
                            <tr>
                                <td>Temperature Warning</td>
                                <td>30°C</td>
                                <td>Threshold for temperature warnings</td>
                            </tr>
                            <tr>
                                <td>Temperature Critical</td>
                                <td>35°C</td>
                                <td>Threshold for critical temperature alerts</td>
                            </tr>
                            <tr>
                                <td>Sound Threshold</td>
                                <td>2000</td>
                                <td>Noise level that triggers alerts</td>
                            </tr>
                            <tr>
                                <td>Light Threshold</td>
                                <td>2000</td>
                                <td>Minimum light level before alerts</td>
                            </tr>
                        </table>
                    </div>
                    
                    <div class="section-blue">
                        <h2><i class="fas fa-video"></i> Video Tutorials</h2>
                        <p>Watch these short videos to learn how to set up and use your system:</p>
                        
                        <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 1rem;">
                            <div style="background: #f8f9fa; padding: 1rem; border-radius: 8px;">
                                <div style="background: #ddd; height: 150px; display: flex; align-items: center; justify-content: center; margin-bottom: 0.5rem;">
                                    <i class="fas fa-play-circle" style="font-size: 2rem; color: #666;"></i>
                                </div>
                                <h3>Unboxing and Hardware Setup</h3>
                                <p>2:45 min</p>
                            </div>
                            <div style="background: #f8f9fa; padding: 1rem; border-radius: 8px;">
                                <div style="background: #ddd; height: 150px; display: flex; align-items: center; justify-content: center; margin-bottom: 0.5rem;">
                                    <i class="fas fa-play-circle" style="font-size: 2rem; color: #666;"></i>
                                </div>
                                <h3>Initial Software Configuration</h3>
                                <p>4:12 min</p>
                            </div>
                            <div style="background: #f8f9fa; padding: 1rem; border-radius: 8px;">
                                <div style="background: #ddd; height: 150px; display: flex; align-items: center; justify-content: center; margin-bottom: 0.5rem;">
                                    <i class="fas fa-play-circle" style="font-size: 2rem; color: #666;"></i>
                                </div>
                                <h3>Mobile Access Setup</h3>
                                <p>3:30 min</p>
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- User Guide Tab -->
                <div id="user-guide" class="tab-content">
                    <div class="section-blue">
                        <h2><i class="fas fa-book"></i> Comprehensive User Guide</h2>
                        <p>Learn all the features and capabilities of your SecureGuard-Pro system.</p>
                        
                        <h3>Dashboard Overview</h3>
                        <table>
                            <tr>
                                <th>Component</th>
                                <th>Description</th>
                                <th>Status Indicators</th>
                            </tr>
                            <tr>
                                <td>Temperature</td>
                                <td>Current ambient temperature</td>
                                <td>
                                    <span class="status-indicator status-green"></span>Normal
                                    <span class="status-indicator status-yellow"></span>Warning
                                    <span class="status-indicator status-red"></span>Critical
                                </td>
                            </tr>
                            <tr>
                                <td>Sound Level</td>
                                <td>Ambient noise monitoring</td>
                                <td>
                                    <span class="status-indicator status-green"></span>Normal
                                    <span class="status-indicator status-red"></span>Alert
                                </td>
                            </tr>
                            <tr>
                                <td>Light Level</td>
                                <td>Ambient light detection</td>
                                <td>
                                    <span class="status-indicator status-green"></span>Normal
                                    <span class="status-indicator status-yellow"></span>Low
                                </td>
                            </tr>
                            <tr>
                                <td>Alarm Status</td>
                                <td>System armed/disarmed state</td>
                                <td>
                                    <span class="status-indicator status-blue"></span>Armed
                                    <span class="status-indicator status-gray"></span>Disarmed
                                </td>
                            </tr>
                        </table>
                        
                        <h3>System Settings</h3>
                        <p>Configure your system to match your specific security needs:</p>
                        <ul>
                            <li><strong>Notification Preferences:</strong> Set up email and SMS alerts</li>
                            <li><strong>Automation Rules:</strong> Create custom automation scenarios</li>
                            <li><strong>User Management:</strong> Add additional users with different access levels</li>
                            <li><strong>System Updates:</strong> Check for and install firmware updates</li>
                        </ul>
                    </div>
                    
                    <div class="section-green">
                        <h2><i class="fas fa-sliders-h"></i> Advanced Configuration</h2>
                        <p>For power users who want to customize their system further.</p>
                        
                        <h3>API Documentation</h3>
                        <p>The SecureGuard-Pro system provides a REST API for integration with other systems:</p>
                        <table>
                            <tr>
                                <th>Endpoint</th>
                                <th>Method</th>
                                <th>Description</th>
                            </tr>
                            <tr>
                                <td>/api/sensor</td>
                                <td>GET</td>
                                <td>Retrieve current sensor readings</td>
                            </tr>
                            <tr>
                                <td>/api/buzzer</td>
                                <td>POST</td>
                                <td>Activate the buzzer</td>
                            </tr>
                            <tr>
                                <td>/api/alarm/toggle</td>
                                <td>POST</td>
                                <td>Toggle alarm state</td>
                            </tr>
                            <tr>
                                <td>/api/led</td>
                                <td>POST</td>
                                <td>Control RGB LED</td>
                            </tr>
                        </table>
                    </div>
                </div>
                
                <!-- FAQ Tab -->
                <div id="faq" class="tab-content">
                    <div class="section-yellow">
                        <h2><i class="fas fa-question-circle"></i> Frequently Asked Questions</h2>
                        <p>Find answers to common questions about your SecureGuard-Pro system.</p>
                        
                        <div class="faq-item">
                            <div class="faq-question" onclick="toggleFAQ(this)">
                                How do I reset the system to factory defaults?
                            </div>
                            <div class="faq-answer">
                                <p>To reset your SecureGuard-Pro to factory settings:</p>
                                <ol>
                                    <li>Locate the small reset button on the device (usually on the back or bottom)</li>
                                    <li>Use a paperclip to press and hold the button for 10 seconds</li>
                                    <li>Wait for the device to reboot (LED will flash rapidly during reset)</li>
                                    <li>All settings will be restored to factory defaults</li>
                                </ol>
                                <p><strong>Note:</strong> This will erase all custom settings and user accounts.</p>
                            </div>
                        </div>
                        
                        <div class="faq-item">
                            <div class="faq-question" onclick="toggleFAQ(this)">
                                Can I change the WiFi credentials?
                            </div>
                            <div class="faq-answer">
                                <p>Yes, you can change the WiFi credentials in the Settings section:</p>
                                <ol>
                                    <li>Go to Settings > Network</li>
                                    <li>Select "WiFi Configuration"</li>
                                    <li>Enter your new SSID and password</li>
                                    <li>Click "Save Changes"</li>
                                    <li>The device will reboot with new WiFi settings</li>
                                </ol>
                                <p><strong>Tip:</strong> Make sure to note down your new credentials in a safe place.</p>
                            </div>
                        </div>
                        
                        <div class="faq-item">
                            <div class="faq-question" onclick="toggleFAQ(this)">
                                How often does the system update sensor readings?
                            </div>
                            <div class="faq-answer">
                                <p>The default sensor update interval is 3 seconds. You can adjust this setting:</p>
                                <ol>
                                    <li>Go to Settings > System</li>
                                    <li>Find "Sensor Update Interval"</li>
                                    <li>Set your preferred interval (1-60 seconds)</li>
                                    <li>Click "Save Changes"</li>
                                </ol>
                                <p><strong>Note:</strong> Shorter intervals provide more real-time data but may increase power consumption.</p>
                            </div>
                        </div>
                        
                        <div class="faq-item">
                            <div class="faq-question" onclick="toggleFAQ(this)">
                                What should I do if the system stops responding?
                            </div>
                            <div class="faq-answer">
                                <p>If your SecureGuard-Pro becomes unresponsive:</p>
                                <ol>
                                    <li><strong>First attempt:</strong> Power cycle the device by unplugging it for 10 seconds and plugging it back in</li>
                                    <li><strong>Second attempt:</strong> Perform a factory reset (see first FAQ question)</li>
                                    <li><strong>If problems persist:</strong> Contact our support team with details of the issue</li>
                                </ol>
                                <p><strong>Pro tip:</strong> Regular firmware updates can prevent many stability issues.</p>
                            </div>
                        </div>
                    </div>
                    
                    <div class="section-green">
                        <h2><i class="fas fa-lightbulb"></i> Tips & Tricks</h2>
                        
                        <div class="faq-item">
                            <div class="faq-question" onclick="toggleFAQ(this)">
                                How can I extend the system's coverage?
                            </div>
                            <div class="faq-answer">
                                <p>To maximize your SecureGuard-Pro coverage:</p>
                                <ul>
                                    <li>Place the device in a central location</li>
                                    <li>Ensure it's elevated for better sensor performance</li>
                                    <li>Consider adding additional sensor modules (sold separately)</li>
                                    <li>For large areas, set up multiple units in a network</li>
                                </ul>
                            </div>
                        </div>
                        
                        <div class="faq-item">
                            <div class="faq-question" onclick="toggleFAQ(this)">
                                What's the best way to configure alerts?
                            </div>
                            <div class="faq-answer">
                                <p>For effective alert configuration:</p>
                                <ul>
                                    <li>Start with default thresholds and adjust based on your environment</li>
                                    <li>Set up email notifications for critical alerts</li>
                                    <li>Use SMS alerts for time-sensitive notifications</li>
                                    <li>Create different alert profiles for day/night modes</li>
                                </ul>
                            </div>
                        </div>
                    </div>
                </div>
                
                <!-- Troubleshooting Tab -->
                <div id="troubleshooting" class="tab-content">
                    <div class="section-red">
                        <h2><i class="fas fa-tools"></i> Troubleshooting Guide</h2>
                        <p>Solutions to common problems with your SecureGuard-Pro system.</p>
                        
                        <h3>Common Issues and Solutions</h3>
                        <table>
                            <tr>
                                <th>Issue</th>
                                <th>Possible Cause</th>
                                <th>Solution</th>
                            </tr>
                            <tr>
                                <td>Can't connect to WiFi</td>
                                <td>Incorrect password, signal issues</td>
                                <td>Double-check password, move closer to router</td>
                            </tr>
                            <tr>
                                <td>False alarms</td>
                                <td>Thresholds too sensitive</td>
                                <td>Adjust thresholds in Settings</td>
                            </tr>
                            <tr>
                                <td>Device not responding</td>
                                <td>Power or firmware issue</td>
                                <td>Power cycle, update firmware</td>
                            </tr>
                            <tr>
                                <td>Sensors not updating</td>
                                <td>Connection or calibration issue</td>
                                <td>Check connections, recalibrate sensors</td>
                            </tr>
                        </table>
                        
                        <h3>Error Codes Reference</h3>
                        <table>
                            <tr>
                                <th>Error Code</th>
                                <th>Description</th>
                                <th>Action</th>
                            </tr>
                            <tr>
                                <td>E101</td>
                                <td>Temperature sensor failure</td>
                                <td>Check sensor connection, reboot</td>
                            </tr>
                            <tr>
                                <td>E202</td>
                                <td>Sound sensor calibration error</td>
                                <td>Recalibrate in Settings</td>
                            </tr>
                            <tr>
                                <td>E303</td>
                                <td>Network connection lost</td>
                                <td>Check WiFi settings, signal strength</td>
                            </tr>
                            <tr>
                                <td>E404</td>
                                <td>System overload</td>
                                <td>Reduce sensor update frequency, reboot</td>
                            </tr>
                        </table>
                    </div>
                    
                    <div class="section-blue">
                        <h2><i class="fas fa-headset"></i> Support Resources</h2>
                        <p>Additional resources to help resolve your issues.</p>
                        
                        <div class="contact-info">
                            <div class="contact-card">
                                <h3><i class="fas fa-envelope"></i> Email Support</h3>
                                <p><i class="fas fa-envelope"></i> eolara68@gmail.com</p>
                                <p><i class="fas fa-clock"></i> 24/7 support</p>
                                <p><i class="fas fa-reply"></i> Response time: 1-2 hours</p>
                            </div>
                            
                            <div class="contact-card">
                                <h3><i class="fas fa-phone"></i> Phone Support</h3>
                                <p><i class="fas fa-phone"></i> +256 770 680938</p>
                                <p><i class="fas fa-clock"></i> Mon-Fri, 9AM-5PM</p>
                                <p><i class="fas fa-language"></i> English, French</p>
                            </div>
                            
                            <div class="contact-card">
                                <h3><i class="fas fa-file-pdf"></i> Documentation</h3>
                                <p><i class="fas fa-download"></i> <a href="#">User Manual PDF</a></p>
                                <p><i class="fas fa-download"></i> <a href="#">Quick Start Guide</a></p>
                                <p><i class="fas fa-download"></i> <a href="#">API Reference</a></p>
                            </div>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    </div>
    
    <script>
        function confirmLogout(event) {
        event.preventDefault();
        window.location.href = '/logout';
    }
        // Tab functionality
        function openTab(tabId) {
            // Hide all tab contents
            const tabContents = document.getElementsByClassName('tab-content');
            for (let i = 0; i < tabContents.length; i++) {
                tabContents[i].classList.remove('active');
            }
            
            // Remove active class from all tab buttons
            const tabButtons = document.getElementsByClassName('tab-btn');
            for (let i = 0; i < tabButtons.length; i++) {
                tabButtons[i].classList.remove('active');
            }
            
            // Show the selected tab content
            document.getElementById(tabId).classList.add('active');
            
            // Add active class to the clicked button
            event.currentTarget.classList.add('active');
        }
        
        // FAQ toggle functionality
        function toggleFAQ(element) {
            element.classList.toggle('active');
            const answer = element.nextElementSibling;
            answer.classList.toggle('active');
        }
        
        // Initialize first FAQ items as open
        document.addEventListener('DOMContentLoaded', function() {
            const firstFaqs = document.querySelectorAll('#faq .faq-item:first-child .faq-question');
            firstFaqs.forEach(faq => {
                faq.classList.add('active');
                faq.nextElementSibling.classList.add('active');
            });
        });
    </script>
</body>
</html>
)rawliteral";

const char *settingsPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SecureGuard-Pro Settings</title>
    <style>
        :root {
            --primary: #3498db;
            --secondary: #2ecc71;
            --danger: #e74c3c;
            --warning: #f39c12;
            --dark: #2c3e50;
            --light: #ecf0f1;
            --gray: #95a5a6;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background-color: #f5f7fa;
            color: var(--dark);
        }
        
        .container {
            display: grid;
            grid-template-columns: 250px 1fr;
            min-height: 100vh;
        }
        
        /* Sidebar Styles */
        .sidebar {
            background-color: var(--dark);
            color: var(--light);
            padding: 1.5rem;
            box-shadow: 2px 0 10px rgba(0,0,0,0.1);
        }
        
        .logo {
            display: flex;
            align-items: center;
            margin-bottom: 2rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        .logo img {
            width: 40px;
            margin-right: 10px;
        }
        
        .logo h2 {
            font-size: 1.2rem;
            font-weight: 500;
        }
        
        .nav-menu {
            list-style: none;
        }
        
        .nav-item {
            margin-bottom: 0.5rem;
        }
        
        .nav-link {
            display: flex;
            align-items: center;
            padding: 0.8rem 1rem;
            color: var(--light);
            text-decoration: none;
            border-radius: 5px;
            transition: all 0.3s;
        }
        
        .nav-link:hover, .nav-link.active {
            background-color: rgba(255,255,255,0.1);
            color: white;
        }
        
        .nav-link i {
            margin-right: 10px;
            font-size: 1.2rem;
        }
        
        /* Main Content Styles */
        .main-content {
            padding: 2rem;
            overflow-y: auto;
        }
        
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2rem;
        }
        
        .page-title h1 {
            font-size: 1.8rem;
            font-weight: 500;
            color: var(--dark);
        }
        
        .page-title p {
            color: var(--gray);
            font-size: 0.9rem;
        }
        
        .user-profile {
            display: flex;
            align-items: center;
        }
        
        .user-profile img {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            margin-right: 10px;
        }
        
        /* Settings Tabs */
        .settings-tabs {
            display: flex;
            border-bottom: 1px solid #ddd;
            margin-bottom: 2rem;
            flex-wrap: wrap;
        }
        
        .settings-tab {
            padding: 0.8rem 1.5rem;
            background: none;
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            color: var(--gray);
            position: relative;
            transition: all 0.3s;
        }
        
        .settings-tab.active {
            color: var(--dark);
        }
        
        .settings-tab.active::after {
            content: '';
            position: absolute;
            bottom: -1px;
            left: 0;
            width: 100%;
            height: 3px;
            background-color: var(--primary);
        }
        
        /* Settings Content */
        .settings-content {
            display: none;
            background-color: white;
            border-radius: 10px;
            padding: 2rem;
            box-shadow: 0 4px 6px rgba(0,0,0,0.05);
            margin-bottom: 2rem;
        }
        
        .settings-content.active {
            display: block;
            animation: fadeIn 0.5s;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        
        /* Form Styles */
        .form-group {
            margin-bottom: 1.5rem;
        }
        
        .form-label {
            display: block;
            margin-bottom: 0.5rem;
            font-weight: 500;
        }
        
        .form-control {
            width: 100%;
            padding: 0.8rem;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 1rem;
            transition: border 0.3s;
        }
        
        .form-control:focus {
            outline: none;
            border-color: var(--primary);
        }
        
        .form-row {
            display: flex;
            gap: 1rem;
            margin-bottom: 1rem;
        }
        
        .form-col {
            flex: 1;
        }
        
        /* Toggle Switch */
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        
        input:checked + .slider {
            background-color: var(--primary);
        }
        
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        
        /* Button Styles */
        .btn {
            padding: 0.8rem 1.5rem;
            border: none;
            border-radius: 5px;
            background-color: var(--primary);
            color: white;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .btn:hover {
            background-color: #2980b9;
            transform: translateY(-2px);
        }
        
        .btn-secondary {
            background-color: var(--gray);
        }
        
        .btn-secondary:hover {
            background-color: #7f8c8d;
        }
        
        .btn-danger {
            background-color: var(--danger);
        }
        
        .btn-danger:hover {
            background-color: #c0392b;
        }
        
        /* Table Styles */
        .table {
            width: 100%;
            border-collapse: collapse;
            margin: 1rem 0;
        }
        
        .table th, .table td {
            padding: 0.8rem;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        
        .table th {
            background-color: #f2f2f2;
            font-weight: 500;
        }
        
        .table tr:hover {
            background-color: #f5f5f5;
        }
        
        /* Alert Styles */
        .alert {
            padding: 1rem;
            border-radius: 5px;
            margin-bottom: 1rem;
        }
        
        .alert-success {
            background-color: #d4edda;
            color: #155724;
        }
        
        .alert-danger {
            background-color: #f8d7da;
            color: #721c24;
        }
        
        /* Responsive Styles */
        @media (max-width: 992px) {
            .container {
                grid-template-columns: 1fr;
            }
            
            .sidebar {
                display: none;
            }
            
            .settings-tabs {
                flex-direction: column;
            }
            
            .settings-tab {
                text-align: left;
                border-bottom: 1px solid #eee;
            }
            
            .form-row {
                flex-direction: column;
                gap: 0;
            }
        }
    </style>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
</head>
<body>
    <div class="container">
        <!-- Sidebar -->
        <div class="sidebar">
            <div class="logo">
                <img src="https://via.placeholder.com/40x40?text=SG" alt="SecureGuard Logo">
                <h2>SecureGuard-Pro</h2>
            </div>
            
            %NAVIGATION%
            
        </div>
        
        <!-- Main Content -->
        <div class="main-content">
            <div class="header">
                <div class="page-title">
                    <h1>System Settings</h1>
                    <p>Configure and customize your SecureGuard-Pro system</p>
                </div>
                
                <div class="user-profile">
                    <img src="https://via.placeholder.com/40x40?text=EM" alt="User Profile">
                    <span>Emma</span>
                </div>
            </div>
            
            <!-- Settings Tabs -->
            <div class="settings-tabs">
                <button class="settings-tab active" onclick="openSettingsTab('network-settings')">
                    <i class="fas fa-wifi"></i> Network
                </button>
                <button class="settings-tab" onclick="openSettingsTab('system-settings')">
                    <i class="fas fa-cog"></i> System
                </button>
                <button class="settings-tab" onclick="openSettingsTab('sensor-settings')">
                    <i class="fas fa-microchip"></i> Sensors
                </button>
                <button class="settings-tab" onclick="openSettingsTab('alert-settings')">
                    <i class="fas fa-bell"></i> Alerts
                </button>
                <button class="settings-tab" onclick="openSettingsTab('user-settings')">
                    <i class="fas fa-users"></i> Users
                </button>
                <button class="settings-tab" onclick="openSettingsTab('maintenance')">
                    <i class="fas fa-tools"></i> Maintenance
                </button>
            </div>
            
            <!-- Network Settings -->
            <div id="network-settings" class="settings-content active">
                <h2><i class="fas fa-wifi"></i> Network Configuration</h2>
                
                <div class="alert alert-success" id="network-alert" style="display: none;">
                    Network settings saved successfully! The system will reboot to apply changes.
                </div>
                
                <form id="network-form">
                    <div class="form-group">
                        <label class="form-label">WiFi Network Mode</label>
                        <div style="display: flex; align-items: center; gap: 1rem;">
                            <label class="switch">
                                <input type="checkbox" id="wifi-mode" checked>
                                <span class="slider"></span>
                            </label>
                            <span id="wifi-mode-label">Access Point Mode</span>
                        </div>
                        <small class="text-muted">In Access Point mode, the device creates its own WiFi network. In Station mode, it connects to your existing WiFi.</small>
                    </div>
                    
                    <div id="ap-settings">
                        <div class="form-group">
                            <label for="ap-ssid" class="form-label">Access Point SSID</label>
                            <input type="text" class="form-control" id="ap-ssid" value="SecureGuard-Pro" required>
                        </div>
                        
                        <div class="form-group">
                            <label for="ap-password" class="form-label">Access Point Password</label>
                            <input type="password" class="form-control" id="ap-password" value="Em.ma.45" required>
                            <small class="text-muted">Minimum 8 characters. Current password is Em.ma.45</small>
                        </div>
                    </div>
                    
                    <div id="station-settings" style="display: none;">
                        <div class="form-group">
                            <label for="sta-ssid" class="form-label">WiFi Network Name (SSID)</label>
                            <input type="text" class="form-control" id="sta-ssid" placeholder="Enter your WiFi network name">
                        </div>
                        
                        <div class="form-group">
                            <label for="sta-password" class="form-label">WiFi Password</label>
                            <input type="password" class="form-control" id="sta-password" placeholder="Enter your WiFi password">
                        </div>
                        
                        <div class="form-group">
                            <label for="ip-config" class="form-label">IP Configuration</label>
                            <select class="form-control" id="ip-config">
                                <option value="dhcp">DHCP (Automatic)</option>
                                <option value="static">Static IP</option>
                            </select>
                        </div>
                        
                        <div id="static-ip-fields" style="display: none;">
                            <div class="form-row">
                                <div class="form-col">
                                    <label for="ip-address" class="form-label">IP Address</label>
                                    <input type="text" class="form-control" id="ip-address" placeholder="192.168.1.100">
                                </div>
                                <div class="form-col">
                                    <label for="subnet-mask" class="form-label">Subnet Mask</label>
                                    <input type="text" class="form-control" id="subnet-mask" placeholder="255.255.255.0">
                                </div>
                            </div>
                            <div class="form-row">
                                <div class="form-col">
                                    <label for="gateway" class="form-label">Default Gateway</label>
                                    <input type="text" class="form-control" id="gateway" placeholder="192.168.1.1">
                                </div>
                                <div class="form-col">
                                    <label for="dns" class="form-label">DNS Server</label>
                                    <input type="text" class="form-control" id="dns" placeholder="8.8.8.8">
                                </div>
                            </div>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <button type="submit" class="btn">Save Network Settings</button>
                        <button type="button" class="btn btn-secondary" onclick="testNetworkConnection()">Test Connection</button>
                    </div>
                </form>
            </div>
            
            <!-- System Settings -->
            <div id="system-settings" class="settings-content">
                <h2><i class="fas fa-cog"></i> System Configuration</h2>
                
                <form id="system-form">
                    <div class="form-group">
                        <label for="device-name" class="form-label">Device Name</label>
                        <input type="text" class="form-control" id="device-name" value="SecureGuard-Pro" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="location" class="form-label">Location</label>
                        <input type="text" class="form-control" id="location" value="Living Room" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="timezone" class="form-label">Timezone</label>
                        <select class="form-control" id="timezone" required>
                            <option value="GMT+3">East Africa Time (GMT+3)</option>
                            <option value="GMT+0">GMT (GMT+0)</option>
                            <option value="GMT-5">Eastern Time (GMT-5)</option>
                            <!-- More timezones can be added -->
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <label for="update-interval" class="form-label">Sensor Update Interval (seconds)</label>
                        <input type="number" class="form-control" id="update-interval" min="1" max="60" value="3" required>
                    </div>
                    
                    <div class="form-group">
                        <label class="form-label">System Features</label>
                        <div style="display: flex; align-items: center; margin-bottom: 0.5rem;">
                            <label class="switch">
                                <input type="checkbox" id="alarm-enabled" checked>
                                <span class="slider"></span>
                            </label>
                            <span style="margin-left: 1rem;">Alarm System Enabled</span>
                        </div>
                        <div style="display: flex; align-items: center; margin-bottom: 0.5rem;">
                            <label class="switch">
                                <input type="checkbox" id="led-indicators" checked>
                                <span class="slider"></span>
                            </label>
                            <span style="margin-left: 1rem;">LED Status Indicators</span>
                        </div>
                        <div style="display: flex; align-items: center;">
                            <label class="switch">
                                <input type="checkbox" id="buzzer-enabled" checked>
                                <span class="slider"></span>
                            </label>
                            <span style="margin-left: 1rem;">Audible Buzzer</span>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <button type="submit" class="btn">Save System Settings</button>
                    </div>
                </form>
            </div>
            
            <!-- Sensor Settings -->
            <div id="sensor-settings" class="settings-content">
                <h2><i class="fas fa-microchip"></i> Sensor Configuration</h2>
                
                <form id="sensor-form">
                    <div class="form-group">
                        <label for="temp-warning" class="form-label">Temperature Warning Threshold (°C)</label>
                        <input type="number" step="0.1" class="form-control" id="temp-warning" value="30.0" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="temp-critical" class="form-label">Temperature Critical Threshold (°C)</label>
                        <input type="number" step="0.1" class="form-control" id="temp-critical" value="35.0" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="sound-threshold" class="form-label">Sound Alert Threshold</label>
                        <input type="range" class="form-control" id="sound-threshold" min="0" max="4095" value="2000" oninput="updateSoundValue(this.value)">
                        <div style="display: flex; justify-content: space-between;">
                            <small>Low Sensitivity</small>
                            <small id="sound-threshold-value">2000</small>
                            <small>High Sensitivity</small>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <label for="light-threshold" class="form-label">Light Alert Threshold</label>
                        <input type="range" class="form-control" id="light-threshold" min="0" max="4095" value="2000" oninput="updateLightValue(this.value)">
                        <div style="display: flex; justify-content: space-between;">
                            <small>Dark</small>
                            <small id="light-threshold-value">2000</small>
                            <small>Bright</small>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <label class="form-label">Sensor Calibration</label>
                        <button type="button" class="btn btn-secondary" onclick="calibrateSensors()">Calibrate Sensors</button>
                        <small class="text-muted">Place sensors in normal conditions and click calibrate</small>
                    </div>
                    
                    <div class="form-group">
                        <button type="submit" class="btn">Save Sensor Settings</button>
                    </div>
                </form>
            </div>
            
            <!-- Alert Settings -->
            <div id="alert-settings" class="settings-content">
                <h2><i class="fas fa-bell"></i> Alert Configuration</h2>
                
                <form id="alert-form">
                    <div class="form-group">
                        <label class="form-label">Notification Methods</label>
                        <div style="display: flex; align-items: center; margin-bottom: 0.5rem;">
                            <label class="switch">
                                <input type="checkbox" id="email-alerts" checked>
                                <span class="slider"></span>
                            </label>
                            <span style="margin-left: 1rem;">Email Notifications</span>
                        </div>
                        <div style="display: flex; align-items: center; margin-bottom: 0.5rem;">
                            <label class="switch">
                                <input type="checkbox" id="sms-alerts">
                                <span class="slider"></span>
                            </label>
                            <span style="margin-left: 1rem;">SMS Notifications</span>
                        </div>
                        <div style="display: flex; align-items: center;">
                            <label class="switch">
                                <input type="checkbox" id="push-alerts" checked>
                                <span class="slider"></span>
                            </label>
                            <span style="margin-left: 1rem;">Push Notifications</span>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <label for="admin-email" class="form-label">Admin Email</label>
                        <input type="email" class="form-control" id="admin-email" value="eolara68@gmail.com" required>
                    </div>
                    
                    <div class="form-group">
                        <label for="admin-phone" class="form-label">Admin Phone Number</label>
                        <input type="tel" class="form-control" id="admin-phone" value="256770680938" required>
                    </div>
                    
                    <div class="form-group">
                        <label class="form-label">Alert Types</label>
                        <table class="table">
                            <tr>
                                <th>Alert Type</th>
                                <th>Email</th>
                                <th>SMS</th>
                                <th>Push</th>
                                <th>Buzzer</th>
                            </tr>
                            <tr>
                                <td>Temperature Critical</td>
                                <td><input type="checkbox" checked></td>
                                <td><input type="checkbox" checked></td>
                                <td><input type="checkbox" checked></td>
                                <td><input type="checkbox" checked></td>
                            </tr>
                            <tr>
                                <td>Sound Alert</td>
                                <td><input type="checkbox" checked></td>
                                <td><input type="checkbox"></td>
                                <td><input type="checkbox" checked></td>
                                <td><input type="checkbox" checked></td>
                            </tr>
                            <tr>
                                <td>Light Alert</td>
                                <td><input type="checkbox"></td>
                                <td><input type="checkbox"></td>
                                <td><input type="checkbox" checked></td>
                                <td><input type="checkbox"></td>
                            </tr>
                            <tr>
                                <td>System Events</td>
                                <td><input type="checkbox" checked></td>
                                <td><input type="checkbox"></td>
                                <td><input type="checkbox"></td>
                                <td><input type="checkbox"></td>
                            </tr>
                        </table>
                    </div>
                    
                    <div class="form-group">
                        <button type="submit" class="btn">Save Alert Settings</button>
                        <button type="button" class="btn btn-secondary" onclick="testAlerts()">Send Test Alert</button>
                    </div>
                </form>
            </div>
            
            <!-- User Settings -->
            <div id="user-settings" class="settings-content">
                <h2><i class="fas fa-users"></i> User Management</h2>
                
                <div class="alert alert-success" id="user-alert" style="display: none;">
                    User settings saved successfully!
                </div>
                
                <h3>Current Users</h3>
                <table class="table">
                    <tr>
                        <th>Username</th>
                        <th>Role</th>
                        <th>Last Login</th>
                        <th>Actions</th>
                    </tr>
                    <tr>
                        <td>Emma (You)</td>
                        <td>Administrator</td>
                        <td>Just now</td>
                        <td>
                            <button class="btn btn-secondary" style="padding: 0.3rem 0.6rem;">Edit</button>
                            <button class="btn btn-secondary" style="padding: 0.3rem 0.6rem;">Reset Password</button>
                        </td>
                    </tr>
                    <tr>
                        <td>John</td>
                        <td>Standard User</td>
                        <td>2 hours ago</td>
                        <td>
                            <button class="btn btn-secondary" style="padding: 0.3rem 0.6rem;">Edit</button>
                            <button class="btn btn-danger" style="padding: 0.3rem 0.6rem;">Delete</button>
                        </td>
                    </tr>
                </table>
                
                <h3>Add New User</h3>
                <form id="user-form">
                    <div class="form-row">
                        <div class="form-col">
                            <label for="new-username" class="form-label">Username</label>
                            <input type="text" class="form-control" id="new-username" required>
                        </div>
                        <div class="form-col">
                            <label for="new-email" class="form-label">Email</label>
                            <input type="email" class="form-control" id="new-email">
                        </div>
                    </div>
                    
                    <div class="form-row">
                        <div class="form-col">
                            <label for="new-password" class="form-label">Password</label>
                            <input type="password" class="form-control" id="new-password" required>
                        </div>
                        <div class="form-col">
                            <label for="confirm-password" class="form-label">Confirm Password</label>
                            <input type="password" class="form-control" id="confirm-password" required>
                        </div>
                    </div>
                    
                    <div class="form-group">
                        <label for="user-role" class="form-label">User Role</label>
                        <select class="form-control" id="user-role" required>
                            <option value="admin">Administrator</option>
                            <option value="standard" selected>Standard User</option>
                            <option value="viewer">View Only</option>
                        </select>
                    </div>
                    
                    <div class="form-group">
                        <button type="submit" class="btn">Add User</button>
                    </div>
                </form>
            </div>
            
            <!-- Maintenance -->
            <div id="maintenance" class="settings-content">
                <h2><i class="fas fa-tools"></i> System Maintenance</h2>
                
                <div class="alert alert-danger" id="maintenance-alert" style="display: none;">
                    Warning: This operation cannot be undone!
                </div>
                
                <h3>Firmware Update</h3>
                <div class="form-group">
                    <label for="firmware-file" class="form-label">Firmware File</label>
                    <input type="file" class="form-control" id="firmware-file" accept=".bin">
                    <small class="text-muted">Current version: 2.1.4</small>
                </div>
                
                <div class="form-group">
                    <button type="button" class="btn" onclick="updateFirmware()">Update Firmware</button>
                    <button type="button" class="btn btn-secondary" onclick="checkForUpdates()">Check for Updates</button>
                </div>
                
                <h3>System Backup</h3>
                <div class="form-group">
                    <button type="button" class="btn" onclick="backupConfig()">Backup Configuration</button>
                    <button type="button" class="btn btn-secondary" onclick="restoreConfig()">Restore Configuration</button>
                    <small class="text-muted">Last backup: 2023-11-15 14:30</small>
                </div>
                
                <h3>System Reset</h3>
                <div class="form-group">
                    <button type="button" class="btn btn-danger" onclick="confirmReset()">Factory Reset</button>
                    <small class="text-muted">This will erase all settings and user accounts</small>
                </div>
                
                <h3>System Information</h3>
                <table class="table">
                    <tr>
                        <td>Device Model</td>
                        <td>SecureGuard-Pro v2</td>
                    </tr>
                    <tr>
                        <td>Firmware Version</td>
                        <td>2.1.4</td>
                    </tr>
                    <tr>
                        <td>Uptime</td>
                        <td>3 days, 14 hours</td>
                    </tr>
                    <tr>
                        <td>Memory Usage</td>
                        <td>45% (2.3MB of 5MB)</td>
                    </tr>
                    <tr>
                        <td>WiFi Signal</td>
                        <td>Excellent (RSSI: -45dBm)</td>
                    </tr>
                </table>
            </div>
        </div>
    </div>
    
    <script>
        // Tab functionality
        function openSettingsTab(tabId) {
            // Hide all tab contents
            const tabContents = document.getElementsByClassName('settings-content');
            for (let i = 0; i < tabContents.length; i++) {
                tabContents[i].classList.remove('active');
            }
            
            // Remove active class from all tab buttons
            const tabButtons = document.getElementsByClassName('settings-tab');
            for (let i = 0; i < tabButtons.length; i++) {
                tabButtons[i].classList.remove('active');
            }
            
            // Show the selected tab content
            document.getElementById(tabId).classList.add('active');
            
            // Add active class to the clicked button
            event.currentTarget.classList.add('active');
        }
        
        // WiFi mode toggle
        document.getElementById('wifi-mode').addEventListener('change', function() {
            const apSettings = document.getElementById('ap-settings');
            const stationSettings = document.getElementById('station-settings');
            const modeLabel = document.getElementById('wifi-mode-label');
            
            if (this.checked) {
                apSettings.style.display = 'block';
                stationSettings.style.display = 'none';
                modeLabel.textContent = 'Access Point Mode';
            } else {
                apSettings.style.display = 'none';
                stationSettings.style.display = 'block';
                modeLabel.textContent = 'Station Mode';
            }
        });
        
        // IP configuration toggle
        document.getElementById('ip-config').addEventListener('change', function() {
            const staticIpFields = document.getElementById('static-ip-fields');
            if (this.value === 'static') {
                staticIpFields.style.display = 'block';
            } else {
                staticIpFields.style.display = 'none';
            }
        });
        
        // Update sound threshold value display
        function updateSoundValue(value) {
            document.getElementById('sound-threshold-value').textContent = value;
        }
        
        // Update light threshold value display
        function updateLightValue(value) {
            document.getElementById('light-threshold-value').textContent = value;
        }
        
        // Form submission handlers
        document.getElementById('network-form').addEventListener('submit', function(e) {
            e.preventDefault();
            document.getElementById('network-alert').style.display = 'block';
            setTimeout(() => {
                document.getElementById('network-alert').style.display = 'none';
            }, 5000);
        });
        
        document.getElementById('user-form').addEventListener('submit', function(e) {
            e.preventDefault();
            document.getElementById('user-alert').style.display = 'block';
            setTimeout(() => {
                document.getElementById('user-alert').style.display = 'none';
            }, 5000);
        });
        
        // Test functions
        function testNetworkConnection() {
            alert('Testing network connection...');
        }
        
        function calibrateSensors() {
            alert('Calibrating sensors. Please wait...');
        }
        
        function testAlerts() {
            alert('Test alert sent to all enabled notification methods');
        }
        
        function updateFirmware() {
            const fileInput = document.getElementById('firmware-file');
            if (fileInput.files.length > 0) {
                alert('Updating firmware with: ' + fileInput.files[0].name);
            } else {
                alert('Please select a firmware file first');
            }
        }
        
        function checkForUpdates() {
            alert('Checking for updates...');
        }
        
        function backupConfig() {
            alert('Configuration backup started');
        }
        
        function restoreConfig() {
            alert('Configuration restore dialog opened');
        }
        
        function confirmReset() {
            if (confirm('Are you sure you want to reset to factory defaults? All settings will be lost.')) {
                alert('Factory reset initiated. System will reboot.');
            }
        }
        function confirmLogout(event) {
        event.preventDefault();
        window.location.href = '/logout';
    }
    </script>
</body>
</html>
)rawliteral";
const char *alertsPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SecureGuard-Pro Alerts</title>
    
    <style>
        :root {
            --primary: #3498db;
            --secondary: #2ecc71;
            --danger: #e74c3c;
            --warning: #f39c12;
            --dark: #2c3e50;
            --light: #ecf0f1;
            --gray: #95a5a6;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background-color: #f5f7fa;
            color: var(--dark);
        }
        
        .container {
            display: grid;
            grid-template-columns: 250px 1fr;
            min-height: 100vh;
        }
        
        /* Sidebar Styles */
        .sidebar {
            background-color: var(--dark);
            color: var(--light);
            padding: 1.5rem;
            box-shadow: 2px 0 10px rgba(0,0,0,0.1);
        }
        
        .logo {
            display: flex;
            align-items: center;
            margin-bottom: 2rem;
            padding-bottom: 1rem;
            border-bottom: 1px solid rgba(255,255,255,0.1);
        }
        
        .logo img {
            width: 40px;
            margin-right: 10px;
        }
        
        .logo h2 {
            font-size: 1.2rem;
            font-weight: 500;
        }
        
        .nav-menu {
            list-style: none;
        }
        
        .nav-item {
            margin-bottom: 0.5rem;
        }
        
        .nav-link {
            display: flex;
            align-items: center;
            padding: 0.8rem 1rem;
            color: var(--light);
            text-decoration: none;
            border-radius: 5px;
            transition: all 0.3s;
        }
        
        .nav-link:hover, .nav-link.active {
            background-color: rgba(255,255,255,0.1);
            color: white;
        }
        
        .nav-link i {
            margin-right: 10px;
            font-size: 1.2rem;
        }
        
        /* Main Content Styles */
        .main-content {
            padding: 2rem;
            overflow-y: auto;
        }
        
        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            margin-bottom: 2rem;
        }
        
        .page-title h1 {
            font-size: 1.8rem;
            font-weight: 500;
            color: var(--dark);
        }
        
        .page-title p {
            color: var(--gray);
            font-size: 0.9rem;
        }
        
        .user-profile {
            display: flex;
            align-items: center;
        }
        
        .user-profile img {
            width: 40px;
            height: 40px;
            border-radius: 50%;
            margin-right: 10px;
        }
        
        /* Settings Tabs */
        .settings-tabs {
            display: flex;
            border-bottom: 1px solid #ddd;
            margin-bottom: 2rem;
            flex-wrap: wrap;
        }
        
        .settings-tab {
            padding: 0.8rem 1.5rem;
            background: none;
            border: none;
            cursor: pointer;
            font-size: 1rem;
            font-weight: 500;
            color: var(--gray);
            position: relative;
            transition: all 0.3s;
        }
        
        .settings-tab.active {
            color: var(--dark);
        }
        
        .settings-tab.active::after {
            content: '';
            position: absolute;
            bottom: -1px;
            left: 0;
            width: 100%;
            height: 3px;
            background-color: var(--primary);
        }
        
        /* Settings Content */
        .settings-content {
            display: none;
            background-color: white;
            border-radius: 10px;
            padding: 2rem;
            box-shadow: 0 4px 6px rgba(0,0,0,0.05);
            margin-bottom: 2rem;
        }
        
        .settings-content.active {
            display: block;
            animation: fadeIn 0.5s;
        }
        
        @keyframes fadeIn {
            from { opacity: 0; }
            to { opacity: 1; }
        }
        
        /* Form Styles */
        .form-group {
            margin-bottom: 1.5rem;
        }
        
        .form-label {
            display: block;
            margin-bottom: 0.5rem;
            font-weight: 500;
        }
        
        .form-control {
            width: 100%;
            padding: 0.8rem;
            border: 1px solid #ddd;
            border-radius: 5px;
            font-size: 1rem;
            transition: border 0.3s;
        }
        
        .form-control:focus {
            outline: none;
            border-color: var(--primary);
        }
        
        .form-row {
            display: flex;
            gap: 1rem;
            margin-bottom: 1rem;
        }
        
        .form-col {
            flex: 1;
        }
        
        /* Toggle Switch */
        .switch {
            position: relative;
            display: inline-block;
            width: 60px;
            height: 34px;
        }
        
        .switch input {
            opacity: 0;
            width: 0;
            height: 0;
        }
        
        .slider {
            position: absolute;
            cursor: pointer;
            top: 0;
            left: 0;
            right: 0;
            bottom: 0;
            background-color: #ccc;
            transition: .4s;
            border-radius: 34px;
        }
        
        .slider:before {
            position: absolute;
            content: "";
            height: 26px;
            width: 26px;
            left: 4px;
            bottom: 4px;
            background-color: white;
            transition: .4s;
            border-radius: 50%;
        }
        
        input:checked + .slider {
            background-color: var(--primary);
        }
        
        input:checked + .slider:before {
            transform: translateX(26px);
        }
        
        /* Button Styles */
        .btn {
            padding: 0.8rem 1.5rem;
            border: none;
            border-radius: 5px;
            background-color: var(--primary);
            color: white;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .btn:hover {
            background-color: #2980b9;
            transform: translateY(-2px);
        }
        
        .btn-secondary {
            background-color: var(--gray);
        }
        
        .btn-secondary:hover {
            background-color: #7f8c8d;
        }
        
        .btn-danger {
            background-color: var(--danger);
        }
        
        .btn-danger:hover {
            background-color: #c0392b;
        }
        
        /* Table Styles */
        .table {
            width: 100%;
            border-collapse: collapse;
            margin: 1rem 0;
        }
        
        .table th, .table td {
            padding: 0.8rem;
            text-align: left;
            border-bottom: 1px solid #ddd;
        }
        
        .table th {
            background-color: #f2f2f2;
            font-weight: 500;
        }
        
        .table tr:hover {
            background-color: #f5f5f5;
        }
        
        /* Alert Styles */
        .alert {
            padding: 1rem;
            border-radius: 5px;
            margin-bottom: 1rem;
        }
        
        .alert-success {
            background-color: #d4edda;
            color: #155724;
        }
        
        .alert-danger {
            background-color: #f8d7da;
            color: #721c24;
        }
        
        /* Responsive Styles */
        @media (max-width: 992px) {
            .container {
                grid-template-columns: 1fr;
            }
            
            .sidebar {
                display: none;
            }
            
            .settings-tabs {
                flex-direction: column;
            }
            
            .settings-tab {
                text-align: left;
                border-bottom: 1px solid #eee;
            }
            
            .form-row {
                flex-direction: column;
                gap: 0;
            }
        }
    </style>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
</head>
<body>
    <div class="container">
        <!-- Sidebar -->
        <div class="sidebar">
            <div class="logo">
                <img src="https://via.placeholder.com/40x40?text=SG" alt="SecureGuard Logo">
                <h2>SecureGuard-Pro</h2>
            </div>
            
            %NAVIGATION%
            
        </div>
        
        <!-- Main Content -->
        <div class="main-content">
            <div class="header">
                <div class="page-title">
                    <h1>Alerts History</h1>
                    <p>View and manage your system alerts</p>
                </div>
                
                <div class="user-profile">
                    <img src="https://via.placeholder.com/40x40?text=EM" alt="User Profile">
                    <span>Emma</span>
                </div>
            </div>
            
            <!-- Alerts content would go here -->
            <div class="alerts-container">
                <!-- Alert items would be dynamically inserted here -->
            </div>
        </div>
    </div>
    <script>
        function confirmLogout(event) {
        event.preventDefault();
        window.location.href = '/logout';
    }
    </script>
</body>
</html>
)rawliteral";

const char *logoutPage = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>SecureGuard-Pro Logout</title>
    <style>
        :root {
            --primary: #3498db;
            --danger: #e74c3c;
            --dark: #2c3e50;
            --light: #ecf0f1;
        }
        
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
        }
        
        body {
            background-color: var(--dark);
            color: var(--light);
            display: flex;
            justify-content: center;
            align-items: center;
            min-height: 100vh;
            text-align: center;
        }
        
        .logout-container {
            background-color: rgba(255,255,255,0.1);
            padding: 2rem;
            border-radius: 10px;
            width: 90%;
            max-width: 400px;
            box-shadow: 0 10px 25px rgba(0,0,0,0.2);
        }
        
        .logo {
            width: 80px;
            margin-bottom: 1.5rem;
        }
        
        h1 {
            margin-bottom: 1.5rem;
            color: white;
        }
        
        p {
            margin-bottom: 2rem;
        }
        
        .button-group {
            display: flex;
            gap: 1rem;
        }
        
        .btn {
            flex: 1;
            padding: 0.8rem;
            border: none;
            border-radius: 5px;
            color: white;
            font-size: 1rem;
            cursor: pointer;
            transition: all 0.3s;
        }
        
        .btn-logout {
            background-color: var(--danger);
        }
        
        .btn-cancel {
            background-color: var(--primary);
        }
        
        .btn:hover {
            opacity: 0.9;
            transform: translateY(-2px);
        }
    </style>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.0.0-beta3/css/all.min.css">
</head>
<body>
    <div class="logout-container">
        <img src="https://via.placeholder.com/80x80?text=SG" alt="SecureGuard Logo" class="logo">
        <h1>Logout</h1>
        <p>Are you sure you want to logout from SecureGuard-Pro?</p>
        
        <div class="button-group">
            <a href="/do-logout" class="btn btn-logout">
                <i class="fas fa-sign-out-alt"></i> Yes, Logout
            </a>
            <a href="/dashboard" class="btn btn-cancel">
                <i class="fas fa-times"></i> Cancel
            </a>
        </div>
    </div>
</body>
</html>
)rawliteral";

String getNavigation(const String &activePage) {
  String nav = navigation;
  nav.replace("%DASHBOARD_ACTIVE%", activePage == "dashboard" ? "active" : "");
  nav.replace("%SETTINGS_ACTIVE%", activePage == "settings" ? "active" : "");
  nav.replace("%ALERTS_ACTIVE%", activePage == "alerts" ? "active" : "");
  nav.replace("%HELP_ACTIVE%", activePage == "help" ? "active" : "");
  nav.replace("%LOGOUT_ACTIVE%", activePage == "logout" ? "active" : "");
  return nav;
}


void handleSensorData(AsyncWebServerRequest *request) {
  DynamicJsonDocument doc(512);
  doc["temperature"] = readTemperature();
  doc["soundLevel"] = analogRead(SOUND_SENSOR_PIN);
  doc["lightLevel"] = analogRead(LDR_PIN);
  doc["alarmEnabled"] = config.alarmEnabled;
  doc["tempWarning"] = config.tempWarning;
  doc["tempCritical"] = config.tempCritical;
  doc["soundThreshold"] = config.soundThreshold;
  doc["lightThreshold"] = config.lightThreshold;
  doc["time"] = millis() / 1000;
  doc["uptime"] = millis() / 1000;

  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void handleLogout(AsyncWebServerRequest *request) {
  // This will clear the authentication and redirect to login
  request->send(401, "text/html", "<script>window.location.href='/login';</script>");
}

void handleBuzzer(AsyncWebServerRequest *request) {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000);
  digitalWrite(BUZZER_PIN, LOW);
  request->send(200, "application/json", "{\"success\":true}");
}

void handleAlarmToggle(AsyncWebServerRequest *request) {
  config.alarmEnabled = !config.alarmEnabled;
  DynamicJsonDocument doc(128);
  doc["success"] = true;
  doc["alarmEnabled"] = config.alarmEnabled;
  String json;
  serializeJson(doc, json);
  request->send(200, "application/json", json);
}

void handleLEDControl(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    String body = request->arg("plain");
    DynamicJsonDocument doc(128);
    deserializeJson(doc, body);

    int r = doc["r"];
    int g = doc["g"];
    int b = doc["b"];

    analogWrite(RED_PIN, r);
    analogWrite(GREEN_PIN, g);
    analogWrite(BLUE_PIN, b);

    request->send(200, "application/json", "{\"success\":true}");
  }
}

void handleSecuritySettings(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    String body = request->arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);

    config.alarmEnabled = doc["alarmEnabled"];
    config.soundThreshold = doc["soundThreshold"];

    request->send(200, "application/json", "{\"success\":true}");
  }
}

void handleTempSettings(AsyncWebServerRequest *request) {
  if (request->method() == HTTP_POST) {
    String body = request->arg("plain");
    DynamicJsonDocument doc(256);
    deserializeJson(doc, body);

    config.tempWarning = doc["tempWarning"];
    config.tempCritical = doc["tempCritical"];
    config.lightThreshold = doc["lightThreshold"];

    request->send(200, "application/json", "{\"success\":true}");
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);  // Give serial monitor time to initialize

  // Initialize pins
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(SOUND_SENSOR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(TEMP_SENSOR_PIN, INPUT);

  // Set initial LED state (off)
  analogWrite(RED_PIN, 0);
  analogWrite(GREEN_PIN, 0);
  analogWrite(BLUE_PIN, 0);

  // Create WiFi Access Point
  Serial.println("\nCreating Access Point...");
  if (!WiFi.softAP("SecureGuard-Pro", "Em.ma.45")) {
    Serial.println("Failed to create Access Point");
    while (true)
      ;  // Halt if AP creation fails
  }

  Serial.println("Access Point Created");
  Serial.print("SSID: SecureGuard-Pro\n");
  Serial.print("Password: Em.ma.45\n");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Configure server routes
  // Root URL - redirect to login
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->authenticate("Emma", "Em.ma.45")) {
      request->redirect("/dashboard");
    } else {
      request->send(200, "text/html", loginPage);
    }
  });

  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
    String username = request->arg("username");
    String password = request->arg("password");

    if (username == "Emma" && password == "Em.ma.45") {
      // Redirect to dashboard on successful login
      request->redirect("/dashboard");
    } else {
      request->send(401, "text/plain", "Invalid credentials");
    }
  });

  // Dashboard page
  server.on("/dashboard", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate("Emma", "Em.ma.45")) {
      return request->requestAuthentication();
    }
    String page = dashboardPage;
    page.replace("%NAVIGATION%", getNavigation("dashboard"));
    request->send(200, "text/html", page);
  });

  // Settings page
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate("Emma", "Em.ma.45")) {
      return request->requestAuthentication();
    }
    String page = settingsPage;
    page.replace("%NAVIGATION%", getNavigation("settings"));
    request->send(200, "text/html", page);
  });

  // Help page
  server.on("/help", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate("Emma", "Em.ma.45")) {
      return request->requestAuthentication();
    }
    String page = helpPage;
    page.replace("%NAVIGATION%", getNavigation("help"));
    request->send(200, "text/html", page);
  });

  server.on("/alerts", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!request->authenticate("Emma", "Em.ma.45")) {
      return request->requestAuthentication();
    }
    String page = alertsPage;
    page.replace("%NAVIGATION%", getNavigation("alerts"));
    request->send(200, "text/html", page);

    // Add this with your other server handlers
    server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/html", logoutPage);
    });

    server.on("/do-logout", HTTP_GET, [](AsyncWebServerRequest *request) {
      // Here you would add any cleanup code needed
      request->redirect("/");
    });
  });

  // API Endpoints
  server.on("/api/sensor", HTTP_GET, handleSensorData);
  server.on("/api/buzzer", HTTP_POST, handleBuzzer);
  server.on("/api/alarm/toggle", HTTP_POST, handleAlarmToggle);
  server.on("/api/led", HTTP_POST, handleLEDControl);
  server.on("/api/settings/security", HTTP_POST, handleSecuritySettings);
  server.on("/api/settings/temperature", HTTP_POST, handleTempSettings);

  // Handle 404 Not Found
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Page Not Found");
  });

  // Start server
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("System ready. Connect to:");
  Serial.println("http://" + WiFi.softAPIP().toString());
  Serial.println("Username: Emma");
  Serial.println("Password: Em.ma.45");
}
void loop() {
  // System monitoring and control
  float temp = readTemperature();
  int soundLevel = analogRead(SOUND_SENSOR_PIN);
  int lightLevel = analogRead(LDR_PIN);

  // Visual indicators based on sensor readings
  if (temp >= config.tempCritical) {
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 0);
  } else if (temp >= config.tempWarning) {
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 100);
    analogWrite(BLUE_PIN, 0);
  } else if (soundLevel > config.soundThreshold) {
    analogWrite(RED_PIN, 255);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 255);
  } else if (lightLevel < config.lightThreshold) {
    analogWrite(RED_PIN, 0);
    analogWrite(GREEN_PIN, 0);
    analogWrite(BLUE_PIN, 100);
  }

  // Buzzer control
  if ((temp >= config.tempCritical || soundLevel > config.soundThreshold) && config.alarmEnabled) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(3000);
    digitalWrite(BUZZER_PIN, LOW);
  }

  delay(config.updateInterval * 1000);
}
