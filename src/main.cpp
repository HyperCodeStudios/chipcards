#include <Arduino.h>
#include "rfid.hpp" // RFID-Funktionalität
#include "http.hpp" // HTTP-Funktionalität
#include "neopixel.hpp" // Neopixel-Funktionalität

#define GPIO_DOOR_CONTROL GPIO_NUM_25 // Türsteuerung GPIO-Pin

#define UART_BAUD_RATE 9600
#define GPIO_RX GPIO_NUM_16
#define GPIO_TX GPIO_NUM_17

#define STATE_IDLE 0
#define STATE_AWAIT 1
#define STATE_ACCEPT 2
#define STATE_DECLINE 3

HardwareSerial uart(2);
int state = STATE_IDLE;

void setup()
{
    Serial.begin(9600); // Serielle Kommunikation starten
    pinMode(GPIO_DOOR_CONTROL, OUTPUT); // Türsteuerung als Ausgang konfigurieren
    digitalWrite(GPIO_DOOR_CONTROL, HIGH); // Tür schließen
    http_setup(); // HTTP initialisieren
    uart.begin(UART_BAUD_RATE, SERIAL_8N1, GPIO_RX, GPIO_TX);
}

void loop()
{
    String message = "";
    while(uart.available() > 0)
    {
        message += uart.read();
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
            uart.print("i");
            uart.write((uint8_t) 1);
            uart.write((uint8_t) 0);
            uart.write((uint8_t) 255);
            uart.write((uint8_t) 0);
            uart.write((uint32_t) 5000);
            uart.println();
            delay(5000); // 5 Sekunden warten, Tür bleibt offen
            digitalWrite(GPIO_DOOR_CONTROL, HIGH); // Tür schließen
        }
        else // UID ist nicht berechtigt
        {
            uart.println("DECLINE");
            delay(3000); // 3 Sekunden warten
        }
        break;
    }
}
