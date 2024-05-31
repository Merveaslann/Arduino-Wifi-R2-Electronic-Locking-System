#include <WiFiNINA.h>
#include <ArduinoJson.h>
#include <LiquidCrystal.h>
#include <Servo.h>
#include <Keypad.h>

// Wi-Fi network information
#define WIFI_SSID "WIFI_SSID"
#define WIFI_PASSWORD "WIFI_PASSWORD"

// Firebase information
#define FIREBASE_HOST "FIREBASE_HOST"
#define FIREBASE_AUTH "FIREBASE_AUTH"

char FIREBASE_URL[200];

WiFiSSLClient client;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo servo1;

const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {A1, A0, 6, 7};
byte colPins[COLS] = {8, 9, 10};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String correctPassword = "";

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi...");

  connectToWiFi();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to Firebase...");

  servo1.attach(13);

  startScreen();
}

void loop() {
  char key = keypad.getKey();
  if (key == '1') {
    if (getFirebasePassword()) {
      Serial.println("Password from Firebase: " + correctPassword);
      enterPasswordScreen();
    } else {
      Serial.println("Failed to get password from Firebase!");
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Firebase error!");
      delay(5000);
      startScreen();
    }
  }
}

void connectToWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Connecting...");
    lcd.setCursor(0, 1);
    lcd.print(".");
    delay(1000);
  }
  Serial.println("Connected to Wi-Fi!");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Wi-Fi connected!");
  delay(2000);
}

void startScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Welcome! Press 1");
  lcd.setCursor(0, 1);
  lcd.print("to enter password.");
}

void enterPasswordScreen() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter your password");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Press '*' when");
  lcd.setCursor(0, 1);
  lcd.print("done.");
  enterPassword();
}

void operationEnd() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enjoy your visit!");
  delay(5000);
  startScreen();
}

void rotateServo() {
  delay(1000);
  servo1.write(180);
  delay(5000);
  servo1.write(0);
}

void enterPassword() {
  char enteredPassword[5];
  int index = 0;
  unsigned long startTime = millis();
  bool keyPressed = false;

  while (millis() - startTime < 10000) {
    char key = keypad.getKey();
    if (key) {
      keyPressed = true;
      if (key == '*') {
        if (index == 4) {
          enteredPassword[index] = '\0';
          lcd.clear();
          lcd.setCursor(0, 0);
          if (strcmp(enteredPassword, correctPassword.c_str()) == 0) {
            lcd.print("Correct password!");
            Serial.println("Correct password!");
            delay(1000);
            rotateServo();
          } else {
            lcd.print("Wrong password!");
            lcd.setCursor(0, 1);
            lcd.print("Try again.");
            Serial.println("Wrong password!");
            delay(5000);
            startScreen();
            return;
          }
          delay(2000);
          operationEnd();
          return;
        } else {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Incomplete password!");
          delay(2000);
          startScreen();
          return;
        }
      } else if (index < 4) {
        enteredPassword[index] = key;
        lcd.setCursor(index, 1);
        lcd.print("*");
        index++;
        startTime = millis();
      }
    }
  }

  if (!keyPressed || index < 4) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Timeout!");
    delay(2000);
    startScreen();
  }
}

bool getFirebasePassword() {
  snprintf(FIREBASE_URL, sizeof(FIREBASE_URL), "https://%s/%s.json?auth=%s", FIREBASE_HOST, "sa****/1****/3***/key***", FIREBASE_AUTH);

  if (client.connect(FIREBASE_HOST, 443)) {
    client.println(String("GET ") + FIREBASE_URL + " HTTP/1.1");
    client.println("Host: " + String(FIREBASE_HOST));
    client.println("Connection: close");
    client.println();

    while (client.connected()) {
      String line = client.readStringUntil('\n');
      if (line == "\r") {
        break;
      }
    }

    String response = client.readString();
    client.stop();

    DynamicJsonDocument doc(1024);
    deserializeJson(doc, response);
    correctPassword = doc.as<String>();
    correctPassword.trim();

    return true;
  } else {
    Serial.println("Failed to connect to Firebase.");
    client.stop();
    return false;
  }
}
