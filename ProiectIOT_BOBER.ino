#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);
byte lastScannedUID[4];
bool hasMasterKey = false;

void setup() {
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();

  Serial.println("Tap RFID/NFC Tag on reader that you want stored as a key");
}

void loop() {
  if (rfid.PICC_IsNewCardPresent()) {
    if (rfid.PICC_ReadCardSerial()) {

      // Dacă nu avem o cheie master setată, o memorăm pe prima scanată
      if (!hasMasterKey) {
        for (byte i = 0; i < rfid.uid.size; i++) {
          lastScannedUID[i] = rfid.uid.uidByte[i];
        }
        Serial.println(" -> UID successfully stored in variable!");
        hasMasterKey = true;
        
      } else {
        // Verificăm dacă noul card corespunde cu cheia master
        bool isMatch = true;
        
        for (byte i = 0; i < rfid.uid.size; i++) {
          if (lastScannedUID[i] != rfid.uid.uidByte[i]) {
            isMatch = false;
            break;
          }
        }

        if (isMatch) {
          Serial.println("Correct key!");
          // Aici vei putea adăuga logica pentru starea "Armat/Dezarmat"
        } else {
          Serial.println("Different key!");
          // Aici vei putea trata accesul respins
        }
        Serial.println();
      }

      // Afișăm în Serial Monitor detalii despre cardul citit
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("UID:");
      for (int i = 0; i < rfid.uid.size; i++) {
        Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
        Serial.print(rfid.uid.uidByte[i], HEX);
      }
      Serial.println();

      Serial.print("Last stored key was: ");
      for (byte i = 0; i < 4; i++) {
        Serial.print(lastScannedUID[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      // Oprim comunicarea cu cardul curent pentru a putea scana altele
      rfid.PICC_HaltA();
      rfid.PCD_StopCrypto1();
    }
  }
}