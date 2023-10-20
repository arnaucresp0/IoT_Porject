#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncTCP.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>

// Define DHT sensor and type
#define DHTTYPE DHT11
const int DHTPin = 5;
DHT dht(DHTPin, DHTTYPE);

const char* ssid = "NOM DEL WIFI";
const char* password = "CONTRASSENYA";
WiFiServer server(80);

static char celsiusTemp[7];
static char humidityTemp[7];

void setup() {
  Serial.begin(115200);
  delay(10);

  dht.begin();

  // Connecting to WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the web server
  server.begin();
  Serial.println("Web server running. Waiting for the ESP IP...");
  delay(10000);

  // Printing the ESP IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("New client");
    boolean blank_line = true;
    while (client.connected()) {
      //If the client is connected and available we will run all the code below:
      if (client.available()) {
        char c = client.read();

        if (c == '\n' && blank_line) {
          //We start the dht sensor readings:

          //----------------DHT----------------
          float h = dht.readHumidity();
          float t = dht.readTemperature();
          float f = dht.readTemperature(true);
          //If the values ​​read are not numerical values, we send an error message:
          if (isnan(h) || isnan(t) || isnan(f)) {
            Serial.println("Failed to read from DHT sensor!");
            strcpy(celsiusTemp, "Failed");
            strcpy(humidityTemp, "Failed");
          } else {
            //The code performs the temperature and humidity calculation:
            float hic = dht.computeHeatIndex(t, h, false);
            dtostrf(hic, 6, 2, celsiusTemp);
            dtostrf(h, 6, 2, humidityTemp);
          }

          //--------SOIL_HUMIDITY_SENSOR----------
          

          //----------PRESENCE_SENSOR-------------
          static char presence[7];

          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println();

          // Send the CSS content within a <style> tag
          client.println("<style>");
          String css = "";
          //Open the CSS file in read mode:
          File cssFile = LittleFS.open("/styles.css", "r"); //ARREGLAR SPIFFS TO LITTLEFS
          if (cssFile) {
            while (cssFile.available()) {
              css += cssFile.readString();
            }
            cssFile.close();
          }
          client.print(css);
          client.println("</style>");

          // Read the HTML file into a string
          String webpage = "";
          File htmlFile = LittleFS.open("/webpage.html", "r"); //ARREGLAR SPIFFS TO LITTLEFS
          if (htmlFile) {
            while (htmlFile.available()) {
              webpage += htmlFile.readString();
            }
            htmlFile.close();
          }
          // Replace placeholders in the HTML content with sensor data
          webpage.replace("{{celsiusTemp}}", celsiusTemp);
          webpage.replace("{{humidity}}", humidityTemp);
          webpage.replace("{{presence}}", presence);
      
          // Send the HTML content
          client.print(webpage);
          break;
        }
        if (c == '\n') {
          blank_line = true;
        } else if (c != '\r') {
          blank_line = false;
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
  }
}
