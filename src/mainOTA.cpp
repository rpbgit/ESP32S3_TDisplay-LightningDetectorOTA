/*
mainOTA.cpp

this file manages the wifi connections and communications to the web client for the lightning detector.

*/
#include <Arduino.h>
#include <string.h>
#include "Lightning_Web.h"       // .h file that stores your html page code
#include <WiFi.h>           // standard library
#include <ESPmDNS.h>        // mDNS for local hostname resolution
#include <ESPAsyncWebServer.h> // Async web server library
#include <ElegantOTA.h>   // ElegantOTA lib
#include "defs.h"
#include "creds.h"  // unique credentials file for sharing this code, unique network stuff here.
#include <ArduinoJson.h>

#define DISABLE_ALL_LIBRARY_WARNINGS 1 // disable all library warnings for tft_eSPI
#include "TFT_eSPI.h"

// a ring buffer of strings that will also concatenate all strings into one.
#include "StrRingBuffer.h"
#include "CmdParser.h"

// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server
#define USE_INTRANET

// start your defines for pins for sensors, outputs etc.
#define PIN_LED 48 // On board LED

unsigned long gLongest_loop_time = 0; // global so that the Lightning_hw.cpp can access it to reset/retrieve the value.
bool bWiFi_Connected = false;

// forward ref function prototype
void SendWebsite(AsyncWebServerRequest *request);
//void SendXML(AsyncWebServerRequest *request);
void SendJSON(AsyncWebServerRequest *request);
void PrintWifiStatus();
void buildXmlTag(char *buffer, size_t bufferSize, const char *tagName, unsigned long value);
// void printRequestDetails(AsyncWebServerRequest *request);

void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total);

void PWR_Select_handler(AsyncWebServerRequest *request);
void R1Select_handler(AsyncWebServerRequest *request);
void TermInput_handler(AsyncWebServerRequest *request);
/* void R2Select_handler(AsyncWebServerRequest *request);
void R3Select_handler(AsyncWebServerRequest *request);
void R4Select_handler(AsyncWebServerRequest *request);
void Aux1_OnSelect_handler(AsyncWebServerRequest *request);
void Aux1_OffSelect_handler(AsyncWebServerRequest *request);
void Aux2_OnSelect_handler(AsyncWebServerRequest *request);
void Aux2_OffSelect_handler(AsyncWebServerRequest *request);
void All_Grounded_handler(AsyncWebServerRequest *request); */

//void WebText(const char *);
void WebText(const char *format, ...);
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);
void startupWiFiWithList();
void showWiFiNetworksFound(int numNetworks); // forward ref

// the XML array size needs to be bigger than your maximum expected size. 2048 is way too big for this example
char XML_buffer[4096] = {0};

// buffer to hold INFO message before being built into an XML msg
StringRingBuffer RingBuffer;

// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

// gotta create a server
AsyncWebServer server(MY_LD_HTTPS_PORT);  // use 50011 an "unknown" port for use on the public WAN.

// this is the command sent by EITHER the VB console -OR- the webpage console
// its the only coupling needed between the WiFi/Web page stuff and the hardware.
HostCmdEnum gHostCmd = HostCmdEnum::NO_OP; // initial value of 255/0xFF = no-op

// external references to tell the linker to look for these things in another object file.
extern void loop2(HostCmdEnum & command);
extern void setup2();
extern RAS_HW_STATUS_STRUCT RAS_Status;
extern RAS_HW_STATUS_STRUCT *pRas; 
extern void get_hardware_status(RAS_HW_STATUS_STRUCT& myhw );
extern void _tft_im_here(int line);
extern TFT_eSPI tft;
extern void _stall();
#define stall() Serial.print("Stall @ line # ");Serial.println(__LINE__);_stall();


void PrintWiFiAddressOnTFT();  // local func prototype

unsigned long lastReconnectAttempt = 0;
const unsigned long reconnectInterval = 5000; // 5 seconds interval for reconnection attempts

void setup()
{
    // standard stuff here
    Serial.begin(115200);
    // Wait for the serial connection, but only for a limited time
    unsigned long startTime = millis();
    while (!Serial && millis() - startTime < 5000) {  // 5 seconds timeout
        delay(50);  // Give time for the serial port to connect if needed
    }

    // Rest of the setup code
    randomSeed(1234L);
   
    setup2(); // setup2() in Lightning_hw.cpp does all the hardware initialization

    // if your web page or XML are large, you may not get a call back from the web page
    // and the ESP will think something has locked up and reboot the ESP
    // not sure I like this feature, actually I kinda hate it
    // disable watch dog timer 0
    disableCore0WDT();

    // maybe disable watch dog timer 1 if needed
    //  disableCore1WDT();
    
    Serial.printf("\nESP-IDF version: %s\n", IDF_VER);

    // just an update to progress
    Serial.println("starting Webserver/WiFi, each dot below is an attempt to connect, we wait 500ms between attempts");
    delay(250);
    // if you have this #define USE_INTRANET,  you will connect to your home intranet, again makes debugging easier
    
    // install a couple of wifi event handlers
    //WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
#ifdef USE_INTRANET

    // 11-JAN-2025 rpb  try a list of AP's as defined in creds.h
    startupWiFiWithList();
	delay(250);
    WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    Actual_IP = WiFi.localIP();
    // since i should be connected here normally, setup reconnect upon loss
#endif

    // if you don't have #define USE_INTRANET, here's where you will create and access point
    // an intranet with no internet connection. But Clients can connect to your intranet and see
    // the web page you are about to serve up
#ifndef USE_INTRANET
    WiFi.softAP(AP_SSID, AP_PASS);
    delay(100);
    WiFi.softAPConfig(PageIP, gateway, subnet);
    delay(100);
    Actual_IP = WiFi.softAPIP();
    Serial.print("IP address: ");
    Serial.println(Actual_IP);
#endif
    PrintWifiStatus();
delay(1000);
    // setup the web server
    // these calls will handle data coming back from your web page
    // this one is a page request, upon ESP getting / string the web page will be sent
 
    // upon typing the URL into the browser, the browser makes a request of the server that is null, which gets intercepted
    // by this "lambda" function to cause the basic authentication (which is built into the browser) dialog to pop-up.
    // until the authentication is successful, it spins in a circle constantly asking for authentication.
    //  once the authentication is successful, the function "SendWebsite()" is called, which downloads to the browser
    // the http/javascript webpage.
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
            if (!request->authenticate(WWW_USERNAME, WWW_PASSWORD)) {  // see creds.h for definitions of uname/pwd
               // return request->requestAuthentication();  // keep spinning, looking for auth to be OK
            }   
            SendWebsite(request);
        }
    );

    // upon ESP getting /XML string, ESP will build and send the XML, this is how we refresh
    // just parts of the web page
    // server.on("/xml", HTTP_GET, SendXML); // use this to send XML data to the web page
    server.on("/json", HTTP_GET, SendJSON); // use this to send JSON data to the web page

    // upon ESP getting various handler strings, ESP will execute the corresponding handler functions
    // same notion for the following .on calls
    server.on("/PWR_Select", HTTP_PUT, PWR_Select_handler);

    // handle terminal input from client
    server.on("/TermInput", HTTP_PUT, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "PUT request received");
    }, NULL, handleBody);  // Handle the body with this function

/*    server.on("/R2Select", HTTP_PUT, R2Select_handler);
    server.on("/R3Select", HTTP_PUT, R3Select_handler);
    server.on("/R4Select", HTTP_PUT, R4Select_handler);
    server.on("/Aux1_OnSel", HTTP_PUT, Aux1_OnSelect_handler);
    server.on("/Aux1_OffSel", HTTP_PUT, Aux1_OffSelect_handler);
    server.on("/Aux2_OnSel", HTTP_PUT, Aux2_OnSelect_handler);
    server.on("/Aux2_OffSel", HTTP_PUT, Aux2_OffSelect_handler);
    server.on("/ALL_GND", HTTP_PUT, All_Grounded_handler); */

    // finally begin the server
    ElegantOTA.begin(&server);
    server.begin();
//TFT_IM_HERE();

    WebText("\nOut of setup() - enter ? for cmnd help\n"); 
    Serial.print("CMD> ");

    delay(25);

}

void loop()
{
    unsigned long loopBegin = millis();

    // Check if WiFi is disconnected and it's time to attempt reconnection
    if (!bWiFi_Connected && millis() - lastReconnectAttempt >= reconnectInterval) {
        Serial.println("Attempting to reconnect to WiFi...");
        WiFi.reconnect();
        lastReconnectAttempt = millis(); // Reset the timer
    }

    // Monitor IP Address every 3 seconds on TFT, see if we lost it
    static unsigned long lastIntervalTime = 0;
    if (millis() - lastIntervalTime > 3000) {
        PrintWiFiAddressOnTFT();
        lastIntervalTime = millis();
    }

    // All the hardware management is handled by loop2() in xxx_hw.ino/cpp
    loop2(gHostCmd);

    // Give other threads/tasks a chance
    delay(2);

    // Now measure elapsed time for the whole loop
    unsigned long elapsed = millis() - loopBegin;
    if (elapsed > gLongest_loop_time) {
        gLongest_loop_time = elapsed;
        Serial.print(F("EXEC_LOOP MAX LATENCY MS: "));
        Serial.println(gLongest_loop_time, DEC);
    }
}

// wifi event handlers.
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    // we got connected, but have not yet received an IP address...
    Serial.println("\tConnected to AP successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
    bWiFi_Connected = true;
    
    // **ADD THIS LINE** - Disable WiFi power management sleep
    WiFi.setSleep(false);
    Serial.println("WiFi sleep mode disabled");

    Serial.print("\n\tWiFi EVENT_WIFI_STA_GOT_IP address: ");
    Serial.println(WiFi.localIP());
    Serial.printf("\tHostname set to: %s\n", WiFi.getHostname());
    
    // Initialize mDNS for local hostname resolution
    if (!MDNS.begin(WiFi.getHostname())) {
        Serial.println("\tError setting up mDNS responder!");
    } else {
        Serial.printf("\tmDNS responder started. Hostname: %s.local\n", WiFi.getHostname());
        Serial.printf("\tTry accessing: http://%s.local/\n", WiFi.getHostname());
        // Add service to mDNS registry
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("lightning", "tcp", 80);
        Serial.println("\tmDNS services registered (http, lightning)");
    }
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    bWiFi_Connected = false;

    Serial.println("\r\tDisconnected from WiFi access point");
    Serial.print("\tWiFi lost connection. Reason: ");
    Serial.println(info.wifi_sta_disconnected.reason);

    // Start the reconnection timer
    lastReconnectAttempt = millis();
}

void PrintWiFiAddressOnTFT() {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5,150,2);
    tft.setTextPadding(tft.width());  // pad it the width of the screen.
    if( bWiFi_Connected ) {
        tft.printf("Network: %s %s %d dBm", WiFi.getHostname(), WiFi.localIP().toString(), WiFi.RSSI()); 
    } else {
        tft.printf("Network: DISCONNECTED   "); 
    }
    tft.setTextSize(1);
}
// functional message handlers 
void PWR_Select_handler(AsyncWebServerRequest *request) {
    //Serial.println(F("POWER_SELECT_Handler"));
    gHostCmd = HostCmdEnum::PWR_SELECT;
    //pRas->PWR_Button = pRas->PWR_Button ? 0 : 1;  // toggle the power button request.
    
    //pRas->PWR_Button ? WebText("Power ON\n") : WebText("Power OFF\n");
        
    // for now just send a 200 OK msg
    request->send(200, "text/plain", ""); // Send web page ack
}

// WiFi setup print out
void PrintWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print(F("SSID: "));
    Serial.print(WiFi.SSID());
  
    // print your boards IP address:
    Actual_IP = WiFi.localIP();
    Serial.print(F(", IP Address: "));
    Serial.print(Actual_IP);
	
    Serial.printf(",  Hostname: %s ", WiFi.getHostname());
  
    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print(F(", signal strength (RSSI):"));
    Serial.print(rssi);
    Serial.println(F(" dBm"));
}

// code to send the main web page
// PAGE_MAIN is a large char defined in Lightning_Web.h, which is derived from .http file
void SendWebsite(AsyncWebServerRequest *request)
{
    //Serial.println("sending web page");
    // Log the client IP using WebText
    IPAddress clientIP = request->client()->remoteIP();
    Serial.printf("Web client connected: %s\n", clientIP.toString().c_str());
    request->send(200, "text/html", PAGE_MAIN);  // This loads the webpage in RAS_Web.h as the web page to the browser.
}

/**
 * @brief Escapes special XML characters in a string to ensure it is safe for inclusion in an XML document.
 * 
 * @param input The input string that may contain special XML characters.
 * @return A new string with special XML characters replaced by their corresponding escape sequences.
 * 
 * @note This function replaces the following characters:
 *       - '&' with "&amp;"
 *       - '<' with "&lt;"
 *       - '>' with "&gt;"
 *       - '"' with "&quot;"
 *       - '\'' with "&apos;"
 * 
 * @details This function is necessary because XML has special characters that must be escaped to avoid breaking the XML structure.
 *          For example, '<' and '>' are used to define tags, and '&' is used for entity references. Escaping these characters ensures
 *          that the XML document remains valid and can be parsed correctly.
 */
String escapeXml(const String& input) {
    String out;
    out.reserve(input.length()); // Reserve at least the input size

    for (size_t i = 0; i < input.length(); ++i) {
        char c = input[i];
        switch (c) {
            case '&':  out += F("&amp;");  break;
            case '<':  out += F("&lt;");   break;
            case '>':  out += F("&gt;");   break;
            case '"':  out += F("&quot;"); break;
            case '\'': out += F("&apos;"); break;
            default:   out += c;           break;
        }
    }
    return out;
}

/**
 * @brief Wraps a given content string in an XML tag, using a CDATA section if possible.
 * 
 * @param tagName The name of the XML tag.
 * @param content The content to be wrapped inside the XML tag.
 * @return A string containing the content wrapped in the specified XML tag.
 * 
 * @note If the content contains the sequence "]]>", which is not allowed inside a CDATA section, the content is escaped using `escapeXml`
 *       and wrapped in standard XML tags. Otherwise, the content is wrapped in a CDATA section for better readability.
 * 
 * @details This function is necessary to safely include dynamic content in an XML document. CDATA sections allow raw text to be included
 *          without escaping special characters, but they cannot contain the sequence "]]>". This function ensures that the content is
 *          safely included in the XML document, either as a CDATA section or as escaped text, depending on the content.
 */
String wrapXmlTag(String tagName, const String& content) {
    if (content.indexOf("]]>") != -1) {
        // Cannot safely use CDATA — fall back to escaping
        return "<" + tagName + ">" + escapeXml(content) + "</" + tagName + ">";
    } else {
        // Safe to use CDATA
        return "<" + tagName + "><![CDATA[" + content + "]]></" + tagName + ">";
    }
}

void SendJSON(AsyncWebServerRequest *request) {
    // Use a slightly larger buffer if needed, but keep it reasonable
    StaticJsonDocument<4096> doc;

    get_hardware_status(RAS_Status);
    doc["PWR"] = pRas->PWR_Button ? 1 : 0;
    doc["NOISE_ACC"] = pRas->noise_accum;
    doc["NOISE_ET"] = pRas->noise_et;
    doc["DISTURB_ACC"] = pRas->disturber_accum;
    doc["DISTURB_ET"] = pRas->disturber_et;
    doc["STRIKE_ACC"] = pRas->strike_accum;
    doc["STRIKE_ET"] = pRas->strike_et;
    doc["STRIKE_DIST"] = pRas->strike_distance;
    doc["STRIKE_ENER"] = pRas->strike_energy;

    JsonArray rates = doc.createNestedArray("RATES");
    rates.add(pRas->strikeRate);
    rates.add(pRas->disturberRate);
    rates.add(pRas->noiseRate);
    rates.add(pRas->purgeRate);

    doc["VER"] = RAS_Status.pSoftwareVersion;

    if (!RingBuffer.isEmpty()) {
        char infoBuffer[4096] = {0};
        RingBuffer.concat_and_remove_all(infoBuffer);
        doc["INFO"] = infoBuffer;
    }

    // Stream JSON directly to response
    AsyncResponseStream *response = request->beginResponseStream("application/json");
    serializeJson(doc, *response);
    request->send(response);

    // Optionally, print JSON to serial for debugging
    #ifdef DEBUG
    String debugBuffer;
    serializeJson(doc, debugBuffer);
    Serial.println("JSON: " + debugBuffer);
    #endif
}
// this function will get the current status of all hardware and create the XML message with status
// and send it back to the page
// void SendXML(AsyncWebServerRequest *request) {
//     // New implementation using String with reserved memory
//     static char tempBuffer[4096] = {0};
//     static String xmlData;
//     xmlData.reserve(4096); // Reserve memory to minimize reallocations

//     xmlData = "<?xml version='1.0'?>\n<Data>\n";

//     // Add hardware status
//     get_hardware_status(RAS_Status);
//     xmlData += pRas->PWR_Button ? "<PWR>1</PWR>\n" : "<PWR>0</PWR>\n";

//     // Add other tags (unchanged)
//     xmlData += "<NOISE_ACC>" + String(pRas->noise_accum) + "</NOISE_ACC>\n";
//     xmlData += "<NOISE_ET>" + String(pRas->noise_et) + "</NOISE_ET>\n";
//     xmlData += "<DISTURB_ACC>" + String(pRas->disturber_accum) + "</DISTURB_ACC>\n";
//     xmlData += "<DISTURB_ET>" + String(pRas->disturber_et) + "</DISTURB_ET>\n";
//     xmlData += "<STRIKE_ACC>" + String(pRas->strike_accum) + "</STRIKE_ACC>\n";
//     xmlData += "<STRIKE_ET>" + String(pRas->strike_et) + "</STRIKE_ET>\n";
//     xmlData += "<STRIKE_DIST>" + String(pRas->strike_distance,1) + "</STRIKE_DIST>\n";
//     xmlData += "<STRIKE_ENER>" + String(pRas->strike_energy) + "</STRIKE_ENER>\n";

//     // --- Add new RATES tag as a comma-separated list from pRas rate fields ---
//     // Use the float rate fields, not the *_accum fields
//     xmlData += "<RATES>";
//     xmlData += String(pRas->strikeRate, 2) + ",";
//     xmlData += String(pRas->disturberRate, 2) + ",";
//     xmlData += String(pRas->noiseRate, 2) + ",";
//     xmlData += String(pRas->purgeRate, 2); // No trailing comma
//     xmlData += "</RATES>\n";
//     // -------------------------------------------------------------------------

//     // Add version
//     xmlData += "<VER>";
//     xmlData += RAS_Status.pSoftwareVersion;
//     xmlData += "</VER>\n";

//     // Add INFO tag if the RingBuffer is not empty
//     if (!RingBuffer.isEmpty()) { 
//         RingBuffer.concat_and_remove_all(tempBuffer);
//         xmlData += wrapXmlTag("INFO", tempBuffer);
//     }
//     xmlData += "</Data>\n";
    
//     // Send the XML data
//     request->send(200, "text/xml", xmlData);
// }
// /*
// // Original implementation using global XML_buffer
// void SendXML(AsyncWebServerRequest *request) {
//     XML_buffer[0] = 0; // make sure the buffer is empty before we start building it.

//     // Collect the hardware status that reflects the current state of the buttons/LEDs
//     get_hardware_status(RAS_Status);

//     strcpy(XML_buffer, "<?xml version='1.0'?>\n<Data>\n");

//     strcat(XML_buffer, pRas->PWR_Button ? "<PWR>1</PWR>\n" : "<PWR>0</PWR>\n");

//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "NOISE_ACC", pRas->noise_accum);
//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "NOISE_ET", pRas->noise_et);
//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "DISTURB_ACC", pRas->disturber_accum);
//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "DISTURB_ET", pRas->disturber_et);
//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "STRIKE_ACC", pRas->strike_accum);
//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "STRIKE_ET", pRas->strike_et);
//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "STRIKE_DIST", pRas->strike_distance);
//     buildXmlTag(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer) - 1, "STRIKE_ENER", pRas->strike_energy);

//     snprintf(XML_buffer + strlen(XML_buffer), sizeof(XML_buffer) - strlen(XML_buffer),
//              "<VER>%s</VER>\n", RAS_Status.pSoftwareVersion);

//     if (!RingBuffer.isEmpty()) {
//         strcat(XML_buffer + strlen(XML_buffer), "<INFO>");
//         RingBuffer.concat_all(XML_buffer + strlen(XML_buffer));
//         strcat(XML_buffer + strlen(XML_buffer), "</INFO>\n");
//     }
//     RingBuffer.delete_all();

//     strcat(XML_buffer, "</Data>\n");

//     request->send(200, "text/xml", XML_buffer);
// }
// */

void WebText(const char *format, ...) {
    // Temporary buffer to hold the formatted string
    static char tempBuf[2048] = {0};
    // Initialize the variable argument list
    va_list args;
    va_start(args, format);

    // Format the input string with the provided arguments
    vsnprintf(tempBuf, sizeof(tempBuf), format, args);

    // End processing the variable arguments
    va_end(args);

    Serial.print(tempBuf);
    RingBuffer.put(tempBuf); // add it to the ring buffer, will ovewrite oldest string if not serviced, but safe
    return;
}

extern CommandParser cp;
void handleBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
    // The body content is received in chunks, so we need to process it
    String bodyContent = "";
    for (size_t i = 0; i < len; i++) {
        bodyContent += (char)data[i];  // Append each byte to the string
    }

    // Print the body content
    //Serial.printf("Body content received: --%s--\n", bodyContent);

    // now handle the command just as a we would a command that was input on the serial port terminal.
    cp.processInput(bodyContent.c_str());

    // Send a response back to the client
    request->send(200, "text/plain", "Rcvd PUT/POST termInput request body");
}

/*
    startup the wifi given a list of AP's, sorted in order of preference.  the array of AP's is in creds.h
    scan the wifi network, only attempt to connect to the AP's in the list.
    this will add some time to the startup, but will allow the ESP to connect to the best AP available.
    useful for a device that may be moved around and connected to different AP's, like iphone vs local wifi AP.
    11-Jan-2025 w9zv
*/

// Generate unique hostname using last 4 hex digits of MAC address
String generateUniqueHostname() {
    uint8_t mac[6];
    WiFi.macAddress(mac);
    
    // Use last 2 octets of MAC address as 4-character hex suffix
    char macSuffix[5]; // 4 chars + null terminator
    snprintf(macSuffix, sizeof(macSuffix), "%02X%02X", mac[4], mac[5]);
    
    // Create unique hostname: storm-monitor-AB12
    String uniqueHostname = String(HOSTNAME) + String(macSuffix);
    
    return uniqueHostname;
}

void startupWiFiWithList()
{
    // Generate unique hostname based on MAC address
    String uniqueHostname = generateUniqueHostname();
    Serial.printf("Setting unique hostname: %s\n", uniqueHostname.c_str());
    
    // Initialize WiFi with unique hostname
    WiFi.setHostname(uniqueHostname.c_str());
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE);
    delay(100);

    // Scan for available networks
    Serial.println("Scanning for WiFi networks... stby");
    int numNetworks = WiFi.scanNetworks();
    showWiFiNetworksFound(numNetworks);

    // Try to connect to each AP in the priority list
    for (int i = 0; i < numAPs; i++) {
        for (int j = 0; j < numNetworks; j++) {
            if (strcmp(WiFi.SSID(j).c_str(), apList[i].ssid) == 0) {
                Serial.printf("Trying to connect to: %s ", apList[i].ssid);

                WiFi.begin(apList[i].ssid, apList[i].password);

                // Wait for connection
                int timeout = 0;
                while (WiFi.status() != WL_CONNECTED && timeout < 40) {
                    Serial.print(".");
                    delay(500);
                    timeout++;
                }

                if (WiFi.status() == WL_CONNECTED) {
                    Serial.printf("Connected to %s\n", apList[i].ssid);
                    // Serial.printf("\nConnected !\n");
                    break; // stop trying to connect to other AP's found in the scan
                } else {
                    Serial.printf("\nFailed to connect to %s\n", apList[i].ssid);
                }
            }
        }
        if (WiFi.status() == WL_CONNECTED) {
            break; // stop trying to connect to other AP's on the preferred list
        }
    }
    // Delete the scan result to free memory
    WiFi.scanDelete();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Failed to connect to any AP");
    }
}

void showWiFiNetworksFound(int numNetworks) {
    if (numNetworks == 0) {
        Serial.println("no networks found");
    } else {
        Serial.print(numNetworks);
        Serial.println(" networks found");
        Serial.println("Nr | SSID                             | RSSI | CH | Encryption");
        for (int i = 0; i < numNetworks; ++i) {
            // Print SSID and RSSI for each network found
            Serial.printf("%2d", i + 1);
            Serial.print(" | ");
            Serial.printf("%-32.32s", WiFi.SSID(i).c_str());
            Serial.print(" | ");
            Serial.printf("%4d", WiFi.RSSI(i));
            Serial.print(" | ");
            Serial.printf("%2d", WiFi.channel(i));
            Serial.print(" | ");

            switch (WiFi.encryptionType(i)) {
            case WIFI_AUTH_OPEN:
                Serial.print("open");
                break;
            case WIFI_AUTH_WEP:
                Serial.print("WEP");
                break;
            case WIFI_AUTH_WPA_PSK:
                Serial.print("WPA");
                break;
            case WIFI_AUTH_WPA2_PSK:
                Serial.print("WPA2");
                break;
            case WIFI_AUTH_WPA_WPA2_PSK:
                Serial.print("WPA+WPA2");
                break;
            case WIFI_AUTH_WPA2_ENTERPRISE:
                Serial.print("WPA2-EAP");
                break;
            case WIFI_AUTH_WPA3_PSK:
                Serial.print("WPA3");
                break;
            case WIFI_AUTH_WPA2_WPA3_PSK:
                Serial.print("WPA2+WPA3");
                break;
            case WIFI_AUTH_WAPI_PSK:
                Serial.print("WAPI");
                break;
            default:
                Serial.print("unknown");
            }
            Serial.println("");
        } 
        Serial.println("END of Networks discovered");
    }
}

// void printRequestDetails(AsyncWebServerRequest *request) {
//     // Print the HTTP method (GET, POST, etc.)
//     Serial.print("Method: ");
//     switch (request->method()) {
//         case HTTP_GET: Serial.println("GET"); break;
//         case HTTP_POST: Serial.println("POST"); break;
//         case HTTP_DELETE: Serial.println("DELETE"); break;
//         case HTTP_PUT: Serial.println("PUT"); break;
//         case HTTP_PATCH: Serial.println("PATCH"); break;
//         case HTTP_HEAD: Serial.println("HEAD"); break;
//         case HTTP_OPTIONS: Serial.println("OPTIONS"); break;
//         default: Serial.println("UNKNOWN"); break;
//     }

//     // Print the URL
//     Serial.print("URL: ");
//     Serial.println(request->url());

//     // Print the client's IP address
//     Serial.print("Client IP: ");
//     Serial.println(request->client()->remoteIP().toString());

//     // Print the HTTP version
//     Serial.print("HTTP Version: ");
//     Serial.println(request->version());

//     // Print the number of headers
//     Serial.print("Headers: ");
//     Serial.println(request->headers());

//     // Print all headers
//     for (int i = 0; i < request->headers(); i++) {
//         AsyncWebHeader *header = request->getHeader(i);
//         Serial.print(header->name());
//         Serial.print(": ");
//         Serial.println(header->value());
//     }

//     // Print query parameters if any
//     Serial.print("Params: ");
//     Serial.println(request->params());
//     for (int i = 0; i < request->params(); i++) {
//         AsyncWebParameter *p = request->getParam(i);
//         Serial.print(p->name());
//         Serial.print(": ");
//         Serial.println(p->value());
//     }

//     // Print the content length (useful for POST/PUT requests)
//     Serial.print("Content Length: ");
//     Serial.println(request->contentLength());
// }

// Function to handle POST request body data
// void handleRequestBody(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
//     String body = String((char*)data).substring(0, len);
//     Serial.println("Body received:");
//     Serial.println(body);
// }

// Function to handle the body data
// void handleBodyData(AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
// Serial.println("handleBodyData() - ");
//     // Convert the body data to a string
//     String body = String((char*)data).substring(0, len);

//     // Print the body data to the serial monitor
//     Serial.println("Body received:");
//     Serial.println(body);

//     // Send a response back to the client
//     request->send(200, "text/plain", "Data received: " + body);
// }

// void printRequestDetails(AsyncWebServerRequest *request) {
//     // Print the HTTP method (GET, POST, etc.)
//     Serial.print("Method: ");
//     switch (request->method()) {
//         case HTTP_GET: Serial.println("GET"); break;
//         case HTTP_POST: Serial.println("POST"); break;
//         case HTTP_DELETE: Serial.println("DELETE"); break;
//         case HTTP_PUT: Serial.println("PUT"); break;
//         case HTTP_PATCH: Serial.println("PATCH"); break;
//         case HTTP_HEAD: Serial.println("HEAD"); break;
//         case HTTP_OPTIONS: Serial.println("OPTIONS"); break;
//         default: Serial.println("UNKNOWN"); break;
//     }

//     // Print the URL
//     Serial.print("URL: ");
//     Serial.println(request->url());

//     // Print the client's IP address
//     Serial.print("Client IP: ");
//     Serial.println(request->client()->remoteIP().toString());

//     // Print the HTTP version
//     Serial.print("HTTP Version: ");
//     Serial.println(request->version());

//     // Print the number of headers
//     Serial.print("Headers: ");
//     Serial.println(request->headers());

//     // Print all headers
//     for (int i = 0; i < request->headers(); i++) {
//         AsyncWebHeader *header = request->getHeader(i);
//         Serial.print(header->name());
//         Serial.print(": ");
//         Serial.println(header->value());
//     }

//     // Print query parameters if any
//     Serial.print("Params: ");
//     Serial.println(request->params());
//     for (int i = 0; i < request->params(); i++) {
//         AsyncWebParameter *p = request->getParam(i);
//         Serial.print(p->name());
//         Serial.print(": ");
//         Serial.println(p->value());
//     }

//     // Print the content length (useful for POST/PUT requests)
//     Serial.print("Content Length: ");
//     Serial.println(request->contentLength());

//     // Print the body if it's a POST request
//     if (request->method() == HTTP_POST || request->method() == HTTP_PUT) {
//         request->onBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
//             Serial.println("Body: ");
//             for (size_t src/main.cpp:438:18: error: 'class AsyncWebServerRequest' has no member named 'onBody'i = 0; i < len; i++) {
//                 Serial.print((char)data[i]);
//             }
//             Serial.println();
//         });
//     }
// }