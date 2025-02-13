#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <WebServer.h>


//mere phone ka net or hotspot chalu rahega or dusre ke phone ka wifi 
//mere hotspot se esp32 or esp32 se dusre ka phone
// Motor pins
#define IN_1 12    // L298N in1 motors Right     GPIO15(D8)
#define IN_2 14    // L298N in2 motors Right      GPIO13(D7)
#define IN_3 27    // L298N in3 motors Left       GPIO2(D4)
#define IN_4 26    // L298N in4 motors Left       GPIO0(D3)

// WiFi credentials
const char* ssid_STA = "BHAVYA"; // WiFi SSID for STA mode
const char* password_STA = "Bhavya@123"; // WiFi password for STA mode
const char* ssid_AP = "ESP32_AP"; // SoftAP SSID
const char* password_AP = "12345678"; // SoftAP password

// Telegram BOT credentials
#define BOTtoken "YOUR_BOT_TOKEN"
#define CHAT_ID "YOUR_CHAT_ID"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

WebServer server(80);

// Motor control variables
String command;             // String to store app command state.
int speedCar = 800;         // 400 - 1023.
int speed_Coeff = 3;

// MPU6050 pins
#define SDA_PIN 23
#define SCL_PIN 22

Adafruit_MPU6050 mpu;

void setup() {
  Serial.begin(115200);

  // Motor pins initialization
  pinMode(IN_1, OUTPUT);
  pinMode(IN_2, OUTPUT);
  pinMode(IN_3, OUTPUT);
  pinMode(IN_4, OUTPUT);

  // MPU6050 pins initialization
  Wire.begin(SDA_PIN, SCL_PIN);

  // Connect to WiFi network (STA mode)
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid_STA, password_STA);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  // Starting server
  server.on("/", HTTP_handleRoot);
  server.onNotFound(HTTP_handleRoot);
  server.begin();

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  bot.sendMessage(CHAT_ID, "Bot started up", "");

  // Additional pin setup
  pinMode(21, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, INPUT);

  // Set up SoftAP (Access Point)
  WiFi.softAP(ssid_AP, password_AP);
}

void loop() {
  server.handleClient();

  command = server.arg("State");
  if (command == "F") goAhead();
  else if (command == "B") goBack();
  else if (command == "L") goLeft();
  else if (command == "R") goRight();
  else if (command == "I") goAheadRight();
  else if (command == "G") goAheadLeft();
  else if (command == "J") goBackRight();
  else if (command == "H") goBackLeft();
  else if (command == "0") speedCar = 400;
  else if (command == "1") speedCar = 470;
  else if (command == "2") speedCar = 540;
  else if (command == "3") speedCar = 610;
  else if (command == "4") speedCar = 680;
  else if (command == "5") speedCar = 750;
  else if (command == "6") speedCar = 820;
  else if (command == "7") speedCar = 890;
  else if (command == "8") speedCar = 960;
  else if (command == "9") speedCar = 1023;
  else if (command == "S") stopRobot();

  // MPU6050 sensor reading
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float gx = g.gyro.x;
  float gy = g.gyro.y;
  float gz = g.gyro.z;
  int value = digitalRead(5);

  Serial.print("AccX: "); Serial.print(ax);
  Serial.print("\tAccY: "); Serial.print(ay);
  Serial.print("\tAccZ: "); Serial.print(az);
  Serial.print("\tGyroX: "); Serial.print(gx);
  Serial.print("\tGyroY: "); Serial.print(gy);
  Serial.print("\tGyroZ: "); Serial.println(gz);
  Serial.println(value);
  delay(100);
  if (ax<-5 || ax>5 || ay<-7 || ay>7) {
    digitalWrite(21, HIGH);
    digitalWrite(4, HIGH);
    bot.sendMessage(CHAT_ID, "Fall Detected");
    Serial.println("Fall Detected! ");
  }
  else if (value == HIGH) {
    bot.sendMessage(CHAT_ID, "🔴🔴🔴Emergency Alert!");
    Serial.println("Emergency Alert!");
  }
  else {
    digitalWrite(21, LOW);
    digitalWrite(4, LOW);
  }
}

void goAhead() {
  analogWrite(IN_1, speedCar);
  digitalWrite(IN_2, LOW);

  analogWrite(IN_3, speedCar);
  digitalWrite(IN_4, LOW);
}

void goBack() {
  digitalWrite(IN_1, LOW);
  analogWrite(IN_2, speedCar);

  digitalWrite(IN_3, LOW);
  analogWrite(IN_4, speedCar);
}

void goRight() {
  analogWrite(IN_1, speedCar);
  digitalWrite(IN_2, LOW);

  digitalWrite(IN_3, LOW);
  analogWrite(IN_4, speedCar);
}

void goLeft() {
  digitalWrite(IN_1, LOW);
  analogWrite(IN_2, speedCar);

  analogWrite(IN_3, speedCar);
  digitalWrite(IN_4, LOW);
}

void goAheadRight() {
  analogWrite(IN_1, speedCar / speed_Coeff);
  digitalWrite(IN_2, LOW);

  analogWrite(IN_3, speedCar);
  digitalWrite(IN_4, LOW);
}

void goAheadLeft() {
  analogWrite(IN_1, speedCar);
  digitalWrite(IN_2, LOW);

  analogWrite(IN_3, speedCar / speed_Coeff);
  digitalWrite(IN_4, LOW);
}

void goBackRight() {
  digitalWrite(IN_1, LOW);
  analogWrite(IN_2, speedCar / speed_Coeff);

  digitalWrite(IN_3, LOW);
  analogWrite(IN_4, speedCar);
}

void goBackLeft() {
  digitalWrite(IN_1, LOW);
  analogWrite(IN_2, speedCar);

  digitalWrite(IN_3, LOW);
  analogWrite(IN_4, speedCar / speed_Coeff);
}

void stopRobot() {
  digitalWrite(IN_1, LOW);
  digitalWrite(IN_2, LOW);

  digitalWrite(IN_3, LOW);
  digitalWrite(IN_4, LOW);
}

void HTTP_handleRoot(void) {
  if (server.hasArg("State")) {
    Serial.println(server.arg("State"));
  }
  server.send(200, "text/html", "");
  delay(1);
}
