#include "http.hpp" // Einbindung der benutzerdefinierten Header-Datei für HTTP-Funktionalität

#include <Arduino.h>
#include <WiFi.h> // Einbindung der WiFi-Bibliothek
#include <HTTPClient.h> // Einbindung der HTTP-Client-Bibliothek

// WiFi-Zugangsdaten
const char WIFI_SSID[] = "SLG-Medien";
const char WIFI_PASSWORD[] = "SommerSonneWetter23!";

// Zugangsdaten für die Website
const char WEBSITE_USER[] = "leoapp";
const char WEBSITE_PASSWORD[] = "leoapp";

// URL der Website zur UID-Überprüfung
String HOST_NAME = "https://secureaccess.it4schools.de/slgweb/chipcards/check.php?id="; // TODO: Update ts

HTTPClient client; // HTTP-Client-Objekt

void http_setup()
{
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // Verbindung zum WiFi-Netzwerk herstellen
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) { // Warten, bis die Verbindung hergestellt ist
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to network with ip=");
  Serial.println(WiFi.localIP()); // IP-Adresse ausgeben
  Serial.println(WiFi.gatewayIP()); // Gateway-IP ausgeben
}

bool http_check_id(String uid)
{
  String url = HOST_NAME + uid; // Vollständige URL erstellen
  client.begin(url); // HTTP-Verbindung starten
  client.setAuthorization(WEBSITE_USER, WEBSITE_PASSWORD); // Authentifizierung setzen
  int httpCode = client.GET(); // HTTP GET-Anfrage senden

  //TODO: Protokollspezifikationen implementieren (derzeit: Fehler = false, kein Fehler = true)

  // httpCode wird negativ bei Fehler
  if(httpCode > 0)
  {
    if(httpCode == HTTP_CODE_OK)
    {
      String payload = client.getString(); // Antwort des Servers lesen
      Serial.println(payload);
      client.end(); // Verbindung beenden
      return true; // Erfolgreiche Überprüfung
    }
    else
    {
      // HTTP-Header wurde gesendet und die Serverantwort wurde verarbeitet
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      String payload = client.getString(); // Antwort des Servers lesen
      Serial.println(payload);
    }
  }
  else
  {
    Serial.printf("[HTTP] GET... failed, error: %s\n", client.errorToString(httpCode).c_str()); // Fehler ausgeben
    return false; // Fehlgeschlagene Überprüfung
  }

  client.end(); // Verbindung beenden
  return false; // Fehlgeschlagene Überprüfung
}
