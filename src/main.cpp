#include <Arduino.h>
#include "rfid.hpp" // RFID-Funktionalität
#include "http.hpp" // HTTP-Funktionalität
#include "neopixel.hpp" // Neopixel-Funktionalität

#define GPIO_DOOR_CONTROL GPIO_NUM_25 // Türsteuerung GPIO-Pin

void setup()
{
    Serial.begin(9600); // Serielle Kommunikation starten
    pinMode(GPIO_DOOR_CONTROL, OUTPUT); // Türsteuerung als Ausgang konfigurieren
    digitalWrite(GPIO_DOOR_CONTROL, HIGH); // Tür schließen
    neopixel_setup(); // Neopixel initialisieren
    neopixel_off();
    rfid_setup(); // RFID initialisieren
    http_setup(); // HTTP initialisieren
}

void loop()
{
    String uid = read_rfid(); // RFID-Karte lesen und UID erhalten
    if(uid != "")
    {
        neopixel_white(); // Neopixel auf weiß setzen (Karte gelesen)
        //if(http_check_id(uid)) // UID mit HTTP-Anfrage überprüfen
        if(uid == "73AB8AF5")
        {
            Serial.println("OPENING DOOR!"); // Nachricht auf der seriellen Konsole ausgeben
            digitalWrite(GPIO_DOOR_CONTROL, LOW); // Tür öffnen
            neopixel_green(); // Neopixel auf grün setzen (Zugang gewährt)
            delay(5000); // 5 Sekunden warten, Tür bleibt offen
            digitalWrite(GPIO_DOOR_CONTROL, HIGH); // Tür schließen
            neopixel_off(); // Neopixel ausschalten
        }
        else // UID ist nicht berechtigt
        {
            neopixel_red(); // Neopixel auf rot setzen (Zugang verweigert)
            delay(3000); // 3 Sekunden warten
            neopixel_off(); // Neopixel ausschalten
        }
    }
}
