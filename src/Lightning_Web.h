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
  <style>
    /* Chart.js and dark theme styles from ratechart.html */
    body {
      background: #1e1e1e;
      color: #d4d4d4;
      margin: 0;
      padding: 0;
      overflow-y: auto;
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
      margin: 16px auto;
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
        height: 40px;
        margin: 0;
        padding: 10px 0px;
        background-color: #FFF;
        color: #000000;
        border-bottom: 5px solid #293578;
    }

    .navtitle {
        font-family: "Copperplate", "Arial", sans-serif;
        font-size: 22px;
        font-weight: bold;
        line-height: 30px;
        padding-left: 7px;
        white-space: nowrap;
    }

    .navinfo {
        display: flex;
        align-items: center;
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
        font-size: 20px;
        position: relative;
        height: 30px;
        text-align: center;
        color: #0e0e0f;
        line-height: 20px;
    }

    .container {
        max-width: 800;
        margin: 0 auto;
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
    }

    .myDiv1 {
        border-radius: 3px;
        margin: auto;
        border: 3px solid rgb(84, 230, 71);
        background-color: rgba(27, 27, 27, 0.473);
        padding:2px;
    }

    .termta {
        border-radius: 3px;
        width:100%;
        max-width: 100%;
        max-height: 99%;
        background-color: rgba(48, 49, 48, 0.473);
        color: cyan;
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

    <!-- buttons start here -->
    <div class="volume-control-container">
    <input type="button" class="button" id="PWR_Select" name="PWR_Select" value="POWER"
        onclick="RadiosGrpButtonPress(this.id)">

    <input type="button" class="button" id="R2Select" name="Radio2" value="Test"
        onclick="playAlarm(600, 250, 'triangle')">
    <!-- <input type="button" class="button" id="R2Select" name="Radio2" value="Test"
        onclick="playStrikeAlarm()"> -->

    <!-- Volume Slider -->
        <label for="volume">Strike Alarm Volume:</label>
        <input type="range" id="volume" min="0" max=".1" step="0.002" value="0.02">
    </div>
    <hr>
    <!-- end of buttons/sliders -->
     
    <div class="grid-container">
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
    </div>

    <!-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% -->
    <hr>
    <div class="myDiv1">
        <textarea readonly class="termta" id="Terminal" name="Terminal" rows="20" cols="200"></textarea>
        <textarea class="termta" id="TermInput" name="TermInput" rows="1" cols="200">CMD> </textarea>
    </div>
    <!-- %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%% -->

    <!-- Chart container for real-time charting -->
    <div class="chart-container">
      <canvas id="myChart"></canvas>
    </div>

    <footer div class="foot" id="Chincey">THIS IS A TEST FOOTER</div>
    </footer>

<script type="text/javascript">
    // ============================================= WEB PAGE ABOVE =================================================
// *********************
// 
    let USE_SIMULATED_DATA = false; // Set to true to use simulated data for testing
//
// *********************
    // global variable visible to all java functions
    var xmlHttp = createXmlHttpObject();

    // helper function to create XML object, returns that created object
    function createXmlHttpObject() {
        if (window.XMLHttpRequest) {
            xmlHttp = new XMLHttpRequest();
        }
        else {
            xmlHttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        return xmlHttp;
    }
    // --------------------- handle the terminal input ------------------------------      
    document.getElementById('TermInput').addEventListener('keypress', function (event) {
        if (event.key === 'Enter') {
            event.preventDefault();

            // Get the command typed by the user
            const command = document.getElementById('TermInput').value;
            var xhttp = new XMLHttpRequest();
            xhttp.open("PUT", 'TermInput', true); // send name of radio button selected to server
            xhttp.send(command.substring(5)); // only send the command not the prompt

            document.getElementById('TermInput').value = 'CMD> ';  // leave a prompt behind
        }
    });
    // -----------------------------------------------------------      

    //  handle a selection made of the Radios button group for the 4 radios
    function RadiosGrpButtonPress(value) {
        console.log("Radio " + value);
        document.getElementById(value).style.backgroundColor = "#ffff00";
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", value, true); // send name of radio button selected to server
        xhttp.send();
    }

    function AllGndPress(value) {
        console.log("AllGndPress " + value);
        document.getElementById("ALL_GND").style.backgroundColor = "#ffff00";
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", value, true); // send name of ALL Grounded button selected to server
        xhttp.send();
    }

    // try to emulate an autoscrolling log using a text area
    function TextLog(value) {
        var dt = new Date();
        var ta = document.getElementById("Terminal");
        ta.value += "\n" + dt.toLocaleTimeString() + "> " + value;
        ta.scrollTop = ta.scrollHeight;
    }

    // function to handle the XML response FROM the ESP
    // let Strike_last_strike_accumulator_value = 0;  // used to determine when to play the strike alarm tone
    // let Strike_this_time = 0;
    function response() {
      if (typeof response.Strike_last_strike_accumulator_value === "undefined") {
        response.Strike_last_strike_accumulator_value = 0;
      }
      // Now use response.Strike_last_strike_accumulator_value like a C static variable
      
      let Strike_acc_this_time = 0;
      var message;
      var barwidth;
      var currentsensor;
      var xmlResponse;
      var dt = new Date();
      var color = "#e8e8e8";

      xmlResponse = xmlHttp.responseXML;
      if (xmlResponse == null) return;

      document.getElementById("date").innerHTML = dt.toLocaleDateString('en-GB', {
              day: '2-digit',
              month: 'short',
              year: 'numeric'
          }).replace(/ /g, '-').toUpperCase();
      document.getElementById("time").innerHTML = dt.toLocaleTimeString();

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
      console.log((new XMLSerializer()).serializeToString(xmlResponse));  // debug

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
      Strike_acc_this_time = Number(message);
      if( Strike_acc_this_time > response.Strike_last_strike_accumulator_value) {
        if( ! USE_SIMULATED_DATA ) {
  //        playStrikeAlarm();  // finally, play the strike alarm tone, multiple tones, had issue with this when lots of strikes
            playAlarm(400, 250, 'triangle') ; // play a single strike tone, 400hz for 250ms
            TextLog(">BEEP<"); // so i dont have to ddlisten to the alarm tone when testing
            response.Strike_last_strike_accumulator_value = Strike_acc_this_time; // save the last strike accumulator value to detect change
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
    }
    
    function getVolume() {
        return document.getElementById('volume').value;
    }
    
    function playStrikeAlarm() {
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
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

    function playAlarm(frequency, duration, type) {
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
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

    function process() {
        if (xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
            xmlHttp.open("GET", "xml", true);
            xmlHttp.onreadystatechange = response;
            xmlHttp.send(null);
        }
        //if(!USE_SIMULATED_DATA)
          setTimeout("process()", 1000); // Poll the server every 500ms
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
    var counter = 4;

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

    // Wait until the DOM is fully loaded before running this function.
    // This ensures that all HTML elements (like the canvas for the chart) are available.
    document.addEventListener("DOMContentLoaded", function() {
      // Get the 2D drawing context from the canvas element with id 'myChart'
      const ctx = document.getElementById('myChart').getContext('2d');

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
            cubicInterpolationMode: 'default',     // Smoother lines (can use 'monotone')
            tension: 0.25                          // Line tension for smoothness
          }))
        },
        options: {
          responsive: true,                        // Chart resizes with container
          maintainAspectRatio: false,              // Fill container height/width
          plugins: {
            legend: {
              labels: {
                color: '#d4d4d4',                 // Legend text color
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

      // Optionally, you could start simulated data updates here after the chart is created:
      if(USE_SIMULATED_DATA) {
        // Call the update function immediately to start the simulation
        updateChartSimulatedData();
        setInterval(updateChartSimulatedData, 1000);
      }
    }); // <-- closes DOMContentLoaded


    // --- REAL-TIME DATA SIMULATION AND SLIDING WINDOW LOGIC ---
    function updateChartSimulatedData() {
      if (!window.myChart) return; // If chart not initialized, exit
      // For each trace, add a new random data point at the start (newest on the left)
      for (let i = 0; i < dataValues.length; i++) {
        let newValue = Math.random() * 12; // Random float between 0 and 12
        // Only allow zero or positive floating point numbers
        if (isNaN(newValue) || newValue < 0) newValue = 0;
        dataValues[i].unshift(newValue);
        if (dataValues[i].length > 60) dataValues[i] = dataValues[i].slice(0, 60);
      }

      // Add new time label at the start (for autoscaling phase)
      xLabels.unshift(counter);
      if (xLabels.length > 60) xLabels = xLabels.slice(0, 60);
      counter++;

      if (xLabels.length < 60) {
        // If fewer than 60 points, let Chart.js autoscale the x-axis
        window.myChart.data.labels = xLabels; // Use the current labels array for the x-axis
        window.myChart.options.scales.x.min = 0; // Remove fixed min so Chart.js can autoscale
        window.myChart.options.scales.x.max = undefined; // Remove fixed max so Chart.js can autoscale
      } else {
        // Once we have 60 or more points, fix the x-axis range to always show 0-59
        window.myChart.data.labels = Array.from({length: 60}, (_, i) => i); // Set labels to 0-59
        window.myChart.options.scales.x.min = 0; // Fix the minimum x value
        window.myChart.options.scales.x.max = 59; // Fix the maximum x value
      }

      // --- AUTOSCALE Y AXIS AFTER INITIALIZATION ---
      if (xLabels.length > 1) {
        // Let Chart.js autoscale the y-axis based on the data
        window.myChart.options.scales.y.min = 0;  // Start y-axis at 0
        window.myChart.options.scales.y.max = undefined;
      }

      // Update each dataset with the latest data
      for (let i = 0; i < dataValues.length; i++) {
        window.myChart.data.datasets[i].data = dataValues[i]; // Update the chart's data for each trace
      }

      window.myChart.update('none'); // Redraw the chart with new data, none eliminates animation
      //window.myChart.update(); // Redraw the chart with new data, none eliminates animation

      // Optional: Auto-scroll to the start (left) for smooth effect
      const container = document.querySelector('.chart-container'); // Get the chart container element
      container.scrollLeft = 0; // Scroll to the leftmost position
    }

    // --- CHART UPDATE FUNCTION FROM XML DATA ---
    // Updates the chart with new data from the <RATES> XML tag.
    // Call this from your response() function after receiving new XML data.
    function updateChartFromRatesXML(xmlResponse) {
      if (!window.myChart) return; // If chart not initialized, exit

      // Extract the <RATES> tag from the XML response
      const ratesTag = xmlResponse.getElementsByTagName("RATES");
      if (!ratesTag || ratesTag.length === 0) return; // If no <RATES> tag found, exit

      // Get the comma-separated string of rates from the tag
      const ratesString = ratesTag[0].textContent || ratesTag[0].firstChild.nodeValue;
      if (!ratesString) return; // If the tag is empty, exit

      // Split the string into an array of floats and sanitize
      const rateValues = ratesString.split(',').map(val => {
        let f = parseFloat(val);
        return (isNaN(f) || f < 0) ? 0 : f;
      });

      // For each trace (Strike, Disturber, Noise, Purge):
      for (let i = 0; i < dataValues.length; i++) {
        dataValues[i].unshift(rateValues[i] || 0);
        if (dataValues[i].length > 60) dataValues[i].pop();
        window.myChart.data.datasets[i].data = dataValues[i];
      }

      // Add a new label to the start of the xLabels array (using a counter)
      xLabels.unshift(counter++);
      
      // Keep only the latest 60 labels
      if (xLabels.length > 60) xLabels.pop();
      
      // Update the chart's x-axis labels
      window.myChart.data.labels = xLabels;

      // Optionally autoscale the x-axis: 
      // If fewer than 60 points, let Chart.js autoscale; otherwise, fix the range to 0-59
      if (xLabels.length < 60) {
        window.myChart.options.scales.x.min = 0;
        window.myChart.options.scales.x.max = undefined;
      } else {
        window.myChart.options.scales.x.min = 0;
        window.myChart.options.scales.x.max = 59;
      }

      // Optionally autoscale the y-axis after initialization
      if (xLabels.length > 1) {
        window.myChart.options.scales.y.min = 0; // Start y-axis at 0
        window.myChart.options.scales.y.max = undefined;
      }

      // Redraw the chart with the new data
      window.myChart.update('none'); // 'none' eliminates animation for smoother updates
      
      // Optional: Auto-scroll to the start (left) for smooth effect
      const container = document.querySelector('.chart-container');
      container.scrollLeft = 0;
    }
</script>
</html>
)=====";