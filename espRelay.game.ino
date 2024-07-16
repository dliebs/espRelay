#include <espWiFiUtils.h>
#include "espHTTPServerUtils.h"

#ifndef STASSID
#define STASSID             "Your-WiFi-SSID"        // WiFi SSID
#define STAPSK              "Your-WiFi-Pass"  // WiFi Password
#endif
#define WiFiHostname        "relayGame"            // WiFi Hostname

#ifdef ESP8266
  #define LED_A     2   //  LED_BUILTIN
  #define LED_B     1
  #define NUMOFRELAYS 4
  #define RELAY_1   15
  #define RELAY_2   14
  #define RELAY_3   12
  #define RELAY_4   13
  int relays[NUMOFRELAYS] = { RELAY_1, RELAY_2, RELAY_3, RELAY_4 };
#endif

#ifdef ESP32        // USE ESP32 Dev Module AS BOARD OR ELSE YOU GET WEIRD BEHAVIOR ON GPIO25 (RELAY_3)
  #define LED_BUILTIN 23
  #define NUMOFRELAYS 8
  #define RELAY_1   32
  #define RELAY_2   33
  #define RELAY_3   25
  #define RELAY_4   26
  #define RELAY_5   27
  #define RELAY_6   14
  #define RELAY_7   12
  #define RELAY_8   13
  int relays[NUMOFRELAYS] = { RELAY_1, RELAY_2, RELAY_3, RELAY_4, RELAY_5, RELAY_6, RELAY_7, RELAY_8 };
#endif

// Define BASICPAGE or TABBEDPAGE
#define TABBEDPAGE

// Webpage Hex Colors
#define BGCOLOR "000"
#define TABBGCOLOR "111"
#define BUTTONCOLOR "222"
#define TEXTCOLOR "a40"
#define FONT "Helvetica"
#define TABHEIGHTEM "47"

#define PATTERNLENGTH 4
#define INITIALONTIME 550;
int onTime = INITIALONTIME;
int score = 0;
int pattern[PATTERNLENGTH];
int userPattern[PATTERNLENGTH];

void setup() {
  // Set relay pins to outputs and do a little dance
  for(int i = 0; i < NUMOFRELAYS; i++) { pinMode(relays[i], OUTPUT); };
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], HIGH); delay(125); };
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); delay(125); };

  // Connect to WiFi, start OTA
  connectWiFi(STASSID, STAPSK, WiFiHostname, LED_BUILTIN);
  initializeOTA(WiFiHostname, STAPSK);

  newLevel();

  // Start HTML Server
  serverSetup();
}
  //*********************************************************************
  //*************NOW LET'S START MEASURING*******************************
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
  server.on("/null", HTTP_GET, null);
  server.onNotFound(handleNotFound);
  server.begin();
}

String body = "<div class=\"tabs\">\n"
                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-0\" name=\"tab-group-1\" checked>\n"
                  "<label for=\"tab-0\">Game</label>\n"
                  "<div class=\"content\">\n"

                    "<p align=\"center\">Copy the pattern!<br>\n"
                                        "Watch the pattern and press the<br>\n"
                                        "corresponding buttons in order.<br>\n"
                                        "Left is #1, Right is #8. Please<br>\n"
                                        "wait for your input to be displayed<br>\n"
                                        "before pressing the next button!<br>\n"
                                        "Score is displayed after each round.</p>\n"
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
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-1\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-1\">Thermostat</label>\n"
                  "<div class=\"content\">\n"

                    "<div class=\"container\">\n"
                      "<div class=\"centered-element\">\n"
                        "<p align=\"center\">\n"
                        "<span style=\"font-size: 12em;\">72</span><span style=\"vertical-align: top; font-size: 2em;\">&#176;F</span><br>\n"
                        "<span style=\"float: left; text-align: left; line-height: 1.4em;\">Mode: Heat<br>\n"
                        "Currently: Heating<br>\n"
                        "Barometer: Normal &#8691;</span>\n"
                        "<span style=\"float: right; text-align: right;\">71.53 &#176;F<br>\n"
                        "43.58&#37; &#127778;<br>\n"
                        "30.05 inHg</span></p>\n"
                        "<form action=\"/null\" method=\"GET\"><input type=\"submit\" value=\"+\" class=\"simpleButton\" style=\"width: 100%; margin-top: 1em;\"></form>\n"
                        "<form action=\"/null\" method=\"GET\"><input type=\"submit\" value=\"-\" class=\"simpleButton\" style=\"width: 100%;\"></form>\n"
                        "<br>\n"
                        "<form action=\"/null\" method=\"GET\"><input type=\"submit\" value=\"Turn Fan On\" class=\"simpleButton\" style=\"width: 100%;\"></form>\n"
                      "</div>\n"
                    "</div>\n"

                  "</div>\n"
                "</div>\n"

                "<div class=\"tab\">\n"
                  "<input type=\"radio\" id=\"tab-2\" name=\"tab-group-1\">\n"
                  "<label for=\"tab-2\">Settings</label>\n"
                  "<div class=\"content\">\n"
                    "<form action=\"/null\" method=\"GET\">\n"
                      "<button name=\"mode\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"1\" style=\"width: 33.33%;\">Heat</button>"
                      "<button name=\"mode\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"0\" style=\"width: 33.33%;\">Off</button>"
                      "<button name=\"mode\" class=\"inputButton\" onclick=\"this.form.submit()\" value=\"2\" style=\"width: 33.33%;\">Cool</button>"
                    "</form>\n"
                    "<br><br>\n"

                    "<form action=\"/null\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"setPoint\" value=\"72\" style=\"width: 15%; text-align: right;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 85%;\" value=\"Set Temperature\">\n"
                    "</form>\n"
                    "<br>\n"
                    "<form action=\"/null\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"tempOffset\" value=\"-5\" style=\"width: 15%; text-align: right;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 85%;\" value=\"Temp Adjust (&#176;F)\">\n"
                    "</form>\n"
                    "<br>\n"
                    "<form action=\"/null\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"overshoot\" value=\"2\" style=\"width: 15%; text-align: right;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 85%;\" value=\"Overshoot (&#176;F)\">\n"
                    "</form>\n"
                    "<br>\n"
                    "<form action=\"/null\" style=\"display: flex;\" method=\"GET\">\n"
                      "<input type=\"text\" class=\"textInput\" name=\"barometerOffset\" value=\"25\" style=\"width: 15%; text-align: right;\">\n"
                      "<input type=\"submit\" class=\"textInput\" style=\"width: 85%;\" value=\"Barometer &#177; (mb)\">\n"
                    "</form>\n"

                  "</div>\n"
                "</div>\n"

              "</div>\n";

void handleRoot() {
  String deliveredHTML = assembleHTML(body);
  server.send(200, "text/html", deliveredHTML);
}

void toggleRelay() {
  int relayNum = server.arg("relayNum").toInt();
  gameHandler(relayNum);
  redirect();
}

void null() {
  redirect();
}

void newLevel() {
  // Clear Display
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); };
  // Print Score
  for(int i = 0; i < score; i++) { digitalWrite(relays[i], HIGH); }
  delay(5000);
  // Clear Display
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); };
  delay(1000);
  // Zero userPattern[]
  for(int i = 0; i < PATTERNLENGTH; i++) { userPattern[i] = -1; }
  // Pick and show a new pattern
  for(int i = 0; i < PATTERNLENGTH; i++) {
    pattern[i] = random(8);
    digitalWrite(relays[pattern[i]], HIGH);
    delay(onTime);
    digitalWrite(relays[pattern[i]], LOW);
    delay(onTime);
  };
}

void gameHandler(int userInput) {
  // Show User Input
  digitalWrite(relays[userInput], HIGH);
  delay(onTime);
  digitalWrite(relays[userInput], LOW);
  // Write User Input
  for(int i = 0; i < PATTERNLENGTH; i++) {
    // Iterate thru userPattern till we find a -1
    if(userPattern[i] == -1) {
      if ( userInput == pattern[i] ) {
        // Correct answer, clear spot, time to write
        userPattern[i] = userInput;
        // If we're at the end of the array length and the last number matches, YOU WIN!
        if ( i == (PATTERNLENGTH - 1) && userPattern[PATTERNLENGTH - 1] == pattern[PATTERNLENGTH - 1] ) {
          youwin();
        }
      }
      // Check for correctness, if not a match, YOU LOSE this round
      else if ( userInput != pattern[i] && userPattern[i] == -1 ) {
        youlose();
      }
      break;
    }
  }
}

void youlose() {
  score--;
  // Flash 1x
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], HIGH); };
  delay(500);
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); };
  delay(500);
  if ( score < 0 ) {
    // You Lose this game
    // Flash 4x more
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], HIGH); };
    delay(500);
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); };
    delay(500);
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], HIGH); };
    delay(500);
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); };
    // Reset score, start again
    score = 0;
    onTime = INITIALONTIME;
  }
  // Increase onTime
  onTime = onTime + 50;
  newLevel();
}

void youwin() {
  score++;
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], HIGH); delay(125); };
  for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); delay(125); };
  if ( score == NUMOFRELAYS ) {
    // You win this game
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], HIGH); delay(125); };
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); delay(125); };
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], HIGH); delay(125); };
    for(int i = 0; i < NUMOFRELAYS; i++) { digitalWrite(relays[i], LOW); delay(125); };
    // Reset score, start again
    score = 0;
    onTime = INITIALONTIME;
  }
  // Reduce onTime
  onTime = onTime - 50;
  newLevel();
}
