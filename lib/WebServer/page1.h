const char htmlPage1[] PROGMEM = 
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
    <img src=" data:image/bmp;base64,Qk0+BAAAAAAAAD4AAAAoAAAAfAAAAEAAAAABAAE
    AAAAAAAAEAADDDgAAww4AAAAAAAAAAAAAAAAAAP///wD////////////////////w///////
    /////////////8P////////////////////D////////////////////w///////////////
    /////8P///8f///////////////D////P///////////////w////3///////////////8P/
    //9////+AAAAAAAAAAfD//P/P///////////////w//z/x////4H/D8/E+Z/D8P/4P4f/+f+
    c/mfPyPyfmfD/+B+H//H/nn5/z8z+P5/w//gfg//D/88+B+fmfz+B8P/4H4P/B//PPmfn5n4
    fmfD/+ceD/B//zz8n5+Z+T8nw//wBh/wf/88/j+fmfOfj8P/4AQf8P//PP//n//////D/8AE
    H/D//5n//8//////w//APB/w//+D///P/////8P/w4A/w//f///////////D/8QAP8P/j///
    ////////w//oAD+D/B///////////8P/8AA/h/g////////////D/+AGP4/4////////////
    w//gCA4f8P///////////8P/4BAAHxD////////////D//BAABwA////////////w//yACgY
    AA///////////8P4AAAhOAAf///////////DwEAAABBif///////////w4MQAAAAfP//////
    /////8OAOgAAAIB////////////DGJgAAAAAf///////////wz3KAAAAAP///////////8MB
    44AAABz//////+HDhw/DAAH4AAB8B//////MmTJnw4HwAAAA/AH//////JkyZ8PAB4AAAfjg
    fv////yZMmfD/gAAAAH4/gD////8mTJnw//+AAAB+H+B////8ZkyZ8P///4AA/B///////yZ
    MmfD///wAAOBf//////8mTJnw///w8AAAT//////zJkyZ8P//w4QAAM//////+HDhw/D//4A
    cgIXP///////////w//8GeQf/z///////////8P/+JzIP/8////////////D//GeGD//n///
    ////////w//zADB//5///////////8P/4AAkf/+H///////////D/+GcAP//yf//////////
    w//nmQH///4MEE2TmHnBOcP/5gMD///+ZPPIk5M5zzPD/+BjB////mTzyJAfOc8nw//g4Q//
    //5k88qZPDnPB8P/88wf///+DBBCGTD5wTPD//HMP////mTzwhkz+c8zw//4AP////5k88cc
    cznPM8P//gf////+DBBHHHhgQQfD////////////////////w////////////////////8P/
    ///////////////////D////////////////////w" width="320" height="160">

    <p><a href="/%s"> Click here to download the latest data file: %s</a></p>
    <table>
      <tr>
        <th>File Name</th>
        <th>Size (Bytes)</th>
        <th>Date Created</th>
        <th>Last Modified</th>
      </tr>
      %s
    </table>
    <h3><a href="/GetLiveData"> Click here to see live data!</a></h3>
    <p>number of page loads: %i</p>
  </body>  
</html>
)=====";
// <h1>Beemaster 3000 HOME</h1>
    // img 
    // {
    //   width:  200%;
    //   height: 100%;
    // }
