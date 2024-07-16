//
//
//  ESP Relay Board - Version 1.1.0
//    Based on espHTTPServer
//    This version was deployed 2024.02.02
//
//  Generic ESP8266 (4MB/2MB) / ESP32 Dev Module Based
//    Support for ESP8266 4 Channel and ESP32 8 Channel Relay Modules
//
//  Changes From Previous Version
//    Transitioned to <espHTTPServer.h>
//
//

#include <espWiFiUtils.h>
#include <espHTTPServer.h>

#ifndef STASSID
#define STASSID             "Your-WiFi-SSID"        // WiFi SSID
#define STAPSK              "Your-WiFi-Pass"  // WiFi Password
#endif
#define WiFiHostname        "espRelay"            // WiFi Hostname

#ifdef ESP8266              // USE Generic ESP8266 Module with 4MB/2MB Flash Size
  #define LED_A       5     // HIGH is off
  #define LED_BUILTIN 2     // HIGH is off
  #define RELAY_1     15    // Default 16 on board but 16 is HIGH at boot
  #define RELAY_2     14
  #define RELAY_3     12
  #define RELAY_4     13
  int relays[4] = { RELAY_1, RELAY_2, RELAY_3, RELAY_4 };
#endif

#ifdef ESP32                // USE ESP32 Dev Module AS BOARD OR ELSE YOU GET WEIRD BEHAVIOR ON GPIO25 (RELAY_3)
  #define LED_A       23    // LOW is off
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

// Webpage Hex Colors
#define PAGETITLE "ESP Relay"
#define BGCOLOR "000"
#define TABBGCOLOR "111"
#define BUTTONCOLOR "222"
#define TEXTCOLOR "a40"
#define FONT "Helvetica"
#define TABHEIGHTEM "47"
#define REFRESHPAGE false
#define PORT 80

// espHTTPServer Object
espHTTPServer httpServer( PAGETITLE, BGCOLOR, TABBGCOLOR, BUTTONCOLOR, TEXTCOLOR, FONT, TABHEIGHTEM, REFRESHPAGE, PORT );

void setup() {
  // Setup LED_A as output and turn off
  pinMode(LED_A, OUTPUT);
#ifdef ESP8266
  digitalWrite(LED_A, HIGH);
#endif
#ifdef ESP32
  digitalWrite(LED_A, LOW);
#endif

  // Set relay pins to outputs and do a little dance
  for(int i = 0; i < sizeof(relays)/sizeof(int); i++) { pinMode(relays[i], OUTPUT); };
  for(int i = 0; i < sizeof(relays)/sizeof(int); i++) { digitalWrite(relays[i], HIGH); delay(125); };
  for(int i = 0; i < sizeof(relays)/sizeof(int); i++) { digitalWrite(relays[i], LOW); delay(125); };

  // Connect to WiFi, start OTA
#ifdef ESP8266
  connectWiFi(STASSID, STAPSK, WiFiHostname, LED_BUILTIN);
#endif
#ifdef ESP32
  connectWiFi(STASSID, STAPSK, WiFiHostname);
#endif
  initializeOTA(WiFiHostname, STAPSK);

  // Start HTML Server
  serverSetup();
}


void loop()
{
  // Webserver
  httpServer.server.handleClient();

  ArduinoOTA.handle();

  yield();
}


/*--------    Server Functions    --------*/

void toggleRelay() {
  int relayNum = httpServer.server.arg("relayNum").toInt();
  digitalWrite(relays[relayNum], !digitalRead(relays[relayNum]));
  httpServer.redirect();
}

void toggleLED() {
  digitalWrite(LED_A, !digitalRead(LED_A));
  httpServer.redirect();
}

/*--------         Webpage        --------*/

void serverSetup() {
  httpServer.server.on("/", handleRoot);
  httpServer.server.on("/toggleRelay", HTTP_GET, toggleRelay);
  httpServer.server.on("/toggleLED", HTTP_GET, toggleLED);
  httpServer.server.onNotFound(handleNotFound);
  httpServer.server.begin();
}

String body = "<div class=\"container\">\n"
                "<div class=\"centered-element\">\n"
                  "<form action=\"/toggleLED\" method=\"GET\">"
                    "<input class=\"simpleButton\" type=\"submit\" value=\"Turn LED %toggleStub%\" style=\"width: 100%;\">"
                  "</form>\n"
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
  String deliveredHTML = httpServer.assembleHTML(body);


#ifdef ESP8266
  if (digitalRead(LED_A)) { deliveredHTML.replace("%toggleStub%", "On"); }
                     else { deliveredHTML.replace("%toggleStub%", "Off"); }
#endif
#ifdef ESP32
  if (digitalRead(LED_A)) { deliveredHTML.replace("%toggleStub%", "Off"); }
                     else { deliveredHTML.replace("%toggleStub%", "On"); }
#endif

  httpServer.server.send(200, "text/html", deliveredHTML);
}

// Simple call back to espHTTPServer object for reasons
void handleNotFound() {
  httpServer.handleNotFound();
}
