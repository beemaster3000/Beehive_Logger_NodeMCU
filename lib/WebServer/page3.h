const char htmlPage3[] PROGMEM =
R"=====(
<html> 
  <head>   
  </head>
  <style>
  table, th, td 
  {
    border-collapse: collapse;
    padding-top: 5px;
    padding-bottom: 5px;
    padding-left: 10px;
    padding-right: 10px;
    text-align: center
  }
  tr:nth-child(even) 
  {
    background-color: #dcdcdc;
  }
  </style>
  <body>    
    <h1>Sensor Live Data</h1>
    <table>
      <tr>
        <th>Sensor Number</th>
        <th>Temperature [deg C]</th>
        <th>Relative Humidity [%%]</th>
      </tr>
      %s
    </table>
    <h2>Click <a href="/">here</a> to return to main page</h2>
  </body>  
</html>
)=====";


// <p> Last Update: %s</p>
// <meta http-equiv="refresh" content="60">
