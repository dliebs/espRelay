//
//
//  ESP Relay Board - Version 1.0.1
//  Based on espHTTPServer
//  This version was not deployed [2024.01.16]
//
//  Generic ESP8266 (4MB/2MB) / ESP32 Dev Module Based
//    Relay Board
//    Feature 2
//
//  Changes From Previous Version
//    Comments, cleanup
//
//

#include <espWiFiUtils.h>
#include "espHTTPServerUtils.h"

#ifndef STASSID
#define STASSID             "Your-WiFi-SSID"        // WiFi SSID
#define STAPSK              "Your-WiFi-Pass"  // WiFi Password
#endif
#define WiFiHostname        "espRelay"            // WiFi Hostname

#ifdef ESP8266
  #define LED_A       1
  #define LED_BUILTIN 2
  #define RELAY_1     15    // Default 16 on board but 16 is HIGH at boot
  #define RELAY_2     14
  #define RELAY_3     12
  #define RELAY_4     13
  int relays[4] = { RELAY_1, RELAY_2, RELAY_3, RELAY_4 };
#endif

#ifdef ESP32                // USE ESP32 Dev Module AS BOARD OR ELSE YOU GET WEIRD BEHAVIOR ON GPIO25 (RELAY_3)
  #define LED_BUILTIN 23
  #define RELAY_1     32
  #define RELAY_2     33
  #define RELAY_3     25
  #define RELAY_4     26
  #define RELAY_5     27
  #define RELAY_6     14
  #define RELAY_7     12
  #define RELAY_8     13
  int relays[8] = { RELAY_1, RELAY_2, RELAY_3, RELAY_4, RELAY_5, RELAY_6, RELAY_7, RELAY_8 };
#endif

// Define BASICPAGE or TABBEDPAGE
#define BASICPAGE

// Webpage Hex Colors
#define BGCOLOR "000"
#define TABBGCOLOR "111"
#define BUTTONCOLOR "222"
#define TEXTCOLOR "a40"
#define FONT "Helvetica"
#define TABHEIGHTEM "47"


void setup() {
  // Set relay pins to outputs and do a little dance
  for(int i = 0; i < sizeof(relays)/sizeof(int); i++) { pinMode(relays[i], OUTPUT); };
  for(int i = 0; i < sizeof(relays)/sizeof(int); i++) { digitalWrite(relays[i], HIGH); delay(125); };
  for(int i = 0; i < sizeof(relays)/sizeof(int); i++) { digitalWrite(relays[i], LOW); delay(125); };

  // Connect to WiFi, start OTA
  connectWiFi(STASSID, STAPSK, WiFiHostname, LED_BUILTIN);
  initializeOTA(WiFiHostname, STAPSK);

  // Start HTML Server
  serverSetup();
}


void loop()
{
  // Webserver
  server.handleClient();

  ArduinoOTA.handle();

  yield();
}

/*--------         Webpage        --------*/

void serverSetup() {
  server.on("/", handleRoot);
  server.on("/toggleRelay", HTTP_GET, toggleRelay);
  server.onNotFound(handleNotFound);
  server.begin();
}

String body = "<div class=\"container\">\n"
                "<div class=\"centered-element\">\n"
                  "<form action=\"/toggleRelay\" method=\"GET\">"
                    "<input type=\"hidden\" name=\"remoteNumber\" value=\"4\"></input>"
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"0\" style=\"width: 50%;\">1</button>"
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"1\" style=\"width: 50%;\">2</button>"
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"2\" style=\"width: 50%;\">3</button>"
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"3\" style=\"width: 50%;\">4</button>"
                  #ifdef ESP32
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"4\" style=\"width: 50%;\">5</button>"
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"5\" style=\"width: 50%;\">6</button>"
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"6\" style=\"width: 50%;\">7</button>"
                    "<button name=\"relayNum\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"7\" style=\"width: 50%;\">8</button>"
                  #endif
                  "</form>"
                "</div>\n"
              "</div>\n";

void handleRoot() {
  String deliveredHTML = assembleHTML(body);
  server.send(200, "text/html", deliveredHTML);
}

void toggleRelay() {
  int relayNum = server.arg("relayNum").toInt();
  digitalWrite(relays[relayNum], !digitalRead(relays[relayNum]));
  redirect();
}
