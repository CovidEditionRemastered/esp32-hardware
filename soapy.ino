#include <WebServer.h>
#include <WiFi.h>
extern "C" {
#include "crypto/base64.h"
}

#define ONBOARD_LED 2

// Access Point Details
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";
IPAddress ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);

// Station details
String HomeSSID = "";
String HomePassword = "";

// Webserver Details
WebServer server(80);

void setup() {
    Serial.begin(115200);
    pinMode(ONBOARD_LED, OUTPUT);

    // ! Set up Access Point
    Serial.print("Setting AP (Access Point)");
    WiFi.softAP(ssid, password);
    WiFi.softAPConfig(ip, gateway, subnet);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // ! Initialize Webserver
    server.on("/", handle_OnConnect);
    server.on("/connect", HTTP_POST, handle_ConnectToHomeWifi);

    server.begin();
    Serial.println("HTTP Webserver has started");
}

void loop() { server.handleClient(); }

String SendHTML() {
    const char ptr[] PROGMEM = R"rawliteral(
        <!DOCTYPE html>
        <html>

        <head>
            <meta name="viewport" content="width=device-width, initial-scale=1.0, user-scalable=no" />
            <title>Soapy! Your friendly Smart Soapbar!</title>
            <style>
                html {
                    font-family: Helvetica;
                    display: inline-block;
                    margin: 0px auto;
                    text-align: center;
                }

                body {
                    margin-top: 50px;
                }

                h1 {
                    color: #444444;
                    margin: 50px auto 30px;
                }

                h3 {
                    color: #444444;
                    margin-bottom: 40px;
                }

                #submit {
                    display: block;
                    width: 80px;
                    background-color: #3498db;
                    border: none;
                    color: white;
                    padding: 12px 15px;
                    text-decoration: none;
                    font-size: 15px;
                    margin: 0px auto 60px;
                    cursor: pointer;
                    border-radius: 4px;
                }

                #submit-network {
                    display: inline-block;
                    background-color: #3498db;
                    border: none;
                    color: white;
                    padding: 12px 30px;
                    text-decoration: none;
                    font-size: 15px;
                    margin: 0px auto 60px;
                    cursor: pointer;
                    border-radius: 4px;
                    overflow: hidden;
                    white-space: nowrap;
                }

                #credentials {
                    margin-bottom: 40px;
                }

                p {
                    font-size: 20px;
                    color: #888;
                    margin-bottom: 10px;
                }
            </style>
        </head>

        <body>
            <h1>Soapy's Initialization Web Server</h1>
            <h3>Please set the SSID and Password to your home network below!</h3>

            <div id=credentials>
                <p>SSID: <input type="text" id="ssid" /></p>
                <p>Password: <input type="password" id="pw" /></p>
            </div>
            <div id="submit">Submit</div>
            <script>
                var ssid = document.getElementById("ssid");
                var pw =  document.getElementById("pw");

                var submit = document.getElementById("submit");

                submit.addEventListener("click", function () {
                    var ssidVal = ssid.value;
                    var password = pw.value;

                    fetch(window.location.origin +  `/connect?ssidVal=${ssidVal}&password=${btoa(password)}`, {
                        method: "POST"
                    });
                })
            </script>

            <h3>Below is the UUID representing this specific Soapy! <br><br> Remember Set a password to secure your device on the network!</h3>
            <div>
                <p>UUID: 84fa598c-9ee1-47b8-9625-8e38e70dc1f3</p>
                <p>Soapy's Password: <input type="network-password" id="network-pw"/></p>
            </div>
            <br>
            <div id="submit-network">Send to Soapy's Server</div>

        </body>

        </html>
    )rawliteral";

    return ptr;
}

// ! HANDLERS FOR ROUTES
void handle_OnConnect() {
    Serial.println("Someone connected");
    server.send(200, "text/html", SendHTML());
}

void handle_ConnectToHomeWifi() {
    Serial.println("Saving SSID and Password to connect to Station");
    HomeSSID = server.arg("ssidVal");

    char* EncodedPassword =
        (char*)server.arg("password").c_str();  // Base64 encoded
    size_t outputLength;

    unsigned char* DecodedPassword =
        base64_decode((const unsigned char*)EncodedPassword,
                      strlen(EncodedPassword), &outputLength);
    HomePassword = String((const char*)DecodedPassword);

    Serial.println("SSID and Password saved");
    char* ssid_n = (char*)HomeSSID.c_str();
    char* pass_n = (char*)HomePassword.c_str();

    Serial.println("Closing Access Point");
    WiFi.softAPdisconnect();
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    delay(100);

    WiFi.begin(ssid_n, pass_n);
    Serial.print("Connecting to ");
    Serial.println(HomeSSID);
    Serial.println(HomePassword);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        digitalWrite(ONBOARD_LED, HIGH);
        delay(500);
        Serial.print(".");
        digitalWrite(ONBOARD_LED, LOW);
    }
    Serial.println("");
    Serial.println("WIFI CONNECTED");
    Serial.println(WiFi.localIP());
}