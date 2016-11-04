/* Sources:
 * Math for radial plotting grabbed from the Generic_RTC_Clock example at this link: http://www.instructables.com/id/DS3231-OLED-clock-with-2-button-menu-setting-and-t/step4/null/
 * My screen is a TFT LCD shield with resistive touch, found at this link: http://www.amazon.com/gp/product/B00UAA2XIC?psc=1&redirect=true&ref_=oh_aui_detailpage_o02_s00
 * Using the mcufriend_kbv library with Arduino IDE 1.6.5, found at this link: https://forum.arduino.cc/index.php?topic=366304.msg2524865#msg2524865
 * Battery read example from:
 * http://www.instructables.com/id/Arduino-Battery-Voltage-Indicator/step2/Arduino-Sketch/
 * http://www.instructables.com/id/Build-a-Lipo-Battery-Gauge/step11/Software-Program/
 * 
 * Summary:
 * Declare pins and global variables.
 * Check status of components.
 * Bootup readout.
 * Build dial labels.
 * Set up component math.
 * The main display loop uses a layout with a top section for "general info", and a bottom section for "selected detail info".
 * Each data type uses an old/new variable comparison to determine when to update.
 * This way the whole screen doesn't update all at once all the time, and there is "time space" for user touch input.
 * When user touches a general dial, it will be marked, and its current numerical value will display on the detail section.
 * UPDATE: Compared to myFoxnovo_project6, this has the summary dials in a row instead of in a square grid. Also changes the dial select indicator from a carat to an underline.
 * 
 */

#include <Adafruit_GFX.h>    // Core graphics library
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#include <TouchScreen.h>
#include <Wire.h>
#include <DS3231.h>
#include <DHT.h>

#if defined(__SAM3X8E__)
#undef __FlashStringHelper::F(string_literal)
#define F(string_literal) string_literal
#endif

// most mcufriend shields use these pins and Portrait mode:
uint8_t YP = A3;  // must be an analog pin, use "An" notation!
uint8_t XM = A2;  // must be an analog pin, use "An" notation!
uint8_t YM = 9;   // can be a digital pin
uint8_t XP = 8;   // can be a digital pin
uint8_t Landscape = 0;

//touch calibration
uint16_t TS_LEFT = 922;
uint16_t TS_RT = 145;
uint16_t TS_TOP = 873;
uint16_t TS_BOT = 193;
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//Useful Variables
int16_t BOXSIZE = tft.width() / 10;
int16_t screenMode = 0;
int16_t tileDial = tft.width()*0.07;
//the dials use these as center points
  int16_t tile1X = tft.width()*0.2;
int16_t tile1Y = tileDial*2;
//  int16_t tile1X = tft.width()*0.33;
//int16_t tile1Y = BOXSIZE*2;
int16_t tile2X = tft.width()*0.4;
int16_t tile2Y = tileDial*2;
//int16_t tile2X = tft.width()*0.66;
//int16_t tile2Y = BOXSIZE*2;
int16_t tile3X = tft.width()*0.6;
int16_t tile3Y = tileDial*2;
//int16_t tile3X = tft.width()*0.33;
//int16_t tile3Y = BOXSIZE*5;
int16_t tile4X = tft.width()*0.8;
int16_t tile4Y = tileDial*2;
//int16_t tile4X = tft.width()*0.66;
//int16_t tile4Y = BOXSIZE*5;
int16_t tdLabel = 30;
int16_t detLnX = BOXSIZE;
int16_t detLn1Y = BOXSIZE*8;
int16_t detLn2Y = BOXSIZE*10;

//RTC
DS3231 clock;
RTCDateTime dt;

//DHT
#define DHTPIN 48
#define DHTTYPE DHT11 // DHT 11
DHT dht(DHTPIN, DHTTYPE);

const int mtrPin = A9;

//Timing
unsigned long prvTRtc = 60001;
unsigned long prvTT = 50001;
unsigned long prvTH = 40001;
unsigned long prvTBtry = 30001;
long intvRtc = 60000;
long intvT = 50000;
long intvH = 40000;
long intvBtry = 30000;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

tft.reset();

//Check: Is the screen working?
    uint16_t identifier = tft.readID();
//    if (identifier == 0) identifier = 0x9341;
    if (0) {
    } else if (identifier == 0x0154) {
        Serial.println(F("Found S6D0154 LCD driver"));
        TS_LEFT = 914; TS_RT = 181; TS_TOP = 957; TS_BOT = 208;
    } else if (identifier == 0x7783) {
        Serial.println(F("Found ST7783 LCD driver"));
        TS_LEFT = 865; TS_RT = 155; TS_TOP = 942; TS_BOT = 153;
        Landscape = 1;
    } else if (identifier == 0x9320) {
        Serial.println(F("Found ILI9320 LCD driver"));
        YP = A3; XM = A2; YM = 9; XP = 8;
        TS_LEFT = 902; TS_RT = 137; TS_TOP = 941; TS_BOT = 134;
    } else if (identifier == 0x9325) {
        Serial.println(F("Found ILI9325 LCD driver"));
        TS_LEFT = 900; TS_RT = 103; TS_TOP = 96; TS_BOT = 904;
    } else if (identifier == 0x9341) {
        Serial.println(F("Found ILI9341 LCD driver BLUE"));
        TS_LEFT = 920; TS_RT = 139; TS_TOP = 944; TS_BOT = 150;
        Landscape = 0;
    } else if (identifier == 0) {
        Serial.println(F("Found ILI9341 LCD driver DealExtreme"));
        TS_LEFT = 893; TS_RT = 145; TS_TOP = 930; TS_BOT = 135;
        Landscape = 1;
    } else if (identifier == 0 || identifier == 0x9341) {
        Serial.println(F("Found ILI9341 LCD driver RED"));
        TS_LEFT = 128; TS_RT = 911; TS_TOP = 105; TS_BOT = 908;
        Landscape = 1;
    } else if (identifier == 0x9488) {
        Serial.println(F("Found ILI9488 LCD driver"));
        TS_LEFT = 904; TS_RT = 170; TS_TOP = 950; TS_BOT = 158;
    } else if (identifier == 0xB509) {
        Serial.println(F("Found R61509V LCD driver"));
        TS_LEFT = 889; TS_RT = 149; TS_TOP = 106; TS_BOT = 975;
        Landscape = 1;
    } else {
        Serial.print(F("Unknown LCD driver chip: "));
        Serial.println(identifier, HEX);
        return;
    }

    //To control LCD
    ts = TouchScreen(XP, YP, XM, YM, 300);     //call the constructor AGAIN with new values.
    tft.begin(identifier);

    #define MINPRESSURE 10
#define MAXPRESSURE 1000

//RTC setup
  // Initialize DS3231
  clock.begin();
  
  // Set sketch compiling time, then COMMENT OUT and UPLOAD
  //clock.setDateTime(__DATE__, __TIME__);
  dht.begin();

pinMode(mtrPin,OUTPUT);
//analogWrite(mtrPin,0);
digitalWrite(mtrPin,LOW);
  
//startup display
tft.fillScreen(BLACK);
tft.setTextSize(1);
tft.setTextColor(WHITE);
tft.setCursor(0,BOXSIZE);
if(tft.readID())
{
tft.println("TOUCH SCREEN READY");
tft.print("ID: ");
tft.println(identifier);
}
//else
//{
//tft.println("TOUCH SCREEN SICK");
//}
tft.println("...........................");
tft.println("");
if(clock.begin())
{
tft.println("REAL TIME CLOCK READY");
}
else
{
tft.println("REAL TIME CLOCK SICK");
}
//I can't do a "DHT READY" message because dht.begin() can't be used as a boolean. shrug.
tft.println("...........................");
tft.println("");
tft.println("CURRENT SKETCH: myFoxnovo_project7");
tft.println("...........................");
tft.println("");
tft.println("WHEN TOUCH, PLEASE KEEP");
tft.println("TOUCH UNTIL RESPONSE!");
delay(5000);
  tft.fillScreen(BLACK);
  
  //DIAL LABELS
tft.setCursor((tile1X-tileDial),(tile1Y+tdLabel));
tft.print("time");
tft.setCursor((tile2X-tileDial),(tile2Y+tdLabel));
tft.print("temp");
tft.setCursor((tile3X-tileDial),(tile3Y+tdLabel));
tft.print("humd");
tft.setCursor((tile4X-tileDial),(tile4Y+tdLabel));
tft.print("power");

//Time RTC circle
tft.drawCircle(tile1X,tile1Y,tileDial,WHITE);
    //Position and draw hour tick marks
    //HINT: 360/30 = 12, so divide 360 by X to determine number of ticks
    for( int z=0; z < 360;z= z + 30 ){
    //Begin at 0° and stop at 360°
      float angle = z ;
      angle=(angle/57.29577951) ; //Convert degrees to radians
      //#o stands for 'outer', #i stands for 'inner'
      int xo=(tile1X+(sin(angle)*(tileDial+2))); 
      //in each of these lines, use (angle)*## to tweak positions of these shapes
      int yo=(tile1Y-(cos(angle)*(tileDial+2)));
      int xi=(tile1X+(sin(angle)*(tileDial)));
      int yi=(tile1Y-(cos(angle)*(tileDial)));
      tft.drawLine(xo,yo,xi,yi,WHITE);
    }

//Dials 2 thru 4
     //Temp
     //Circle
          tft.drawCircle(tile2X,tile2Y,tileDial,WHITE);
               //Minmax tick
     tft.drawLine(tile2X,tile2Y-(tileDial+1),tile2X,tile2Y-(tileDial+3),WHITE);

          //Humidity
              //Circle
         tft.drawCircle(tile3X,tile3Y,tileDial,WHITE);
              //Minmax tick
     tft.drawLine(tile3X,tile3Y-(tileDial+1),tile3X,tile3Y-(tileDial+3),WHITE);

//Battery
//Circle
tft.drawCircle(tile4X,tile4Y,tileDial,WHITE);
      //Minmax tick
     tft.drawLine(tile4X,tile4Y-(tileDial+1),tile4X,tile4Y-(tileDial+3),WHITE);
}

void loop() {
  // put your main code here, to run repeatedly:

//Math prep
  //RTC info get
  dt = clock.getDateTime();

//DHT info
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

    // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

/* To display DHT readings, use following characters:
 *  h for humidity
 *  t for temp in C
 *  f for temp in F
 *  hic for heat index in C
 *  hif for heat index in F
 */

//Battery
int btryVal = analogRead(A4);
//int btryVal2 = map(btryVal,850,1000,0,360);
float btryVal2 = btryVal * 0.004882814;

//More useful variables
int minCur = dt.minute;
int minOld = 0;
int tmpCur = f;
int tmpOld = 0;
int humCur = h;
int humOld = 0;
int btryCur = btryVal2;
int btryOld = 0;

//Timing
unsigned long curTime = millis();

//Time RTC

if(curTime - prvTRtc > intvRtc)
{
 tft.fillCircle(tile1X,tile1Y,tileDial-1,BLACK);
 tft.drawCircle(tile1X,tile1Y,3,WHITE);
 //Minute hand
    //HINT: 360/60 (as in minutes) = 6, so divide 360 by X according to value you are incrementing
float     angle = dt.minute * 6; //Retrieve stored minutes and apply
    angle=(angle/57.29577951) ; //Convert degrees to radians  
int       xo=(tile1X+(sin(angle)*(tileDial))); 
   int          yo=(tile1Y-(cos(angle)*(tileDial)));
      int xi=(tile1X+(sin(angle)*(1)));
      int yi=(tile1Y-(cos(angle)*(1)));
          tft.drawLine(xo,yo,xi,yi,WHITE);

//Hour hand
        angle = dt.hour * 30 + int((dt.minute / 12) * 6); //Retrieve stored hour and minutes and apply
    angle=(angle/57.29577951) ; //Convert degrees to radians  
           xo=(tile1X+(sin(angle)*(tileDial*0.75))); 
       yo=(tile1Y-(cos(angle)*(tileDial*0.75)));
       xi=(tile1X+(sin(angle)*(1)));
      yi=(tile1Y-(cos(angle)*(1)));
     tft.drawLine(xo,yo,xi,yi,WHITE);
     tft.drawPixel(xo,yo,BLACK);
 if(screenMode==1)
{
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print(clock.dateFormat("h:ia", dt));
   tft.setCursor(detLnX,detLn2Y);
   tft.print(clock.dateFormat("jS M y", dt)); 
}
 prvTRtc = curTime;
}

     //DHT
     //Temperature
     if(curTime - prvTT > intvT)
{
 tft.fillCircle(tile2X,tile2Y,tileDial-1,BLACK);
      //Temp F hand
         //DHT11 is known good for values 32F - 122F (0C-50C) https://learn.adafruit.com/dht/overview
 float   angle = f * 3; //room temp low 70s should be a little more than half
    angle=(angle/57.29577951) ; //Convert degrees to radians  
 int   xo=(tile2X+(sin(angle)*(tileDial-2)));
   int yo=(tile2Y-(cos(angle)*(tileDial-2)));
   int xi=(tile2X+(sin(angle)*(1)));
   int yi=(tile2Y-(cos(angle)*(1)));
    if(f>=32 && f<70)
    {
          tft.drawLine(xo,yo,xi,yi,WHITE);
    }
        if(f>69 && f<75)
    {
          tft.drawLine(xo,yo,xi,yi,CYAN);
    }
        if(f>74 && f<85)
    {
          tft.drawLine(xo,yo,xi,yi,MAGENTA);
    }
            if(f>84)
    {
          tft.drawLine(xo,yo,xi,yi,RED);
    }
 if(screenMode==2)
{
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print(f);
  tft.print(" F degrees");
  tft.setCursor(detLnX,detLn2Y);
  tft.print(t);
  tft.print(" C degrees");
}
prvTT = curTime;
}

//Humidity
        if(curTime - prvTH > intvH)
{
 tft.fillCircle(tile3X,tile3Y,tileDial-1,BLACK);
     //DHT11 is known good for values 20% - 80%
 float       angle = h * 3; //the 30s should be about a quarter way around
    angle=(angle/57.29577951) ; //Convert degrees to radians  
    int xo=(tile3X+(sin(angle)*(tileDial-2)));
    int yo=(tile3Y-(cos(angle)*(tileDial-2)));
  int  xi=(tile3X+(sin(angle)*(1)));
int    yi=(tile3Y-(cos(angle)*(1)));
    if(h>15 && h<35)
    {
          tft.drawLine(xo,yo,xi,yi,YELLOW);
    }
        if(h>34 && h<50)
    {
          tft.drawLine(xo,yo,xi,yi,MAGENTA);
    }
        if(h>49 && h<65)
    {
          tft.drawLine(xo,yo,xi,yi,CYAN);
    }
            if(h>64)
    {
          tft.drawLine(xo,yo,xi,yi,BLUE);
    }
    
    // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
tft.fillCircle(tile3X,tile3Y,tileDial,RED);
    return;
  }
 if(screenMode==3)
{
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print(h);
  tft.print(" %");
}
 prvTH = curTime;
}
  
//Power
if(curTime - prvTBtry > intvBtry)
{
 tft.fillCircle(tile4X,tile4Y,tileDial-1,BLACK);
   float      angle = btryVal2 * 72;
    angle=(angle/57.29577951) ; //Convert degrees to radians  
  int  xo=(tile4X+(sin(angle)*(tileDial-2)));
 int   yo=(tile4Y-(cos(angle)*(tileDial-2)));
 int   xi=(tile4X+(sin(angle)*(1)));
 int   yi=(tile4Y-(cos(angle)*(1)));
    if(btryVal > 1)
    {
          tft.drawLine(xo,yo,xi,yi,GREEN);
    }
       else
    {
          tft.drawLine(xo,yo,xi,yi,RED);
    }
 if(screenMode==4)
{
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print("Voltage: ");
  tft.setCursor(detLnX,detLn2Y);
  tft.print(btryVal2);
}
prvTBtry = curTime;
}

//Touch math
int tmp;
    TSPoint p = ts.getPoint();

    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);
    // we have some minimum pressure we consider 'valid'
    // pressure of 0 means no pressing!

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
        if (Landscape) {   // swap X and Y
            tmp = p.x;
            p.x = p.y;
            p.y = tmp;
        }
        // scale from 0->1023 to tft.width  i.e. left = 0, rt = width
        // most mcufriend have touch (with icons) that extends below the TFT 
        // screens without icons need to reserve a space for "erase"
        p.x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
        p.y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
    }

//Screen change on user input
if(((p.x>tile1X-tileDial)&&(p.x<tile1X+tileDial))&&((p.y>tile1Y-tileDial)&&p.y<tile1Y+tileDial))
{
screenMode = 1;
tft.drawFastHLine(0,tile1Y+tdLabel+10,tft.width(),BLACK);
tft.drawFastHLine(tile1X-tileDial,tile1Y+tdLabel+10,tileDial*2,WHITE);
tft.setTextSize(1);
/*
tft.setTextColor(WHITE);
tft.setCursor((tile1X+tileDial),(tile1Y+tdLabel));
tft.print("<");
tft.setTextColor(BLACK);
tft.setCursor((tile2X+tileDial),(tile2Y+tdLabel));
tft.print("<");
tft.setCursor((tile3X+tileDial),(tile3Y+tdLabel));
tft.print("<");
tft.setCursor((tile4X+tileDial),(tile4Y+tdLabel));
tft.print("<");
*/
tft.setTextColor(WHITE);
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print(clock.dateFormat("h:ia", dt));
   tft.setCursor(detLnX,detLn2Y);
   tft.print(clock.dateFormat("jS M y", dt)); 
   analogWrite(mtrPin,150);
   delay(100);
   analogWrite(mtrPin,0);
}

    if(((p.x>tile2X-tileDial)&&(p.x<tile2X+tileDial))&&((p.y>tile2Y-tileDial)&&p.y<tile2Y+tileDial))
{
screenMode = 2;
tft.drawFastHLine(0,tile2Y+tdLabel+10,tft.width(),BLACK);
tft.drawFastHLine(tile2X-tileDial,tile2Y+tdLabel+10,tileDial*2,WHITE);
tft.setTextSize(1);
/*
tft.setTextColor(BLACK);
tft.setCursor((tile1X+tileDial),(tile1Y+tdLabel));
tft.print("<");
tft.setCursor((tile3X+tileDial),(tile3Y+tdLabel));
tft.print("<");
tft.setCursor((tile4X+tileDial),(tile4Y+tdLabel));
tft.print("<");
*/
tft.setTextColor(WHITE);
/*
tft.setCursor((tile2X+tileDial),(tile2Y+tdLabel));
tft.print("<");
*/
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print(f);
  tft.print(" F degrees");
    tft.setCursor(detLnX,detLn2Y);
  tft.print(t);
  tft.print(" C degrees");
   analogWrite(mtrPin,150);
   delay(100);
   analogWrite(mtrPin,0);
}

if(((p.x>tile3X-tileDial)&&(p.x<tile3X+tileDial))&&((p.y>tile3Y-tileDial)&&p.y<tile3Y+tileDial))
{
screenMode = 3;
tft.drawFastHLine(0,tile3Y+tdLabel+10,tft.width(),BLACK);
tft.drawFastHLine(tile3X-tileDial,tile3Y+tdLabel+10,tileDial*2,WHITE);
tft.setTextSize(1);
/*
tft.setTextColor(BLACK);
tft.setCursor((tile1X+tileDial),(tile1Y+tdLabel));
tft.print("<");
tft.setCursor((tile2X+tileDial),(tile2Y+tdLabel));
tft.print("<");
tft.setCursor((tile4X+tileDial),(tile4Y+tdLabel));
tft.print("<");
*/
tft.setTextColor(WHITE);
/*
tft.setCursor((tile3X+tileDial),(tile3Y+tdLabel));
tft.print("<");
*/
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print(h);
  tft.print(" %");
   analogWrite(mtrPin,150);
   delay(100);
   analogWrite(mtrPin,0);
}

if(((p.x>tile4X-tileDial)&&(p.x<tile4X+tileDial))&&((p.y>tile4Y-tileDial)&&p.y<tile4Y+tileDial))
{
screenMode = 4;
tft.drawFastHLine(0,tile4Y+tdLabel+10,tft.width(),BLACK);
tft.drawFastHLine(tile4X-tileDial,tile4Y+tdLabel+10,tileDial*2,WHITE);
tft.setTextSize(1);
/*
tft.setTextColor(BLACK);
tft.setCursor((tile1X+tileDial),(tile1Y+tdLabel));
tft.print("<");
tft.setCursor((tile2X+tileDial),(tile2Y+tdLabel));
tft.print("<");
tft.setCursor((tile3X+tileDial),(tile3Y+tdLabel));
tft.print("<");
*/
tft.setTextColor(WHITE);
/*
tft.setCursor((tile4X+tileDial),(tile4Y+tdLabel));
tft.print("<");
*/
  tft.fillRect(0,detLn1Y-1,tft.width(),BOXSIZE*4,BLACK);
  tft.setTextSize(2);
  tft.setCursor(detLnX,detLn1Y);
  tft.print("Voltage: ");
  tft.setCursor(detLnX,detLn2Y);
  tft.print(btryVal2);   
   analogWrite(mtrPin,150);
   delay(100);
   analogWrite(mtrPin,0);
}

}


