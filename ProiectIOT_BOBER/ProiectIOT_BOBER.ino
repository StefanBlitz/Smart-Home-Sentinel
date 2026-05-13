#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SS_PIN 10
#define RST_PIN 9
#define ALARM_PIN 8

MFRC522 rfid(SS_PIN, RST_PIN);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

byte masterKey[4];
bool hasMasterKey = false;
bool isArmed = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
  
  pinMode(ALARM_PIN, OUTPUT);
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }

  bootScreen();
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  // 1. SETARE MASTER KEY
  if (!hasMasterKey) {
    for (byte i = 0; i < 4; i++) masterKey[i] = rfid.uid.uidByte[i];
    hasMasterKey = true;
    
    drawUI("MASTER KEY", "INITIALIZED", SSD1306_WHITE);
    successSound();
    delay(2000);
  } 
  
  // 2. VERIFICARE ACCES
  else {
    bool match = true;
    for (byte i = 0; i < 4; i++) {
      if (rfid.uid.uidByte[i] != masterKey[i]) match = false;
    }

    if (match) {
      isArmed = !isArmed;
      if(isArmed) {
        drawUI("SYSTEM", "ARMED", SSD1306_WHITE);
        armSound();
      } else {
        drawUI("SYSTEM", "DISARMED", SSD1306_WHITE);
        disarmSound();
      }
    } else {
      drawUI("ACCESS", "DENIED!", SSD1306_WHITE);
      errorSound();
    }
  }

  delay(2000);
  refreshDisplay();
  
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// --- FUNCTII GRAFICE ---

void bootScreen() {
  display.clearDisplay();
  display.drawRect(0, 0, 128, 64, SSD1306_WHITE);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(20, 20);
  display.print("S.O.S. SENTINEL");
  display.setCursor(15, 40);
  display.print("Waiting for Key...");
  display.display();
}

void drawUI(String header, String status, int color) {
  display.clearDisplay();
  display.drawRoundRect(0, 0, 128, 64, 8, SSD1306_WHITE); // Chenar rotunjit
  display.setTextSize(1);
  display.setCursor(10, 10);
  display.print(header);
  
  display.setTextSize(2);
  int xCoord = 64 - (status.length() * 6); // Centrare aproximativa
  display.setCursor(10, 30);
  display.print(status);
  display.display();
}

void refreshDisplay() {
  display.clearDisplay();
  display.drawRect(0, 0, 128, 20, SSD1306_WHITE);
  display.setTextSize(1);
  display.setCursor(15, 6);
  display.print("SENTINEL ACTIVE");
  
  display.setTextSize(2);
  display.setCursor(10, 35);
  if(isArmed) display.print("[ ARMED ]");
  else display.print("[ SAFE ]");
  display.display();
}

// --- FUNCTII SUNET ---

void successSound() {
  tone(ALARM_PIN, 1000, 200); delay(250);
  tone(ALARM_PIN, 1500, 200);
}

void armSound() {
  tone(ALARM_PIN, 800, 100); delay(150);
  tone(ALARM_PIN, 1000, 300);
}

void disarmSound() {
  tone(ALARM_PIN, 1000, 100); delay(150);
  tone(ALARM_PIN, 800, 300);
}

void errorSound() {
  for(int i=0; i<3; i++) {
    tone(ALARM_PIN, 200, 300);
    delay(400);
  }
}