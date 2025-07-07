#include "rfid.hpp" // Einbindung der benutzerdefinierten Header-Datei für RFID-Funktionalität

#include <Arduino.h>
#include <SPI.h> // Einbindung der SPI-Bibliothek
#include <MFRC522.h> // Einbindung der MFRC522-Bibliothek für RFID

#define SS_PIN  GPIO_NUM_5  // ESP32 GPIO5 für SS (Slave Select)
#define RST_PIN GPIO_NUM_27 // ESP32 GPIO27 für RST (Reset)

MFRC522 rfid(SS_PIN, RST_PIN); // Initialisierung des RFID-Lesers

void rfid_setup()
{
  SPI.begin(); // Initialisierung der SPI-Schnittstelle
  rfid.PCD_Init(); // Initialisierung des RFID-Lesers
}

char hex[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'}; // Hexadezimale Zeichen

String read_rfid()
{
  if (rfid.PICC_IsNewCardPresent()) { // Prüfen, ob eine neue Karte verfügbar ist
    Serial.println("Card present");
    if (rfid.PICC_ReadCardSerial()) { // UID der Karte wurde gelesen
      String uid; // String zur Speicherung der UID
      for (int i = 0; i < rfid.uid.size; i++)
      {
        uid += hex[(rfid.uid.uidByte[i] >> 4) & 0xF]; // Höhere 4 Bits in hexadezimal konvertieren
        uid += hex[rfid.uid.uidByte[i] & 0xF]; // Niedrigere 4 Bits in hexadezimal konvertieren
      }
      rfid.PICC_HaltA(); // Karte anhalten
      rfid.PCD_StopCrypto1(); // Verschlüsselung stoppen
      Serial.println("UID: " + uid); // UID auf der seriellen Konsole ausgeben
      return uid; // UID zurückgeben
    }
  }
  return ""; // Keine Karte vorhanden oder Fehler
}
