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

//hola

// Replace with your network credentials
const char* ssid = "dragino-1c0ad8";
const char* password = "dragino-dragino";
//const char* ssid = "Fibracat_10156";
//const char* password = "9f6496aa11";
//Replace with your country code (i.e. +34) and phone number
String phoneNumber = "+34722254551"; 
String apiKey = "1084263";

//DHT
#define DHTPIN 5              // (PIN D1 ESP8266)
#define DHTTYPE    DHT11      // DHT 11 (AM2302)
//MOISTURE PIN
#define SOIL_MOISTURE_PIN A0  // (PIN A0 ESP8266)
//PIR SENSOR
#define PIRPIN 4              // (PIN D2 ESP8266)
//WATER BOMB PIN
#define WATER_BOMB_PIN 0      // (PIN D3 ESP8266)
//ALARM PIN
#define ALARM_PIN 12          // (PIN D6 ESP8266)

DHT dht(DHTPIN, DHTTYPE);

// ---------------------------LOOP_VARIABLES-------------------------

//Temperature variable
float t = 0.0;
//Relative air humidity variable
float h = 0.0;
//Soil moisture variable
float sh = 0.0;
//Presence variable
String Presence = "Detectada";
//Indicates if it is need to water the plant for WhatsApp notification function
bool WaterAlert = false;
// Updates DHT readings every 10 seconds
const long interval = 10000;  
// will store last time DHT was updated
unsigned long previousMillis = 0;  
// These two variables set the equation to convert from analog reading to %  
float m = -0.23;
float n = 173.515;
//These three string variables are for displaying the WhatsApp
String tempAlert;
String SoilAlert;
String PresenceAlert;
//Alert counter to send WhApp messages every 30 sec.
int AlertCounter = 6;
//Automatic mode variable.
bool AutoModeVar;
//Variable that enable the WApp notifications.
bool whatsappNotificationsEnabled = false;
//Variable that enable the Alarm system.
bool alarmSystemEnabled = false;
unsigned long AlarmStartTime = 0;
const unsigned long AlarmDuration = 3000; // milliseconds
unsigned long waterStartTime = 0;
const unsigned long waterDuration = 3000; // milliseconds


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//Full HTML & CSS & JavaScript code
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>

<head>
    <meta http-equiv="cache-control" content="no-cache">
    <meta http-equiv="Pragma" content="no-cache">
    <meta http-equiv="Expires" content="-1">
    <style type="text/css">
        body {
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

        .soil_moisture {
            font-size: 30px;
        }

        .presence {
            font-size: 30px;
        }

        .symbol {
            font-size: 40px;
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

        .toggle-switch:before,
        .toggle-switch:after {
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

        .toggle-checkbox:checked+.toggle-switch {
            background: #56c080;
        }

        .toggle-checkbox:checked+.toggle-switch:before {
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
            font-family: Circular, Helvetica, sans-serif;
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
            font-family: Circular, Helvetica, sans-serif;
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
        }
    </style>

    <title>IOT PROJECT</title>
    <link rel="stylesheet" trype="text/css" href="styles.css">
    <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
    <link rel="icon" href="data:,">
</head>

<body>
    <div class="container">
        <h1>PLANT MONITORING AND WATERING WEB</h1>
        <p>Temperature: <span  id="temperature" class="temperature">%TEMPERATURE%</span></p>
        <p>Moisture: <span  id="humidity" class="humidity">%HUMIDITY%</span></p>
        <p>Soil Moisture: <span id="soil_moisture" class="soil_moisture">%SOIL_MOISTURE%</span></p>
        <p>Presence: <span id= "presence" class="presence">%PRESENCE%</span></p>
        <p class="symbol" id="plantSymbol"> <i class="fab fa-pagelines" style="color: #24bc52;"></i> </p>
        <h2>AUTOMATIC MODE:</h2>
        <div>
            <label class="toggle">
                <input class="toggle-checkbox" type="checkbox" id="Mode" onclick="toggleAutoMode(this)">
                <div class="toggle-switch"></div>
                <span class="toggle-label">Automatic Mode</span>
            </label>
        </div>
        <h2>MANUAL ACTIONS:</h2>
        <button class="waterButton" onclick="waterPlant()">Water the plant</button>
        <p></p>
        <div>
          <label class="toggle">
              <input class="toggle-checkbox" type="checkbox" id="AlarmToggle" onclick="toggleAlarmSystem(this)">
              <div class="toggle-switch"></div>
              <span class="toggle-label">Enable alarm system</span>
          </label>
        </div>
        <p></p>
        <div>
            <label class="toggle">
                <input class="toggle-checkbox" type="checkbox" id="whatsappToggle" onclick="toggleWhatsAppNotifications(this)">
                <div class="toggle-switch"></div>
                <span class="toggle-label">WhatsApp Notifications</span>
            </label>
        </div>
        <style>
            .spacer {
                margin-bottom: 20px;
            }
        </style>
        <div class="spacer"></div>
        <div class="footer">©<span id="year"> </span><span> Universitat politecnica de Catalunya. All rights
                reserved.</span></div>
      </div>
    
    <script>
      setInterval ( function () {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function () {
              if (this.readyState == 4 && this.status == 200) {
                  document.getElementById("temperature").innerHTML = this.responseText;
              }
          };
          xhttp.open("GET", "/temperature", true);
          xhttp.send();
      }, 5000 );

      setInterval ( function () {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function () {
              if (this.readyState == 4 && this.status == 200) {
                  document.getElementById("humidity").innerHTML = this.responseText;
              }
          };
          xhttp.open("GET", "/humidity", true);
          xhttp.send();
      }, 5000 );

      setInterval ( function () {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function () {
              if (this.readyState == 4 && this.status == 200) {
                  document.getElementById("soil_moisture").innerHTML = this.responseText;
              }
          };
          xhttp.open("GET", "/soil_moisture", true);
          xhttp.send();
      }, 5000 );

      setInterval ( function () {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function () {
              if (this.readyState == 4 && this.status == 200) {
                  document.getElementById("presence").innerHTML = this.responseText;
              }
          };
          xhttp.open("GET", "/presence", true);
          xhttp.send();
      }, 2000 );

      function toggleWhatsAppNotifications(element) {
        var xhttp = new XMLHttpRequest();
        if (element.checked) {
          xhttp.open("GET", "/update?output=whatsappToggle&state=1", true);
        } else {
          xhttp.open("GET", "/update?output=whatsappToggle&state=0", true);
        }
        xhttp.send();
      }

      function toggleAlarmSystem(element) {
        var xhttp = new XMLHttpRequest();
        if (element.checked) {
          xhttp.open("GET", "/update?output=AlarmToggle&state=1", true);
        } else {
          xhttp.open("GET", "/update?output=AlarmToggle&state=0", true);
        }
        xhttp.send();
      }

      function toggleAutoMode(element) {
        var xhttp = new XMLHttpRequest();
        if (element.checked) {
          xhttp.open("GET", "/update?output=Mode&state=1", true);
        } else {
          xhttp.open("GET", "/update?output=Mode&state=0", true);
        }
        xhttp.send();
      }

      function waterPlant() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function () {
              if (this.readyState == 4 && this.status == 200) {
                  console.log("Arduino response:", this.responseText);
              }
          };
          xhttp.open("GET", "/controlWater?state=true", true); 
          xhttp.send();
      }
    </script>
</body>
</html>
)rawliteral";

// Replaces placeholder with sensor values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return String(t);
  }
  else if(var == "HUMIDITY"){
    return String(h);
  }
  else if(var == "SOIL_MOISTURE"){
    return String(sh);
  }
  else if (var == "PRESENCE"){
    return String(Presence);
  }
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
    Serial.println("Notificacions habilitades.");
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
      if(sh < 50){
        SoilAlert = "La planta necessita aigua.";
      }
      else{
        SoilAlert = "La planta està ben regada.";
      }
      //Check the presence
      if(Presence = "Detectada"){
        PresenceAlert = "Compta s'ha detectat una presència.";
      }
      else{
        PresenceAlert = "La planta està tranquila.";
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
      Serial.println("Alarma habilitada.");
      if (millis() - AlarmStartTime < AlarmDuration) {
        // Activate the water bomb pin
        tone(ALARM_PIN, 500);
        Serial.println("Alarm ringing !");
      } 
      else{
        // Deactivate the water bomb pin
        noTone(ALARM_PIN);
      }
  }
  else{
    noTone(ALARM_PIN);
  }
} 

void PlantWatering(){
  if (millis() - waterStartTime < waterDuration) {
    // Activate the water bomb pin
    digitalWrite(WATER_BOMB_PIN, HIGH);
    Serial.println("Water Bomb activated!");
  } else {
    // Deactivate the water bomb pin
    digitalWrite(WATER_BOMB_PIN, LOW);
  }
}

void AutoMode(){
  //Set the automatic mode for watering and alarm usage.
  if(AutoModeVar == true){
    if (sh <= 60){
      waterStartTime = millis();
      PlantWatering();
    }
    if(Presence == "Detectada"){
      AlarmStartTime = millis();
      AlarmOn();
    }
  }
}

//SETUP
void setup(){

  pinMode(WATER_BOMB_PIN, OUTPUT);
  pinMode(ALARM_PIN, OUTPUT);
  pinMode(DHTPIN, INPUT);
  pinMode(SOIL_MOISTURE_PIN, INPUT);
  pinMode(PIRPIN,INPUT);

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
    server.on("/presence", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", String(Presence).c_str());
  });
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    String output = request->arg("output");
    String state = request->arg("state");
    if (output == "whatsappToggle") {
      if (state == "1") {
        whatsappNotificationsEnabled = true;
      } else if (state == "0") {
        whatsappNotificationsEnabled = false;
      }}
    else if (output == "AlarmToggle") {
      if (state == "1") {
        alarmSystemEnabled = true;
      } else if (state == "0") {
        alarmSystemEnabled = false;
      }}
      else if (output == "Mode") {
      if (state == "1") {
        AutoModeVar = true;
      } else if (state == "0") {
        AutoModeVar = false;
      }}
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
    // if temperature read failed, don't change t value
    if (isnan(newT)) {
      Serial.println("Failed to read temperature from DHT sensor!");
    }
    else {
      t = newT;
      Serial.println(t);
    }
    // Read Humidity
    float newH = dht.readHumidity();
    // if humidity read failed, don't change h value 
    if (isnan(newH)) {
      Serial.println("Failed to read humidity from DHT sensor!");
    }
    else {
      h = newH;
      Serial.println(h);
    }
    //-----------------SOIL_MOISTURE----------------
    // Read the value from the analog sensor:
    float sensorValue = analogRead(SOIL_MOISTURE_PIN);
    Serial.print("Analog read:" );
    Serial.println(sensorValue);
    //Transform the analog calue to %
    sh = ((m * sensorValue) + n);    
    Serial.print("Soil moisture reading:" );
    Serial.println(sh);
    AlertManager();
  }
  //----------------PIR_SENSOR-----------------
    int val = digitalRead(PIRPIN);
    Serial.println("PIR value:");
    Serial.println(val);
    if (val == HIGH) {    // Comprobar si el sensor está en HIGH
       Presence = "Detectada";        
    }
    else {
       Presence = "No Detectada";
    }
  AutoMode();
  PlantWatering();
}