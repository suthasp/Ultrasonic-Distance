/*
True Innovation
*/

#ifdef ESP32
  #include <WiFi.h>
  #include <HTTPClient.h>
#else
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClient.h>
#endif

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

// Define Trig and Echo pin:
#define trigPin 12
#define echoPin 13
#define relayPin 2

// Define variables:
long duration;
int distance;
int waterlevel;
int rainspout_high = 200;
int litre;


// Replace with your network credentials
const char* ssid     = "smartmonitor";
const char* password = "01004714";

// REPLACE with your Domain name and URL path or IP address with path
//const char* serverName = "https://www.folkza.com/project/post-esp-data.php";
const char* host = "script.google.com";
const int httpsPort = 443;

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

// Timers auxiliar variables

long now = millis();

long lastMeasure = 0;

String GAS_ID = "AKfycbwuwv4IaGyTl-udOPM9cLlvY0TMfuxN1BXUuibHqZ0vcChVsm3NCctOfoENw8QOgbMT"; //--> spreadsheet script ID

String sensorName = "Water proof 1";
int sensorLocation = 99;

void setup() {
    // Define inputs and outputs
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(relayPin, OUTPUT);
  Serial.begin(115200);
  
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { 
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

}

void loop() {

    now = millis();

  // Publishes new temperature and humidity every 3 seconds

  if (now - lastMeasure > 10000) {

    lastMeasure = now;

 // Clear the trigPin by setting it LOW:
  digitalWrite(trigPin, LOW);
 // digitalWrite(relayPin, LOW);
  
  delayMicroseconds(10);

 // Trigger the sensor by setting the trigPin high for 10 microseconds:
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(20);
  digitalWrite(trigPin, LOW);
  
  
  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance:
  distance = duration*0.034/2;
  waterlevel = rainspout_high-distance;
  litre = waterlevel*23;
      Serial.print("Distance: ");
      Serial.println(distance);
      Serial.print("Duration: ");
      Serial.println(duration);
      Serial.print("Litre: ");
      Serial.println(litre);
    
    sendData(sensorLocation,waterlevel); 
 

  }

}

//==============================================================================


void sendData(int value,int value2) {

  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);

 
  //----------------------------------------Connect to Google host

  if (!client.connect(host, httpsPort)) {

    Serial.println("connection failed");

    return;

  }

  //----------------------------------------


  //----------------------------------------Proses dan kirim data  


  int string_location = value;

  int string_waterlevel = value2;

  String url = "/macros/s/" + GAS_ID + "/exec?location=" + string_location + "&level="+string_waterlevel; //  2 variables

  Serial.print("requesting URL: ");

  Serial.println(url);


  client.print(String("GET ") + url + " HTTP/1.1\r\n" +

         "Host: " + host + "\r\n" +

         "User-Agent: BuildFailureDetectorESP32\r\n" +

         "Connection: close\r\n\r\n");


  Serial.println("request sent");

  //---------------------------------------
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp32/Arduino CI successfull!");
  } else {
    Serial.println("esp32/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  client.stop();
  //----------------------------------------
  //Add Tirgger Relay if water level more than 90 cm.
    if(waterlevel>90)
      {
        digitalWrite(relayPin, HIGH);
      }
      else
        {
          digitalWrite(relayPin, LOW);
        }
}

//===============================================
