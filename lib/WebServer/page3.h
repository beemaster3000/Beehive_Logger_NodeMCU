const char htmlPage3[] PROGMEM =
R"=====(
<html> 
  <head>   
  </head>  
  <body>    
    <h1>Sensor Live Data</h1>
    <table>
      <tr>
        <td>Sensor Number &emsp;&emsp;</td>
        <td>Temperature [deg C] &emsp;&emsp;</td>
        <td>Relative Humidity</td>
      </tr>
      %s
    </table>
    <h2>Click <a href="/">here</a> to return to main page</h2>
  </body>  
</html>
)=====";



// <table>
//   <tr>
//     <td>Sensor Number &emsp;</td>
//     <td>Temperature (deg C)&emsp;</td>
//     <td>Relative Humidity (%%)&emsp;</td>
//   </tr>
//   %s
// </table>
// <td>Relative Humidity [%%]</td>


// <p> Last Update: %s</p>
// <meta http-equiv="refresh" content="60">
