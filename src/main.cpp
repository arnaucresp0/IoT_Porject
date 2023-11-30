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
//WATER BOMB PIN
#define WATER_BOMB_PIN 3
//ALARM PIN
#define ALARM_PIN 2

DHT dht(DHTPIN, DHTTYPE);

// ---------------------------LOOP_VARIABLES-------------------------

//Temperature variable
float t = 0.0;
//Relative air humidity variable
float h = 0.0;
//Soil moisture variable
float sh = 0.0;
//Presence variable
String Presence = "No Detectada";
//Indicates if it is need to water the plant for WhatsApp notification function
bool WaterAlert = false;
// Updates DHT readings every 10 seconds
const long interval = 10000;  
// will store last time DHT was updated
unsigned long previousMillis = 0;  
// These two variables set the equation to convert from analog reading to %  
float m = 67.57;
float n = 70.27;
//These three string variables are for displaying the WhatsApp
String tempAlert;
String SoilAlert;
String PresenceAlert;
//Alert counter to send WhApp messages every 30 sec.
int AlertCounter = 3;
//Automatic mode variable.
bool AutoModeVar;
//Variable that enable the WApp notifications.
bool whatsappNotificationsEnabled = false;
//Variable that enable the Alarm system.
bool alarmSystemEnabled = false;
unsigned long waterStartTime = 0;
const unsigned long waterDuration = 3000; // milliseconds


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Full HTML & CSS & JavaScript code
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
    font-size: 30px;
}

.moisture {
    font-size: 30px;
}

.soil_moisture{
  font-size: 30;
}

.presence {
    font-size: 30px;
}

.symbol {
    font-size: 50px;
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
        <p>Soil Moisture: <span class= "soil_moisture">{{soil}}%</span></p>
        <p>Presence: <span class="presence">{{presence}}</span></p>
        <p class="symbol" id="plantSymbol">🌵</p>
        <h2>AUTOMATIC MODE:</h2>
        <div>
            <label class="toggle">
            <input class="toggle-checkbox" type="checkbox" id="Mode" onclick="toggleMode()">
            <div class="toggle-switch"></div>
            <span class="toggle-label">Automatic Mode</span>
          </label>
        </div>
        <h2>MANUAL ACTIONS:</h2>
        <button class="waterButton" onclick="waterPlant()">Water the plant</button>
        <p></p>
        <div>
            <label class="toggle">
            <input class="toggle-checkbox" type="checkbox" id="AlarmToggle" onclick="toggleAlarmSystem()">
            <div class="toggle-switch"></div>
            <span class="toggle-label">Enable alarm system</span>
          </label>
        </div>
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
                margin-bottom: 40px; /* Adjust the value as needed */
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
      document.getElementById("soil_moisture").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/soil_moisture", true);
  xhttp.send();
}, 10000 ) ;

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

function toggleWhatsAppNotifications(element) {
  var xhttp = new XMLHttpRequest();
  if(element.checked){ xhttp.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhttp.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhttp.send();
}

function toggleAlarmSystem(element) {
  var xhttp = new XMLHttpRequest();
  if(element.checked){ xhttp.open("GET", "/update?output="+element.id+"&state=1", true); }
  else { xhttp.open("GET", "/update?output="+element.id+"&state=0", true); }
  xhttp.send();
}

function waterPlant() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      // Optionally, you can handle the response from the Arduino here
      console.log("Arduino response:", this.responseText);
    }
  };
  xhttp.open("GET", "/controlWater?state=true", true); // Adjust the endpoint "/controlWater" as needed
  xhttp.send();
}

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
  else if(var == "SOIL_MOISTURE")
    return String(sh);

  return String();
}

//Function declarations:

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

void AlertManager(){
  if (whatsappNotificationsEnabled == true){
    //This counter is to send WhatsApp notifications every 30 seconds.
    if (AlertCounter == 0){
      //Check the temperature
      if(t >= 25.0){
        tempAlert = "La termperatura està per sobre de 25ºC."; 
      }
      else if( t < 10.0){
        tempAlert = "La temperatura està per sota de 10ªC.";
      }
      else{
        tempAlert = "La temperatura és adecuada.";
      }
      //Check the soil moisture
      if(sh < 500){
        SoilAlert = "La planta necessita aigua.";
      }
      else{
        SoilAlert = "La planta està ben regada.";
      }
      //Check the presence
      if(Presence = "Detectada"){
        PresenceAlert = "Compta s'ha detectat una presència.";
      } 
      sendMessage(tempAlert);
      sendMessage(SoilAlert);
      sendMessage(PresenceAlert);}
    else{
      AlertCounter--;
    }
  }
}

void AlarmOn(){
  if (alarmSystemEnabled == true) {
      while(millis() < 3000){
        digitalWrite(ALARM_PIN, HIGH);
        delay(50);
        digitalWrite(ALARM_PIN,LOW);
      }
  
  }
} 

void PlantWatering(){
  if (millis() - waterStartTime < waterDuration) {
    // Activate the water bomb pin
    digitalWrite(WATER_BOMB_PIN, HIGH);
  } else {
    // Deactivate the water bomb pin
    digitalWrite(WATER_BOMB_PIN, LOW);
  }
}

void AutoMode(){
  //Set the automatic mode for watering and alarm usage.
  if(AutoModeVar == true){
    if (sh <= 500){
      PlantWatering();
    }
    if(Presence == "Detectada"){
      AlarmOn();
    }
  }
}

//SETUP
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
    server.on("/soil_moisture", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(sh).c_str());
  });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
  String output = request->arg("output");
  String state = request->arg("state");
  if (output == "whatsappToggle") {
    whatsappNotificationsEnabled = (state == "1");
  }
  request->send(200, "text/plain", "OK");
  });

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    String output = request->arg("output");
    String state = request->arg("state");

    // Assuming you have an output named "AlarmToggle"
    if (output == "AlarmToggle") {
      // Update the variable based on the state received
      alarmSystemEnabled = (state == "1");
    }

    request->send(200, "text/plain", "OK");
  });

  server.on("/controlWater", HTTP_GET, [](AsyncWebServerRequest *request){
  String state = request->arg("state");

  if (state == "true") {
    waterStartTime = millis();
  }

  request->send(200, "text/plain", "OK");
  });

  
  // Start server
  server.begin();
}
//MAIN
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
    float sensorValue = analogRead(SOIL_MOISTURE_PIN);
    //Transform the analog calue to %
    sh = (m * sensorValue - n);

    //----------------PIR_SENSOR-----------------
    int val = digitalRead(PIRPIN);
    if (val == HIGH) {    // Comprobar si el sensor está en HIGH
       Presence = "Detectada";        
    }
    else {
       Presence = "No Detectada";
    }
    AlertManager();

  }
}



