#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <NewPing.h>

// Pin assignments
const int flameSensorPin = 32;
const int buzzerPin = 25;
const int ledPin = 22;

#define TRIG_PIN 27
#define ECHO_PIN 26
#define MAX_DISTANCE 2000

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// Wi-Fi credentials
const char* ssid = "Galaxy M13 FDE6";
const char* password = "krushhh1234";

AsyncWebServer server(80);

bool surveillanceActive = false;

void setup() {
  Serial.begin(115200);

  pinMode(flameSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body><h1>Fire and Distance Detection</h1>";
    html += "<p>Surveillance Status: <b>" + String(surveillanceActive ? "Active" : "Inactive") + "</b></p>";
    html += "<p>Flame Sensor: " + String((digitalRead(flameSensorPin) == LOW) ? "Fire Detected" : "No Fire") + "</p>";
    html += "<p>Ultrasonic Sensor Distance: " + String(sonar.ping_cm()) + " cm</p>";
    html += surveillanceActive 
            ? "<p><a href='/stop'>Stop Surveillance</a></p>" 
            : "<p><a href='/start'>Start Surveillance</a></p>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  server.on("/start", HTTP_GET, [](AsyncWebServerRequest *request){
    surveillanceActive = true;
    request->send(200, "text/html", "<html><body><h1>Surveillance Started</h1><a href='/'>Go back</a></body></html>");
  });

  server.on("/stop", HTTP_GET, [](AsyncWebServerRequest *request){
    surveillanceActive = false;
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
    request->send(200, "text/html", "<html><body><h1>Surveillance Stopped</h1><a href='/'>Go back</a></body></html>");
  });

  server.begin();
}

void loop() {
  if (surveillanceActive) {
    int flameState = digitalRead(flameSensorPin);
    int distance = sonar.ping_cm();

    if (flameState == LOW || (distance > 0 && distance < 20)) {
      Serial.println("Fire or Object Detected!");
      digitalWrite(buzzerPin, HIGH);
      digitalWrite(ledPin, HIGH);
    } else {
      digitalWrite(buzzerPin, LOW);
      digitalWrite(ledPin, LOW);
    }

    Serial.print("Flame Sensor State: ");
    Serial.println(flameState == LOW ? "Fire Detected" : "No Fire");
    Serial.print("Ultrasonic Sensor Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
  } else {
    digitalWrite(buzzerPin, LOW);
    digitalWrite(ledPin, LOW);
  }

  delay(100);
}
