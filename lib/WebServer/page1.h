const char htmlPage1[] PROGMEM = 
R"=====(
<html> 
  <head>   
  </head>  
  <body>    
    <h1>Beemaster 3000 HOME</h1>
    <p><a href="/%s"> Click here to download the latest data file</a></p>
    <table>
      <tr>
        <td>File Name &emsp;</td>
        <td>Size (Bytes) &emsp;</td>
        <td>Date Created &emsp;&emsp;&emsp;&emsp;</td>
        <td>Last Modified</td>
      </tr>
      %s
    </table>
    <h3><a href="/GetLiveData"> Click here to see live data!</a></h3>
    <p>number of page loads: %i</p>
  </body>  
</html>
)=====";
