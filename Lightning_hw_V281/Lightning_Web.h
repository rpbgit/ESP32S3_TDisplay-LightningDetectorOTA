/*
  OK, ya ready for some fun? HTML + CSS styling + javascript all in and undebuggable environment

  one trick I've learned to how to debug HTML and CSS code.

  get all your HTML code (from html to /html) and past it into this test site
  muck with the HTML and CSS code until it's what you want
  https://www.w3schools.com/html/tryit.asp?filename=tryhtml_intro

  No clue how to debug javascrip other that write, compile, upload, refresh, guess, repeat

  I'm using class designators to set styles and id's for data updating
  for example:
  the CSS class .tabledata defines with the cell will look like
  <td><div class="tabledata" id = "switch"></div></td>

  the XML code will update the data where id = "switch"
  java script then uses getElementById
  document.getElementById("switch").innerHTML="Switch is OFF";


  .. now you can have the class define the look AND the class update the content, but you will then need
  a class for every data field that must be updated, here's what that will look like
  <td><div class="switch"></div></td>

  the XML code will update the data where class = "switch"
  java script then uses getElementsByClassName
  document.getElementsByClassName("switch")[0].style.color=text_color;


  the main general sections of a web page are the following and used here

  <html>
    <style>
    // dump CSS style stuff in here
    </style>
    <body>
      <header>
      // put header code for cute banners here
      </header>
      <main>
      // the buld of your web page contents
      </main>
      <footer>
      // put cute footer (c) 2021 xyz inc type thing
      </footer>
    </body>
    <script>
    // you java code between these tags
    </script>
  </html>


*/

// note R"KEYWORD( html page code )KEYWORD"; 
// again I hate strings, so char is it and this method let's us write naturally

const char PAGE_MAIN[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en" class="js-focus-visible">
<meta name="viewport" content="width=device-width, initial-scale=1.0">

<title>Lightning Detector v1.8</title>

<style>
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
        white-space: nowrap; /* Prevent text from wrapping */
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
        white-space: nowrap; /* Prevent text from wrapping */
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
        /* display: inline-block;
        background-color: #f5cba4d7;
        width: 20px;
        color: #000000;
        text-align: center; */
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
        /* width: 50px; 
      height: 20px; */
    }

    .myDiv1 {
        border-radius: 3px;
        max-width: 1100px;
        margin: auto;
        border: 3px solid rgb(84, 230, 71);
        background-color: rgba(27, 27, 27, 0.473);
        /* text-align: center; */
        /* padding: 3px 5px; */
    }

    .termta {
        border-radius: 3px;
        /* max-width: 800px; */
        max-width: 99%;
        max-height: 99%;
        margin: auto;
        background-color: rgba(48, 49, 48, 0.473);
        color: cyan;
        box-sizing: border-box;
    }

    .grid-container {
        display: grid;
        grid-template-columns: repeat(4, auto);
        /* grid-template-columns: repeat(4, 1fr); */
        /* 4 columns */
        grid-template-rows: repeat(2, auto);
        /* 2 rows */
        gap: 10px;
        /* space between elements */
        align-items: center;
        /* vertically align labels and textareas */
        max-width: 800px;

        margin: auto; /* center grid added 9-feb-2025 rpb */
    }

    label {
        text-align: right;
        padding-right: 10px;
        /* font-weight: bold; */
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
        /* Optional: space between label and input */
    }
</style>

<body style="background-color: #7a8285" onload="OnMyFormLoad()">

    <header>
        <div class="navbar fixed-top">
            <div class="container">
                <div class="navtitle" id="headerTitle">ESP32 Lightning Detector v1.8</div>
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
    <footer div class="foot" id="Chincey">WA9FVP Beta Test</div>
    </footer>

</body>


<script type="text/javascript">
    // ============================================= WEB PAGE ABOVE =================================================

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
            /*         
                    // Display the command in the terminal and clear input
                    const terminal = document.getElementById('Terminal');
                    terminal.value += '\nlocalcmd> ' + command;
             */
            var xhttp = new XMLHttpRequest();
            xhttp.open("PUT", 'TermInput', true); // send name of radio button selected to server
            xhttp.send(command.substring(5)); // only send the command not the prompt

            document.getElementById('TermInput').value = 'CMD> ';  // leave a prompt behind
        }
    });
    // -----------------------------------------------------------      

    //  handle a selection made of the Radios button group for the 4 radios
    function RadiosGrpButtonPress(value) {
        // passing in the value of the object clicked/selected is one way... but...
        console.log("Radio " + value);
        //TextLog(value);

        // turn the button yellow indicating a request was made of the server.
        // the next hardware update should change this appropriately
        document.getElementById(value).style.backgroundColor = "#ffff00";

        // another is to just query which one has been checked in the radio group
        //console.log(">>>Selected value for RadiosGroup: " + document.querySelector("input[name=RadioGrp]:checked").value); // rpb
        //document.getElementById('Terminal').value = value;
        /*
        xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("button1").innerHTML = this.responseText;
          }
        }
        */
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", value, true); // send name of radio button selected to server
        xhttp.send();
    }

    // handler for a selection made of one of the two buttons in the Aux1 button group
    function Aux1GrpButtonPress(value) {
        console.log("Aux1 " + value);
        //TextLog(value);

        // turn the button yellow indicating a request was made of the server.
        // the next hardware update should change this appropriately
        document.getElementById(value).style.backgroundColor = "#ffff00";
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", value, true); // send name of Aux1 button selected to server
        xhttp.send();
    }

    // handler for a selection made of one of the two buttons in the Aux2 button group
    function Aux2GrpButtonPress(value) {
        console.log("Aux2 " + value);
        //TextLog(value);

        // turn the button yellow indicating a request was made of the server.
        // the next hardware update should change this appropriately
        document.getElementById(value).style.backgroundColor = "#ffff00";
        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", value, true); // send name of Aux2 button selected to server
        xhttp.send();
    }

    function AllGndPress(value) {
        console.log("AllGndPress " + value);
        //TextLog(value);
        document.getElementById("ALL_GND").style.backgroundColor = "#ffff00";

        var xhttp = new XMLHttpRequest();
        xhttp.open("PUT", value, true); // send name of ALL Grounded button selected to server
        xhttp.send();
    }

    // try to emulate an autoscrolling log using a text area
    // call this function if you want to post something to the textarea on the webpage
    function TextLog(value) {
        var dt = new Date();
        var ta = document.getElementById("Terminal");
        //ta.value += dt.toLocaleTimeString() + "> " + value + "\n";
        ta.value += dt.toLocaleTimeString() + "> " + value;

        ta.scrollTop = ta.scrollHeight;
    }

    // function to handle the response FROM the ESP
    var Strike_last_strike_accumulator_value = 0;
    var Strike_this_time = 0;
    function response() {
        var message;
        var barwidth;
        var currentsensor;
        var xmlResponse;
        var xmldoc;
        var dt = new Date();
        var color = "#e8e8e8";

        // get the xml stream
        xmlResponse = xmlHttp.responseXML;

        // TODO:  WHY so many null responses ? 
        if (xmlResponse == null) return;

        // get host date and time
        // document.getElementById("date").innerHTML = dt.toLocaleDateString('en-GB', {
        //     day: '2-digit',
        //     month: 'short',
        //     year: 'numeric'
        //     }).toUpperCase();
        document.getElementById("date").innerHTML = dt.toLocaleDateString('en-GB', {
                day: '2-digit',
                month: 'short',
                year: 'numeric'
            }).replace(/ /g, '-').toUpperCase();
        document.getElementById("time").innerHTML = dt.toLocaleTimeString();

        // handle the case where INFO tag is null/undefined, its the only tag which may be so
        xmldoc = xmlResponse.getElementsByTagName("INFO");
        try {
            message = xmldoc[0].firstChild.nodeValue;
            TextLog("ESP32: " + message);
            console.log(message);
        } catch (e) {
            ;//console.log("Catch on reading INFO");
        }

        console.log((new XMLSerializer()).serializeToString(xmlResponse));  // debug, remove if desired

        // for now, hijack the R1 button for testing... 
        xmldoc = xmlResponse.getElementsByTagName("PWR");
        message = xmldoc[0].firstChild.nodeValue;
        message == "1" ? color = "#00ff00" : color = "#ff0000";  // either selected green (on), or red (off)
        document.getElementById("PWR_Select").style.backgroundColor = color;

        //ff.
        xmldoc = xmlResponse.getElementsByTagName("NOISE_ACC");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("NoiseAcc").value = message;
        //console.log(message);

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
        Strike_this_time = Number(message);
        if( Strike_this_time > Strike_last_strike_accumulator_value) {
//            playStrikeAlarm();  // finally, play the strike alarm tone
            playAlarm(400, 250, 'triangle') ;

            Strike_last_strike_accumulator_value++;
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

        // get the version and put it in the header
        xmldoc = xmlResponse.getElementsByTagName("VER");
        message = xmldoc[0].firstChild.nodeValue;
        document.getElementById("headerTitle").textContent = "ESP32 Storm detector " + message ;

        //console.log(message);
        /*
       
          xmldoc = xmlResponse.getElementsByTagName("ALL_GND"); 
          message = xmldoc[0].firstChild.nodeValue;
          message == "1" ? color="#ffcce6" : color = "#ffffff";  // either selected green, or white
          document.getElementById("ALL_GND").style.backgroundColor=color; 
          //console.log(message);
    
          xmldoc = xmlResponse.getElementsByTagName("Xmit_Ind"); 
          message = xmldoc[0].firstChild.nodeValue;
          message == "1" ? color="#ff0000" : color = "#ffffff";  // either selected red, or white
          document.getElementById("Xmit_Ind").style.fill=color; 
          //console.log(message);
         */
    }
    
    // Get the volume value from the slider
    function getVolume() {
        return document.getElementById('volume').value;
    }
    
    function playStrikeAlarm() {
        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
        const oscillator = audioContext.createOscillator();
        const gainNode = audioContext.createGain();
TextLog("playStrikeAlarm() called...\n");
        oscillator.stop();
        oscillator.type = 'triangle'; // Triangle wave for a softer tone
        oscillator.frequency.setValueAtTime(400, audioContext.currentTime); // 400 Hz tone

        gainNode.gain.value = getVolume(); // Set volume
        //gainNode.gain.value = .02; // override the above for  now...

        oscillator.connect(gainNode);
        gainNode.connect(audioContext.destination);

        // Start the pulsating alarm
        oscillator.start();

        // Create a pulse effect by periodically changing the gain
        let pulsing = true;
        const interval = setInterval(() => {
            gainNode.gain.setValueAtTime(pulsing ? 0 : getVolume(), audioContext.currentTime); // Toggle between full volume and silence
            //gainNode.gain.setValueAtTime(pulsing ? 0 : .02, audioContext.currentTime); // Toggle between full volume and silence
            pulsing = !pulsing;
        }, 250); // Change every 250ms

        // Stop the sound and pulsing after 5 seconds
        setTimeout(() => {
            clearInterval(interval);
            oscillator.stop();
        }, 2000);
    }

    //generic Function to create an alarm-like sound with a specific waveform, frequency, and duration
    function playAlarm(frequency, duration, type) {

//const randomNumber = Math.floor(Math.random() * 100);

//document.getElementById("headerTitle").textContent = "ESP32 Storm detector " + randomNumber.toString();
//TextLog("random num is " + randomNumber + "\n");
//TextLog( "document.getElementById(\"headerTitle\").textContent = " + document.getElementById("headerTitle").textContent + "\n");

        const audioContext = new (window.AudioContext || window.webkitAudioContext)();
        const oscillator = audioContext.createOscillator();
        const gainNode = audioContext.createGain();
//TextLog("playAlarm( " + frequency  + ", " + duration + ", " + type + ' ) called...\n');
        // Set the oscillator type (waveform)
        oscillator.type = type;
        // Set the frequency of the alarm
        oscillator.frequency.setValueAtTime(frequency, audioContext.currentTime);
        // Set the volume from the slider
        gainNode.gain.value = getVolume();

        // Connect oscillator -> gainNode -> destination (speakers)
        oscillator.connect(gainNode);
        gainNode.connect(audioContext.destination);

        // Start the alarm sound
        oscillator.start();
        // Stop the sound after the duration
        oscillator.stop(audioContext.currentTime + duration / 1000);
    }

    // sound (Sawtooth wave, low pitch sweep)
    // function playAlarm2() {
    //     const audioContext = new (window.AudioContext || window.webkitAudioContext)();
    //     const oscillator = audioContext.createOscillator();
    //     const gainNode = audioContext.createGain();

    //     // Set the oscillator type to sawtooth for a more aggressive sound
    //     oscillator.type = 'sawtooth';
    //     oscillator.frequency.setValueAtTime(200, audioContext.currentTime); // Start at 200 Hz

    //     // Set frequency sweep to simulate a siren
    //     oscillator.frequency.exponentialRampToValueAtTime(800, audioContext.currentTime + 2); // Up to 800 Hz in 2 seconds

    //     gainNode.gain.value = getVolume(); // Set volume

    //     oscillator.connect(gainNode);
    //     gainNode.connect(audioContext.destination);

    //     // Start the sound
    //     oscillator.start();
    //     // Stop the sound after 2 seconds
    //     oscillator.stop(audioContext.currentTime + 2);
    // }

    function OnMyFormLoad() {
        const textarea = document.getElementById("TermInput");
        const position = 5; // Set cursor to the 5th character
        
        textarea.focus(); // Focus the textarea on page load
        textarea.selectionStart = position;
        textarea.selectionEnd = position;
        process();
    };
    // general processing code for the web page to ask for an XML steam
    // this is actually why you need to keep sending data to the page to 
    // force this call with stuff like this
    // server.on("/xml", SendXML);
    // otherwise the page will not request XML from the ESP, and updates will not happen
    function process() {
        // var xhttp = new XMLHttpRequest();
        // if(xhttp.readyState == 0 || xhttp.readyState == 4) {
        //     xhttp.open("GET","xml",true); 
        //     xhttp.onreadystatechange = response;
        //     xhttp.send(null);
        // }

        if (xmlHttp.readyState == 0 || xmlHttp.readyState == 4) {
            xmlHttp.open("GET", "xml", true);
            xmlHttp.onreadystatechange = response;
            xmlHttp.send(null);
        }
        // you may have to play with this value, big pages need more porcessing time, and hence
        // a longer timeout
        setTimeout("process()", 500); // this defines the polling interval/update freq in ms
    }
</script>

</html>


)=====";