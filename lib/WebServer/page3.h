const char htmlPage3[] PROGMEM =
R"=====(
<html> 
  <head>
    <meta http-equiv="refresh" content="60">
  </head>
  <body>
  <h1>Live Sensor Data</h1>
  <p> Last Update: %s</p>
    <table>
      <tr>
        <td>Sensor Number &emsp;</td>
        <td>Temperature (deg C)&emsp;</td>
        <td>Relative Humidity (%)&emsp;</td>
      </tr>
      %s
    </table>
    <h3>Click <a href="/">here</a> to return to main page</h3>
  </body>  
</html>
)=====";
