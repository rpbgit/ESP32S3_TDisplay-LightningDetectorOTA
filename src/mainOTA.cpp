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
// *NEW 
void performHealthCheck();
void analyzeMemoryUsage();
const char* getWiFiStatusString(int status);
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
        WebText("Attempting WiFi reconnection...\n");
        WiFi.reconnect();
        lastReconnectAttempt = millis();
    }

    // Monitor IP Address every 3 seconds on TFT
    static unsigned long lastIntervalTime = 0;
    if (millis() - lastIntervalTime > 3000) {
        PrintWiFiAddressOnTFT();
        lastIntervalTime = millis();
    }

    // **NEW** - Perform automated health checks (includes full system health on first run)
    performHealthCheck();

    // Keep memory analysis (every 30 minutes)
    static unsigned long lastMemoryAnalysis = 0;
    if (millis() - lastMemoryAnalysis > 1800000) {  // 30 minutes
        analyzeMemoryUsage();
        lastMemoryAnalysis = millis();
    }

    // Reset longest loop time every hour
    static unsigned long lastLoopReset = 0;
    if (millis() - lastLoopReset > 3600000) {  // 1 hour
        if (gLongest_loop_time > 50) {
            WebText("Hourly report - Max loop time: %lu ms\n", gLongest_loop_time);
        }
        gLongest_loop_time = 0;
        lastLoopReset = millis();
    }

    // Hardware management
    loop2(gHostCmd);
    delay(2);

    // Loop timing
    unsigned long elapsed = millis() - loopBegin;
    if (elapsed > gLongest_loop_time) {
        gLongest_loop_time = elapsed;
        WebText("EXEC_LOOP MAX LATENCY MS: %lu ms\n", gLongest_loop_time);
        
        if (elapsed > 100) {
            WebText("WARNING: Long loop time: %lu ms\n", elapsed);
        }
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
    
    // Disable WiFi power management sleep
    WiFi.setSleep(false);
    WebText("WiFi connected - sleep mode disabled\n");

    WebText("WiFi EVENT_WIFI_STA_GOT_IP address: %s\n", WiFi.localIP().toString().c_str());
    WebText("Hostname set to: %s\n", WiFi.getHostname());
    WebText("SSID: %s\n", WiFi.SSID().c_str());
    WebText("BSSID: %s\n", WiFi.BSSIDstr().c_str());
    WebText("RSSI: %d dBm\n", WiFi.RSSI());
    WebText("Channel: %d\n", WiFi.channel());
    WebText("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    WebText("Subnet: %s\n", WiFi.subnetMask().toString().c_str());
    WebText("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    
    // Log connection quality
    int rssi = WiFi.RSSI();
    if (rssi > -50) {
        WebText("Signal strength: Excellent (%d dBm)\n", rssi);
    } else if (rssi > -60) {
        WebText("Signal strength: Good (%d dBm)\n", rssi);
    } else if (rssi > -70) {
        WebText("Signal strength: Fair (%d dBm)\n", rssi);
    } else {
        WebText("WARNING: Signal strength: Poor (%d dBm) - may cause disconnections\n", rssi);
    }
    
    // Initialize mDNS for local hostname resolution
    if (!MDNS.begin(WiFi.getHostname())) {
        WebText("ERROR: mDNS setup failed\n");
    } else {
        WebText("mDNS responder started. Hostname: %s.local\n", WiFi.getHostname());
        WebText("Try accessing: http://%s.local/\n", WiFi.getHostname());
        // Add service to mDNS registry
        MDNS.addService("http", "tcp", 80);
        MDNS.addService("lightning", "tcp", 80);
        WebText("mDNS services registered (http, lightning)\n");
    }
    
    // Log memory status after successful connection
    WebText("Free heap after connection: %d bytes\n", ESP.getFreeHeap());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
    bWiFi_Connected = false;

    WebText("Disconnected from WiFi access point\n");
    WebText("WiFi lost connection. Reason: %d (", info.wifi_sta_disconnected.reason);
    
    // Add detailed reason descriptions
    switch(info.wifi_sta_disconnected.reason) {
        case 1: 
            WebText("UNSPECIFIED - General failure)\n");
            break;
        case 2: 
            WebText("AUTH_EXPIRE - Authentication expired)\n");
            break;
        case 3: 
            WebText("AUTH_LEAVE - Deauthenticated because sending STA is leaving)\n");
            break;
        case 4: 
            WebText("ASSOC_EXPIRE - Disassociated due to inactivity)\n");
            break;
        case 5: 
            WebText("ASSOC_TOOMANY - Too many clients)\n");
            break;
        case 6: 
            WebText("NOT_AUTHED - Not authenticated)\n");
            break;
        case 7: 
            WebText("NOT_ASSOCED - Not associated)\n");
            break;
        case 8: 
            WebText("ASSOC_LEAVE - ESP32 initiated disconnect (POWER MGMT?))\n");
            break;
        case 15: 
            WebText("4WAY_HANDSHAKE_TIMEOUT - WPA handshake timeout)\n");
            break;
        case 200: 
            WebText("BEACON_TIMEOUT - Lost beacon from AP)\n");
            break;
        case 201: 
            WebText("NO_AP_FOUND - Access point not found)\n");
            break;
        case 202: 
            WebText("AUTH_FAIL - Authentication failed)\n");
            break;
        case 203: 
            WebText("ASSOC_FAIL - Association failed)\n");
            break;
        case 204: 
            WebText("HANDSHAKE_TIMEOUT - Handshake timeout)\n");
            break;
        default: 
            WebText("UNKNOWN_%d)\n", info.wifi_sta_disconnected.reason);
            break;
    }

    // Log additional WiFi status information
    WebText("Previous RSSI: %d dBm\n", WiFi.RSSI());
    WebText("WiFi Mode: %d\n", WiFi.getMode());
    WebText("Auto Reconnect: %s\n", WiFi.getAutoReconnect() ? "enabled" : "disabled");
    
    // Log memory status at disconnect
    WebText("Free heap at disconnect: %d bytes\n", ESP.getFreeHeap());
    WebText("Min free heap: %d bytes\n", ESP.getMinFreeHeap());

    // Start the reconnection timer
    lastReconnectAttempt = millis();
    
    // Force a memory check and log if memory is critically low
    if (ESP.getFreeHeap() < 10000) {
        WebText("WARNING: Low memory at WiFi disconnect - %d bytes free\n", ESP.getFreeHeap());
    }
}

void PrintWiFiAddressOnTFT() {
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5,150,2);
    tft.setTextPadding(tft.width());  // pad it the width of the screen.
    
    if( bWiFi_Connected ) {
        int rssi = WiFi.RSSI();
        tft.printf("Network: %s %s %d dBm", WiFi.getHostname(), WiFi.localIP().toString(), rssi); 
        
        // Log weak signals that might cause future disconnections
        if (rssi < -75) {
            static unsigned long lastWeakSignalLog = 0;
            if (millis() - lastWeakSignalLog > 30000) {  // Log every 30 seconds
                WebText("WARNING: Weak WiFi signal: %d dBm\n", rssi);
                lastWeakSignalLog = millis();
            }
        }
    } else {
        tft.printf("Network: DISCONNECTED   "); 
        
        // Show reconnection status
        static unsigned long lastReconnectDisplay = 0;
        if (millis() - lastReconnectDisplay > 5000) {  // Update every 5 seconds
            unsigned long timeSinceDisconnect = millis() - lastReconnectAttempt;
            if (timeSinceDisconnect < reconnectInterval) {
                WebText("Reconnect attempt in %lu ms\n", reconnectInterval - timeSinceDisconnect);
            }
            lastReconnectDisplay = millis();
        }
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

void SendJSON(AsyncWebServerRequest *request) {
    // Use StaticJsonDocument to avoid heap allocation (suppressed deprecation warning)
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

#pragma GCC diagnostic pop

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

// Helper function to convert WiFi status to string
const char* getWiFiStatusString(int status) {
    switch(status) {
        case WL_IDLE_STATUS: return "IDLE";
        case WL_NO_SSID_AVAIL: return "NO_SSID_AVAILABLE";
        case WL_SCAN_COMPLETED: return "SCAN_COMPLETED";
        case WL_CONNECTED: return "CONNECTED";
        case WL_CONNECT_FAILED: return "CONNECT_FAILED";
        case WL_CONNECTION_LOST: return "CONNECTION_LOST";
        case WL_DISCONNECTED: return "DISCONNECTED";
        default: return "UNKNOWN";
    }
}

// Health check function that monitors system status and alerts on issues
void performHealthCheck() {
    static unsigned long lastHealthCheck = 0;
    static int healthCheckCount = 0;
    static bool firstHealthCheckDone = false;
    
    // Check if it's time for a health check
    bool timeForCheck = false;
    
    if (!firstHealthCheckDone) {
        // First health check - run immediately
        timeForCheck = true;
        firstHealthCheckDone = true;
    } else {
        // Subsequent checks - only run every 2 minutes
        if (millis() - lastHealthCheck >= 120000) {
            timeForCheck = true;
        }
    }
    
    if (!timeForCheck) {
        return;
    }
    
    lastHealthCheck = millis();
    healthCheckCount++;
    
    // For the first health check, print full system health report 
    if (healthCheckCount == 1) {
        WebText("=== INITIAL SYSTEM HEALTH CHECK ===\n");
        
        WebText("Free heap: %d bytes\n", ESP.getFreeHeap());
        WebText("Min free heap: %d bytes\n", ESP.getMinFreeHeap());
        WebText("Largest free block: %d bytes\n", ESP.getMaxAllocHeap());
        WebText("Total heap size: %d bytes\n", ESP.getHeapSize());
        WebText("WiFi status: %d (%s)\n", WiFi.status(), getWiFiStatusString(WiFi.status()));
        
        if (bWiFi_Connected) {
            WebText("RSSI: %d dBm\n", WiFi.RSSI());
            WebText("SSID: %s\n", WiFi.SSID().c_str());
            WebText("IP: %s\n", WiFi.localIP().toString().c_str());
            WebText("Hostname: %s\n", WiFi.getHostname());
        }
        
        WebText("Uptime: %lu ms (%.1f hours)\n", millis(), millis() / 3600000.0);
        WebText("Longest loop time: %lu ms\n", gLongest_loop_time);
        WebText("ESP chip: %s\n", ESP.getChipModel());
        WebText("CPU frequency: %d MHz\n", ESP.getCpuFreqMHz());
        WebText("Flash size: %d bytes\n", ESP.getFlashChipSize());
        WebText("Flash speed: %d Hz\n", ESP.getFlashChipSpeed());
        WebText("Sketch size: %d bytes\n", ESP.getSketchSize());
        WebText("Free sketch space: %d bytes\n", ESP.getFreeSketchSpace());
        WebText("Temperature: %.1f°C\n", temperatureRead());
        
        // Initial memory analysis
        size_t totalHeap = ESP.getHeapSize();
        size_t freeHeap = ESP.getFreeHeap();
        size_t usedHeap = totalHeap - freeHeap;
        size_t maxAllocHeap = ESP.getMaxAllocHeap();
        
        float fragmentation = 0;
        if (freeHeap > 0) {
            fragmentation = ((float)(freeHeap - maxAllocHeap) / freeHeap) * 100.0;
        }
        WebText("Heap fragmentation: %.1f%%\n", fragmentation);
        WebText("Stack high water mark: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
        
        WebText("=== INITIAL HEALTH CHECK COMPLETE ===\n");
    } else {
        // Regular scheduled health checks (abbreviated)
        WebText("--- Health Check #%d ---\n", healthCheckCount);
    }
    
    // Common health checks for all runs (first and subsequent)
    size_t freeHeap = ESP.getFreeHeap();
    size_t minFreeHeap = ESP.getMinFreeHeap();
    
    if (freeHeap < 20000) {
        WebText("ALERT: Critical low memory - %d bytes free\n", freeHeap);
    } else if (freeHeap < 50000) {
        WebText("WARNING: Low memory - %d bytes free\n", freeHeap);
    }
    
    // Check memory fragmentation
    if (freeHeap > minFreeHeap + 30000) {
        WebText("WARNING: Possible memory fragmentation (current: %d, min: %d)\n", freeHeap, minFreeHeap);
    }
    
    // Check WiFi health
    if (!bWiFi_Connected) {
        WebText("ALERT: WiFi disconnected for %lu seconds\n", (millis() - lastReconnectAttempt) / 1000);
    } else {
        int rssi = WiFi.RSSI();
        if (rssi < -80) {
            WebText("WARNING: Very weak WiFi signal: %d dBm\n", rssi);
        } else if (rssi < -70) {
            WebText("INFO: Weak WiFi signal: %d dBm\n", rssi);
        }
    }
    
    // Check loop performance
    if (gLongest_loop_time > 200) {
        WebText("WARNING: Slow loop performance - max %lu ms\n", gLongest_loop_time);
    }
    
    // Check temperature
    float temp = temperatureRead();
    if (temp > 80.0) {
        WebText("ALERT: High temperature: %.1f°C\n", temp);
    } else if (temp > 70.0) {
        WebText("WARNING: Elevated temperature: %.1f°C\n", temp);
    }
    
    // Final status
    if (healthCheckCount == 1) {
        WebText("Initial health check complete\n");
    } else if (freeHeap >= 50000 && bWiFi_Connected && gLongest_loop_time <= 200 && temp <= 70.0) {
        WebText("Health check complete - System OK\n");
    }
}

// Memory usage analysis function
void analyzeMemoryUsage() {
    WebText("--- Memory Analysis ---\n");
    
    size_t totalHeap = ESP.getHeapSize();
    size_t freeHeap = ESP.getFreeHeap();
    size_t usedHeap = totalHeap - freeHeap;
    size_t minFreeHeap = ESP.getMinFreeHeap();
    size_t maxAllocHeap = ESP.getMaxAllocHeap();
    
    WebText("Total heap: %d bytes\n", totalHeap);
    WebText("Used heap: %d bytes (%.1f%%)\n", usedHeap, (usedHeap * 100.0) / totalHeap);
    WebText("Free heap: %d bytes (%.1f%%)\n", freeHeap, (freeHeap * 100.0) / totalHeap);
    WebText("Min free heap: %d bytes\n", minFreeHeap);
    WebText("Max allocatable block: %d bytes\n", maxAllocHeap);
    
    // Calculate fragmentation
    float fragmentation = 0;
    if (freeHeap > 0) {
        fragmentation = ((float)(freeHeap - maxAllocHeap) / freeHeap) * 100.0;
    }
    WebText("Heap fragmentation: %.1f%%\n", fragmentation);
    
    if (fragmentation > 50.0) {
        WebText("WARNING: High heap fragmentation detected\n");
    }
    
    WebText("Stack high water mark: %d bytes\n", uxTaskGetStackHighWaterMark(NULL));
    WebText("----------------------\n");
}

