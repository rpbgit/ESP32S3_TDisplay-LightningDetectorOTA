
// apparently, this file middleware will not run unless fiveserver is kicked off manually not from the fs 
// vscode extension.   so until i do this, this file is not used, but is kept for future possible use.

const fs = require('fs');
module.exports = {
  middleware: [
    // Prevent favicon.ico fetch
    function (req, res, next) {
      if (req.url === '/favicon.ico') {
        res.writeHead(204); // No Content
        res.end();
      } else {
        next();
      }
    },
    function (req, res, next) {
      fs.appendFileSync('server.log', `Request: ${req.url}\n`);
      if (req.url === '/xml') { // <-- FIXED HERE
        res.setHeader('Content-Type', 'application/xml');
        res.end(`
          <root>
            <INFO>Test info from five-server middleware</INFO>
            <PWR>1</PWR>
            <NOISE_ACC>12</NOISE_ACC>
            <NOISE_ET>34</NOISE_ET>
            <DISTURB_ACC>56</DISTURB_ACC>
            <DISTURB_ET>78</DISTURB_ET>
            <STRIKE_ACC>90</STRIKE_ACC>
            <STRIKE_ET>12</STRIKE_ET>
            <STRIKE_DIST>3</STRIKE_DIST>
            <STRIKE_ENER>7</STRIKE_ENER>
            <VER>1.0.0</VER>
            <RATES>1,2,3,4</RATES>
          </root>
        `);
      } else {
        next();
      }
    }
  ]
};