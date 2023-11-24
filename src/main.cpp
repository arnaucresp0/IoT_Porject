#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncTCP.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <UrlEncode.h>
#include <Hash.h>




// Replace with your network credentials
const char* ssid = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";
//Replace with your country code (i.e. +34) and phone number
String phoneNumber = "+34722254551"; 
String apiKey = "1084263";

//DHT
#define DHTPIN 5     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//MOISTURE PIN
#define SOIL_MOISTURE_PIN A0
//PIR SENSOR
#define PIRPIN 4

DHT dht(DHTPIN, DHTTYPE);

// current temperature & humidity, updated in loop()
float t = 0.0;
float h = 0.0;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;    // will store last time DHT was updated

// Updates DHT readings every 10 seconds
const long interval = 10000;  

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head><meta http-equiv="cache-control" content="no-cache"><meta http-equiv="Pragma" content="no-cache"><meta http-equiv="Expires" content="-1"><style type="text/css">body {
    font-family: Arial, sans-serif;
    text-align: center;
}

.container {
    margin: 0 auto;
    max-width: auto;
    padding: 25px;
    border: 2px solid #000000;
    border-radius: 10px;
    background-color: #ffac9c;
}

h1 {
    color: #333;
}

.temperature {
    font-size: 35px;
}

.moisture {
    font-size: 35px;
}

.presence {
    font-size: 35px;
}

.symbol {
    font-size: 100px;
}

*,
*:before,
*:after {
  box-sizing: border-box;
}

body {
  font-family: -apple-system, ".SFNSText-Regular", "Helvetica Neue", "Roboto", "Segoe UI", sans-serif;
}

.toggle {
  cursor: pointer;
  display: inline-block;
}

.toggle-switch {
  display: inline-block;
  background: #ccc;
  border-radius: 16px;
  width: 58px;
  height: 32px;
  position: relative;
  vertical-align: middle;
  transition: background 0.25s;
}
.toggle-switch:before, .toggle-switch:after {
  content: "";
}
.toggle-switch:before {
  display: block;
  background: linear-gradient(to bottom, #fff 0%, #eee 100%);
  border-radius: 50%;
  box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.25);
  width: 24px;
  height: 24px;
  position: absolute;
  top: 4px;
  left: 4px;
  transition: left 0.25s;
}
.toggle:hover .toggle-switch:before {
  background: linear-gradient(to bottom, #fff 0%, #fff 100%);
  box-shadow: 0 0 0 1px rgba(0, 0, 0, 0.5);
}
.toggle-checkbox:checked + .toggle-switch {
  background: #56c080;
}
.toggle-checkbox:checked + .toggle-switch:before {
  left: 30px;
}

.toggle-checkbox {
  position: absolute;
  visibility: hidden;
}

.toggle-label {
  margin-left: 5px;
  position: relative;
  top: 2px;
}


.waterButton {
  background: #fff;
  backface-visibility: hidden;
  border-radius: .375rem;
  border-style: solid;
  border-width: .125rem;
  box-sizing: border-box;
  color: #212121;
  cursor: pointer;
  display: inline-block;
  font-family: Circular,Helvetica,sans-serif;
  font-size: 1.125rem;
  font-weight: 700;
  letter-spacing: -.01em;
  line-height: 1.3;
  padding: .875rem 1.125rem;
  position: relative;
  text-align: left;
  text-decoration: none;
  transform: translateZ(0) scale(1);
  transition: transform .2s;
  user-select: none;
  -webkit-user-select: none;
  touch-action: manipulation;
}

.waterButton:not(:disabled):hover {
  transform: scale(1.05);
}

.waterButton:not(:disabled):hover:active {
  transform: scale(1.05) translateY(.125rem);
}

.waterButton:focus {
  outline: 0 solid transparent;
}

.waterButton:focus:before {
  content: "";
  left: calc(-1*.375rem);
  pointer-events: none;
  position: absolute;
  top: calc(-1*.375rem);
  transition: border-radius;
  user-select: none;
}

.waterButton:focus:not(:focus-visible) {
  outline: 0 solid transparent;
}

.waterButton:focus:not(:focus-visible):before {
  border-width: 0;
}

.waterButton:not(:disabled):active {
  transform: translateY(.125rem);
}

.alarmButton {
  background: #fff;
  backface-visibility: hidden;
  border-radius: .375rem;
  border-style: solid;
  border-width: .125rem;
  box-sizing: border-box;
  color: #212121;
  cursor: pointer;
  display: inline-block;
  font-family: Circular,Helvetica,sans-serif;
  font-size: 1.125rem;
  font-weight: 700;
  letter-spacing: -.01em;
  line-height: 1.3;
  padding: .875rem 1.125rem;
  position: relative;
  text-align: left;
  text-decoration: none;
  transform: translateZ(0) scale(1);
  transition: transform .2s;
  user-select: none;
  -webkit-user-select: none;
  touch-action: manipulation;
}

.alarmButton:not(:disabled):hover {
  transform: scale(1.05);
}

.alarmButton:not(:disabled):hover:active {
  transform: scale(1.05) translateY(.125rem);
}

.alarmButton:focus {
  outline: 0 solid transparent;
}

.alarmButton:focus:before {
  content: "";
  left: calc(-1*.375rem);
  pointer-events: none;
  position: absolute;
  top: calc(-1*.375rem);
  transition: border-radius;
  user-select: none;
}

.alarmButton:focus:not(:focus-visible) {
  outline: 0 solid transparent;
}

.alarmButton:focus:not(:focus-visible):before {
  border-width: 0;
}

.alarmButton:not(:disabled):active {
  transform: translateY(.125rem);
}</style>


    <title>IOT PROJECT</title>
    <link rel="stylesheet" trype="text/css" href="styles.css">
</head>

<body>
    <div class="container">
        <h1>PLANT MONITORING AND WATERING WEB</h1>
        <p>Temperature: <span class="temperature">{{celsiusTemp}} °C</span></p>
        <p>Moisture: <span class="moisture">{{humidity}} %</span></p>
        <p>Presence: <span class="presence">{{presence}}</span></p>
        <p class="symbol" id="plantSymbol">🌵</p>
        
        <h2>Actions:</h2>
        <button class="waterButton" onclick="waterPlant()">Water the plant</button>
        <button class="alarmButton" onclick="alarmPlant()">Activate the alarm</button>
        <p></p>
        <div>
            <label class="toggle">
            <input class="toggle-checkbox" type="checkbox" id="whatsappToggle" onclick="toggleWhatsAppNotifications()">
            <div class="toggle-switch"></div>
            <span class="toggle-label">WhatsApp Notifications</span>
          </label>
          
        </div>
        <style>
            .spacer {
                margin-bottom: 60px; /* Adjust the value as needed */
            }
        </style>
        <div class="spacer"></div>
        <div class="footer">©<span id="year"> </span><span> Universitat politècnica de Catalunya. All rights reserved.</span></div>
    </div>


<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;


setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>
)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  return String();
}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  dht.begin();
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println(".");
  }

  // Print ESP8266 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(t).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(h).c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){  
  unsigned long currentMillis = millis();
  //Update the readings every 10 seconds
  if (currentMillis - previousMillis >= interval) {
    // save the last time you updated the DHT values
    previousMillis = currentMillis;
    //-----------------DHT----------------
    // Read temperature as Celsius (the default)
    float newT = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    //float newT = dht.readTemperature(true);
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
    //-----------------SOIL_MOISTURE----------------
    // Read the value from the analog sensor:
    int sensorValue = analogRead(SOIL_MOISTURE_PIN);
    Serial.print("Sensor value: ");
    Serial.println(sensorValue);

    if (sensorValue < 500){
      Serial.println("No need for watering");
    } 
    else {
      Serial.println("Time to water your plant");
    }
    //----------------PIR_SENSOR-----------------
    int val = digitalRead(PIRPIN);
    if (val == HIGH) {    // Comprobar si el sensor está en HIGH
      String Presence = "Detectada";
      //definir variable         
    }
    else {
      String Presence = "No Detectada";
    }
  }
  //Check for sending messages with WhatsApp.
}


void sendMessage(String message){
  // Data to send with HTTP POST
  String url = "http://api.callmebot.com/whatsapp.php?phone=" + phoneNumber + "&apikey=" + apiKey + "&text=" + urlEncode(message);
  WiFiClient client;    
  HTTPClient http;
  http.begin(client, url);

  // Specify content-type header
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  // Send HTTP POST request
  int httpResponseCode = http.POST(url);
  if (httpResponseCode == 200){
    Serial.print("Message sent successfully");
  }
  else{
    Serial.println("Error sending the message");
    Serial.print("HTTP response code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
}
