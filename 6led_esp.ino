#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

/*const char *ssid = "inwi Home 4G 91ADAB";
const char *password = "44333098";*/
const char *ssid = "Etudiants";
const char *password = "ENSAJ2020";

ESP8266WebServer server(803);

const int red1Pin = 16;    // red1 d0
const int orange1Pin = 5;  // orange1 D1
const int green2Pin = 2;   // green2 D4
const int red2Pin = 3;     // red2 RX
const int green1Pin = 4;   // green1f D2
const int orange2Pin = 0;  // orange2 D3
int stopTraffic = 0;
int ledOrder[] = {red1Pin, green2Pin, orange1Pin};  // Red1 with Green2, Green2 with Red2, Orange1 with Orange2
int additionalPins[] = {green1Pin, red2Pin, orange2Pin};  // Corresponding additional pins

String trafficLightIds[] = {"light3", "light4"};
String trafficLightColors[] = {"green", "red", "orange"};
int index1;
int index2;
int indexTab[2] = {index1, index2};
volatile int currentLedIndex = 0;
unsigned long previousMillis = 0;
unsigned long previousClientMillis = 0;  // Declaration for previousClientMillis
const long interval = 4000;
const long clientHandlingInterval = 100;

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: " + WiFi.localIP().toString() + ", Port: 803");

  pinMode(red1Pin, OUTPUT);
  pinMode(orange1Pin, OUTPUT);
  pinMode(green2Pin, OUTPUT);
  pinMode(red2Pin, OUTPUT);
  pinMode(green1Pin, OUTPUT);
  pinMode(orange2Pin, OUTPUT);

  analogWrite(red1Pin, 0);
  analogWrite(orange1Pin, 0);
  analogWrite(green2Pin, 0);
  analogWrite(red2Pin, 0);
  analogWrite(green1Pin, 0);
  analogWrite(orange2Pin, 0);

  server.on("/change-sequence", HTTP_POST, handleChangeSequence);
  server.begin();
}

void loop() {
  server.handleClient();  // Handle incoming HTTP requests
  runTrafficLightSequence();  // Run the traffic light sequence
}

void runTrafficLightSequence() {
  unsigned long currentMillis = millis();

  if (stopTraffic == 1)
    return;

  if (currentMillis - previousMillis >= interval) {
    turnOffLed(ledOrder[currentLedIndex]);

    currentLedIndex = (currentLedIndex + 1) % 3;
    turnOnLed(ledOrder[currentLedIndex]);

  
     
    
    
    sendTrafficState();
    previousMillis = currentMillis;
  }

  // Periodically call server.handleClient() to handle incoming requests
  if (currentMillis - previousClientMillis >= clientHandlingInterval) {
    server.handleClient();
    previousClientMillis = currentMillis;
  }
}


void turnOnLed(int pin) {
  // Turn off all LEDs
  analogWrite(red1Pin, 0);
  analogWrite(orange1Pin, 0);
  analogWrite(green2Pin, 0);
  analogWrite(red2Pin, 0);
  analogWrite(green1Pin, 0);
  analogWrite(orange2Pin, 0);

  // Turn on the selected LED
  analogWrite(pin, 255);

  // Turn on the corresponding additional pin
  analogWrite(additionalPins[currentLedIndex], 255);
}

void turnOffLed(int pin) {
  analogWrite(pin, 0);

  // Turn off additional pin as well
  analogWrite(additionalPins[currentLedIndex], 0);
}

void sendTrafficState() {
  WiFiClient client;
  String color[2]={"gro",""};
  //color[0] : Représente la couleur associée au premier feu de signalisation.
  //color[1] : Représente la couleur associée au deuxième feu de signalisation.
  if (ledOrder[currentLedIndex] == red1Pin) {
        color[0]="red";
        color[1]="green";
    } else if (ledOrder[currentLedIndex] == green2Pin) {
        color[1]="red";
        color[0]="green";
    } else if (ledOrder[currentLedIndex] == orange1Pin) {
        color[0]="orange";
        color[1]="orange";
    }
  
  for (int i = 0; i < 2; ++i) {
    // Construct the URL for the PATCH request with color as a query parameter
    //String url = "http://192.168.0.130:5000/traffic/" + trafficLightIds[i] + "?color=" + color[i];
    String url = "http://128.10.3.128:5000/traffic/" + trafficLightIds[i] + "?color=" + color[i];


    // Send the PATCH request
    HTTPClient http;
    http.begin(client, url);
    int httpResponseCode = http.PATCH("");

    Serial.print("HTTP Response code for ");
    Serial.print(trafficLightIds[i]);  // Use index 'i' to select the correct ID
    Serial.print(": ");
    Serial.println(httpResponseCode);
    Serial.println(url);

    http.end();
    delay(350);  // Add a small delay between requests to avoid issues
  }
}