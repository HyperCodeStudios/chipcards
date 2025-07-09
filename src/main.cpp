#include <Arduino.h>
#include "rfid.hpp" // RFID-Funktionalität
#include "http.hpp" // HTTP-Funktionalität
#include "neopixel.hpp" // Neopixel-Funktionalität

#define GPIO_DOOR_CONTROL GPIO_NUM_25 // Türsteuerung GPIO-Pin

#define UART_BAUD_RATE 115200

#define STATE_IDLE 0
#define STATE_AWAIT 1
#define STATE_ACCEPT 2
#define STATE_DECLINE 3

// HardwareSerial Serial1(1);
int state = STATE_IDLE;

void setup()
{
    Serial.begin(115200); // Serielle Kommunikation starten
    Serial.println("helalo world");
    pinMode(GPIO_DOOR_CONTROL, OUTPUT); // Türsteuerung als Ausgang konfigurieren
    Serial.println("helblo world");
    digitalWrite(GPIO_DOOR_CONTROL, HIGH); // Tür schließen
    Serial.println("helclo world");
    http_setup(); // HTTP initialisieren
    Serial.println("heldlo world");
    Serial1.begin(UART_BAUD_RATE, SERIAL_8N1, 10, 9); // RX: GPIO10, TX: GPIO09
    Serial.println("helelo world");
}

void loop()
{
    Serial1.println("p");
    delay(1000);
    String message = "";
    while(Serial1.available() > 0)
    {
        message += Serial1.read();
        Serial.println("MESSAGE RECEIVED: " + message); // Nachricht auf der seriellen Konsole ausgeben
    }
    
    switch(state)
    {
    case STATE_IDLE:
        if(message == "") break;
        String uid = message.substring(0, message.length() - 1);
        if(http_check_id(uid)) // UID mit HTTP-Anfrage überprüfen
        //if(uid == "73AB8AF5")
        {
            Serial.println("OPENING DOOR!"); // Nachricht auf der seriellen Konsole ausgeben
            state = STATE_ACCEPT;
            digitalWrite(GPIO_DOOR_CONTROL, LOW); // Tür öffnen
            Serial1.print('i');
            Serial1.write((uint8_t) 1);
            Serial1.write((uint8_t) 0);
            Serial1.write((uint8_t) 255);
            Serial1.write((uint8_t) 0);
            Serial1.write((uint32_t) 5000);
            Serial1.println();
            delay(5000); // 5 Sekunden warten, Tür bleibt offen
            digitalWrite(GPIO_DOOR_CONTROL, HIGH); // Tür schließen
        }
        else // UID ist nicht berechtigt
        {
            Serial1.println("DECLINE");
            delay(3000); // 3 Sekunden warten
        }
        break;
    }
}
