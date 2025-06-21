/*
  OK, ya ready for some fun? HTML + CSS styling + javascript all in and undebuggable environment

  
*/

// note R"KEYWORD( html page code )KEYWORD"; 
// again I hate strings, so char is it and this method let's us write naturally

const char PAGE_MAIN[] PROGMEM = R"=====(

<!DOCTYPE html>
<html lang="en" class="js-focus-visible">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Lightning Detector & Rate Chart</title>
  <!-- Chart.js for real-time charting -->
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
<!-- CW morse code library -->
  <script src="https://fkurz.net/ham/jscwlib/releases/jscwlib-0.3.0.js"></script>
  <style>
    /* Chart.js and dark theme styles from ratechart.html */
    html, body {
        height: 100%;
        margin: 0;
        padding: 0;
    }

    body {
        display: flex;
        flex-direction: column;
        min-height: 100vh;
    }

    .main-content {
        flex: 1 1 auto;
        display: flex;
        flex-direction: column;
        min-height: 0; /* Allow flex children to shrink */
    }

    .chart-container {
      width: calc(100vw - 16px);
      height: 30vh;
      border: 2px solid #333;
      border-radius: 16px;
      box-sizing: border-box;
      box-shadow: 0 4px 24px #000a;
      background: #1e1e1e;
      display: flex;
      justify-content: center;
      align-items: center;
      padding-left: 8px;
      padding-right: 8px;
      padding-top: 0;
      padding-bottom: 0;
      overflow: hidden;
      margin: 4px auto;
    }
    #myChart {
      width: 100% !important;
      height: 100% !important;
      display: block;
      background: #1e1e1e;
    }
    /* Lightning_Web.html styles */
    .bodytext {
        font-family: "Verdana", "Arial", sans-serif;
        font-size: 24px;
        text-align: left;
        font-weight: light;
        border-radius: 5px;
        display: inline;
    }

    .navbar {
        display: flex;
        justify-content: space-between;
        align-items: center;
        width: 100%;
        height: 28px;           /* Reduce height */
        margin: 0;
        padding: 2px 0px;       /* Reduce padding */
        background-color: #FFF;
        color: #000000;
        border-bottom: 3px solid #293578; /* Thinner border */
    }

    .container {
        display: flex;
        justify-content: space-between;
        align-items: center;
        width: 100%;
        max-width: 100%;
        margin: 0;
        padding: 0;
    }

    .navtitle {
        font-family: "Copperplate", "Arial", sans-serif;
        font-size: 16px;        /* Smaller font */
        font-weight: bold;
        line-height: 20px;      /* Tighter line height */
        padding-left: 4px;      /* Less padding */
        white-space: nowrap;
    }

    .navinfo {
        display: flex;
        align-items: center;
        white-space: nowrap;
        gap: 6px;               /* Less gap */
    }

    .navheading, .navdata {
        font-family: "Verdana", "Arial", sans-serif;
        font-size: 15px;
        font-weight: bold;
        line-height: 20px;
        padding-right: 20px;
        white-space: nowrap;
    }

    .navheading {
        margin-left: auto;
    }

    .navdata {
        font-size: 12px;        /* Smaller font */
        line-height: 16px;
        padding-right: 8px;     /* Less padding */
        margin-left: 10px;
    }

    .category {
        font-family: "Verdana", "Arial", sans-serif;
        font-weight: bold;
        font-size: 22px;
        line-height: 50px;
        padding: 20px 10px 0px 10px;
        color: #000000;
    }

    .foot {
        font-family: "Verdana", "Arial", sans-serif;
        font-size: 10px;
        position: relative;
        height: 20px;
        text-align: center;
        color: #0e0e0f;
        line-height: 20px;
    }

    .button {
        font-weight: bold;
        border-radius: 5px
    }

    .myDatabox {
        font-weight: bold;
        border-radius: 4px;
        resize: none;
        text-align: center;
        background-color: bisque;
        margin-right: 10px;
        min-width: 6ch
    }

    .myDiv1 {
        flex: 1 1 auto;
        display: flex;
        flex-direction: column;
        border-radius: 3px;
        margin: auto;
        border: 3px solid rgb(84, 230, 71);
        background-color: rgba(27, 27, 27, 0.473);
        padding: 2px;
        width: 100%;
        max-width: none;
        overflow: hidden;
        box-sizing: border-box;
        min-height: 0; /* Allow flex children to shrink */
        margin-top: 2px;
    }

    .termta#Terminal,
    .termta#TermInput {
        width: 100%;           /* <-- fill container horizontally */
        min-width: 0;          /* <-- allow shrinking in flexbox */
        box-sizing: border-box;
        background-color: #2c2c2c !important;
        color: cyan !important;
    }

    .termta#Terminal {
        flex: 1 1 auto;
        resize: none;
        min-height: 0;
        width: 100%;
        border-radius: 3px;
        box-sizing: border-box;
    }

    .termta#TermInput {
        flex: 0 0 auto;
        height: 2.2em;
        resize: none;
        margin-top: 2px;
        width: 100%;
        border-radius: 3px;
        box-sizing: border-box;
    }

    .grid-container {
        display: grid;
        grid-template-columns: repeat(4, auto);
        grid-template-rows: repeat(2, auto);
        gap: 10px;
        align-items: center;
        max-width: 800px;
        margin: auto;
    }

    label {
        text-align: right;
        padding-right: 10px;
        font-size: 1rem;
        color: #000;
    }

    input {
        margin: 10px;
    }

    .volume-control-container {
        display: flex;
        align-items: center;
        gap: 5px;
    }

    hr {
    border: none;
    border-top: 2px solid #293578; /* or any color you want */
    margin:4px 0;
    width: 100%;
    background: transparent;
}
  </style>
</head>
<body style="background-color: #7a8285" onload="OnMyFormLoad()">

    <header>
        <div class="navbar fixed-top">
            <div class="container">
                <div class="navtitle" id="headerTitle">ESP32 Lightning Detector v999.999</div>
                <div class="navinfo">
                    <div class="navdata" id="date">mm/dd/yyyy</div>
                    <div class="navdata" id="time">00:00:00</div>
                </div>
            </div>
        </div>
    </header>

    <main class="main-content">
        <!-- Controls Section -->
        <section class="volume-control-container">
            <input type="button" class="button" id="PWR_Select" name="PWR_Select" value="POWER"
                onclick="PowerButtonHandler(this.id)">
            <input type="button" class="button" id="R2Select" name="Radio2" value="Test"
                onclick="TestButtonHandler(this.id)">
            <label for="volume">Strike Alarm Volume:</label>
            <input type="range" id="volume" min="0" max=".1" step="0.02" value="0.02">
        </section>
        <hr>
        <!-- Sensor Data Section -->
        <section class="grid-container">
            <label for="NoiseAcc">Noise Accumulator:</label>
            <textarea readonly class="myDatabox" id="NoiseAcc" name="NoiseAcc" rows="1" cols="4">123456</textarea>
            <label for="NoiseET">Noise ET since last:</label>
            <textarea readonly class="myDatabox" id="NoiseET" name="NoiseET" rows="1" cols="4">123456</textarea>
            <label for="DisturberAcc">Disturber Accumulator:</label>
            <textarea readonly class="myDatabox" id="DisturberAcc" name="DisturberAcc" rows="1" cols="4">123456</textarea>
            <label for="DisturberET">Disturber ET since last:</label>
            <textarea readonly class="myDatabox" id="DisturberET" name="DisturberET" rows="1" cols="4">123456</textarea>
            <label for="StrikeAcc">Strike Accumulator:</label>
            <textarea readonly class="myDatabox" id="StrikeAcc" name="StrikeAcc" rows="1" cols="4">123456</textarea>
            <label for="StrikeET">Strike ET since last:</label>
            <textarea readonly class="myDatabox" id="StrikeET" name="StrikeET" rows="1" cols="4">123456</textarea>
            <label for="StrikeDist">Strike Distance(mi):</label>
            <textarea readonly class="myDatabox" id="StrikeDist" name="StrikeDist" rows="1" cols="4">123456</textarea>
            <label for="StrikeEnergy">Strike Energy:</label>
            <textarea readonly class="myDatabox" id="StrikeEnergy" name="StrikeEnergy" rows="1" cols="5">123456</textarea>
        </section>
        <hr>
        <!-- Chart Section -->
        <section class="chart-container">
            <canvas id="myChart"></canvas>
        </section>
        <hr>
        <!-- Terminal Section -->
        <section class="myDiv1">
            <textarea readonly class="termta" id="Terminal" name="Terminal"></textarea>
            <textarea class="termta" id="TermInput" name="TermInput" rows="1" cols="200">CMD> </textarea>
        </section>
    </main>

    <footer class="foot" id="Chincey">THIS IS A TEST FOOTER</footer>

  <script type="text/javascript">
  // ============================================= WEB PAGE ABOVE =================================================
  // ============================================= WEB PAGE ABOVE =================================================
  // *********************
  // 
  let USE_SIMULATED_DATA = false;

  // Detect if running as a local file (file://)
  if (window.location.protocol === "file:") {
      USE_SIMULATED_DATA = true;
      console.log("Running as local file: using simulated data.");
  }
  // Detect if running on Five Server (commonly localhost:5500 or 127.0.0.1:5500)
  else if (
      window.location.hostname === "localhost" ||
      window.location.hostname === "127.0.0.1" ||
      window.location.port === "5500"
  ) {
      USE_SIMULATED_DATA = true;
      console.log("Running on Five Server: using simulated data.");
  } else {
      USE_SIMULATED_DATA = false;
      console.log("Running on ESP32 or production server: using real data.");
  }

  // *********************
  // global variable visible to all java functions
  var xmlHttp = createXmlHttpObject();

  // helper function to create XML object, returns that created object
  function createXmlHttpObject() {
      if (window.XMLHttpRequest) {
          xmlHttp = new XMLHttpRequest();
      }
      else {
          xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");  // for older IE versions
      }
      return xmlHttp;
  }

  const audioContext = new (window.AudioContext || window.webkitAudioContext)();

  // jscw library for Morse code playback    
  // At the top of your script, create a global instance:
  const morsePlayer = new jscw({
      "wpm": 30,
      "freq": 550, // tone frequency (Hz)
      "volume": .2 // initial volume
  });

  // --------------------- handle the terminal input ------------------------------      
  document.getElementById('TermInput').addEventListener('keypress', function (event) {
      if (event.key === 'Enter') {
          event.preventDefault();

          // Get the command typed by the user
          const command = document.getElementById('TermInput').value;
          morsetext = command.replace("CMD> ", ""); // remove the prompt from the command
          morsetext = morsetext.replace(":", ' ')
          playMorse(morsetext); // play the command in morse code

          // 05-June-2025 new feature to handle "update" command locally without server interaction
          // This allows the user to change the chart update rate dynamically to between 1000ms and 60000ms
          // and see the effect immediately without needing to reload the page or wait for the server response.
          // --- Parse "update" command (case-insensitive) and adjust chart update rate ---
          // Accepts "CMD> update", "CMD> update 4000", or "CMD> update:4000"
          const updateMatch = command.match(/^CMD>\s*update(?:[\s:]+(\d+))?/i);
          if (updateMatch) {
              if (updateMatch[1]) {
                  let newRate = parseInt(updateMatch[1], 10);
                  // Clamp the value between 1000 and 60000
                  newRate = Math.max(1000, Math.min(60000, newRate));
                  chartUpdateRateInMs = newRate;
                  TextLog(`Chart update rate has been changed to ${newRate} ms x-axis labels may not be useful`);
                  document.getElementById('Chincey').textContent = `Chart update rate: ${(newRate/1000).toFixed(1)} s`; // Update footer
              } else {
                  TextLog(`Current chart update rate is ${chartUpdateRateInMs} ms`);
                  document.getElementById('Chincey').textContent = `Chart update rate: ${(chartUpdateRateInMs/1000).toFixed(1)} s`; // Update footer
              }
              // Do not send to ESP32 server if this is a locally handled command
          } else {
              // If not a local/client command, forward to the server
              var xhttp = new XMLHttpRequest();
              xhttp.open("PUT", 'TermInput', true); // send name of radio button selected to server
              xhttp.send(command.substring(5)); // only send the command not the prompt
              console.log(`http PUT sent: ${command}`);
          }

          document.getElementById('TermInput').value = 'CMD> ';  // leave a prompt behind
      }
  });
  // -----------------------------------------------------------      

  //  handle a selection made
  function PowerButtonHandler(value) {
      console.log("Radio " + value);
      document.getElementById(value).style.backgroundColor = "#ffff00";
      var xhttp = new XMLHttpRequest();
      xhttp.open("PUT", value, true); // send name of radio button selected to server
      xhttp.send();
  }

  function TestButtonHandler(value) {
      console.log("TestButtonHandler " + value);
      //playAlarm(600, 250, 'triangle');
      playMorse("TEST"); // play TEST in morse code
      //document.getElementById("value").style.backgroundColor = "#ffff00";
      //var xhttp = new XMLHttpRequest();
      //xhttp.open("PUT", value, true); // send name of ALL Grounded button selected to server
      //xhttp.send();
  }

  // try to emulate an autoscrolling log using a text area
  function TextLog(value) {
      var dt = new Date();
      var ta = document.getElementById("Terminal");
      ta.value += "\n" + dt.toLocaleTimeString() + "> " + value;
      ta.scrollTop = ta.scrollHeight;
  }

  // function to handle the XML response FROM the ESP <<< NO LONGER USED >>>
  // let Strike_last_strike_accumulator_value = 0;  // used to determine when to play the strike alarm tone
  // let Strike_this_time = 0;
  /* function response() {  // this version parses XML response from ESP32
    // implement an equiv to the old C style static variables, survives across function calls to avoid global variables
    if (typeof response.Strike_last_strike_accumulator_value === "undefined") {
      response.Strike_last_strike_accumulator_value = 0; // Only initialized once
    }
    // response.Strike_last_strike_accumulator_value = 0;  // used to determine when to play the strike alarm tone
    let Strike_this_time = 0;
    let message;
    let barwidth;
    let currentsensor;
    let xmlResponse;
    let date = new Date();
    let color = "#e8e8e8";

    xmlResponse = xmlHttp.responseXML;
    if (xmlResponse == null) return;

    document.getElementById("date").innerHTML = date.toLocaleDateString('en-GB', {
            day: '2-digit',
            month: 'short',
            year: 'numeric'
        }).replace(/ /g, '-').toUpperCase();
    document.getElementById("time").innerHTML = date.toLocaleTimeString();

    let xmldoc = xmlResponse.getElementsByTagName("INFO");
    try {
        if (xmldoc.length > 0 ){
            let message = xmldoc[0].firstChild.nodeValue.trim();
            TextLog(message);
            console.log(message);
        }
    } catch (e) {
        console.error("Error reading INFO tag:", e);
    }
    //console.log((new XMLSerializer()).serializeToString(xmlResponse));  // debug

    xmldoc = xmlResponse.getElementsByTagName("PWR");
    message = xmldoc[0].firstChild.nodeValue;
    message == "1" ? color = "#00ff00" : color = "#ff0000";
    document.getElementById("PWR_Select").style.backgroundColor = color;

    xmldoc = xmlResponse.getElementsByTagName("NOISE_ACC");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("NoiseAcc").value = message;

    xmldoc = xmlResponse.getElementsByTagName("NOISE_ET");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("NoiseET").value = message;

    xmldoc = xmlResponse.getElementsByTagName("DISTURB_ACC");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("DisturberAcc").value = message;

    xmldoc = xmlResponse.getElementsByTagName("DISTURB_ET");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("DisturberET").value = message;


    xmldoc = xmlResponse.getElementsByTagName("STRIKE_ACC");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("StrikeAcc").value = message;
    // the following code is use to determine when to play the strike alarm tone
    // if the strike accumulator value is greater than the last strike accumulator value,
    // we have a new strike, so then play the strike alarm tone  
    Strike_this_time = Number(message);
    if( Strike_this_time > response.Strike_last_strike_accumulator_value) {
      if( ! USE_SIMULATED_DATA ) {
//        playStrikeAlarm();  // finally, play the strike alarm tone, multiple tones, had issue with this when lots of strikes
          playMorse("SOS"); // play SOS in morse code
          //playAlarm(400, 250, 'triangle') ; // play a single strike tone, 400hz for 250ms
          response.Strike_last_strike_accumulator_value = Strike_this_time; // save the last strike accumulator value
        } else {
          TextLog(">SIMBEEP<"); // so i dont have to listen to the alarm tone when testing
          //playAlarm(400, 250, 'triangle') ; // play a single strike tone, 400hz for 250ms
          response.Strike_last_strike_accumulator_value = 0; // reset the strike accumulator       
      } 
    }

    xmldoc = xmlResponse.getElementsByTagName("STRIKE_ET");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("StrikeET").value = message;

    xmldoc = xmlResponse.getElementsByTagName("STRIKE_DIST");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("StrikeDist").value = message;

    xmldoc = xmlResponse.getElementsByTagName("STRIKE_ENER");
    message = xmldoc[0].firstChild.nodeValue;
    document.getElementById("StrikeEnergy").value = message;

    xmldoc = xmlResponse.getElementsByTagName("VER");
    if (xmldoc.length > 0) {
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("headerTitle").textContent = "ESP32 Storm detector " + message;
    }
    
    // Update chart data from <RATES> XML tag
      if( !USE_SIMULATED_DATA )
      updateChartFromRatesXML(xmlResponse);
  } */
     
  // Place this outside the function, at the top of your script
  let Strike_last_strike_accumulator_value = 0;
  
  let lastChartUpdateTime = 0; // Add this global to track last chart update time
  let chartUpdateRateInMs = 60000; // Update every 60 seconds by default
  
  // This version of the response function handles the JSON response from the ESP32
  function response() { 
    let Strike_this_time = 0;
    let message;
    let color = "#e8e8e8";

    // --- Parse JSON response from ESP32 ---
    let jsonResponse;
    try {
        jsonResponse = JSON.parse(xmlHttp.responseText);
    } catch (e) {
        TextLog("Error parsing JSON response: " + e);
        return;
    }
console.log('Received JSON:', jsonResponse);
    // --- Update date and time fields ---
    let date = new Date();
    document.getElementById("date").innerHTML = date.toLocaleDateString('en-GB', {
        day: '2-digit',
        month: 'short',
        year: 'numeric'
    }).replace(/ /g, '-').toUpperCase();
    document.getElementById("time").innerHTML = date.toLocaleTimeString();

    // --- INFO log ---
    if (jsonResponse.INFO) {
        try {
            let infoMsg = jsonResponse.INFO.trim();
            TextLog(infoMsg);
            console.log(infoMsg);
        } catch (e) {
            console.error("Error reading INFO field:", e);
        }
    }

    // --- Power status color ---
    if (jsonResponse.PWR !== undefined) {
        color = jsonResponse.PWR == "1" ? "#00ff00" : "#ff0000";
        document.getElementById("PWR_Select").style.backgroundColor = color;
    }

    // --- Noise Accumulator ---
    if (jsonResponse.NOISE_ACC !== undefined)
        document.getElementById("NoiseAcc").value = jsonResponse.NOISE_ACC;

    // --- Noise ET ---
    if (jsonResponse.NOISE_ET !== undefined)
        document.getElementById("NoiseET").value = jsonResponse.NOISE_ET;

    // --- Disturber Accumulator ---
    if (jsonResponse.DISTURB_ACC !== undefined)
        document.getElementById("DisturberAcc").value = jsonResponse.DISTURB_ACC;

    // --- Disturber ET ---
    if (jsonResponse.DISTURB_ET !== undefined)
        document.getElementById("DisturberET").value = jsonResponse.DISTURB_ET;

    // --- Strike Accumulator and alarm logic ---
    if (jsonResponse.STRIKE_ACC !== undefined) {
        document.getElementById("StrikeAcc").value = jsonResponse.STRIKE_ACC;
        let Strike_this_time = Number(jsonResponse.STRIKE_ACC);
        // Play alarm if new strike detected
        if (Strike_this_time > Strike_last_strike_accumulator_value) {
            if (!USE_SIMULATED_DATA) {
                playMorse("SOS"); // play SOS in morse code
                Strike_last_strike_accumulator_value = Strike_this_time;
            } else {
                TextLog(">SIMBEEP<");
                Strike_last_strike_accumulator_value = 0;
            }
        }
    }

    // --- Strike ET ---
    if (jsonResponse.STRIKE_ET !== undefined)
        document.getElementById("StrikeET").value = jsonResponse.STRIKE_ET;

    // --- Strike Distance ---
    if (jsonResponse.STRIKE_DIST !== undefined) {
        let strikeDist = jsonResponse.STRIKE_DIST;
        strikeDist = Number(strikeDist).toFixed(1);; // Ensure 1 decimal place
        document.getElementById("StrikeDist").value = strikeDist
    }
    
    // --- Strike Energy ---
    if (jsonResponse.STRIKE_ENER !== undefined)
        document.getElementById("StrikeEnergy").value = jsonResponse.STRIKE_ENER;

    // --- Version info in header ---
    if (jsonResponse.VER !== undefined)
        document.getElementById("headerTitle").textContent = "ESP32 Storm detector " + jsonResponse.VER;

    // --- Update chart data from RATES array ---
    // Expecting RATES as an array of numbers, e.g. [strike, disturber, noise, purge]
    if (!USE_SIMULATED_DATA && Array.isArray(jsonResponse.RATES)) {
        // Only update once per minute unless overridden by user command
        const now = Date.now();
        if (now - lastChartUpdateTime < chartUpdateRateInMs) return; // if'n its not time to update, return do nothing
        lastChartUpdateTime = now;
        updateChartWithRates(jsonResponse.RATES.map(val => {
            let f = parseFloat(val);
            return (isNaN(f) || f < 0) ? 0 : f;
        }));
    }
  }

  function getVolume() {
      return document.getElementById('volume').value;
  }
  
  function playStrikeAlarm() {
    const oscillator = audioContext.createOscillator();
    const gainNode = audioContext.createGain();
    TextLog("playStrikeAlarm() called...\n");
    oscillator.stop();
    oscillator.type = 'triangle';
    oscillator.frequency.setValueAtTime(400, audioContext.currentTime);
    gainNode.gain.value = getVolume();
    oscillator.connect(gainNode);
    gainNode.connect(audioContext.destination);
    oscillator.start();
    let pulsing = true;
    const interval = setInterval(() => {
        gainNode.gain.setValueAtTime(pulsing ? 0 : getVolume(), audioContext.currentTime);
        pulsing = !pulsing;
    }, 250);
    setTimeout(() => {
        clearInterval(interval);
        oscillator.stop();
    }, 2000);
  }

  // Function to play Morse code using the jscw library
  function playMorse(text) {
    let vol = getVolume();
    morsePlayer.setVolume(vol);
    morsePlayer.setText(text);
    if (audioContext.state === 'suspended') {
      audioContext.resume().then(() => {
        morsePlayer.play();
      }).catch(e => console.error('playMorse()-Could not resume audio context:', e));
    } else {
      morsePlayer.play();
    }
    //TextLog("Playing Morse: " + text + " vol " + vol);
  }

  function playAlarm(frequency, duration, type) {
    if (audioContext.state === 'suspended') {
      audioContext.resume().catch(e => console.error('PlayAlarm()-Audio resume failed:', e));
    }
    // Initialize player with desired settings
    const oscillator = audioContext.createOscillator();
    const gainNode = audioContext.createGain();
    oscillator.type = type;
    oscillator.frequency.setValueAtTime(frequency, audioContext.currentTime);
    gainNode.gain.value = getVolume();
    oscillator.connect(gainNode);
    gainNode.connect(audioContext.destination);
    oscillator.start();
    oscillator.stop(audioContext.currentTime + duration / 1000);
  }

  function OnMyFormLoad() {
    const textarea = document.getElementById("TermInput");
    const position = 5;
    textarea.focus();
    textarea.selectionStart = position;
    textarea.selectionEnd = position;
    process();
  };

    // --- XML timeout and auto-reload logic ---
  let xmlTimeoutCount = 0;                // Track consecutive XML timeouts
  const XML_TIMEOUT_MS = 5000;            //  seconds per XML request
  const XML_TIMEOUT_LIMIT = 3;            // Reload after 3 consecutive timeouts
  let reloadInterval = null;              // Interval handle for repeated reloads

  function process() {
      let timedOut = false;
      // Start a timeout for the XML request
      let timeoutHandle = setTimeout(function() {
          timedOut = true;
          xmlTimeoutCount++;
          TextLog(`XML request timed out (${xmlTimeoutCount}/${XML_TIMEOUT_LIMIT})`);
          if (xmlTimeoutCount >= XML_TIMEOUT_LIMIT) {
              TextLog("Too many XML timeouts. Attempting to reload the web page...");
              tryReloadUntilSuccess();
          }
      }, XML_TIMEOUT_MS);

      // Only send a new request if ready
      if (xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
          // xmlHttp.open("GET", "xml", true);
          xmlHttp.open("GET", "json", true); // Use JSON endpoint for better performance
          xmlHttp.onreadystatechange = function() {
              if (xmlHttp.readyState == 4) {
                  clearTimeout(timeoutHandle);
                  if (!timedOut) {
                      if (xmlTimeoutCount > 0) TextLog("XML request succeeded, resetting timeout counter.");
                      xmlTimeoutCount = 0; // Reset on success
                      response();
                  }
              }
          };
          xmlHttp.send(null);
      }
      // Continue polling if not using simulated data and not in reload mode
      if (!USE_SIMULATED_DATA && reloadInterval === null)
          setTimeout(process, 500); // Poll the server every 500ms
  }

  // This function will continuously try to reload the page until it succeeds
  function tryReloadUntilSuccess() {
      if (reloadInterval !== null) return; // Already trying to reload

      TextLog("Starting continuous reload attempts...");
      reloadInterval = setInterval(function() {
          TextLog("Reloading page (location.reload)...");
          // Try to reload from the server (not cache)
          location.reload(true);
          // If reload succeeds, this script will be reloaded and interval will be cleared by browser
          // If reload fails, this interval will keep trying every 5 seconds
      }, 5000);
  }

// --- Chart.js initialization (add your chart logic here) ---
  // Example: basic real-time chart setup
  // --- DATA INITIALIZATION ---

  // Initial x-axis labels (time points)
  let xLabels = [0, 1, 2, 3];

  // Initial data for 4 traces (each array is a trace)
  let dataValues = [
    [0, 0, 0, 0], // Trace 1: Strikes
    [0, 0, 0, 0], // Trace 2: Disturbers
    [0, 0, 0, 0], // Trace 3: Noise
    [0, 0, 0, 0]   // Trace 4: Purges
  ];

  // Counter for generating new time points
  let counter = 4;
  
  // This function initializes the chart when the DOM is fully loaded.
  // Wait until the DOM is fully loaded before running this function.
  // This ensures that all HTML elements (like the canvas for the chart) are available.
  document.addEventListener("DOMContentLoaded", function() {
    // Get the 2D drawing context from the canvas element with id 'myChart'
    const ctx = document.getElementById('myChart').getContext('2d');
    const colors = [
      '#569CD6', // blue (Strike)
      '#D7BA7D', // yellow (Disturber)
      '#C586C0', // purple (Noise)
      '#9CDCFE'  // cyan (Purge)
    ];
    // const colors = [
          // Accent colors for each trace (high contrast)
    //   '#FF3333', // red (Strike)
    //   '#FFD700', // gold/yellow (Disturber)
    //   '#33B5FF', // bright blue (Noise)
    //   '#00FF99'  // bright green/cyan (Purge)
    // ];

    // Human-friendly names for each trace
    const traceNames = ['Strike', 'Disturber', 'Noise', 'Purge'];

    // Create a new Chart.js line chart and assign it to a global variable for later updates
    window.myChart = new Chart(ctx, {
      type: 'line',
      data: {
        labels: xLabels, // Initial x-axis labels
        datasets: dataValues.map((arr, i) => ({
          label: traceNames[i],                  // Name for legend and tooltips
          data: arr,                             // Initial data for this trace
          borderColor: colors[i],                // Line color
          backgroundColor: colors[i] + '33',     // 20% opacity fill under line
          cubicInterpolationMode: 'monotone',    // Smoother lines (can use 'monotone')
          tension: 0.1                           // Line tension for smoothness
        }))
      },
      options: {
        responsive: true,                        // Chart resizes with container
        maintainAspectRatio: false,              // Fill container height/width
        plugins: {
          legend: {
            labels: {
              color: '#d4d4d4',                 // Legend text color
              boxWidth: 16,   // default is 40, reduce for smaller boxes
              boxHeight: 8,   // default is 12, reduce for smaller boxes
              // Custom label generation to show latest value in legend
              generateLabels: function(chart) {
                // Get the default labels
                const original = Chart.defaults.plugins.legend.labels.generateLabels(chart);
                // For each dataset, append the latest value to the label
                return original.map((label, i) => {
                  // Get the latest value for this dataset (leftmost, since you unshift)
                  const dataArr = chart.data.datasets[i].data;
                  const latest = dataArr.length > 0 ? dataArr[0] : '';
                  return {
                    ...label,
                    text: `${chart.data.datasets[i].label}: ${latest}`
                  };
                });
              }
            }
          },
          title: {
            display: true,
            text: 'Sliding Event Rates',
            color: '#d4d4d4',
            font: {
              size: 14,      // Reduce from 20 to 14 (or smaller if you want)
              weight: 'bold'
            },
            padding: { top: 4, bottom: 6 } // Reduce padding above/below title
          }
        },
        scales: {
          x: {
            title: { display: true, text: 'Time (min)', color: '#d4d4d4' }, // X axis label
            min: 0,                  // Start at 0 (fixed after 60 points)
            max: 59,                 // End at 59 (fixed after 60 points)
            ticks: {
              color: '#d4d4d4',      // Tick label color
              callback: function(value, index, ticks) {
                // Always show 0-59, left to right
                return index;
              }
            },
            grid: {
              color: 'rgba(136,136,136,0.49)',      // Dimmed grid lines (49% opacity)
              lineWidth: 1,
              drawBorder: true,
              borderColor: 'rgba(255,255,255,0.49)', // Dimmed axis line
              borderWidth: 2
            }
          },
          y: {
            title: { display: true, text: 'Rate/min', color: '#d4d4d4' }, // Y axis label
            min: 0,                // Start y-axis at 0
            max: 10,               // Start y-axis at 10 (will autoscale later)
            ticks: { color: '#d4d4d4' },                                  // Tick label color
            grid: {
              color: 'rgba(136,136,136,0.49)',      // Dimmed grid lines (49% opacity)
              lineWidth: 1,
              drawBorder: true,
              borderColor: 'rgba(255,255,255,0.49)', // Dimmed axis line
              borderWidth: 2
            }
          }
        }
      } // <-- closes options
    }); // <-- closes new Chart

    // Set initial footer value on page load
    document.getElementById('Chincey').textContent = `Chart update rate: ${(chartUpdateRateInMs/1000).toFixed(1)} s`;

    // Optionally, you could start simulated data updates here after the chart is created:
    if(USE_SIMULATED_DATA) {
      // Call the update function immediately to start the simulation
      updateChartSimulatedData();
      setInterval(updateChartSimulatedData, 1000);
    }
  }); // <-- closes DOMContentLoaded


  // --- REAL-TIME DATA SIMULATION AND SLIDING WINDOW LOGIC ---
  function updateChartSimulatedData() {
    // Exit if the chart is not initialized
    if (!window.myChart) return;

    // Generate a new random value for each trace (Strike, Disturber, Noise, Purge)
    let rateValues = dataValues.map(() => {
        // Generate a random value between 0 and 12, rounded to 1 decimal place
        let newValue = Math.random() * 12;
        newValue = Math.round(newValue * 10) / 10;
        // Ensure the value is not negative or NaN
        return (isNaN(newValue) || newValue < 0) ? 0 : newValue;
    });

    // Pass the simulated rate values to the unified chart update function
    updateChartWithRates(rateValues);
  }

  // --- CHART UPDATE FUNCTION FROM XML DATA ---
  // Updates the chart with new data from the <RATES> XML tag.
  // Call this from your response() function after receiving new XML data.

  function updateChartFromRatesXML(xmlResponse) {
    // Only update once per minute
    const now = Date.now();
    if (now - lastChartUpdateTime < chartUpdateRateInMs) return; // 60000 ms = 1 minute
    lastChartUpdateTime = now;

    // Exit if the chart is not initialized
    if (!window.myChart) return;

    // Get the <RATES> tag from the XML response
    const ratesTag = xmlResponse.getElementsByTagName("RATES");
    let rateValues;

    // If <RATES> tag is missing or empty, use zeros as fallback
    if (!ratesTag || ratesTag.length === 0) {
      rateValues = [0, 0, 0, 0];
    } else {
      // Extract the comma-separated string from the <RATES> tag
      const ratesString = ratesTag[0].textContent || ratesTag[0].firstChild.nodeValue;
      if (!ratesString) return; // If the tag is empty, exit
      // Split the string into an array and parse each value as a float
      // Replace invalid or negative values with 0
      rateValues = ratesString.split(',').map(val => {
        let f = parseFloat(val);
        return (isNaN(f) || f < 0) ? 0 : f;
      });
    }

    // Pass the parsed rate values to the unified chart update function
    updateChartWithRates(rateValues);
  }

  function updateChartWithRates(rateValues) {
    // For each trace (Strike, Disturber, Noise, Purge):
    // Insert the new value at the start of the array (most recent on the left)
    // Keep only the most recent 60 values for each trace
    for (let i = 0; i < dataValues.length; i++) {
      dataValues[i].unshift(rateValues[i] || 0); // Add new value or 0 if missing
      if (dataValues[i].length > 60) dataValues[i].pop(); // Remove oldest if over 60
      window.myChart.data.datasets[i].data = dataValues[i]; // Update chart dataset
    }

    // Add a new label (time/sample index) to the start of the xLabels array
    // Keep only the most recent 60 labels
    xLabels.unshift(counter++);
    if (xLabels.length > 60) xLabels.pop();

    // --- X AXIS LOGIC ---
    if (xLabels.length < 60) {
      // If fewer than 60 points, use dynamic labels and let Chart.js autoscale x-axis
      window.myChart.data.labels = xLabels;
      window.myChart.options.scales.x.min = undefined;
      window.myChart.options.scales.x.max = undefined;
    } else {
      // Once 60 or more points, use fixed labels 0-59 and fix x-axis range
      window.myChart.data.labels = Array.from({length: 60}, (_, i) => i);
      window.myChart.options.scales.x.min = 0;
      window.myChart.options.scales.x.max = 59;
    }

    // --- Y AXIS LOGIC ---
    // Always keep y-axis minimum at 0 (never show negative values)
    // Let Chart.js autoscale the maximum
    window.myChart.options.scales.y.min = 0;
    window.myChart.options.scales.y.max = undefined;

    // Update the chart without animation for real-time effect
    window.myChart.update('none');

    // Optional: Auto-scroll the chart container to the left (if scrollable)
    const container = document.querySelector('.chart-container');
    if (container) container.scrollLeft = 0;
  }

  // --------------------- handle the terminal input with command history ------------------------------      
  const commandHistory = [];
  let historyIndex = -1;

  document.getElementById('TermInput').addEventListener('keydown', function (event) {
    // Up arrow
    if (event.key === 'ArrowUp') {
        if (commandHistory.length > 0) {
            if (historyIndex === -1) historyIndex = commandHistory.length - 1;
            else if (historyIndex > 0) historyIndex--;
            this.value = commandHistory[historyIndex];
            // Move cursor to end
            this.setSelectionRange(this.value.length, this.value.length);
        }
        event.preventDefault();
    }
    // Down arrow
    else if (event.key === 'ArrowDown') {
        if (commandHistory.length > 0 && historyIndex !== -1) {
            if (historyIndex < commandHistory.length - 1) {
                historyIndex++;
                this.value = commandHistory[historyIndex];
            } else {
                historyIndex = -1;
                this.value = 'CMD> ';
            }
            // Move cursor to end
            this.setSelectionRange(this.value.length, this.value.length);
        }
        event.preventDefault();
    }
  });

  // Replace previous keypress handler with keydown for Enter
  document.getElementById('TermInput').addEventListener('keydown', function (event) {
    if (event.key === 'Enter') {
        event.preventDefault();

        // Get the command typed by the user
        const command = document.getElementById('TermInput').value;

        // Only store non-empty, non-duplicate consecutive commands
        if (command.trim() !== '' && (commandHistory.length === 0 || command !== commandHistory[commandHistory.length - 1])) {
            commandHistory.push(command);
            if (commandHistory.length > 16) commandHistory.shift(); // Keep only last 16
        }
        historyIndex = -1; // Reset history navigation

        morsetext = command.replace("CMD> ", ""); // remove the prompt from the command
        morsetext = morsetext.replace(":", ' ')
        playMorse(morsetext); // play the command in morse code

        // --- Parse "update" command (case-insensitive) and adjust chart update rate ---
        const updateMatch = command.match(/^CMD>\s*update(?:[\s:]+(\d+))?/i);
        if (updateMatch) {
            if (updateMatch[1]) {
                let newRate = parseInt(updateMatch[1], 10);
                // Clamp the value between 1000 and 60000
                newRate = Math.max(1000, Math.min(60000, newRate));
                chartUpdateRateInMs = newRate;
                TextLog(`Chart update rate has been changed to ${newRate} ms x-axis labels may not be useful`);
                document.getElementById('Chincey').textContent = `Chart update rate: ${(newRate/1000).toFixed(1)} s`; // Update footer
            } else {
                TextLog(`Current chart update rate is ${chartUpdateRateInMs} ms`);
                document.getElementById('Chincey').textContent = `Chart update rate: ${(chartUpdateRateInMs/1000).toFixed(1)} s`; // Update footer
            }
            // Do not send to ESP32 server if this is a locally handled command
        } else {
            // If not a local/client command, forward to the server
            var xhttp = new XMLHttpRequest();
            xhttp.open("PUT", 'TermInput', true); // send name of radio button selected to server
            xhttp.send(command.substring(5)); // only send the command not the prompt
            console.log(`http PUT sent: ${command}`);
        }

        document.getElementById('TermInput').value = 'CMD> ';  // leave a prompt behind
    }
  });
</script>
</html>


)=====";