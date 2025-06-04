/*

*/
#include <Arduino.h>
#define DISABLE_ALL_LIBRARY_WARNINGS 1 // disable all library warnings for tft_eSPI
#include "TFT_eSPI.h"
#include <Wire.h>
#include <AS3935SPI.h>
// #include <SPI.h>
//#include <SparkFun_AS3935.h>

#include "SkylineWithCall2.h"  // chicago skyline startup image
#include "pin_config.h"
#include "defs.h"
#include "CmdParser.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

/*
try to remember to bump this each time a functional mod is done
10-Sept-2024    v1.0    Release 1.0 added button to select simulation on boot, trimmed "last" numbers to mod 10000 so it 
                          cant overrun display space.
11-Sept-2024    v1.1    Added executive time counter to track max time in milliseconds to execute the loop() function
                          as well as added the loopt command to display it when desired. moved ET stuff into its own function.
11-Sept-2024    v1.2    Added REGTST to perform a r/w test of the sensor tune register... verifies we can r/w the device
12-Sept-2024    v1.3    Added variable screen brightness, including off.  pushing top button sequences thru settings.
21-Sept-2024    v1.4    Added ring buffer queue for collecting WebText content/manage the buffer prior to sending up to client, corrected WebText accumulated vs ET numbers swapped
03-Oct-2024     v1.5    fixed bogus initial ET counts (first time init error), fixed blocking on Serial.flush() (by checking 
                          if Serial is present) if not connected to port
12-Oct-2024     v1.6    updated libraries to Async v3.2.6, ElegantOTA v3.1.5, ESPAsyncWebServer v3.3.12, Sparkfun v1.4.9.  Added OTA updates for downloading
                          new binaries over the WiFi.  couple of other cleanup/fixes, added command CALOSC which will calibrate the two internal oscillators.
24-Oct-2024     v1.7    added network address and RSSI to TFT, strike alarm tones, and volume control to web page.  eliminated extra newlines in web page textbox.
25-Oct-2024     v1.8    expanded ringbuffer to handle dump commands (we were missing some lines), added \n to many webtext() calls, more granular volume control,
                          created container/div for buttons on html page to fix alignment issue.
15-Nov-2024     v2.0    changed TuningCap handler so it prints out value of register in decimal AND picofarads.
                        replaced the sparkfun AS3935 library with the AS3935MI library with self calibration of antenna and oscillators. lib
                            reference is https://github.com/christandlg/AS3935MI.  because of the degree of change major release number bumped
                        added self calibration of resonant freq of antenna into setup and via command.
                        removed STANDALONE compilation capability.
20-Nov-2024     v2.1    added command to put the sensor into a mode where it ouputs the resonant freq of the antenna onto the IRQ pin.
                            as a digital signal.  this is to allow the user to measure the freq with a scope or freq counter.
                        fixed tunecap command it was allowing invalid values (part of changing lib to AS3935MI).
                        when the LD is in CALMODE, the LD will not respond to lightning events.
                        added EVT_DSBL command to allow the user to disable the LD from responding to lightning events.
14-Dec-2024     v2.2     removed leftover sparkfun lib (no longer used),  added control of external WA9FVP latching power control
                          relay in non-blocking way (power_relay_xx functions).  must generate non-blocking pulse to set/reset relay. Mimic
                          a push on push off momentary behavior.
06-Jan-2025     v2.3    removed blocking delay forever loops in setup2 if sensor fails to initialize.   Added frequency display for CALANT command.
12-Jan-2025     v2.4    added scanning of wifi network and allow a list of APs in order of preference/priority to connect to.
06-Feb-2025 w9zv    v2.5    added sofware version into the XML code sent to the web page so it stays up to date. 
12-Feb-2025 w9zv    v2.6    sorted help command in alpha order, changed hardware SIMULATOR mode define, now in project compiler defs
                              some minor cosmetics on web page.
20-Mar-2025 w9zv    v2.7    Added manage_tft_power_field() function to manage the TFT power on/off field, changed the behavior of the simulator so that
                              the TFT power field always follows the state of the RELAY_SENSE_STATE pin.  TFT power field is now green for on, red for off
                              based on reading the RELAY_SENSE_STATE pin.  now the TFT power field is accurate
                              whether or not you have the hardware connected or running in sim mode.
02-Apr-2025 w9zv    v2.8    Added reconnection logic changes.  monitor the WiFi connection and if it goes down, try to reconnect every 5 seconds.  changed
                              hostname to FVP-STORM-MONITOR in creds.h
16-Apr-2025 w9zv    v2.9    Added clearing of statistics when lightning event that passes the lightning strike integration threshold count occurs.  if this is not done, once
                                the threshold is reached, we get an interrupt on every succeeding lightning event until the next self-purge. also added handling of 
                                the case where the interrupt occurred because of the device self purging of statistics, not because of a lightning event 
                                occurring.  register value of 0x00.  Also added elapsed time since the last self-purge to the log.   
19-Apr-2025 w9zv    v3.0    changed interrupt handling to a switch statement rather than if/else if.  this is more efficient and easier to read.  Also created
                                the station_management() function to allow for creating stats and and developing a strategy/algorithm for when to turn off
                                the power to the station, without affecting/influencing any existing functionality.  this is a work in progress.\
30-Apr-2025 w9zv    v3.1    fixed handling of strike energy when using simulator, bug introduced as side effect of adding PASSTHRU_INT
01-May-2025 w9zv    v4.0    quite a few changes to fix bug with concurrency of ring buffer with web page updates while it is being read by converting SendXML() 
                            to use String objects.  done so the mainOTA request->send(200, "text/xml", xmlData); copies the buffer rather than references it. 
                            that caused a side effect of a minor change to defs.h.
                            Also, needed to modify WebText() to translate the string to be sent to the web page so its valid XML, special characters for XML
                            must be escaped. This is what was causing WebText() messages sent to the webpage to be missed. added helper functions to do so. 
                            modified ringbuffer a bit. also fixed javascript to handle missing VER field in XML.  the old version of SendXML() has been 
                            retained and commented out in case we need to go back to it.  if all is well remove it in the next release.  added a few 
                            more comments to the code to help explain what is going on.
14-May-2025 w9zv    v4.21   added floating point math to the distance conversion, handling of Storm overhead, and Storm Out of Range per page 33 of the 
                            AS3935MI datasheet.   Added examples of tracking rate/min of events in station_management() function.  
16-May-2025 w9zv    v4.22   fixed bug in distance calculation, added handling of storm overhead and out of range, accidentally overwrote
                            the distance with fake_distance value.  added handling of storm overhead and out of range to the web page.  webpage now shows
                            floating point distance to storm. 
21-May-2025 w9zv    v4.31   added tracking of max rate of events in station_management. refactor station_management: implement circular buffers for event 
                            timestamps and improve rate calculations
26-May-2025 w9zv    v4.32   changed setup2() to ensure factory defaults for all config registers for AS3935 device are set.
28-May-2025 w9zv    v4.4    modified station_management rate data printing. track max rates, webtext on rate change. added add'l dummy values to aid 
                                detection for charting.\
01-Jun-2025 w9zv    v5.0    introduced the real-time charting of the lightning rates, strikes, noise, disturbers, and energy.  this is done using Chart.js
                            library. 
03-Jun-2025 w9zv    v5.1    replace smgmt() with a stats_generate(), unified chart update() whether sim or xml data, move chart above textbox on web, 
                            slimdown the header on the web page, all on one line now, other misc changes.
04-Jun-2025 w9zv    v5.2    added strike rate threshold command, STRTRH, to allow the user to set the strike rate threshold for the station management
                            function.  this is used to determine when to turn off the power to the station.
                            also added a command to turn on/off the power to the station via command line, powreq.  new station management function, passes
                            all needed stats, wa9vfp changes into station_management() function.
                            
*/

// define the version of the code which is displayed on TFT/Serial/and web page. This is the version of the code, not the hardware.
// pse update this whenver a new version of the code is released.
constexpr const char* CODE_VERSION_STR = "v5.2";  // a string for the application version number

// a widget to stop/hold further execution of the code until we get sent something from the host
// it will also print out the line of source code it is being executed in.
void _stall(){
  while(Serial.read() != -1){;}
  while(!Serial.available()){;}   // spin here waiting for something from the pc to be sent.
  delay(100);
  while(Serial.read() != -1){;}  // gobble up any chars in the input buffer, then go on
}
#define stall() Serial.print("Stall @ line # ");Serial.println(__LINE__);_stall();

// function prototype for milliseconds to D:H:M:S format string.
int msToDHMS(unsigned long ms, char *buffer, size_t bufferSize);

void elapsed_timer_display();
extern void WebText(const char *format, ...);

// Function prototype for power relay control
void power_relay(bool on_off);    //Power On/Off Hardware function.  true for on, false for off
void power_relay_fsm() ; // must run this on every loop() to manage the power relay state machine
bool power_relay_get_state(); // get the current state of the power relay, true for on, false for off
void stats_generation(int interrupt_source_register); 
void manage_tft_power_field() ; // manage the power field on the TFT screen, green for on, red for off.
void station_management(int interrupt_source, int distance, long energy, const statistics_struct& stats);

// Function prototypes for command handlers
void handleMotorCommand(char *param);
void handle_TUNECAP_Command(char *param);
void handle_MASK_Command(char *param);
void handle_AFE_Command(char *param);
void handle_Noise_Command(char *param);
void handle_DOGGY_Command(char *param);
void handle_SPIKE_Command(char *param);
void handle_THRESH_Command(char *param);
void handle_DIST_Command(char *param);
void handle_ENERGY_Command(char *param);
void handle_FACT_Command(char *param);
void handle_CLRSTATS_Command(char *param);
void handle_DIVRAT_Command(char *param);
void handle_CALANT_Command(char *param);
void handle_DISPLAYINVERT_Command(char* param); 
void handle_SIMULATOR_Command(char* param); 
void handle_GETMAXLOOPTIME_Comnmand(char* param);
void handle_RESET_Command(char *param); 
void handle_CALOSC_Command(char *param);
void handle_EVTDSBL_Command(char *param);
void handle_CALMODE_Command(char *param);
void handle_STRTRH_Command(char *param);  // strike threshold command, used to set the strike rate threshold for the station management
void handle_PWRRQST_Command(char *param); // turn power on/off the station via commandline

void sensor_register_test( char *param );
void handle_DUMP_Command(char *param);
void handle_HELP_Command(char *param);

CommandEntry commandTable[] = {
    {"afe",      handle_AFE_Command },
    {"noise",    handle_Noise_Command },
    {"doggy",    handle_DOGGY_Command },
    {"spike",    handle_SPIKE_Command },
  {"thresh",   handle_THRESH_Command },
  {"dist",     handle_DIST_Command },
  {"energy",   handle_ENERGY_Command },
  {"tunecap",  handle_TUNECAP_Command },
  {"mask",     handle_MASK_Command },
  {"divrat",   handle_DIVRAT_Command },
  {"sim",      handle_SIMULATOR_Command },
  {"loopt",    handle_GETMAXLOOPTIME_Comnmand },
  {"calmode",  handle_CALMODE_Command },
  {"evtdsbl",  handle_EVTDSBL_Command },
  {"fact",     handle_FACT_Command },
  {"cstats",   handle_CLRSTATS_Command },
  {"inv",      handle_DISPLAYINVERT_Command },
  {"calant",   handle_CALANT_Command }, 
  {"calosc",   handle_CALOSC_Command },
  {"strtrh",   handle_STRTRH_Command },
  {"powreq",   handle_PWRRQST_Command },

  // put any new commands above this line so that the reset command doesnt recurse on itself or do help
  {"regtst",   sensor_register_test }, 
  {"reset",    handle_RESET_Command },
  {"dump",     handle_DUMP_Command },
  {"?",        handle_HELP_Command }
};

// create an instance of the CommandParser, passing the function pointer table to it, and the number of functions in the list.
CommandParser cp(commandTable, sizeof(commandTable) / sizeof(CommandEntry));

//create an AS3935 object using the SPI interface, chip select pin 10 and IRQ pin number 3
#define PIN_IRQ 3
const int DetectorIntrReqPin = PIN_IRQ; // Interrupt pin for lightning detection 
AS3935SPI Sensor(SS, DetectorIntrReqPin); // constructor needs chipselect and interrupt pin gpio

//  Output pins for latching realy Set and  reset
#define RELAY_SET 17
#define RELAY_RESET 18
#define RELAY_SENSE_STATE 16
#define RELAY_PULSEWIDTH 250
#define PWR_ON true
#define PWR_OFF false

#define PWR_BUTTON_INTEGRATION_TIME 1000  // 1 second

/*
14-Feb-2025 w9zv    this is now in a compiler define file so that it always compiles with the correct setting.
// comment this out if actually using hardware, otherwise we simulate it with GPIO RELAY_SENSE_STATE set for an output 
#define SIMULATE_HARDWARE_MODE 1  
*/

// create the tft object
TFT_eSPI tft = TFT_eSPI();  // create the instance of the tft screen object we will use

// LCD/TFT settings no attempt made to define all... just ones we use.
#define SCREEN_WIDTH 320  // OLED display width, in pixels
#define SCREEN_HEIGHT 170 // OLED display height, in pixels
#define FONT_WIDTH_PIXELS 12                          // small font is 6 pixels wide per character, use to calculate column for set_cursor
#define FONT_WIDTH_FONT1 6
#define FONT_WIDTH_FONT1_SIZE2 (FONT_WIDTH_FONT1*2)
#define FONT_HEIGHT_PIXELS 16                         // small font is 8 pixels high per character, use to calculate row for set_cursor
#define TFT_ROW(row) (FONT_HEIGHT_PIXELS * (row - 1)) // rows and columns on the device start with 0
#define TFT_COL(col) (FONT_WIDTH_PIXELS * (col - 1))  // but humans think of the first as 1
#define FONT_DEFAULT 1
#define FONT_7SEGMENT 7
#define FONT_ONE 1
#define FONT_TWO 2
#define FONT_FOUR 4
#define FONT_SIX 6

// widget to print the line number of where this code got executed at
void _tft_im_here(int line) ;
// formatted printf to the TFT screen at a specific location.
void tft_printf_text_at_pos(TFT_eSPI tft, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg,  const char *format, ...);

int gBrightnessTable[] = {0, 35, 70, 105, 140, 175, 210, 250, 255 }; // 255 == 100% full on
int gBrightnessIndex = 4; // initially, middle setting
void manage_tft_brightness();

// initial values for modifying the AS3935 IC settings in setup(). NOT necessarily defaults for device
#define NOISE_FLOOR_VAL 2
#define WATCHDOG_VAL 1
#define SPIKE_VAL 1
#define LIGHTNING_THRESHOLD_VAL 1 

// the value for the settings acceptable by the sensor for the Indoor/Outdoor sensitivity register.
#define INDOOR 0x12 
#define OUTDOOR 0xE

// these 3 values are what is read out of the interrupt register of the sensor if an event occurs.
#define LIGHTNING_INT 0x08
#define DISTURBER_INT 0x04
#define NOISE_INT 0x01
#define PASSTHRU_INT 0xff   // created to do nothing if running simulator when no valid events occur.

#define ALARM_FLASH_INTERVAL 100  // time of each flash
#define ALARM_FLASH_COUNT_PER_ALARM 8  // make this double the number of flashes/alarm you want - 8 == 4 flashes.

long gSimulated_Events = 0;  // value == interval in ms between generated events.
bool gEvents_Active_Flag = false;  // flag to indicate we have disable handling of events from the sensor. 

RAS_HW_STATUS_STRUCT RAS_Status;
RAS_HW_STATUS_STRUCT *pRas = &RAS_Status; // create a pointer to the RAS_STatus structure
statistics_struct gStats = {0};

extern unsigned long gLongest_loop_time;
bool gPowerCondition = false; // flag to indicate we have requested a power condition change, used by the power_relay_fsm() function

// global variable used at the end of the station management placed here for testing
unsigned long gStrikeRateThreshold = 5;  

// declare the global structure and a pointer to it that is shared between the hardware handling done here
// and the web page message population and status updates.  
void get_hardware_status(RAS_HW_STATUS_STRUCT& myhw );


void setup2()
{
    pRas->pSoftwareVersion = CODE_VERSION_STR; // set the software version string in the structure
    //set the IRQ pin as an input pin. do not use INPUT_PULLUP - the AS3935 will pull the pin 
	//high if an event is registered.
    // When lightning is detected the interrupt pin goes HIGH.
    pinMode(DetectorIntrReqPin, INPUT);

    // (POWER ON)IO15 must be set to HIGH before starting, otherwise the screen will not display when using battery
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);

    // esp32-s3 T-display user input buttons.
    pinMode(PIN_BUTTON_1,INPUT); // upper button, used for something TBD, board has pullup...
    pinMode(PIN_BUTTON_2,INPUT); // lower button, used for restarting a new sequence, board has pullup...
    
    // pinmode for the power relay set/reset pins
    pinMode(RELAY_SET,OUTPUT);   //Relay set output pulse
	pinMode(RELAY_RESET,OUTPUT);  //Relay reset output pulse 

#ifdef SIMULATE_HARDWARE_MODE
// w9zv for testing without the hardware, make the relay sense an output so i can read/write it to emulate the relay
//      on the esp32 you can read the state of an output pin, so we can use this to simulate the relay state.
    pinMode(RELAY_SENSE_STATE,OUTPUT);  //To relay monitor state circuit
#else
    pinMode(RELAY_SENSE_STATE,INPUT_PULLUP);  //To relay monitor state circuit
#endif
    delay(10);

    // setup initial state of the relay
	digitalWrite(RELAY_SET,LOW);  //Make sure the SET IS low
	digitalWrite(RELAY_RESET,HIGH);  // Pulse the relay reset pin HIGH
	delay(50);                     //DELAY IT
	digitalWrite(RELAY_RESET,LOW);    // Set Relay reset pin LOW again

    // set the initial state of the power on boot whatever the relay sense state is.
    gPowerCondition = digitalRead(RELAY_SENSE_STATE);

    randomSeed(1234L);  // seed the random number generator for simulator events

    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setRotation(3);
    tft.setSwapBytes(true);
    tft.pushImage(0, 0, 320, 170, (uint16_t *)SkylineWithCall2);

    ledcSetup(0, 2000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    //ledcWrite(0, 255); // duty cycle - 255 = 100% on
    ledcWrite(0, gBrightnessTable[gBrightnessIndex]);

    tft.setTextSize(FONT_DEFAULT);
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // default is to be transparent, if FG/BG colors are diff, its opaque
    // allow enough time for the platformIO terminal to startup after a download before printing anything.
    delay(3000);    // leave image on screen for a few seconds...
    tft.fillScreen(TFT_BLACK);
   
//     tft.setTextSize(1);
//     tft.println(F("123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890")); // tft size 1 has enough space for 26 text characters/row, each row is 320 pixels
// stall();
//     tft.setTextSize(FONT_DEFAULT);
//     tft.println(F("12345678901234567890123456")); // tft has enough space for 26 text characters/row, each row is 320 pixels
// stall();


    // draw a rounded rectangle around the data area, below the elapsed timer.
    tft.drawRoundRect(0,50, tft.width(),tft.height()-55, 5, TFT_RED);
    tft.setTextWrap(false, false);  // turn of both X and Y axis text wrapping.

    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5,55,FONT_DEFAULT);
    tft.setTextPadding(tft.width());  // pad it the width of the screen.
    tft.printf("Noise:     %4d Last  %4d", 0, 0);
    tft.setCursor(5,75);
    tft.setTextPadding(tft.width());  // pad it the width of the screen.
    tft.printf("Disturber: %4d Last  %4d", 0, 0);

    tft.setCursor(0,0, FONT_DEFAULT);
    tft.printf( "Lightning Detector %s \n", CODE_VERSION_STR );
    WebText("\n\nAS3935 Franklin Lightning Detector %s \n", CODE_VERSION_STR );//CODE_VERSION_STR

    // set the initial state of the power on boot to OFF on the TFT, leave a space after
    tft_printf_text_at_pos( tft, SCREEN_WIDTH - FONT_WIDTH_FONT1_SIZE2 * 8, 135, TFT_YELLOW, TFT_RED,  " POWER ");


    //tft.println(F("Compiled on: ")); tft.println(__TIMESTAMP__);
    // printf version
    tft.setTextColor(TFT_GREEN, TFT_BLACK );
    tft.printf("Compiled on:\n%s %s", __DATE__, __TIME__);
    WebText("Compiled on: %s %s\n", __DATE__, __TIME__);

    //delay(3000);

    // always start with events enabled
    gEvents_Active_Flag = true;
    
    // look for simulator on button to be pressed for a few seconds, if so, we automatically generate some psuedo data
    for (int i = 0; i < 250; i++) {
        if(digitalRead(PIN_BUTTON_2) == LOW ){
            delay(25);
            if(digitalRead(PIN_BUTTON_2) == LOW) {
                gSimulated_Events = 2000;  // default of 2 seconds between event generation
                WebText("Simulator enabled every %d ms\n",gSimulated_Events);
                tft.setTextColor(TFT_GOLD, TFT_BLACK);
                tft.setTextSize(2);
                tft.setCursor(5,115,FONT_DEFAULT);
                tft.setTextPadding(tft.width());  // pad it the width of the screen.
                tft.printf("Simulator enabled %4d ms", gSimulated_Events); 
                //break;
            } 
        }
        if (gSimulated_Events) break;
        delay(10);
    }

    // SPI.begin(); // For SPI
    SPI.begin(SCK, MISO, MOSI, SS); // w9zv - use these pins for the SPI interface, GPIO12,13,11,10

    if (!Sensor.begin() ) {
        WebText("ERROR - Lightning Detector did not start up, is it connected ?\n");
        // while (1)
        //     delay(50);
    }
	
    //check SPI connection.
	if (!Sensor.checkConnection()) {
		WebText("ERROR - checkConnection() failed. check your SPI connection and SPI chip select pin.\n");
		//while (1);
	} else
		WebText("SPI connection check passed.\n");
    
    //check the IRQ pin connection.
	if (!Sensor.checkIRQ()) {
		WebText("ERROR - checkIRQ() failed. check if the correct IRQ pin was passed to the AS3935SPI constructor?\n");
		//while (1);
	} else
		WebText("IRQ pin connection check passed.\n");
    
    // since a soft reset of the ESP32 does NOT make it to the detector, execute the factory reset command
    Sensor.resetToDefaults();  // make sure we reset all detector register settings to factory default. 
    delay(50); // let it settle

	//calibrate the resonance frequency. failing the resonance frequency could indicate an issue 
	//of the sensor. resonance frequency calibration will take about 1.7 seconds to complete.
	int32_t frequency = 0;
	if (!Sensor.calibrateResonanceFrequency(frequency)) {
		WebText("Resonance Frequency Calibration failed @ %d Hz, should be 482500 Hz - 517500 Hz\n",frequency);
		//while (1);
	} else
		WebText("Resonance Frequency Calibration passed.\n");

    WebText("\tResonance Frequency is %d Hz\n", frequency);
    int temp = Sensor.readAntennaTuning();
    WebText("\tResulting tuning cap register value is: %#04x or %02d pf\n", temp, temp * 8  ); // print out reg hex value AND picofarads

	//calibrate the RCO.
	if( !Sensor.calibrateRCO() ) {
		WebText("RCP Calibration failed.\n");
		//while (1);
	} else
		WebText("RCO Calibration passed.\n");

    // "Disturbers" are events that are false lightning events. If you find
    // yourself seeing a lot of disturbers you can have the chip not report those
    // events on the interrupt lines.   Default is not to mask them.
    Sensor.writeMaskDisturbers(false);

    int maskVal = Sensor.readMaskDisturbers();
    Serial.printf("Are disturbers being masked: ");
    if (maskVal == 1)
        Serial.printf("YES\n");
    else if (maskVal == 0)
        Serial.printf("NO\n");

    // The lightning detector defaults to an indoor setting (less
    // gain/sensitivity), if you plan on using this outdoors Default is INDOORS
    // uncomment the following line:
    Sensor.writeAFE(AS3935MI::AS3935_INDOORS);

    int enviVal = Sensor.readAFE();
    Serial.printf("Are we set for indoor or outdoor: ");
    if (enviVal == AS3935MI::AS3935_INDOORS)
        Serial.printf("Indoor.\n");
    else if (enviVal == AS3935MI::AS3935_OUTDOORS)
        Serial.printf("Outdoor.\n");
    else
        Serial.println(enviVal, BIN);

    // Noise floor setting from 1-7, one being the lowest. Default setting is
    // two. If you need to check the setting, the corresponding function for
    // reading the function follows.  Device default is 2 (NFL_2), which is the
    Sensor.writeNoiseFloorThreshold(AS3935MI::AS3935_NFL_2);

    int noiseVal = Sensor.readNoiseFloorThreshold();
    Serial.printf("Noise Level is set at: %#04x\n", noiseVal);

    // Watchdog threshold setting can be from 1-10, one being the lowest. Default setting is
    // 2. If you need to check the setting, the corresponding function for
    // reading the function follows.
    Sensor.writeWatchdogThreshold(AS3935MI::AS3935_WDTH_2);

    int watchDogVal = Sensor.readWatchdogThreshold();
    Serial.printf("Watchdog Threshold is set to: %#04x\n", watchDogVal);

    // Spike Rejection setting from 1-11, one being the lowest. Default setting is
    // 2. If you need to check the setting, the corresponding function for
    // reading the function follows.
    // The shape of the spike is analyzed during the chip's
    // validation routine. You can round this spike at the cost of sensitivity to
    // distant events.
    Sensor.writeSpikeRejection(AS3935MI::AS3935_SREJ_2);

    int spikeVal = Sensor.readSpikeRejection();
    Serial.printf("Spike Rejection is set to:  %#04x\n", spikeVal);

    // This setting will change when the lightning detector issues an interrupt.
    // For example you will only get an interrupt after five lightning strikes
    // instead of one. Default is one, and it takes settings of 1, 5, 9 and 16.
    // Followed by its corresponding read function. 
    Sensor.writeMinLightnings(AS3935MI::AS3935_MNL_1);

    uint8_t lightVal = Sensor.readMinLightnings();
    Serial.printf("The minimum number of lightning strikes register value is: %#04x\n", lightVal);

    // When the distance to the storm is estimated, it takes into account other
    // lightning that was sensed in the past 15 minutes. If you want to reset
    // them, then you can call this function.
    Sensor.clearStatistics();

    // The power down function has a BIG "gotcha". When you wake up the board
    // after power down, the internal oscillators will be recalibrated. They are
    // recalibrated according to the resonance frequency of the antenna - which
    // should be around 500kHz. It's highly recommended that you calibrate your
    // antenna before using these two functions, or you run the risk of screwing
    // the timing of the chip.

    // lightning.powerDown();
    // delay(1000);
    // if( lightning.wakeUp() )
    //  Serial.println("Successfully woken up!");
    // else
    // Serial.println("Error recalibrating internal osciallator on wake up.");
    // Set too many features? Reset them all with the following function.
    // lightning.resetSettings();
    //neopixelWrite(RGB_BUILTIN, random(4), random(4), random(4));   

#ifdef SIMULATE_HARDWARE_MODE
    WebText("\nHardware Simulation - running in power relay SIMULATION MODE, RELAY_SENSE_STATE GPIO is\n\t");
    WebText("configured to a GPIO OUTPUT to sense the relay state for SIM...\n");
#endif
    Serial.printf("\nOut of setup2()\n");

    delay(20);
    Sensor.readInterruptSource();  // clean off any pending interrupts.
    delay(20);
    Sensor.readInterruptSource();  // clean off any pending interrupts.
    delay(20);
  
    if(Serial) Serial.flush();  // only flush if serial port is present, udderwise it blocks
}

void loop2(HostCmdEnum & host_command)
{
    int faked_event = PASSTHRU_INT;
    int fake_distance = 0;
    int distance = 0; // distance to the storm in km, 0 means out of range
    long fake_energy = 0;
    long lightEnergy = 0;
    int interrupt_reg_value = PASSTHRU_INT; // interrupt status register value from the AS3935

    static bool alm_display_state = true;
    static int alm_enable_flashcount = 0;  // enable flashing alarm with non-zero, and value when non-zero is number of flashes.
    unsigned long now = millis(); // read this only once each time thru the loop, use it many...
    static long last_event = now;

    tft.setTextFont(FONT_DEFAULT);

    // these are used to debounce the button press and to prevent the relay from oscillating
    static unsigned long relayLastToggle = 0;

    if(gSimulated_Events) {
        if( now - last_event >= gSimulated_Events ) {  // every FAKED_DATA_INTERVAL seconds we "might" generate an event
            // generate a fake event.
            //faked_event = rand() % 9; // only 1,4,8 will be valid.
            static int validValues[] = {1, 4, 8}; // Array of valid faked ISR values (Strike/Disturber/Noise)
            // Generate a random index (0, 1, or 2) to select a value from the array
            int randomIndex = rand() % 3;  // Generate a random index (0, 1, or 2)
            faked_event = validValues[randomIndex]; // Assign a random value from the array of valid values
//Serial.printf("->>>>>> Faked event: %d\n", faked_event);
//faked_event = LIGHTNING_INT;
            fake_distance = rand() % 41;
//fake_distance = fake_distance == 1 ? 40 : 1;  // toggle between 1 and 40 km
            fake_energy = rand() % (1 << 20);  // 20 bit register, range of values up to 2^20 possible

            Serial.print(".");
            last_event = now;
        }
    }

    // if im handling events, and the AS3935 has generated an interrupt via GPIO, or we are in simulator mode, then handle the event.
    //    there is a CLI command it deactiveate handling events (for manual testing of tuning) 
    if ( gEvents_Active_Flag == true && (digitalRead(DetectorIntrReqPin) == HIGH || gSimulated_Events) ) {
        
        // according to spec sheet, we need to wait 2ms before reading the interrupt source register
        // the older sparkfun library had a delay(2) in the interrupt handler, so we will do the same.
        delay(2); 
        
        // Hardware has alerted us to an event, now we read the interrupt register
        // to see exactly what it is... or we are running the sim, so use the sim generated value/event
        interrupt_reg_value = gSimulated_Events ? faked_event : Sensor.readInterruptSource();
//if(interrupt_reg_value != PASSTHRU_INT)Serial.printf("INTERRUPT SOURCE REG VALUE: %#04x\n", interrupt_reg_value);
        switch (interrupt_reg_value) {
            case NOISE_INT: {
                static unsigned int noise_accumulator = 0;
                static unsigned long noise_last = 0;  // Initialize static variable
                if (noise_accumulator == 0) noise_last = now;  // Initialize `noise_last` if first event
                noise_accumulator++; // Increment the noise event counter
                unsigned long elapsed = ((now - noise_last) / 1000) % 10000; // Calculate elapsed time in seconds
                WebText("Noise - accumulated since reset %d, ET since last %d\n", noise_accumulator, elapsed);
        
                // Update the structure used to build XML response messages
                pRas->noise_accum = noise_accumulator;
                pRas->noise_et = elapsed;
        
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setTextSize(2);
                tft.setCursor(5, 55, FONT_DEFAULT);
                tft.setTextPadding(tft.width());  // Pad it to the width of the screen
                tft.printf("Noise:     %4d Last  %4d", noise_accumulator, elapsed);
                noise_last = now; // Save the current time for the next event
                break;
            }
        
            case DISTURBER_INT: {
                static unsigned long disturber_accumulator = 0;
                static unsigned long disturber_last = 0;  // Initialize static variable
                if (disturber_accumulator == 0) disturber_last = now; // Initialize `disturber_last` if first event
                disturber_accumulator++; // Increment the disturber event counter
                unsigned long elapsed = ((now - disturber_last) / 1000) % 10000; // Calculate elapsed time in seconds
                WebText("Disturber - accumulated since reset %d, ET since last %d\n", disturber_accumulator, elapsed);
        
                // Update the structure used to build XML response messages
                pRas->disturber_accum = disturber_accumulator;
                pRas->disturber_et = elapsed;
        
                tft.setTextColor(TFT_GREEN, TFT_BLACK);
                tft.setTextSize(2);
                tft.setCursor(5, 75, FONT_DEFAULT);
                tft.setTextPadding(tft.width());  // Pad it to the width of the screen
                tft.printf("Disturber: %4d Last  %4d", disturber_accumulator, elapsed);
                disturber_last = now; // Save the current time for the next event
                break;
            }
        
            case LIGHTNING_INT: {
                static unsigned long stroke_accumulator = 0;
                static unsigned long stroke_last = 0; // Initialize static variable
                
                if (stroke_accumulator == 0) stroke_last = now; // Initialize `stroke_last` if first event

                stroke_accumulator++; // Increment the lightning strike counter
                unsigned long elapsed = ((now - stroke_last) / 1000) % 10000; // Calculate elapsed time in seconds
                WebText("\nStrike - accumulated since reset %d, ET since last %d\n", stroke_accumulator, elapsed);
        
                // Update the structure used to build XML response messages
                pRas->strike_accum = stroke_accumulator;
                pRas->strike_et = elapsed;
        
                tft.setTextColor(TFT_GOLD, TFT_BLACK);
                tft.setTextSize(2);
                tft.setCursor(5, 95, FONT_DEFAULT);
                tft.setTextPadding(tft.width());  // Pad it to the width of the screen
                tft.printf("Strikes:   %4d Last  %4d", stroke_accumulator, elapsed);
        
                // Lightning! Now how far away is it? Distance estimation takes into account previously seen events.
                distance = Sensor.readStormDistance();
                distance = gSimulated_Events ? fake_distance : distance;
//Serial.printf("\tDistance: %d km\n", distance);

                tft.setCursor(5, 115, FONT_DEFAULT);
                tft.setTextPadding(tft.width());  // Pad it to the width of the screen
                
                // Convert distance to miles (device register output is in km)
//distance = ((long)distance * 621371L) / 1000000L;
                const float KM_TO_MILES = 0.621371;
                float miles = (float)distance * KM_TO_MILES;
                if (distance == 40 ) {  // Handle the case where the device says out of range
                    tft.printf(" Storm is Out of Range    ");
                    WebText("\tOut of Range ");
                } else if (distance <= 1) { // Handle the case where the device says 1 km
                    tft.printf(" Storm is Overhead        ");
                    WebText("\tStorm is overhead ");
                } else {
                    tft.printf(" Approx %.1f miles distant  ", miles);
                    WebText("\tApproximately: %.1f mi away! ", miles);
                } 
        
                // "Lightning Energy" is a pure number without physical meaning
                lightEnergy = Sensor.readEnergy();
                lightEnergy = gSimulated_Events ? fake_energy : lightEnergy; //

                tft.setCursor(5, 135, FONT_DEFAULT);
                tft.setTextPadding(tft.width());  // Pad it to the width of the screen
                tft.printf(" Energy:  %d\n", lightEnergy);
                WebText("\tLightning Energy: %d\n", lightEnergy);
        
                // Update the structure used to build XML response messages
                pRas->strike_energy = lightEnergy;
                pRas->strike_distance = miles;
        
                alm_enable_flashcount = ALARM_FLASH_COUNT_PER_ALARM;
                alm_display_state = true;
                stroke_last = now; // Save the current time for the next event
        
                Sensor.clearStatistics(); // Clear the statistics and strike register accumulation
                break;
            }

            case PASSTHRU_INT: {
                // Handle the case where the interrupt occurred due to a pass-through event
                // which was created solely to support the simulator mode, and do nothing (passathru), now that we know that the ISR may be 
                // valid with value of 0x00 indicating a purge
                //WebText("Pass-through event detected, IntSrcReg - %02X\n", interrupt_reg_value);
                //Serial.printf("Pass-through event detected, IntSrcReg - %02X\n", interrupt_reg_value);
                break;
            }
        
            default: {
                // Handle the case where the interrupt occurred due to self-purging of statistics
                // when ISR could be a value of 0x00
                static unsigned long purge_last = 0;
                unsigned long purge_elapsed = ((now - purge_last) / 1000);
                WebText("Purging old events/stats, last purge ET %d secs, IntSrcReg - %02X\n", purge_elapsed, interrupt_reg_value);
                Serial.printf("Purging old events/stats , last purge ET %d secs, IntSrcReg - %02X\n", purge_elapsed, interrupt_reg_value);
                purge_last = now; // Save the current time for the next purge
                break;
            }
        }
    }
    
    // generate rate stats
    stats_generation(interrupt_reg_value); // Pass the ISR value to the power management function
    
    // Call the station management function decide what to do with the station control/power management
    station_management(interrupt_reg_value, distance, lightEnergy, gStats);

    // let commandparser handle any user input commands.
    cp.processInput();

    // Alarm flash handling by inverting the display a few times to get your attention.
    static unsigned long alarm_last_time = millis(); // grab the tick count after reset
    unsigned long alarm_now = millis(); // dont let time be dialated by lightning events.
    if( alm_enable_flashcount ) {
        if ( alarm_now - alarm_last_time >= ALARM_FLASH_INTERVAL ) { // put whatever delay between updates here in ms
            //Serial.printf("TickDiff %d\n", alarm_now - alarm_last_time);
            if(alm_display_state){
                tft.invertDisplay(false);  // by default the TFT is coming up IN inverted mode (black background)
                //Serial.printf("NORM %d\n", __LINE__);
                alm_display_state = false;  // toggle 
               //stall();
            } else {
                alm_display_state = true;
                tft.invertDisplay(true);  // by default the TFT is coming up IN inverted mode (black background)
                //Serial.printf("NORM %d\n", __LINE__);
            }

            alm_enable_flashcount--; // decrement this each time we toggle the diplay
            if( alm_enable_flashcount <= 0 ) {
                tft.invertDisplay(true);  // inverted is text on black
                alm_display_state = false;
                //Serial.printf("ALARM OFF %d\n", __LINE__);
               // stall();
            }
            alarm_last_time = alarm_now;
        }
    }

    // take care of the elapsed timer display
    elapsed_timer_display();

    // manage screen brightness with button 1
    manage_tft_brightness();

    // manage the power field display in the tft and keep it updated
    manage_tft_power_field();

    // handle the power relay control button 2 and the soft button on client
    if (now - relayLastToggle >= PWR_BUTTON_INTEGRATION_TIME) {
        if (digitalRead(PIN_BUTTON_2) == LOW || host_command == HostCmdEnum::PWR_SELECT) {
            power_relay(!power_relay_get_state()); // Toggle the relay
            relayLastToggle = now; // Update the last toggle time
        }
    }

    // Call the function periodically to process non-blocking transitions
    power_relay_fsm();

    // we are done with this loop, reset the command to NO_OP
    host_command = HostCmdEnum::NO_OP;
}

constexpr bool OFF = false;
constexpr bool ON = true;
void station_management(int interrupt_source, int distance, long energy, const statistics_struct& stats)
{
    // Static variables to track power-off request and timing
    static bool powerOffRequested = false;
    static unsigned long lastBelowThresholdTime = 0;
    static const unsigned long REARM_DELAY_MS = 30000; // 30 seconds

    unsigned long now = millis();

    // Only the actual stats are kept in the struct; the sliding window buffers are now local static variables.
    // keep in mind, that the value for distance and energy is only valid if the interrupt source is LIGHTNING_INT
    if (interrupt_source == LIGHTNING_INT) {
        // If we have a lightning strike, distance and energy are valid
    }

    // If the strike rate is above the threshold, request power off only once
    if (stats.strikeRate >= gStrikeRateThreshold) {
        if (!powerOffRequested) {
            power_relay(OFF);
            powerOffRequested = true;
            WebText("\n>>>>> Strike rate above threshold (%d), requesting power off. <<<<<\n", gStrikeRateThreshold);
        }
        // Reset the timer since we're still above threshold
        lastBelowThresholdTime = now;
    } else {
        // If we just dropped below threshold, start/restart the timer
        if (powerOffRequested && (now - lastBelowThresholdTime >= REARM_DELAY_MS)) {
            powerOffRequested = false; // Allow another power off after 30s below threshold
            WebText("\n>>>>> Strike rate below threshold, rearming power off request. <<<<<\n");
        }
    }
}

// Main station management function
#define MAX_EVENT_TIMESTAMPS 64

// Helper function to add an event timestamp to a circular buffer
// buf: pointer to the buffer array
// head: pointer to the head index (oldest event)
// count: pointer to the current number of events in the buffer
// now: current timestamp (millis)
void add_event_timestamp(unsigned long* buf, int* head, int* count, unsigned long now)
{
    // Store the new timestamp at the next available position in the circular buffer
    buf[(*head + *count) % MAX_EVENT_TIMESTAMPS] = now;
    if (*count < MAX_EVENT_TIMESTAMPS) {
        // If buffer not full, just increment count
        (*count)++;
    } else {
        // If buffer full, move head forward to overwrite oldest event
        *head = (*head + 1) % MAX_EVENT_TIMESTAMPS;
    }
}

// Helper function to count events in the buffer that occurred after the cutoff time
// buf: pointer to the buffer array
// head: head index (oldest event)
// count: current number of events in the buffer
// cutoff: timestamps must be >= cutoff to be counted
int count_events_in_window(unsigned long *buf, int head, int count, unsigned long cutoff) 
{
    int n = 0;
    for (int i = 0; i < count; ++i) {
        int idx = (head + i) % MAX_EVENT_TIMESTAMPS; // Calculate circular buffer index
        if (buf[idx] >= cutoff) n++; // Count if timestamp is within window
    }
    return n;
}

/**
 * 
 * This function tracks event statistics (strikes, disturbers, noise, purges) and calculates
 * their rates per minute using a sliding window algorithm. It uses circular buffers to store
 * timestamps of recent events for each type, allowing accurate, up-to-date rate calculations
 * that reflect activity in the last N seconds (typically 60s).
 * 
 * Why circular buffers?  
 * - They efficiently store only the most recent N event timestamps, using fixed memory.
 * - They allow fast insertion and fast counting of events within a time window.
 * - This enables a true "events per minute" rate that is responsive and accurate, even if
 *   events are sparse or bursty, and avoids the inaccuracy of simple interval-based counters.
 * 
 * The function updates rates every 2 seconds for responsiveness, but only prints rates to the
 * web interface every 10 seconds, and only after at least 1 minute of rate calculations has
 * occurred (to ensure the sliding window is fully populated).
 * 
 * Only the actual stats are kept in the struct; the sliding window buffers are now local static variables.
 */
void stats_generation(int interrupt_source_register)
{
    // Static structure to hold all event counts, rates, and max rates
    unsigned long now = millis();

    // --- Sliding window buffers for event timestamps (now local static variables) ---
    static unsigned long strikeTimestamps[MAX_EVENT_TIMESTAMPS];
    static int strikeHead = 0, strikeCountInBuf = 0;

    static unsigned long disturberTimestamps[MAX_EVENT_TIMESTAMPS];
    static int disturberHead = 0, disturberCountInBuf = 0;

    static unsigned long noiseTimestamps[MAX_EVENT_TIMESTAMPS];
    static int noiseHead = 0, noiseCountInBuf = 0;

    static unsigned long purgeTimestamps[MAX_EVENT_TIMESTAMPS];
    static int purgeHead = 0, purgeCountInBuf = 0;

    // --- 1. Record event timestamps in circular buffers ---
    switch (interrupt_source_register) {
        case LIGHTNING_INT:
            gStats.strikeCount++;
            add_event_timestamp(strikeTimestamps, &strikeHead, &strikeCountInBuf, now);
            break;
        case DISTURBER_INT:
            gStats.disturberCount++;
            add_event_timestamp(disturberTimestamps, &disturberHead, &disturberCountInBuf, now);
            break;
        case NOISE_INT:
            gStats.noiseCount++;
            add_event_timestamp(noiseTimestamps, &noiseHead, &noiseCountInBuf, now);
            break;
        case PASSTHRU_INT:
            break;
        default:
            gStats.purgeCount++;
            add_event_timestamp(purgeTimestamps, &purgeHead, &purgeCountInBuf, now);
            break;
    }

    // --- 2. Update rates every 2 seconds for responsiveness ---
    const unsigned long RATE_WINDOW_SECONDS = 60;
    const unsigned long RATE_UPDATE_INTERVAL_MS = 2000;
    static unsigned long lastRateUpdate = 0;
    static unsigned long firstRateUpdate = 0;
    static int rateUpdateCount = 0;
    if (now - lastRateUpdate >= RATE_UPDATE_INTERVAL_MS) {
        unsigned long windowMillis = RATE_WINDOW_SECONDS * 1000UL;
        unsigned long cutoff = now - windowMillis;

        int strikes = count_events_in_window(strikeTimestamps, strikeHead, strikeCountInBuf, cutoff);
        int disturbers = count_events_in_window(disturberTimestamps, disturberHead, disturberCountInBuf, cutoff);
        int noises = count_events_in_window(noiseTimestamps, noiseHead, noiseCountInBuf, cutoff);
        int purges = count_events_in_window(purgeTimestamps, purgeHead, purgeCountInBuf, cutoff);

        float windowMinutes = (float)RATE_WINDOW_SECONDS / 60.0f;
        gStats.strikeRate = strikes / windowMinutes;
        gStats.disturberRate = disturbers / windowMinutes;
        gStats.noiseRate = noises / windowMinutes;
        gStats.purgeRate = purges / windowMinutes;

        // Track max rates for each event type
        if (gStats.strikeRate    > gStats.maxStrikeRate)    gStats.maxStrikeRate    = gStats.strikeRate;
        if (gStats.disturberRate > gStats.maxDisturberRate) gStats.maxDisturberRate = gStats.disturberRate;
        if (gStats.noiseRate     > gStats.maxNoiseRate)     gStats.maxNoiseRate     = gStats.noiseRate;
        if (gStats.purgeRate     > gStats.maxPurgeRate)     gStats.maxPurgeRate     = gStats.purgeRate;

        if (firstRateUpdate == 0) firstRateUpdate = now;
        rateUpdateCount++;
        lastRateUpdate = now;
    }

    // --- 3. Print to web only every 10 seconds, but only after 1 min of rate calcs ---
    static unsigned long lastWebPrint = 0;
    const unsigned long WEB_PRINT_INTERVAL_MS = 10000; // 10 seconds

    bool anyRateNonZero = (gStats.strikeRate != 0.0f) || (gStats.disturberRate != 0.0f) || (gStats.noiseRate != 0.0f) || (gStats.purgeRate != 0.0f);

    // Store summation of previous rates to detect changes
    static float prevStrikeRate = -1.0f, prevDisturberRate = -1.0f, prevNoiseRate = -1.0f, prevPurgeRate = -1.0f;
    
    // Check if rates have changed since last print
    bool ratesChanged =
        (gStats.strikeRate    != prevStrikeRate)    ||
        (gStats.disturberRate != prevDisturberRate) ||
        (gStats.noiseRate     != prevNoiseRate)     ||
        (gStats.purgeRate     != prevPurgeRate);

    if (ratesChanged && (rateUpdateCount * RATE_UPDATE_INTERVAL_MS) >= 60000UL ) {
        if(anyRateNonZero) {
            WebText(
                "SM Rates: strike (%.1f/min), disturber (%.1f/min), noise (%.1f/min), purge (%.1f/min) 7, 7, 7, 7,\n",
                gStats.strikeRate, gStats.disturberRate, gStats.noiseRate, gStats.purgeRate
            );
        } else 
            WebText("SM Rates are now zero. 0 0 0 0 0 0 0 0 \n");
        
        WebText( "\tMax Rate: strike %.1f/min, disturbers %.1f/min, noise %.1f/min, purges %.1f/min\n",
                    gStats.maxStrikeRate, gStats.maxDisturberRate, gStats.maxNoiseRate, gStats.maxPurgeRate );
    }
    // Update previous rates to detect change
    prevStrikeRate     = gStats.strikeRate;
    prevDisturberRate  = gStats.disturberRate;
    prevNoiseRate      = gStats.noiseRate;
    prevPurgeRate      = gStats.purgeRate;

    // finally, update the RAS_Status structure with the latest statistics to xmit to the web interface
    RAS_Status.strikeRate     = gStats.strikeRate;
    RAS_Status.disturberRate  = gStats.disturberRate;       
    RAS_Status.noiseRate      = gStats.noiseRate;
    RAS_Status.purgeRate      = gStats.purgeRate;
}

// manage the tft power field display by reading the RELAY_SENSE_STATE GPIO pin, encapsulate it here if how we do so changes.
void manage_tft_power_field()
{
    static bool last = digitalRead(RELAY_SENSE_STATE);
    bool bPowerState = digitalRead(RELAY_SENSE_STATE);

    // only burn the cpu cycles to update the display IF there was a change from the last time. 
    if(last == bPowerState) return;

    if(bPowerState)
        tft_printf_text_at_pos( tft, SCREEN_WIDTH - FONT_WIDTH_FONT1_SIZE2 * 8, 135, TFT_BLACK, TFT_GREEN,  " POWER "); // ON
    else
        tft_printf_text_at_pos( tft, SCREEN_WIDTH - FONT_WIDTH_FONT1_SIZE2 * 8, 135, TFT_YELLOW, TFT_RED,  " POWER "); // OFF

    last = bPowerState;  // save the current state for next time around.
}

bool power_relay_get_state() 
{
    // This function returns the current power state of the relay
    // It reads the RELAY_SENSE_STATE GPIO pin to determine if the relay is ON or OFF
    //return digitalRead(RELAY_SENSE_STATE);
    // or the state of the global variable gPowerCondition
    return gPowerCondition; // this is the state of the power relay, which is set by the power_relay() function  
}
void power_relay(bool on_off)    //Power On/Off Hardware function.  true for on, false for off
{
    gPowerCondition = on_off; // set the global request variable to the requested state
}
// jack implemented a latching relay pair config setup as latching for the power relay, so we need to pulse it to set/reset it.  
// This function does this in a non-blocking way.
// it also supports the simulator mode such that in the absence of the relay hardware, it will simulate the relay state.
// it does this by setting the RELAY_SENSE_STATE GPIO to an output and setting it to HIGH when the relay is ON, and LOW when OFF.
// fortunately, the ESP32 GPIO pin implementations allow reading the state of a GPIO pin that is config'd as an OUTPUT.
// this function must be called periodically from the loop2() function, and it will manage the FSM and relay state transitions
void power_relay_fsm() 
{
    // Static variables to hold state and timing
    static enum { IDLE, TURNING_ON, TURNING_OFF } sm_State = IDLE;
    static unsigned long relayTimer = 0;
    static bool currentPowerState = false;
    unsigned long currentTime = millis();
    static bool firstRun = true;

    if(firstRun) {
        currentPowerState = digitalRead(RELAY_SENSE_STATE); // set the initial state of the PWR_Button based on what is read from the GPIO pin
        firstRun = false; // Set firstRun to false after the initial read, this only happens once
    }  

    switch (sm_State) {
        case IDLE:
            if (gPowerCondition != currentPowerState) { // Start only if state changes
                relayTimer = currentTime;
                currentPowerState = gPowerCondition;
                if (gPowerCondition) {
#ifdef SIMULATE_HARDWARE_MODE
                    digitalWrite(RELAY_SENSE_STATE, HIGH); // to Simulate turning relay ON we read the RELAY_SENSE_STATE pin as the relay state
#endif
                    digitalWrite(RELAY_SET, HIGH); // Start the set pulse
                    sm_State = TURNING_ON;
                } else {
#ifdef SIMULATE_HARDWARE_MODE
                    digitalWrite(RELAY_SENSE_STATE, LOW); // to Simulate turning relay OFF, we read the RELAY_SENSE_STATE pin as the relay state
#endif
                    digitalWrite(RELAY_RESET, HIGH); // Start the reset pulse
                    sm_State = TURNING_OFF;
                }
            }
            break;

        case TURNING_ON:
            if (currentTime - relayTimer >= RELAY_PULSEWIDTH) {
                digitalWrite(RELAY_SET, LOW); // End the set pulse
                WebText("PWR Relay ON\n");
                sm_State = IDLE; // Transition back to idle
            }
            break;

        case TURNING_OFF:
            if (currentTime - relayTimer >= RELAY_PULSEWIDTH) {
                digitalWrite(RELAY_RESET, LOW); // End the reset pulse
                WebText("PWR Relay OFF\n");
                sm_State = IDLE; // Transition back to idle
            }
            break;
    }
}

// manage the TFT backlight brightness with the button 1
// this function is called from the loop2() function, and is used to manage the TFT backlight brightness
// each time the button is pressed, the tft brightness is cycled through the table of values.
void manage_tft_brightness()
{
    static bool debounce = false;
    if (digitalRead(PIN_BUTTON_1) == 0) { // if button pushed, change brightness
        if (debounce == false) {
            debounce = true;
            gBrightnessIndex++;
            if (gBrightnessIndex == sizeof(gBrightnessTable) / sizeof(int))
                gBrightnessIndex = 0;

            ledcWrite(0, gBrightnessTable[gBrightnessIndex]);
            //Serial.printf("Backlight : %#04x\n",gBrightnessTable[gBrightnessIndex] );
        }
    } else
        debounce = false;
}

void elapsed_timer_display()
{
    // every second, display the time since reboot
    static unsigned long time_interval_last = millis(); // grab the tick count after reset
    static char time_buffer[20];
    unsigned long tick_now = millis(); // dont let time be dialated by lightning events.
    if (tick_now - time_interval_last >= 1000) { // put whatever delay between updates here in ms
        int strsize = msToDHMS( tick_now , time_buffer, sizeof(time_buffer));
        tft.setTextSize(1); // 1x multiplier is the smallest, zero is not allowed
        tft.setTextColor( TFT_SKYBLUE, TFT_BLACK );
        tft.setTextPadding(tft.width());  // pad it the width of the screen.
        tft.drawString(time_buffer, TFT_COL(1), TFT_ROW(1), FONT_7SEGMENT);
        time_interval_last = tick_now;
    }
}

// this function is used to populate the HW_STATUS_STRUCTure that is used
// by main.cpp to build the XML status responses and updates to the webpage
// we put this here to isolate all hardware related accesses to this file.
void get_hardware_status(RAS_HW_STATUS_STRUCT& myhw ) 
{
    myhw.pSoftwareVersion = CODE_VERSION_STR;

//myhw.PWR_Button = myhw.PWR_Button ? 0: 1;  // cause toggle for testing

    // get the status of whatever you want to report back to the web page as the PWR_Button XML element// 
    myhw.PWR_Button = digitalRead(RELAY_SENSE_STATE); 

}   

void handle_SIMULATOR_Command(char* param)
{
    long regval = -1;
    if (param != NULL) {
        regval = cp.parseParameter(param);
        gSimulated_Events = regval;
    }
    WebText("\t- simulator events generated every %d ms\n", gSimulated_Events);
}

void handle_DISPLAYINVERT_Command(char* param)
{
    long regval = -1;
    if (param != NULL) {
        regval = cp.parseParameter(param);
        regval > 0 ? tft.invertDisplay(true) : tft.invertDisplay(false);
    }
    if(regval == -1)
        WebText("\t- Null/invalid parameter, nothing executed...must be boolean\n");
    else
        WebText("\t- tft.DisplayInvert(%s) applied\n", regval ? "true" : "false");
}

void handle_MASK_Command(char* param)
{
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        regval > 0 ? Sensor.writeMaskDisturbers(true) : Sensor.writeMaskDisturbers(false);
    }
    WebText("\t- Disturber Mask is: %s\n", Sensor.readMaskDisturbers() ? "true" : "false");
}

void handle_DIST_Command(char* param)
{
    // Lightning! Now how far away is it? Distance estimation takes into
    // account previously seen events.
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        WebText("\t- distance to storm register is read only -\n");
    }
    int distance = Sensor.readStormDistance();
    // km to miles conversionFactor = 621371 (0.621371 * 1000000)
    // Convert and scale down by 1000000 to get the integer miles value
    distance = ((long)distance * 621371L) / 1000000L;
    WebText("\t- Distance to storm is approximately: %d miles away\n", distance);
}

void handle_ENERGY_Command(char* param)
{
    // "Lightning Energy" and I do place into quotes intentionally, is a pure
    // number that does not have any physical meaning.
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        WebText("\t- register is read only -");
    }
    WebText("\t- Lightning energy is : %d\n", Sensor.readEnergy());
}

void handle_AFE_Command(char* param)
{
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        if(regval == AS3935MI::AS3935_OUTDOORS)
            Sensor.writeAFE(AS3935MI::AS3935_OUTDOORS);
        else if (regval == AS3935MI::AS3935_INDOORS) 
            Sensor.writeAFE(AS3935MI::AS3935_INDOORS);
        else  
            WebText("\t- only acceptable values are 0x12 (Indoor) or 0x0e (Outdoor)\n");
    }
    WebText("\t- AFE indoor/outdoor is %s\n", Sensor.readAFE() == AS3935MI::AS3935_OUTDOORS ? "OUTDOOR" : "INDOOR");
}

void handle_Noise_Command(char* param) 
{
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        Sensor.writeNoiseFloorThreshold(regval);
    }
    WebText("\t- NOISE floor threshold is set to: %#0x\n", Sensor.readNoiseFloorThreshold() );
}

void handle_DOGGY_Command(char *param) 
{
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        Sensor.writeWatchdogThreshold(regval);
    }
    WebText("\t- WatchDog Threshold is set to: %#04x\n", Sensor.readWatchdogThreshold());
}

void handle_SPIKE_Command(char *param)
{  
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        Sensor.writeSpikeRejection(regval);
    }
    WebText("\t- Spike rejection is set to: %#04x\n", Sensor.readSpikeRejection());
}

void handle_THRESH_Command(char* param) 
{
    // This setting will change when the lightning detector issues an interrupt.
    // instead of one. Register is a 2 bit field, therefore values of 0-0x3, corresponding strike count of 
    //  of 1, 5, 9 and 16 respectively are accepted.
    // Followed by its corresponding read function. Default value is zero (interrupt on one stroke).
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        // only allow values of 1,5,9,16 per datasheet
        //if (regval == 1 || regval == 5 || regval == 9 || regval == 16)
        if(regval >= 0 && regval <= 3)  // only 0-3 are valid
            Sensor.writeMinLightnings(regval);
        else
            printf("- Invalid value only 0x00-0x03 are valid \n");
    }
    WebText("\t- Strike Threshold - register value is: %#04x\n", Sensor.readMinLightnings());
}

void handle_TUNECAP_Command(char *param) 
{
    if (param != NULL) {
        // possibly write value to 4 bit tuning cap register field, value x 8pf = capacitance
        byte regvalue = cp.parseParameter(param);

        // register field is 4 bits, so only values of 0-15 decimal are valid, capacitance is 8pf per step
        if (regvalue <= 15 ) {  // 
            Sensor.writeAntennaTuning(regvalue); // write value
        } else {
            printf("\t invalid value, 4 bit field, only 0-15d (or 0x0 to 0x0e ) capacitance is 8pf per step, unchanged\n");
        }
    }
    int temp = Sensor.readAntennaTuning();
    WebText("\t- Tuning Cap register value: %#04x, %02d dec or %02d pf\n", temp, temp, temp * 8  ); // print out reg hex value AND picofarads
}

void handle_CALMODE_Command(char *param) 
{
    static int calmode_flag = false;
    if (param != NULL) {
        byte regvalue = cp.parseParameter(param);

        // put LD code into calibration mode, we can put a scope prope onto the IRQ pin and see the resonant frequency of the antenna
        // and adjust the tuning cap to get it to 500kHz.  
        // we also must set a flag to disable the LD code from handling the IRQ pin, so we can see the raw signal.
        // without being pummelled by event handling
        // This will tell the IC to display the resonance frequency as a digital signal on the interrupt pin. 
        if (regvalue > 0 ) {  
            Sensor.displayLcoOnIrq(true);
            calmode_flag = true;
            // now set a flag to disable the LD code from handling the IRQ pin, so we can see the raw signal.
            handle_EVTDSBL_Command((char*)"1");
        } else {
            Sensor.displayLcoOnIrq(false);
            calmode_flag = false;
            // re-enable the LD code to handle the IRQ pin
            handle_EVTDSBL_Command((char*)"0");
        }
    }
    WebText("\t- Antenna resonance Calibration Test Mode is  %s\n", calmode_flag ? "ON" : "OFF");
}

void handle_EVTDSBL_Command(char *param) 
{
    if (param != NULL) {
        long regvalue = cp.parseParameter(param);
        gEvents_Active_Flag = gEvents_Active_Flag ? false : true; // toggle the flag
        //regvalue > 0 ? gEvents_Active_Flag = false : gEvents_Active_Flag = true;
    } 
    WebText("\t- Events are %s\n", gEvents_Active_Flag ? "Enabled" : "Disabled");
}

void handle_FACT_Command(char *param)
{
    if (param != NULL) {
        // reset sensor registers to the factory default values.
        long regvalue = cp.parseParameter(param);
        // toss any paramter away, none required
        WebText("\t- Registers reset to factory defaults");
        if (regvalue > 0 )
            Sensor.resetToDefaults(); // nothing to write, parameterless function
    } else {
        WebText("\t- Factory - Reset Sensor Registers to factory defaults, non-zero parameter to initiate\n");
    }
}

void handle_CLRSTATS_Command(char *param) 
{
    if (param != NULL) {
        // clear the lighning strike stats that have accumulated in the last 15m integration window.
        long regvalue = cp.parseParameter(param);
        Sensor.clearStatistics(); // nothing to write, parameterless function
        WebText("\t- Sensor Strike statistics cleared");
    } else
        WebText("\t- Clear Sensor Stats - No action taken, requires any non-zero parameter to do something\n");
}

void handle_DIVRAT_Command(char *param)
{
    
    // REG0x03, bit [7:6], manufacturer default: 0 (16 division ratio).
    // The antenna is designed to resonate at 500kHz and so can be tuned with the
    // following setting. The accuracy of the antenna must be within 3.5 percent of
    // that value for proper signal validation and distance estimation.
    // This returns the current division ratio of the resonance frequency.
    // The antenna resonance frequency should be within 3.5 percent of 500kHz, and
    // so when modifying the resonance frequency with the internal capacitors
    // (tuneCap()) it's important to keep in mind that the displayed frequency on
    // the IRQ pin is divided by this number.
    if (param != NULL) {
        long regval = cp.parseParameter(param);
        Sensor.writeDivisionRatio(regval);
    }
    WebText("\t- Division ratio register is set to: %d\n", Sensor.readDivisionRatio());
}

void handle_CALANT_Command(char *param) 
{
    int32_t frequency = 0;
    // run the calibrate oscillators function in the library.
    if (param != NULL) {
        long regvalue = cp.parseParameter(param); 
        if(regvalue > 0 ) {
            if( Sensor.calibrateResonanceFrequency(frequency) ) {
                int temp = Sensor.readAntennaTuning();
                // print out reg hex value AND picofarads AND frequency
                WebText("\t- Antenna calibration successful reg value is : %#04x or %02d pf resonant frequency is %d Hz\n", temp, temp * 8, frequency  ); 
            } else {
                WebText("\t- Antenna Calibration failed\n");
            }
        }
    } else {
        WebText("\t- Auto-Calibrate antenna resonance - nothing done, non-zero parameter value to initiate\n");
    }
}

void handle_CALOSC_Command(char *param) 
{
    // run the calibrate oscillators function in the library.
    if (param != NULL) {
        long regvalue = cp.parseParameter(param); 
        if(regvalue > 0 ) {
            // calibrate the RCO.
            if (!Sensor.calibrateRCO()) {
                WebText("\tRCP Calibration failed.\n");
            } else
                WebText("\tRCO Calibration passed.\n");
        }
    } else {
        WebText("\t- RCO oscillator calibration - nothing done,  non-zero parameter value to initiate\n");
    }
}

// ************************************************

void handle_RESET_Command(char *param) 
{
    if (param != NULL) {
        long regvalue = cp.parseParameter(param);  
        if(regvalue > 0 ) {
            WebText("\t- Reset ESP32 in 3 seconds\n");
            if(Serial) Serial.flush(); // only flush if serial port is present, udderwise it blocks
            delay(3000);
            ESP.restart();
        }
    } else 
        WebText("\t- Reset ESP32 in 3 seconds, non-zero parameter to initiate");
}

void handle_DUMP_Command(char *param) 
{
    if (param != NULL) {
        long regvalue = cp.parseParameter(param);  // toss it out
    } 
    // run all the handlers with the exception of the last few, which are "dump" and help() which we dont need here.
    int numHandlers = sizeof(commandTable) / sizeof(CommandEntry);

    WebText("\nDump - Execute all %d handlers, passing no parameters\n", numHandlers-4);
    for (int i = 0; i < numHandlers-4; i++) {  //skip the last 4
        WebText("  %s - ",commandTable[i].commandName);
        commandTable[i].handler(NULL); // Call the handler with null parameter
        //delay(100);
    }
    Serial.println("");
}

// retrieve and print the max time in milliseconds it has take to run thru the executive loop, optionally clear it
void handle_GETMAXLOOPTIME_Comnmand(char *param) 
{
    if (param != NULL) {
        long regvalue = cp.parseParameter(param);  // toss it out
        gLongest_loop_time = 0;
    } 
    WebText("\t- maximum loop() execution time is %d\n", gLongest_loop_time);
};

void handle_STRTRH_Command(char* param){
    if (param != NULL) {
        gStrikeRateThreshold = cp.parseParameter(param); // if i have a parameter, use it to assign it
    }
    WebText("\t- Strike Rate Threshold = %d\n", gStrikeRateThreshold);
}

void handle_PWRRQST_Command(char *param) // turn power on/off the station via commandline
{
    bool val;
    if (param != NULL) {
        val = cp.parseParameter(param); // if i have a parameter, use it to assign it
        power_relay(val);
    }
    WebText("\t- Power status is = %s\n", digitalRead(RELAY_SENSE_STATE) ? "ON" : "OFF" );
}

void handle_HELP_Command(char *param) {
  if (param == NULL) {
    // Handle parameterless input
    WebText("\nCommand string        Description");
    WebText("\n  AFE     ----- Analog Front end, R/W, Indoor 0x12, Outdoor (0x0E), default 0x12, REG0x00[5:1]");
    WebText("\n  CALANT  ----- calibrates the antenna resonant frequency, about 2 seconds to complete");
    WebText("\n  CALMODE ----- puts the LD code into calibration mode, to display the resonance frequency on the IRQ pin, event handling disabled");
    WebText("\n  CALOSC  ----- calibrates the RCO oscillator");
    WebText("\n  CSTATS  ----- Clears sensor (REG0x02, bit [6]), internal lightning strike accumulator. ");
    WebText("\n  DIST    ----- Distance to storm est. in km, R/O, default 0, REG0x07, bit [5:0],");
    WebText("\n  DIVRAT  ----- Reads/sets the Divisor Ratio Register, R/W, default 16, only 16,32,64,128 are valid, REG0x03, bit [7:6]");
    WebText("\n  DOGGY   ----- Watchdog Threshold level, R/W, default 0x02, REG0x01, bits[3:0]");
    WebText("\n  DUMP    ----- run all the command handlers with no parameter (dump and help excluded)");
    WebText("\n  ENERGY  ----- lightning Energy 20 bits, R/O, a pure number - reg LSB REG0x04, bits[7:0],MSB REG0x05, bits[7:0],MMSB REG0x06, bits[4:0]");
    WebText("\n  EVTDSBL ----- disables the lightning detector code from handling the IRQ pin, so we can see the raw signal");
    WebText("\n  FACT    ----- Reset all registers to factory defaults, non-zero parameter to initiate, REG0x3C");
    WebText("\n  INV     ----- bool value applied to the tft.invertdisplay(bool) method ");
    WebText("\n  LOOPT   ----- retrieve/clear the max milliseconds it has taken to execute loop(), or clear with 0");
    WebText("\n  MASK    ----- Disturber Mask on/off, R/W, bool value  REG0x03, bit [5]");
    WebText("\n  NOISE   ----- Noise Floor Level, R/W, default: 0x02, REG0x01, bits [6:4]");
    WebText("\n  REGTST  ----- performs a r/w register test of the sensor's tuning cap reg bits, useful to see if u are really talking to it");
    WebText("\n  RESET   ----- ESP restart, in 3 seconds, non-zero param to initiate");
    WebText("\n  SIM     ----- turns on simulated event generator - 0 = OFF, nonzero value = time in MS between event gen ");
    WebText("\n  SPIKE   ----- Spike rejection filter, R/W, default 0x02, REG0x02, bits [3:0]");
    WebText("\n  THRESH  ----- Lightning Threshold, R/W, number of strikes before int pin triggered, default 0x0 (One stroke), manpage 35, REG0x02, bits [5:4]");
    WebText("\n  TUNECAP ----- Tuning Cap Register, R/W, 0-15 dec or 0x0-0xe, each step is 8pf, max of 16 possible steps REG0x08[3:0]");
    WebText("\n  ?       ----- print usage \n");
  } else {
    long junk = cp.parseParameter(param);
    WebText("\n\rInvalid parameter");
  }
}

int msToDHMS(unsigned long ms, char *buffer, size_t bufferSize) 
{
    unsigned long totalSeconds = ms / 1000;

    // Calculate days, hours, minutes, and seconds
    int days = totalSeconds / 86400;
    int hours = (totalSeconds % 86400) / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;
//Serial.printf("ET %03d:%02d:%02d:%02d\n", days,hours,minutes,seconds);
    // Format the string, checking for buffer size to prevent overrun
    // Ensuring bufferSize >= 12 to accommodate "D:H:M:S" format (10 characters + null terminator)
    if (bufferSize < 12) {
        // If the buffer is too small, indicate an error
        if (bufferSize > 0) {
            buffer[0] = '\0';  // Return an empty string if the buffer is too small
        }
        return -1;
    }

    // Using snprintf to format the string safely within the buffer size
    return snprintf(buffer, bufferSize, "%03d:%02d:%02d:%02d", days, hours, minutes, seconds);
}

void sensor_register_test( char *param )  { 
    if (param != NULL) {
        long regvalue = cp.parseParameter(param);  // toss it out
    } 
    // test the register to see if we are really reading/writing .
    WebText("\ntuning cap Register read/write LOOP test\n");
    bool result = false;
    for (int i = 0; i < 120; i++) {
        if (i % 8 == 0) {   // only values that are a multiple of 8 are valid for this register.
            Sensor.writeAntennaTuning(i); // write value
            delay(5);
            uint8_t value = Sensor.readAntennaTuning();
            delay(5);

            if (value != i) { // and try to read it back
                WebText("\tERROR on write of %#04x value read is %#02x\n", i, value);
                result |= true;
            }
        }
    }
    if(result == false)
        WebText("\tRead/Write Register Test SUCCESS\n");
}


void _tft_im_here(int line) {
    tft.setTextColor(TFT_MAGENTA, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(5,120,2);
    tft.setTextPadding(tft.width());  // pad it the width of the screen.
    tft.printf("TFT_IM_HERE -> %d \n", line); 
    tft.setTextSize(FONT_DEFAULT);
}

// formatted printf to the TFT screen at a specific location.
void tft_printf_text_at_pos(TFT_eSPI tft, uint16_t x, uint16_t y, uint16_t fg, uint16_t bg,  const char *format, ...){
    char buffer[256];  // create a buffer to hold the formatted string
    va_list args;      // create a va_list to hold the variable arguments
    va_start(args, format);  // initialize the va_list with the variable arguments
    vsnprintf(buffer, sizeof(buffer), format, args);  // format the string into the buffer
    va_end(args);  // cleanup the va_list
    tft.setTextColor(fg, bg, true);  // set the text color
    tft.setTextSize(2);  // set the text size
    tft.setCursor(x, y, FONT_DEFAULT);  // set the cursor position
    tft.print(buffer);  // print the buffer to the screen
}

