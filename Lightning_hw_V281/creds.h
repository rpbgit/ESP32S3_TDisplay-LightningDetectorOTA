
#ifndef CREDS_H
#define CREDS_H  // prevent multiple definitions on Arduino IDE.

// replace this with your home WiFi credentials
#define LOCAL_SSID "Home-w9zv_2.4G"
#define LOCAL_PASS "811tamms"
//#define LOCAL_SSID "Robert's iPhone"
//#define LOCAL_PASS "12345678"
//#define LOCAL_SSID "Max2.4"
//#define LOCAL_PASS "00988900"
// #define LOCAL_SSID "Binford 2.4"
// #define LOCAL_PASS "00988900"

// Change the hostname, hostname can only contain letters, numbers, and hyphens.
const char* HOSTNAME = "FVP-STORM-DETECTOR";

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "TestWebSite"
#define AP_PASS "023456789"

#define WWW_USERNAME "Alfred"   // 11-apr-2024 added to support basic authentication
#define WWW_PASSWORD "ENewman"  // only defs in here, no allocations.

#define MY_RAS_HTTP_PORT 50011   // for now, adjust as desired, this seems to work.

// Define a structure for the WiFi APs
typedef struct {
  const char* ssid;
  const char* password;
} WiFiAP;

// Define the WiFi APs in an array of WiFiAP structures
WiFiAP apList[] = {
  // highest priority is first
 // {"Verizon-MiFi8800L-B39A",  "39231b36"},  // jet direct card from zv's hanger
  {"Willco Electronics",   "132435465768"},  //wa9fvp's  WiFi
  {"Jacks iPad",   "1324354657"},  // Jack's Ipad
  {"JacksPhone", "132435465768"},  // Jack's Iphone
 // {"AP1",               "password1"}, // next lower priority
 // {"AP2",               "password2"}, // next lowest priority
  //{"AP3",               "password3"}, // mext lowest priority
  //{"AP4",               "password4"}  // lowest priority, add as many as you see fit.
};
const int numAPs = sizeof(apList) / sizeof(apList[0]);

#endif