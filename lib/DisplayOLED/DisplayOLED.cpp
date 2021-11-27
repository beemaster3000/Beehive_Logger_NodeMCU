
#include "DisplayOLED.hpp" 
#include "welcomeScreen.h" 


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
 
DisplayOLED::DisplayOLED()
{
    u8g2_ = U8G2_SSD1306_128X64_NONAME_1_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
}

DisplayOLED::~DisplayOLED()
{

}

void DisplayOLED::setup()
{   
    u8g2_.begin();
    // u8g2_.setFont(u8g2_font_helvR12_tr); // helvetica
    u8g2_.setFont(u8g2_font_helvB12_tr); // bold helvetica **
    // u8g2_.setFont(u8g2_font_crox3c_tr);  // add b after h or c for bold 
    // u8g2_.setFont(u8g2_font_crox3h_tr);  // add b after h or c for bold %
    // u8g2_.setFont(u8g2_font_crox1h_tr);  // add b after h or c for bold
    // u8g2_.setFont(u8g2_font_10x20_tr);
    // u8g2_.setFont(u8g2_font_helvR08_tr); // helvetica 8
}

void DisplayOLED::clear()
{   
    u8g2_.clear();
}


void DisplayOLED::drawWelcomeScreen()
{   
    u8g2_.firstPage(); 
    do 
    {
       u8g2_.drawXBMP( 0, 0,SCREEN_WIDTH, SCREEN_HEIGHT, welcomeScreen);
    } while( u8g2_.nextPage() );
}

void DisplayOLED::printTRHvalues(int short sensorNumber, float t, float h)
{     
    u8g2_.firstPage();
    do 
    {
        u8g2_.setCursor(0,15);
        u8g2_.print(F("Sensor : "));
        u8g2_.print(sensorNumber);
        u8g2_.setCursor(0,40);
        u8g2_.print(F("Temp: "));
        u8g2_.setCursor(65,40);
        u8g2_.print(t,1);
        u8g2_.print(F(" C"));
        u8g2_.setCursor(0,60);
        u8g2_.print(F("RH:"));
        u8g2_.setCursor(65,60);
        u8g2_.print(h,1);
        u8g2_.println(F(" %"));
    } while ( u8g2_.nextPage() );
}

void DisplayOLED::printMainPage(char filename[13],float battVolt,char dateTime[20])
{     
    u8g2_.firstPage();
    do 
    {
        u8g2_.setCursor(0,15);
        u8g2_.print(dateTime);
        u8g2_.setCursor(0,38);
        u8g2_.print(F("File: "));
        u8g2_.print(filename);
        u8g2_.setCursor(0,62);
        u8g2_.print(F("Batt Volt: "));
        u8g2_.print(battVolt,1);
        u8g2_.println(F(" V"));
    } while ( u8g2_.nextPage() );
}

void DisplayOLED::println(const char *textIn,int lineNumber)
{
    int lineht;  
    lineht = 12;
    u8g2_.setCursor(0,lineht*lineNumber);
    u8g2_.print(textIn);
    u8g2_.updateDisplay();
} 