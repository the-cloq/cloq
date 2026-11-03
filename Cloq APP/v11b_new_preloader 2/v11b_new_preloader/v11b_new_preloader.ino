 #include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <NTPClient.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <TimeLib.h>
#include <NeoPixelBrightnessBus.h>
#include <Arduino_JSON.h>

///OpenWeatherMap//
    
unsigned long lastTime = 0;
unsigned long timerDelay = 30 * 60000; // Timer set to 30 minutes
String jsonBuffer;

String APIKEY;                                     //API Key - openweathermap.org                            
String CityID;                                     //City ID - openweathermap.org    
unsigned int lastStringLength = APIKEY.length();                        

String weather;
int sunrise;
int sunset;
int dt;
float Temp;
float Knock;

//----------//

#define Dop_Motion_Sensor D5 //Dop D5
#define touch_sensor_1 D8
#define Pir_Motion_Sensor D7 //PIR D7

int mode = 0;
int colour_mode = 0;
int dopplerVal;
int pirVal;

const int ldrSensor = A0;
int  ldrSensorValue;

int8_t mood;                      //Dimming. The value changes upon LDR readings.
int8_t totalBrightness = 0;       //Brightness + mood. Brightness is set via EEPROM.

//WIFI and Web
//////////////
ESP8266WebServer  server(80);
WiFiClient  client;
WiFiUDP     ntpUDP;
char servername[]="api.openweathermap.org";
String result;

String      NTP_Server = "europe.pool.ntp.org"; // or "pool.ntp.org";

NTPClient   timeClient(ntpUDP, NTP_Server.c_str(), 3600, 60000);

//display time
//////////////
String      NTPtime;
String      NTPSimplytime;
String      DST_mode; //auto / summer / winter
int         SENSOR_mode;
float       cool; 
float       warm;
float       hot;
int         summer_offset, winter_offset;
int         brightness;
int         motionSensor = 1;
int         ldrSensor_status = 1;
int         sensor_timer = 1;
int         show_weather;
int         ambience;
int         seconds_status;
bool        summer;
String      language;
String      Temp_mode;

//Network
/////////
String  ssid1, pass1, ssid2, pass2;
String  Accesspoint_Name; 
bool    AP_running;
bool wifiConnected = false;
bool reconnection_allowed = false; //set to true, if there was a connection possible

IPAddress Accesspoint_IP(192, 168, 4, 1);
IPAddress Accesspoint_netmask(255, 255, 255, 0);

//Neopixels
///////////
const uint16_t PixelCount = 271;
const uint8_t PixelPin = 2;  // ignored for Esp8266, but necessary for function call
//connect ESP GPI02 pin D1 mini = D4


unsigned long pixelPrevious = 0;        // Previous Pixel Millis
unsigned long patternPrevious = 0;      // Previous Pattern Millis
int           patternCurrent = 0;       // Current Pattern Number
int           patternInterval = 333.3333;    // Pattern Interval (ms)
int           squidInterval = 1000;    // Pattern Interval (ms)
uint16_t      pixelCurrent = 0;         // Pattern Current Pixel Number
uint16_t      pixelNumber = PixelCount; // Total Number of Pixels

NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Uart1800KbpsMethod> strip(PixelCount, PixelPin); //Wemos D1 mini

RgbColor red(120, 0, 0);
RgbColor orangered(95, 25, 0);
RgbColor orange(80,30,0);
RgbColor yellow(75,45,0);
RgbColor green(0,80,0);
RgbColor blue(0, 0, 120);
RgbColor white(50);
RgbColor cyan(0, 60, 60);
RgbColor pink(60,0,60); 
RgbColor black(0);

RgbColor this_color(black);
RgbColor this_seconds_color(green);

String color = "";        //friendly color name
String scolor = "";       //friendly 'seconds' color name


//system
////////
unsigned long last_second = 99; //99 to force startup
unsigned long last_minute = 99; //99 to force startup
unsigned long last_hour   = 99; //99 to sync

int j;
bool display_on = true;

unsigned long startTime;

//********************************************************

void setup() 
{
  EEPROM.begin(512);
  Serial.begin(115200);
  delay(500);         //wait for serial startup
  timeClient.begin();
  StartupNeopixels();

  pinMode(ldrSensor, INPUT);
  mood = 0; // default mood value

  GetConfiguration(); 
  
  WiFi.mode(WIFI_AP_STA);   
  StartupAccesspoint(); 
  connectWifi();
  
  ConfigWebServer();
  
  if (wifiConnected) 
  {
    CalculateDST();     //first
    GetNtpTime();       //second
    getWeatherFirst();  //get weather update
    ShowMyIP();         //show IP on clock display
  }

  while (!Serial); // wait for serial attach
  
  Serial.println("Startup done");
}

//********************************************************

void loop()
{
  global();         //Combines all Modes
  //temphum_();       //Mode: Temperature and Humidity
  //digitalclock_();  //Mode: Digital Clock
  //cloq_();            //Mode: Cloq
  //invaders();       //Mode: Space Invader Animation
}

//********************************************************

void once_per_second() {
    if (second() != last_second) {
        last_second = second();

        if (!wifiConnected) {
            strip.SetPixelColor(270, red);
            strip.Show();

            if (reconnection_allowed) connectWifi(); //connection lost, try reconnection
        }
        strip.Show(); // display it
        server.handleClient(); //check for browser request and handle it

        shutdown_accesspoint(5 * 60000); //shutdown accesspoint 5 minutes after power on  

        dopplerVal = digitalRead(Dop_Motion_Sensor);
        pirVal = digitalRead(Pir_Motion_Sensor);

        if ((String) motionSensor == "1" ) {

            if ((String) SENSOR_mode == "1" ) { //Doppler Sensor
              
                if (dopplerVal == HIGH) {
                    display_on = true;
                    startTime = millis();
                } else if (millis() - startTime >= sensor_timer * 60000) {
                    display_on = false;
                } 
             }
      
             else if ((String) SENSOR_mode == "2" ) { //PIR Sensor
                
                if (pirVal == HIGH) {
                    display_on = true;
                    startTime = millis();
                } else if (millis() - startTime >= sensor_timer * 60000) {
                    display_on = false;
                }    
             }
          
            else {
              display_on = true;
            }
          }
          
        last_minute = 99; //force refresh cloq's display
    }
}

void once_per_half_second() {
    if (second() != last_second * 3) {
        last_second = second();

        if (!wifiConnected) {
            strip.SetPixelColor(270, red);
            strip.Show();

            if (reconnection_allowed) connectWifi(); //connection lost, try reconnection
        }
        strip.Show(); // display it
        server.handleClient(); //check for browser request and handle it

        shutdown_accesspoint(5 * 60000); //shutdown accesspoint 5 minutes after power on  

        dopplerVal = digitalRead(Dop_Motion_Sensor);
        pirVal = digitalRead(Pir_Motion_Sensor);

        if ((String) motionSensor == "1" ) {

            if ((String) SENSOR_mode == "1") { //Doppler Sensor
              
                if (dopplerVal == HIGH) {
                    display_on = true;
                    startTime = millis();
                } else if (millis() - startTime >= sensor_timer * 60000) {
                    display_on = false;
                }  
              }
      
              else if ((String) SENSOR_mode == "2") { //PIR Sensor
                
                if (pirVal == HIGH) {
                    display_on = true;
                    startTime = millis();
                } else if (millis() - startTime >= sensor_timer * 60000) {
                    display_on = false;
                }     
            }
          
            else {
              display_on = true;
            }
          }
          
        last_minute = 99; //force refresh CLOQ's display
    }
}

void once_per_minute()    //will be forced by HTML post
{
  if (minute() != last_minute)
  {
    last_minute = minute();

    GetConfiguration();   //refresh configuration
    
    if (wifiConnected) 
    {
      CalculateDST(); //call this first
      GetNtpTime();   //call this second
    }
    
    if(reconnection_allowed) //we are or we were connected: show time
    {
     AllLedsOff(); //prepare display: all leds off
     
    if(display_on == true)
    {    
      TimeToText();
      getWeather();
      displayConditions(Temp);
      //Serial.println(Temp); 
      ldr();
      }
       
    strip.Show();
    }
  } 
}

void once_per_digital_minute()    //will be forced by HTML post
{
  if (minute() != last_minute)
  {
    last_minute = minute();

    GetConfiguration();   //refresh configuration
    
    if (wifiConnected) 
    {
      CalculateDST(); //call this first
      GetNtpTime();   //call this second
    }
    
    if(reconnection_allowed) //we are or we were connected: show time
    {
     AllLedsOff(); //prepare display: all leds off
     
    if(display_on == true)
    {    
      TimeToDigital();             // Set Digital LEDs
      ldr();                       //light dependant resistor
      }
       strip.Show();
    }
  } 
}

void in_temperature_humidity()    //will be forced by HTML post
{
  if (minute() != last_minute)
  {
    last_minute = minute();

    GetConfiguration();           //refresh configuration
    
    if (wifiConnected) 
    {
      CalculateDST();             //call this first
      GetNtpTime();               //call this second
    }
    
    if(reconnection_allowed) //we are or we were connected: show time
    {
     AllLedsOff(); //prepare display: all leds off
     
    if(display_on == true)
    {    
      //TimeToSeconds();            // Seconds_Numbers(second());
      ldr();                      //light dependant resistor
      }
      strip.Show();
    }
  } 
}

void outside_temperature()        //will be forced by HTML post
{
  if (minute() != last_minute)
  {
    last_minute = minute();

    GetConfiguration();           //refresh configuration
    
    if (wifiConnected) 
    {
      CalculateDST();             //call this first
      GetNtpTime();               //call this second
    }
    
    if(reconnection_allowed)      //we are or we were connected: show time
    {
     AllLedsOff();                //prepare display: all leds off
     
    if(display_on == true)
    {    
      //TimeToSeconds();            // Seconds_Numbers(second());
      //getWeather();
      displayTemp(Temp);   
      displayWeather(weather);
      ldr();                      //light dependant resistor
      }
      strip.Show();
    }
  } 
}

void animations()                 //will be forced by HTML post
{
  if (minute() != last_minute)
  {
    last_minute = minute();

    GetConfiguration();           //refresh configuration
    
    if (wifiConnected) 
    {
      CalculateDST();             //call this first
      GetNtpTime();               //call this second
    }
    
    if(reconnection_allowed)      //we are or we were connected: show time
    {
     AllLedsOff();                //prepare display: all leds off
     
    if(display_on == true)
    {    
      invaders();                 //animation
      ldr();                       //light dependant resistor
      }
      strip.Show();
    }
  } 
}

void animations2()                //will be forced by HTML post
{
  if (minute() != last_minute)
  {
    last_minute = minute();

    GetConfiguration();           //refresh configuration
    
    if (wifiConnected) 
    {
      CalculateDST();             //call this first
      GetNtpTime();               //call this second
    }
    
    if(reconnection_allowed)      //we are or we were connected: show time
    {
     AllLedsOff();                //prepare display: all leds off
     
    if(display_on == true)
    {    
      manicminer();               //animation
      }
      strip.Show();
    }
  } 
}

///--------OpenWeatherMap--------//

void getWeatherFirst()
{
  if( APIKEY.length() == 32 || CityID != "" || (String) show_weather == "1" || (String) ambience == "1")
 {
      // Send an HTTP GET request
      if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?id="+CityID+"&APPID="+APIKEY;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      int Temperature = myObject["main"]["temp"];
      Temp = Temperature - 272.65; //273.15

      weather = JSON.stringify(myObject["weather"][0]["description"]);
      dt = myObject["dt"];
      sunrise = myObject["sys"]["sunrise"];
      sunset = myObject["sys"]["sunset"];
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
 }
}

void getWeather() 
{
  if( APIKEY.length() == 32 || CityID != "" || (String) show_weather == "1" )
 {
  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?id="+CityID+"&APPID="+APIKEY;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }

      int Temperature = myObject["main"]["temp"];
      Temp = Temperature - 273.15;
      
      weather = JSON.stringify(myObject["weather"][0]["description"]);      
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
 }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}

void displayWeather(String weather)
{

 if( APIKEY.length() == 32 || CityID != "" || (String) show_weather == "1" )
 {
    
  if (weather == "\"clear sky\"" && (dt > sunrise && dt < sunset) )
    {
    for(j = 238 ; j <= 240 ; j++) strip.SetPixelColor(j, yellow); //sun//
    }
  if (weather == "\"clear sky\"" && (dt < sunrise || dt > sunset) )
    {
    for(j = 213 ; j <= 217 ; j++) strip.SetPixelColor(j, white); //clear//
    }
  if (weather == "\"few clouds\"" && (dt > sunrise && dt < sunset) || weather == "\"scattered clouds\"" && (dt > sunrise && dt < sunset) )
    {
    for(j = 238 ; j <= 240 ; j++) strip.SetPixelColor(j, yellow); //sun//
    for(j = 241 ; j <= 245 ; j++) strip.SetPixelColor(j, white); //few clouds//scattered clouds//
    }
  if (weather == "\"few clouds\"" && (dt < sunrise || dt > sunset) || weather == "\"scattered clouds\"" && (dt < sunrise || dt > sunset) )
    {
    for(j = 241 ; j <= 245 ; j++) strip.SetPixelColor(j, white); //few clouds//scattered clouds//
    }
  if (weather == "\"broken clouds\"" || weather == "\"overcast clouds\"")
    {
    for(j = 241 ; j <= 245 ; j++) strip.SetPixelColor(j, white); //broken/overcast clouds//
    } 
  if (weather == "\"rain\"" || weather == "\"shower rain\"" || weather == "\"light rain\"")
    {
    for(j = 246 ; j <= 249 ; j++) strip.SetPixelColor(j, cyan); //rain//
    }
  if (weather == "\"snow\"")
    {
    for(j = 250 ; j <= 253 ; j++) strip.SetPixelColor(j, white); //snaw//
    }
  }
}

void once_per_hour() //nothing to do in this version
{
  if (hour() != last_hour)
  {
    last_hour = hour();
  }
}

void TimeToText()
{
  Serial.println("Time to display:");
  Serial.print(hour());Serial.print(":");Serial.print(minute());Serial.print(":");Serial.print(second());
  
  if(language == "de")
  {
    
  }  
  if(language == "de1")
  {
    
  }
  if(language == "en")
  {
    //for(j = 0 ; j <= 2 ; j++) strip.SetPixelColor(j, this_color); //the
    //for(j = 4 ; j <= 7 ; j++) strip.SetPixelColor(j, this_color); //time
    //for(j = 9 ; j <= 10 ; j++) strip.SetPixelColor(j, this_color); //is
    TimeToSeconds(); //Seconds_Numbers(second());
    Minutes_Text_en(minute()); //minute(s) 1, 2, 3, ...
    Hours_Text_en(hour());
  }
}

void TimeToDigital()
{
  Serial.println("Time to display:");
  Serial.print(hour());Serial.print(":");Serial.print(minute());Serial.print(":");Serial.print(second());
  if(language == "de")
  {
    
  }  
  if(language == "de1")
  {
    
  }
  if(language == "en")
  {
    //for(j = 0 ; j <= 2 ; j++) strip.SetPixelColor(j, this_color); //the
    //for(j = 4 ; j <= 7 ; j++) strip.SetPixelColor(j, this_color); //time
    //for(j = 9 ; j <= 10 ; j++) strip.SetPixelColor(j, this_color); //is
    TimeToSeconds(); //Seconds_Numbers(second());
    DotsToSeconds();
    Minutes_Digital_en(minute()); //minute(s) 1, 2, 3, ...
    Hours_Digital_en(hour());
  }
}

void TimeToSeconds()
{
    Seconds_Numbers(second());
}

void DotsToSeconds()
{
    Seconds_Dots(second());
}
 
void Hours_Text_de(int hours)
{
  if(language == "de") //German standard
    if(minute() > 19) hours = hours + 1;  //8:10 zehn nach 8, 8:15 fünfzehn nach 8
  if(language == "de1") //German South
    if(minute() > 14) hours = hours + 1; //8:10 zehn nach 8, 8:15 viertel neun
  
  if (hours > 12) hours = hours - 12;
  if (hours == 0) hours = 12;
  show_number_de(hours);
}

void Hours_Text_en(int hours)
{
  if(language == "en") //English
    if(minute() > 30) hours = hours + 1; //8:30 half past 8, 8:35 twentyfive to 9
    
  if(hours == 0 && minute() < 1)
  {
    for(j = 153 ; j <= 158 ; j++) strip.SetPixelColor(j, this_color); // twelve
    for(j = 188 ; j <= 195 ; j++) strip.SetPixelColor(j, this_color); // midnight
    for(j = 197 ; j <= 202 ; j++) strip.SetPixelColor(j, black); //o’clock
  }

  if(hours == 12 && minute() < 1)
  {
    for(j = 153 ; j <= 158 ; j++) strip.SetPixelColor(j, this_color); // twelve
    for(j = 204 ; j <= 207 ; j++) strip.SetPixelColor(j, this_color); // noon
    for(j = 197 ; j <= 202 ; j++) strip.SetPixelColor(j, black); //o’clock
  }
    
  show_number_en(hours);
}

void Hours_Digital_en(int hours)
{
  if(language == "en") //English
    if(minute() > 30) hours = hours + 0; //8:30 half past 8, 8:35 twentyfive to 9  
  
  show_number_digital_en(hours);
}

void show_number_en(int number)
{  
  switch (number)
  {
    case  0:
      for(j = 153 ; j <= 158 ; j++) strip.SetPixelColor(j, this_color);  //twelve
      break;
    case  1:
      for(j = 138 ; j <= 140 ; j++) strip.SetPixelColor(j, this_color);  //one   
      break;
    case  2:
      for(j = 136 ; j <= 138 ; j++) strip.SetPixelColor(j, this_color);  //two    
      break;
    case  3:
      for(j = 159 ; j <= 163 ; j++) strip.SetPixelColor(j, this_color);  //three   
      break;
    case  4:
      for(j = 170 ; j <= 173 ; j++) strip.SetPixelColor(j, this_color);  //four   
      break;
    case  5:
      for(j = 174 ; j <= 177 ; j++) strip.SetPixelColor(j, this_color);  //five   
      break;
    case  6:
      for(j = 182 ; j <= 184 ; j++) strip.SetPixelColor(j, this_color);  //six   
      break;
    case  7:
      for(j = 164 ; j <= 168 ; j++) strip.SetPixelColor(j, this_color);  //seven  
      break;
    case  8:
      for(j = 177 ; j <= 181 ; j++) strip.SetPixelColor(j, this_color);  //eight  
      break;
    case  9:
      for(j = 145 ; j <= 148 ; j++) strip.SetPixelColor(j, this_color);  //nine   
      break;
    case 10:
      strip.SetPixelColor(151, this_color);
      strip.SetPixelColor(153, this_color);
      strip.SetPixelColor(185, this_color);  //ten
      break;
    case 11:
      for(j = 140 ; j <= 145 ; j++) strip.SetPixelColor(j, this_color);  //eleven         
      break;
    case 12:
      for(j = 153 ; j <= 158 ; j++) strip.SetPixelColor(j, this_color);  //twelve
      break;
    case 13:
      for(j = 138 ; j <= 140 ; j++) strip.SetPixelColor(j, this_color);  //one   
      break;
    case 14:
      for(j = 136 ; j <= 138 ; j++) strip.SetPixelColor(j, this_color);  //two   
      break;
    case 15:
      for(j = 159 ; j <= 163 ; j++) strip.SetPixelColor(j, this_color);  //three    
      break;
    case 16:
      for(j = 170 ; j <= 173 ; j++) strip.SetPixelColor(j, this_color);  //four   
      break;
    case 17:
      for(j = 174 ; j <= 177 ; j++) strip.SetPixelColor(j, this_color);  //five   
      break;
    case 18:
      for(j = 182 ; j <= 184 ; j++) strip.SetPixelColor(j, this_color);  //six   
      break;
    case 19:
      for(j = 164 ; j <= 168 ; j++) strip.SetPixelColor(j, this_color);  //seven  
      break;
    case 20:
      for(j = 177 ; j <= 181 ; j++) strip.SetPixelColor(j, this_color);  //eight  
      break;
    case 21:
      for(j = 145 ; j <= 148 ; j++) strip.SetPixelColor(j, this_color);  //nine  
      break;
    case 22:
      strip.SetPixelColor(151, this_color);
      strip.SetPixelColor(153, this_color);
      strip.SetPixelColor(185, this_color);  //ten
      break;
    case 23:
      for(j = 140 ; j <= 145 ; j++) strip.SetPixelColor(j, this_color);  //eleven        
      break;
    case 24:
      for(j = 153 ; j <= 158 ; j++) strip.SetPixelColor(j, this_color);  //twelve
      break;
    default:
      break;
  }  
}

void show_number_de(int number)   //led numbers
{  

}

void Minutes_Text_de(int minutes) //led numbers
{

}

void Minutes_Text_de1(int minutes) //led numbers
{
 
}

void show_IP_en(int number)
{  
  switch (number)
  {
    case  0:
      for(j = 51 ; j <= 54 ; j++) strip.SetPixelColor(j, this_color);    //zero
      break;
    case  1:
      for(j = 138 ; j <= 140 ; j++) strip.SetPixelColor(j, this_color);  //one   
      break;
    case  2:
      for(j = 136 ; j <= 138 ; j++) strip.SetPixelColor(j, this_color);  //two    
      break;
    case  3:
      for(j = 159 ; j <= 163 ; j++) strip.SetPixelColor(j, this_color);  //three   
      break;
    case  4:
      for(j = 170 ; j <= 173 ; j++) strip.SetPixelColor(j, this_color);  //four   
      break;
    case  5:
      for(j = 174 ; j <= 177 ; j++) strip.SetPixelColor(j, this_color);  //five   
      break;
    case  6:
      for(j = 182 ; j <= 184 ; j++) strip.SetPixelColor(j, this_color);  //six   
      break;
    case  7:
      for(j = 164 ; j <= 168 ; j++) strip.SetPixelColor(j, this_color);  //seven  
      break;
    case  8:
      for(j = 177 ; j <= 181 ; j++) strip.SetPixelColor(j, this_color);  //eight  
      break;
    case  9:
      for(j = 145 ; j <= 148 ; j++) strip.SetPixelColor(j, this_color);  //nine   
      break;
    case 10:
      strip.SetPixelColor(151, this_color);                              //t
      strip.SetPixelColor(153, this_color);                              //e
      strip.SetPixelColor(185, this_color);                              //n
      break;
    case 11:
      for(j = 140 ; j <= 145 ; j++) strip.SetPixelColor(j, this_color);  //eleven         
      break;
    case 12:
      for(j = 153 ; j <= 158 ; j++) strip.SetPixelColor(j, this_color);  //twelve
      break;
      default:
      break;
  }  
}

void show_number_digital_en(int number)
{  
  switch (number)
  {
    case  0:
      t_0_(); t_0();
      break;
    case  1:
      t_0_(); t_1();  
      break;
    case  2:
      t_0_(); t_2();   
      break;
    case  3:
      t_0_(); t_3();  
      break;
    case  4:
      t_0_(); t_4();
      break;
    case  5:
      t_0_(); t_5();
      break;
    case  6:
      t_0_(); t_6(); 
      break;
    case  7:
      t_0_(); t_7(); 
      break;
    case  8:
      t_0_(); t_8();
      break;
    case  9:
      t_0_(); t_9(); 
      break;
    case 10:
      t_1_(); t_0();
      break;
    case 11:
      t_1_(); t_1();       
      break;
    case 12:
      t_1_(); t_2();
      break;
    case 13:
      t_1_(); t_3(); 
      break;
    case 14:
      t_1_(); t_4();  
      break;
    case 15:
      t_1_(); t_5(); 
      break;
    case 16:
      t_1_(); t_6(); 
      break;
    case 17:
      t_1_(); t_7(); 
      break;
    case 18:
      t_1_(); t_8();  
      break;
    case 19:
      t_1_(); t_9();
      break;
    case 20:
      t_2_(); t_0();
      break;
    case 21:
      t_2_(); t_1();  
      break;
    case 22:
      t_2_(); t_2();
      break;
    case 23:
      t_2_(); t_3();   
      break;
    case 24:
      t_0_(); t_0();
      break;
    default:
      break;
  }  
}

void Minutes_Text_en(int minutes)
{
  switch (minutes)
  {
    case  0: 
      for(j = 197 ; j <= 202 ; j++) strip.SetPixelColor(j, this_color); //o’clock
      break;
    case  1: 
      for(j = 17 ; j <= 19 ; j++) strip.SetPixelColor(j, this_color); //one
      for(j = 129 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minute
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  2: 
      for(j = 44 ; j <= 46 ; j++) strip.SetPixelColor(j, this_color); //two
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  3: 
      for(j = 113 ; j <= 117 ; j++) strip.SetPixelColor(j, this_color); //three
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  4: 
      for(j = 97 ; j <= 100 ; j++) strip.SetPixelColor(j, this_color); //four
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  5: 
      for(j =  55 ; j <=  58 ; j++) strip.SetPixelColor(j, this_color); //five
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  6: 
      for(j = 37 ; j <= 39 ; j++) strip.SetPixelColor(j, this_color); //six
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  7: 
      for(j = 68 ; j <= 72 ; j++) strip.SetPixelColor(j, this_color); //seven
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  8: 
      for(j = 62 ; j <= 66 ; j++) strip.SetPixelColor(j, this_color); //eight
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case  9: 
      for(j = 76 ; j <= 79 ; j++) strip.SetPixelColor(j, this_color); //nine
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 10: 
      for(j = 34 ; j <= 36 ; j++) strip.SetPixelColor(j, this_color); //ten
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 11: 
      for(j = 107 ; j <= 112 ; j++) strip.SetPixelColor(j, this_color); //eleven
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 12: 
      for(j = 102; j <= 107 ; j++) strip.SetPixelColor(j, this_color); //twelve
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 13: 
      for(j = 85 ; j <= 92 ; j++) strip.SetPixelColor(j, this_color); //thirteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 14: 
      for(j = 93; j <= 100 ; j++) strip.SetPixelColor(j, this_color); //fourteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 15: 
      for(j = 26 ; j <= 32 ; j++) strip.SetPixelColor(j, this_color); //quarter
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 16: 
      for(j = 37 ; j <= 43 ; j++) strip.SetPixelColor(j, this_color); //sixteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 17: 
      for(j = 68; j <= 76; j++) strip.SetPixelColor(j, this_color); //seventeen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 18: 
      for(j = 59; j <= 66 ; j++) strip.SetPixelColor(j, this_color); //eighteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 19: 
      for(j = 76 ; j <= 83; j++) strip.SetPixelColor(j, this_color); //nineteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 20: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); //twenty
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 21: 
      for(j = 17 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); //twenty one 
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 22: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 44 ; j <= 46 ; j++) strip.SetPixelColor(j, this_color); //twenty two
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 23: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 113 ; j <= 117 ; j++) strip.SetPixelColor(j, this_color); //twenty three
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 24: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 97 ; j <= 100 ; j++) strip.SetPixelColor(j, this_color); //twenty four
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 25: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j =  55 ; j <=  58 ; j++) strip.SetPixelColor(j, this_color); //twenty five
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 26: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 37 ; j <= 39 ; j++) strip.SetPixelColor(j, this_color); //twenty six
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 27: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 68 ; j <= 72 ; j++) strip.SetPixelColor(j, this_color); //twenty seven
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 28: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 62 ; j <= 66 ; j++) strip.SetPixelColor(j, this_color); //twenty eight
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 29: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 76 ; j <= 79 ; j++) strip.SetPixelColor(j, this_color); //twenty nine
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 30: 
      for(j = 12 ; j <= 15 ; j++) strip.SetPixelColor(j, this_color); //half
      for(j = 123 ; j <= 126 ; j++) strip.SetPixelColor(j, this_color); //past
      break;
    case 31: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 76 ; j <= 79 ; j++) strip.SetPixelColor(j, this_color); //twenty nine
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 32: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 62 ; j <= 66 ; j++) strip.SetPixelColor(j, this_color); //twenty eight
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 33: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 68 ; j <= 72 ; j++) strip.SetPixelColor(j, this_color); //twenty seven
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 34: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 37 ; j <= 39 ; j++) strip.SetPixelColor(j, this_color); //twenty six
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 35: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j =  55 ; j <=  58 ; j++) strip.SetPixelColor(j, this_color); //twenty five
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 36: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 97 ; j <= 100 ; j++) strip.SetPixelColor(j, this_color); //twenty four
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 37: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 113 ; j <= 117 ; j++) strip.SetPixelColor(j, this_color); //twenty three
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 38: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); for(j = 44 ; j <= 46 ; j++) strip.SetPixelColor(j, this_color); //twenty two
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 39: 
      for(j = 17 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); //twenty one
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 40: 
      for(j = 20 ; j <= 25 ; j++) strip.SetPixelColor(j, this_color); //twenty
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 41: 
      for(j = 76 ; j <= 83; j++) strip.SetPixelColor(j, this_color); //nineteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 42: 
      for(j = 59; j <= 66 ; j++) strip.SetPixelColor(j, this_color); //eighteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 43: 
      for(j = 68; j <= 76; j++) strip.SetPixelColor(j, this_color); //seventeen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 44: 
      for(j = 37 ; j <= 43 ; j++) strip.SetPixelColor(j, this_color); //sixteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 45: 
      for(j = 26 ; j <= 32 ; j++) strip.SetPixelColor(j, this_color); //quarter
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 46: 
      for(j = 93; j <= 100 ; j++) strip.SetPixelColor(j, this_color); //fourteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 47: 
      for(j = 85 ; j <= 92 ; j++) strip.SetPixelColor(j, this_color); //thirteen
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 48: 
      for(j = 102; j <= 107 ; j++) strip.SetPixelColor(j, this_color); //twelve
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 49: 
      for(j = 107 ; j <= 112 ; j++) strip.SetPixelColor(j, this_color); //eleven
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 50: 
      for(j = 34 ; j <= 36 ; j++) strip.SetPixelColor(j, this_color); //ten
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 51: 
      for(j = 76 ; j <= 79 ; j++) strip.SetPixelColor(j, this_color); //nine
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 52: 
      for(j = 62 ; j <= 66 ; j++) strip.SetPixelColor(j, this_color); //eight
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 53: 
      for(j = 68 ; j <= 72 ; j++) strip.SetPixelColor(j, this_color); //seven
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 54: 
      for(j = 37 ; j <= 39 ; j++) strip.SetPixelColor(j, this_color); //six
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 55: 
      for(j =  55 ; j <=  58 ; j++) strip.SetPixelColor(j, this_color); //five
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 56: 
      for(j = 97 ; j <= 100 ; j++) strip.SetPixelColor(j, this_color); //four
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 57: 
      for(j = 113 ; j <= 117 ; j++) strip.SetPixelColor(j, this_color); //three
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 58: 
      for(j = 44 ; j <= 46 ; j++) strip.SetPixelColor(j, this_color); //two
      for(j = 128 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minutes
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    case 59: 
      for(j = 17 ; j <= 19 ; j++) strip.SetPixelColor(j, this_color); //one
      for(j = 129 ; j <= 134 ; j++) strip.SetPixelColor(j, this_color); //minute
      for(j = 122 ; j <= 123 ; j++) strip.SetPixelColor(j, this_color); //to
      break;
    default: 
      break;
  }  
}

void Minutes_Digital_en(int minutes)
{
  switch (minutes)
  {
    case  0: 
      b_0_(); b_0(); //o’clock
      break;
    case  1: 
      b_0_(); b_1();
      break;
    case  2: 
      b_0_(); b_2();
      break;
    case  3: 
      b_0_(); b_3();
      break;
    case  4: 
      b_0_(); b_4();
      break;
    case  5: 
      b_0_(); b_5();
      break;
    case  6: 
      b_0_(); b_6();
      break;
    case  7: 
      b_0_(); b_7();
      break;
    case  8: 
      b_0_(); b_8();
      break;
    case  9: 
      b_0_(); b_9();
      break;
    case 10: 
      b_1_(); b_0();
      break;
    case 11: 
      b_1_(); b_1();
      break;
    case 12: 
      b_1_(); b_2();
      break;
    case 13: 
      b_1_(); b_3();
      break;
    case 14: 
      b_1_(); b_4();
      break;
    case 15: 
      b_1_(); b_5();
      break;
    case 16: 
      b_1_(); b_6();
      break;
    case 17: 
      b_1_(); b_7();
      break;
    case 18: 
      b_1_(); b_8();
      break;
    case 19: 
      b_1_(); b_9();
      break;
    case 20: 
      b_2_(); b_0();
      break;
    case 21: 
      b_2_(); b_1();
      break;
    case 22: 
      b_2_(); b_2();
      break;
    case 23: 
      b_2_(); b_3();
      break;
    case 24: 
      b_2_(); b_4();
      break;
    case 25: 
      b_2_(); b_5();
      break;
    case 26: 
      b_2_(); b_6();
      break;
    case 27: 
      b_2_(); b_7();
      break;
    case 28: 
      b_2_(); b_8();
      break;
    case 29: 
      b_2_(); b_9();
      break;
    case 30: 
      b_3_(); b_0();
      break;
    case 31: 
      b_3_(); b_1();
      break;
    case 32: 
      b_3_(); b_2();
      break;
    case 33: 
      b_3_(); b_3();
      break;
    case 34: 
      b_3_(); b_4();
      break;
    case 35: 
      b_3_(); b_5();
      break;
    case 36: 
      b_3_(); b_6();
      break;
    case 37: 
      b_3_(); b_7();
      break;
    case 38: 
      b_3_(); b_8();
      break;
    case 39: 
      b_3_(); b_9();
      break;
    case 40: 
      b_4_(); b_0();
      break;
    case 41: 
      b_4_(); b_1();
      break;
    case 42: 
      b_4_(); b_2();
      break;
    case 43: 
      b_4_(); b_3();
      break;
    case 44: 
      b_4_(); b_4();
      break;
    case 45: 
      b_4_(); b_5();
      break;
    case 46: 
      b_4_(); b_6();
      break;
    case 47: 
      b_4_(); b_7();
      break;
    case 48: 
      b_4_(); b_8();
      break;
    case 49: 
      b_4_(); b_9();
      break;
    case 50: 
      b_5_(); b_0();
      break;
    case 51: 
      b_5_(); b_1();
      break;
    case 52: 
      b_5_(); b_2();
      break;
    case 53: 
      b_5_(); b_3();
      break;
    case 54: 
      b_5_(); b_4();
      break;
    case 55: 
      b_5_(); b_5();
      break;
    case 56: 
      b_5_(); b_6();
      break;
    case 57: 
      b_5_(); b_7();
      break;
    case 58: 
      b_5_(); b_8();
      break;
    case 59: 
      b_5_(); b_9();
      break;
    default: 
      break;
  }  
}

void ShowMyIP()
{
  IPAddress myIP = WiFi.localIP();
  String ThisIP = String(myIP[0],DEC) + "." + String(myIP[1],DEC) + "." + String(myIP[2],DEC) + "." + String(myIP[3],DEC);
  String digit;

  Serial.print("IP address: ");
  Serial.println(ThisIP);
  
  for (int j = 0 ; j < ThisIP.length() ; j++) 
  {
    AllLedsOff();
    digit = ThisIP.charAt(j);

    if( ThisIP.charAt(j) == '.')
    {
      AllLedsOff();
      strip.Show();
      delay(500);
    }
    else
    {
   if(language == "en") //English
      show_IP_en (digit.toInt());
   else                 // German
    show_number_de (digit.toInt());
    }  
    strip.Show();
    delay(700);
  }
  
  AllLedsOff();
  strip.Show();
  delay(500);
}

void Seconds_Dots(int seconds)
  {
      strip.SetPixelColor(54, this_color);
      strip.SetPixelColor(80, this_color);
  }


void ldr() {

  ldrSensorValue = analogRead(ldrSensor);

  if ((String) ldrSensor_status == "1" ) {
    
    if(analogRead(ldrSensor) >=0 && analogRead(ldrSensor) <=50) //Very dark 
    {
      mood = 0; //up to 50
    }
  
    if(analogRead(ldrSensor) >=51 && analogRead(ldrSensor) <=300) //Not so dark 
    {
      mood = 30; //up to 80
    }
  
    if(analogRead(ldrSensor) >=301 && analogRead(ldrSensor) <=600) //Standard ambient light 
    {
      mood = 60; //up to 110
    }
  
    if(analogRead(ldrSensor) >=601 && analogRead(ldrSensor) <=800) //Bright room lights
    {
      mood = 80; //up to 130
    }
  
    if(analogRead(ldrSensor) >=801 && analogRead(ldrSensor) <=1024) //Very bright room lights
    {
      mood = 150; //up to 200
    }
  
   totalBrightness = brightness + mood;
   strip.SetBrightness(totalBrightness); 
  }
  else {
    mood = 0;
  }

   totalBrightness = brightness + mood;
   strip.SetBrightness(totalBrightness); 
/*
 Serial.println("...Total Brightness...");
 Serial.println(totalBrightness);
 Serial.println("...Sensor Value...");
 Serial.println(ldrSensorValue);
 Serial.println("...Mood Value...");
 Serial.println(mood);
*/
}

void Seconds_Numbers(int seconds)
{
  if ( (String) seconds_status == "1")
  {
  switch (seconds)
  {
    case  0: 
      strip.SetPixelColor(255, black);
      strip.SetPixelColor(265, black);
      strip.SetPixelColor(264, this_seconds_color);
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  1: 
      strip.SetPixelColor(264, black);
      strip.SetPixelColor(263, this_seconds_color);
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  2: 
      strip.SetPixelColor(263, black);
      strip.SetPixelColor(262, this_seconds_color);  
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  3: 
      strip.SetPixelColor(262, black);
      strip.SetPixelColor(261, this_seconds_color);
      strip.SetPixelColor(270, this_seconds_color);  
      break;
    case  4: 
      strip.SetPixelColor(261, black);
      strip.SetPixelColor(260, this_seconds_color); 
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  5: 
      strip.SetPixelColor(260, black);
      strip.SetPixelColor(259, this_seconds_color); 
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  6: 
      strip.SetPixelColor(259, black);
      strip.SetPixelColor(258, this_seconds_color);
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  7: 
      strip.SetPixelColor(258, black);
      strip.SetPixelColor(257, this_seconds_color);
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  8: 
      strip.SetPixelColor(257, black);
      strip.SetPixelColor(256, this_seconds_color); 
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case  9: 
      strip.SetPixelColor(256, black);
      strip.SetPixelColor(255, this_seconds_color); 
      strip.SetPixelColor(270, this_seconds_color);
      break;
    case 10: 
      strip.SetPixelColor(255, black);
      strip.SetPixelColor(270, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(264, this_seconds_color);
      break;
    case 11:
      strip.SetPixelColor(264, black); 
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(263, this_seconds_color);
      break;
    case 12: 
      strip.SetPixelColor(263, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(262, this_seconds_color);
      break;
    case 13: 
      strip.SetPixelColor(262, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(261, this_seconds_color);
      break;
    case 14: 
      strip.SetPixelColor(261, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(260, this_seconds_color);
      break;
    case 15: 
      strip.SetPixelColor(260, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(259, this_seconds_color);
      break;
    case 16: 
      strip.SetPixelColor(259, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(258, this_seconds_color);
      break;
    case 17: 
      strip.SetPixelColor(258, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(257, this_seconds_color);
      break;
    case 18: 
      strip.SetPixelColor(257, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(256, this_seconds_color);
      break;
    case 19: 
      strip.SetPixelColor(256, black);
      strip.SetPixelColor(269, this_seconds_color); strip.SetPixelColor(255, this_seconds_color);
      break;
    case 20: 
      strip.SetPixelColor(255, black); strip.SetPixelColor(269, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(264, this_seconds_color);
      break;
    case 21: 
      strip.SetPixelColor(264, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(263, this_seconds_color);
      break;
    case 22: 
      strip.SetPixelColor(263, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(262, this_seconds_color);
      break;
    case 23: 
      strip.SetPixelColor(262, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(261, this_seconds_color);
      break;
    case 24: 
      strip.SetPixelColor(261, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(260, this_seconds_color);
      break;
    case 25: 
      strip.SetPixelColor(260, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(259, this_seconds_color);
      break;
    case 26: 
      strip.SetPixelColor(259, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(258, this_seconds_color);
      break;
    case 27: 
      strip.SetPixelColor(258, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(257, this_seconds_color);
      break;
    case 28:
      strip.SetPixelColor(257, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(256, this_seconds_color);
      break;
    case 29: 
      strip.SetPixelColor(256, black);
      strip.SetPixelColor(268, this_seconds_color); strip.SetPixelColor(255, this_seconds_color);
      break;
    case 30: 
      strip.SetPixelColor(255, black); strip.SetPixelColor(268, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(264, this_seconds_color);
      break;
    case 31: 
      strip.SetPixelColor(264, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(263, this_seconds_color);
      break;
    case 32: 
      strip.SetPixelColor(263, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(262, this_seconds_color);
      break;
    case 33: 
      strip.SetPixelColor(262, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(261, this_seconds_color);
      break;
    case 34: 
      strip.SetPixelColor(261, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(260, this_seconds_color);
      break;
    case 35: 
      strip.SetPixelColor(260, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(259, this_seconds_color);
      break;
    case 36: 
      strip.SetPixelColor(259, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(258, this_seconds_color);
      break;
    case 37: 
      strip.SetPixelColor(258, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(257, this_seconds_color);
      break;
    case 38: 
      strip.SetPixelColor(257, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(256, this_seconds_color);
      break;
    case 39: 
      strip.SetPixelColor(256, black);
      strip.SetPixelColor(267, this_seconds_color); strip.SetPixelColor(255, this_seconds_color);
      break;
    case 40: 
      strip.SetPixelColor(255, black); strip.SetPixelColor(267, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(264, this_seconds_color);
      break;
    case 41: 
      strip.SetPixelColor(264, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(263, this_seconds_color);
      break;
    case 42: 
      strip.SetPixelColor(263, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(262, this_seconds_color);
      break;
    case 43: 
      strip.SetPixelColor(262, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(261, this_seconds_color);
      break;
    case 44: 
      strip.SetPixelColor(261, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(260, this_seconds_color);
      break;
    case 45: 
      strip.SetPixelColor(260, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(259, this_seconds_color);
      break;
    case 46: 
      strip.SetPixelColor(259, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(258, this_seconds_color);
      break;
    case 47: 
      strip.SetPixelColor(258, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(257, this_seconds_color);
      break;
    case 48: 
      strip.SetPixelColor(257, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(256, this_seconds_color);
      break;
    case 49: 
      strip.SetPixelColor(256, black);
      strip.SetPixelColor(266, this_seconds_color); strip.SetPixelColor(255, this_seconds_color);
      break;
    case 50: 
      strip.SetPixelColor(255, black); strip.SetPixelColor(266, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(264, this_seconds_color);
      break;
    case 51: 
      strip.SetPixelColor(264, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(263, this_seconds_color);
      break;
    case 52: 
      strip.SetPixelColor(263, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(262, this_seconds_color);
      break;
    case 53: 
      strip.SetPixelColor(262, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(261, this_seconds_color);
      break;
    case 54: 
      strip.SetPixelColor(261, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(260, this_seconds_color);
      break;
    case 55: 
      strip.SetPixelColor(260, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(259, this_seconds_color);
      break;
    case 56: 
      strip.SetPixelColor(259, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(258, this_seconds_color);
      break;
    case 57: 
      strip.SetPixelColor(258, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(257, this_seconds_color);
      break;
    case 58: 
      strip.SetPixelColor(257, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(256, this_seconds_color);
      break;
    case 59: 
      strip.SetPixelColor(256, black);
      strip.SetPixelColor(265, this_seconds_color); strip.SetPixelColor(255, this_seconds_color);
      break;
    default: 
      break;
  }
 }
}

//TOP half of matrix plotted numbers
//digits (tens) for digital clock and temperature
void t_0_()
  {
for(j = 1 ; j <= 3; j++) strip.SetPixelColor(j, this_color); 
for(j = 28; j <= 29; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(32, this_color);   
strip.SetPixelColor(34, this_color); 
strip.SetPixelColor(36, this_color);   
strip.SetPixelColor(38, this_color);  
strip.SetPixelColor(62, this_color);   
strip.SetPixelColor(65, this_color);   
strip.SetPixelColor(66, this_color);   
strip.SetPixelColor(68, this_color);  
strip.SetPixelColor(72, this_color);   
strip.SetPixelColor(96, this_color);   
strip.SetPixelColor(100, this_color); 
for(j = 103; j <= 105; j++) strip.SetPixelColor(j, this_color); 
}

void t_1_()
  {
strip.SetPixelColor(3, this_color);   
for(j = 29 ; j <= 30; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(35, this_color);   
strip.SetPixelColor(37, this_color); 
strip.SetPixelColor(63, this_color);   
strip.SetPixelColor(71, this_color);   
strip.SetPixelColor(97, this_color);   
strip.SetPixelColor(105, this_color);  
}

void t_2_()
  {
for(j = 1; j <= 3; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(28, this_color); 
strip.SetPixelColor(32, this_color); 
strip.SetPixelColor(38, this_color); 
strip.SetPixelColor(63, this_color); 
strip.SetPixelColor(70, this_color); 
strip.SetPixelColor(99, this_color); 
for(j = 102; j <= 106; j++) strip.SetPixelColor(j, this_color); 
}

void t_3_()
  {
for(j = 1; j <= 3; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(28, this_color); 
strip.SetPixelColor(32, this_color); 
strip.SetPixelColor(38, this_color); 
for(j = 63; j <= 64; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(72, this_color); 
strip.SetPixelColor(96, this_color); 
strip.SetPixelColor(100, this_color); 
for(j = 103; j <= 105; j++) strip.SetPixelColor(j, this_color); 
}

void t_4_()
  {
strip.SetPixelColor(3, this_color); 
for(j = 29; j <= 30; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(35, this_color); 
strip.SetPixelColor(37, this_color); 
strip.SetPixelColor(63, this_color); 
strip.SetPixelColor(66, this_color); 
for(j = 68; j <= 72; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(97, this_color); 
strip.SetPixelColor(105, this_color);  
}

void t_5_()
  {
for(j = 0; j <= 4; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(32, this_color); 
for(j = 34; j <= 37; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(62, this_color); 
strip.SetPixelColor(72, this_color); 
strip.SetPixelColor(96, this_color); 
strip.SetPixelColor(100, this_color); 
for(j = 103; j <= 105; j++) strip.SetPixelColor(j, this_color);
}

void t_6_()
  {
for(j = 1; j <= 3; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(28, this_color); 
strip.SetPixelColor(32, this_color); 
strip.SetPixelColor(34, this_color); 
for(j = 63; j <= 66; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(68, this_color); 
strip.SetPixelColor(72, this_color); 
strip.SetPixelColor(96, this_color); 
strip.SetPixelColor(100, this_color); 
for(j = 103; j <= 105; j++) strip.SetPixelColor(j, this_color);
}

void t_7_()
  {
for(j = 0; j <= 4; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(28, this_color); 
strip.SetPixelColor(37, this_color); 
strip.SetPixelColor(64, this_color); 
strip.SetPixelColor(69, this_color); 
strip.SetPixelColor(99, this_color); 
strip.SetPixelColor(103, this_color); 
}

void t_8_()
  {
for(j = 1; j <= 3; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(28, this_color); 
strip.SetPixelColor(32, this_color); 
strip.SetPixelColor(34, this_color); 
strip.SetPixelColor(38, this_color); 
for(j = 63; j <= 65; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(68, this_color); 
strip.SetPixelColor(72, this_color); 
strip.SetPixelColor(96, this_color); 
strip.SetPixelColor(100, this_color); 
for(j = 103; j <= 105; j++) strip.SetPixelColor(j, this_color);
}

void t_9_()
  {
for(j = 1; j <= 3; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(28, this_color); 
strip.SetPixelColor(32, this_color); 
strip.SetPixelColor(34, this_color); 
strip.SetPixelColor(38, this_color); 
for(j = 62; j <= 65; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(72, this_color); 
strip.SetPixelColor(96, this_color); 
strip.SetPixelColor(100, this_color); 
for(j = 103; j <= 105; j++) strip.SetPixelColor(j, this_color);
}

//digits (ones) for digital clock and temperature
void t_0()
  {
for(j = 7; j <= 9; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(23, this_color); 
strip.SetPixelColor(26, this_color); 
strip.SetPixelColor(40, this_color); 
strip.SetPixelColor(42, this_color); 
strip.SetPixelColor(44, this_color); 
strip.SetPixelColor(56, this_color); 
for(j = 59; j <= 60; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(74, this_color); 
strip.SetPixelColor(78, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
}

void t_1()
  {
strip.SetPixelColor(9, this_color);   
strip.SetPixelColor(23, this_color); 
strip.SetPixelColor(24, this_color);   
strip.SetPixelColor(41, this_color);   
strip.SetPixelColor(43, this_color);   
strip.SetPixelColor(57, this_color);   
strip.SetPixelColor(77, this_color);   
strip.SetPixelColor(91, this_color);   
strip.SetPixelColor(111, this_color);  
  
}

void t_2()
  {
for(j = 7; j <= 9; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(26, this_color); 
strip.SetPixelColor(44, this_color); 
strip.SetPixelColor(57, this_color); 
strip.SetPixelColor(76, this_color); 
strip.SetPixelColor(93, this_color); 
for(j = 108; j <= 112; j++) strip.SetPixelColor(j, this_color); 
 
}

void t_3()
  {
for(j = 7; j <= 9; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(26, this_color); 
strip.SetPixelColor(44, this_color); 
for(j = 57; j <= 58; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(78, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
 
}

void t_4()
  {
strip.SetPixelColor(9, this_color); 
for(j = 23; j <= 24; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(41, this_color); 
strip.SetPixelColor(43, this_color); 
strip.SetPixelColor(57, this_color); 
strip.SetPixelColor(60, this_color); 
for(j = 74; j <= 78; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(91, this_color); 
strip.SetPixelColor(111, this_color); 
 

}

void t_5()
  {
for(j = 6; j <= 10; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(26, this_color); 
for(j = 40; j <= 43; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(56, this_color); 
strip.SetPixelColor(78, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
 
}

void t_6()
  {
for(j = 7; j <= 9; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(26, this_color); 
strip.SetPixelColor(40, this_color); 
for(j = 57; j <= 60; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(74, this_color); 
strip.SetPixelColor(78, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
 
}

void t_7()
  {
for(j = 6; j <= 10; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(43, this_color); 
strip.SetPixelColor(58, this_color); 
strip.SetPixelColor(75, this_color); 
strip.SetPixelColor(93, this_color); 
strip.SetPixelColor(109, this_color); 
 
}

void t_8()
  {
for(j = 7; j <= 9; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(26, this_color); 
strip.SetPixelColor(40, this_color); 
strip.SetPixelColor(44, this_color); 
for(j = 57; j <= 59; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(74, this_color); 
strip.SetPixelColor(78, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
 
}

void t_9()
  {
for(j = 7; j <= 9; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(26, this_color); 
strip.SetPixelColor(40, this_color); 
strip.SetPixelColor(44, this_color); 
for(j = 56; j <= 59; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(78, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
 
}

//BOTTOM half of matrix plotted numbers
//digits (tens) for digital clock and temperature
void b_0_()
  {
for(j = 137 ; j <= 139; j++) strip.SetPixelColor(j, this_color); 
for(j = 164; j <= 165; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(168, this_color);   
strip.SetPixelColor(170, this_color);
strip.SetPixelColor(172, this_color); 
strip.SetPixelColor(174, this_color); 
strip.SetPixelColor(198, this_color);
for(j = 201; j <= 202; j++) strip.SetPixelColor(j, this_color);   
strip.SetPixelColor(204, this_color);   
strip.SetPixelColor(208, this_color); 
strip.SetPixelColor(232, this_color);
strip.SetPixelColor(236, this_color);   
for(j = 239; j <= 241; j++) strip.SetPixelColor(j, this_color); 
 
}

void b_1_()
  {
strip.SetPixelColor(139, this_color);  
for(j = 165 ; j <= 166; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(171, this_color);   
strip.SetPixelColor(173, this_color); 
strip.SetPixelColor(199, this_color);   
strip.SetPixelColor(207, this_color);   
strip.SetPixelColor(233, this_color);   
strip.SetPixelColor(241, this_color);   
 
}

void b_2_()
  {
for(j = 137 ; j <= 139; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(164, this_color);   
strip.SetPixelColor(168, this_color); 
strip.SetPixelColor(174, this_color);   
strip.SetPixelColor(199, this_color);   
strip.SetPixelColor(206, this_color);   
strip.SetPixelColor(235, this_color);   
for(j = 238; j <= 242; j++) strip.SetPixelColor(j, this_color); 
 
}

void b_3_()
  {
for(j = 137 ; j <= 139; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(164, this_color);   
strip.SetPixelColor(168, this_color);  
strip.SetPixelColor(174, this_color);    
for(j = 199 ; j <= 200; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(208, this_color); 
strip.SetPixelColor(232, this_color);   
strip.SetPixelColor(236, this_color);   
for(j = 239; j <= 241; j++) strip.SetPixelColor(j, this_color); 
 
}

void b_4_()
  {
strip.SetPixelColor(139, this_color);   
for(j = 165 ; j <= 166; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(171, this_color);  
strip.SetPixelColor(173, this_color);    
strip.SetPixelColor(199, this_color); 
strip.SetPixelColor(202, this_color); 
for(j = 204; j <= 208; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(233, this_color);   
strip.SetPixelColor(241, this_color);  
 
}

void b_5_()
  {
for(j = 136 ; j <= 140; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(168, this_color); 
for(j = 170; j <= 173; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(198, this_color);    
strip.SetPixelColor(208, this_color); 
strip.SetPixelColor(232, this_color); 
strip.SetPixelColor(236, this_color);
for(j = 239; j <= 241; j++) strip.SetPixelColor(j, this_color); 
 
}

void b_6_()
  {
for(j = 137 ; j <= 139; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(164, this_color);   
strip.SetPixelColor(168, this_color); 
strip.SetPixelColor(170, this_color); 
for(j = 199 ; j <= 202; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(204, this_color);   
strip.SetPixelColor(208, this_color);   
strip.SetPixelColor(232, this_color);   
strip.SetPixelColor(236, this_color); 
for(j = 239; j <= 241; j++) strip.SetPixelColor(j, this_color); 
 
}

void b_7_()
  {
for(j = 136 ; j <= 140; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(164, this_color);  
strip.SetPixelColor(173, this_color);    
strip.SetPixelColor(200, this_color); 
strip.SetPixelColor(205, this_color); 
strip.SetPixelColor(235, this_color); 
strip.SetPixelColor(239, this_color); 
 
}

void b_8_()
  {
for(j = 137 ; j <= 139; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(164, this_color); 
strip.SetPixelColor(168, this_color);
strip.SetPixelColor(170, this_color);  
strip.SetPixelColor(174, this_color); 
for(j = 199; j <= 201; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(204, this_color);   
strip.SetPixelColor(208, this_color); 
strip.SetPixelColor(232, this_color);   
strip.SetPixelColor(236, this_color);   
for(j = 239; j <= 241; j++) strip.SetPixelColor(j, this_color); 
 
}

void b_9_()
  {
for(j = 137; j <= 139; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(164, this_color); 
strip.SetPixelColor(168, this_color);
strip.SetPixelColor(170, this_color);  
strip.SetPixelColor(174, this_color); 
for(j = 198; j <= 201; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(208, this_color); 
strip.SetPixelColor(232, this_color);   
strip.SetPixelColor(236, this_color);   
for(j = 239; j <= 241; j++) strip.SetPixelColor(j, this_color); 
 
}

//digits (ones) for digital clock and temperature
void b_0()
  {
for(j = 143; j <= 145; j++) strip.SetPixelColor(j, this_color);  
for(j = 158; j <= 159; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(162, this_color); 
strip.SetPixelColor(176, this_color);
strip.SetPixelColor(178, this_color);  
strip.SetPixelColor(180, this_color); 
strip.SetPixelColor(192, this_color); 
for(j = 195; j <= 196; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(210, this_color); 
strip.SetPixelColor(214, this_color); 
strip.SetPixelColor(226, this_color); 
strip.SetPixelColor(230, this_color); 
for(j = 245; j <= 247; j++) strip.SetPixelColor(j, this_color);  
 
}

void b_1()
  {
strip.SetPixelColor(145, this_color);  
for(j = 159; j <= 160; j++) strip.SetPixelColor(j, this_color);   
strip.SetPixelColor(177, this_color); 
strip.SetPixelColor(179, this_color); 
strip.SetPixelColor(193, this_color); 
strip.SetPixelColor(213, this_color); 
strip.SetPixelColor(227, this_color); 
strip.SetPixelColor(247, this_color); 
 
}

void b_2()
  {
for(j = 143; j <= 145; j++) strip.SetPixelColor(j, this_color);   
strip.SetPixelColor(158, this_color); 
strip.SetPixelColor(162, this_color); 
strip.SetPixelColor(180, this_color); 
strip.SetPixelColor(193, this_color); 
strip.SetPixelColor(212, this_color); 
strip.SetPixelColor(229, this_color); 
for(j = 244; j <= 248; j++) strip.SetPixelColor(j, this_color);   
 
}

void b_3()
  {
for(j = 143; j <= 145; j++) strip.SetPixelColor(j, this_color);   
strip.SetPixelColor(158, this_color); 
strip.SetPixelColor(162, this_color); 
strip.SetPixelColor(180, this_color); 
for(j = 193; j <= 194; j++) strip.SetPixelColor(j, this_color);   
strip.SetPixelColor(214, this_color); 
strip.SetPixelColor(226, this_color); 
strip.SetPixelColor(230, this_color); 
for(j = 245; j <= 247; j++) strip.SetPixelColor(j, this_color);   
 
}

void b_4()
  {
strip.SetPixelColor(145, this_color); 
for(j = 159; j <= 160; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(177, this_color); 
strip.SetPixelColor(179, this_color); 
strip.SetPixelColor(193, this_color); 
strip.SetPixelColor(196, this_color); 
for(j = 210; j <= 214; j++) strip.SetPixelColor(j, this_color);   
strip.SetPixelColor(227, this_color); 
strip.SetPixelColor(247, this_color); 
 
}

void b_5()
  {
for(j = 142 ; j <= 146; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(162, this_color); 
for(j = 176 ; j <= 179; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(192, this_color); 
strip.SetPixelColor(214, this_color); 
strip.SetPixelColor(226, this_color); 
strip.SetPixelColor(230, this_color); 
for(j = 245 ; j <= 247; j++) strip.SetPixelColor(j, this_color); 
   
}

void b_6()
  {
for(j = 143; j <= 145; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(158, this_color); 
strip.SetPixelColor(162, this_color); 
strip.SetPixelColor(176, this_color); 
for(j = 193; j <= 196; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(210, this_color); 
strip.SetPixelColor(214, this_color); 
strip.SetPixelColor(226, this_color); 
strip.SetPixelColor(230, this_color); 
for(j = 245; j <= 247; j++) strip.SetPixelColor(j, this_color);   
 
}

void b_7()
  {
for(j = 142; j <= 146; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(158, this_color); 
strip.SetPixelColor(179, this_color); 
strip.SetPixelColor(194, this_color); 
strip.SetPixelColor(211, this_color); 
strip.SetPixelColor(229, this_color); 
strip.SetPixelColor(245, this_color); 
 
}

void b_8()
  {
for(j = 143; j <= 145; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(158, this_color); 
strip.SetPixelColor(162, this_color); 
strip.SetPixelColor(176, this_color); 
strip.SetPixelColor(180, this_color); 
for(j = 193; j <= 195; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(210, this_color); 
strip.SetPixelColor(214, this_color); 
strip.SetPixelColor(226, this_color); 
strip.SetPixelColor(230, this_color); 
for(j = 245; j <= 247; j++) strip.SetPixelColor(j, this_color);  
 
}

void b_9()
  {
for(j = 143; j <= 145; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(158, this_color); 
strip.SetPixelColor(162, this_color); 
strip.SetPixelColor(176, this_color); 
strip.SetPixelColor(180, this_color); 
for(j = 192; j <= 195; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(214, this_color); 
strip.SetPixelColor(226, this_color); 
strip.SetPixelColor(230, this_color); 
for(j = 245; j <= 247; j++) strip.SetPixelColor(j, this_color);  
 
}

//Symbols
void t_deg() //º (degrees)
  {
for(j = 12; j <= 14; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(18, this_color); 
strip.SetPixelColor(20, this_color);     
for(j = 46; j <= 48; j++) strip.SetPixelColor(j, this_color); 

}

void b_pc() //% (percent)
  {
for(j = 155; j <= 156; j++) strip.SetPixelColor(j, this_color);  
for(j = 182; j <= 183; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(185, this_color); 
strip.SetPixelColor(188, this_color); 
strip.SetPixelColor(217, this_color); 
for(j = 221; j <= 222; j++) strip.SetPixelColor(j, this_color);  
strip.SetPixelColor(224, this_color); 
for(j = 252; j <= 253; j++) strip.SetPixelColor(j, this_color);

}


 void displayConditions(int Temp) //Outside warmth
{
      
   if( APIKEY.length() == 32 && CityID != "" && (String) ambience == "1")
   
  {
    if(Temp < cool ){  //(Temp <=5.49){
    for(j = 209; j <= 211 ; j++) strip.SetPixelColor(j, white); //AND
    for(j = 233; j <= 236 ; j++) strip.SetPixelColor(j, blue); //COLD
  }
  else if(Temp >= cool && Temp < warm){  //(Temp >=5.50 && Temp <=15.49){
    for(j = 209; j <= 211 ; j++) strip.SetPixelColor(j, white); //AND
    for(j = 229; j <= 232 ; j++) strip.SetPixelColor(j, cyan); //COOL 
  }
  else if(Temp >= warm && Temp < hot){  //(Temp >=15.50 && Temp <=21.49){
    for(j = 209; j <= 211 ; j++) strip.SetPixelColor(j, white); //AND
    for(j = 225; j <= 228 ; j++) strip.SetPixelColor(j, yellow); //WARM 
  }
  /*
  else if(Temp >=21.50 && Temp <=25.49){
    for(j = 209; j <= 211 ; j++) strip.SetPixelColor(j, white); //AND
    for(j = 225; j <= 228 ; j++) strip.SetPixelColor(j, orange); //WARM 
  }
  else if(Temp >=25.50 && Temp <=32.49){
    for(j = 209; j <= 211 ; j++) strip.SetPixelColor(j, white); //AND
    for(j = 222; j <= 224 ; j++) strip.SetPixelColor(j, orange); //HOT
  }
  */
  else if(Temp >= hot){
    for(j = 209; j <= 211 ; j++) strip.SetPixelColor(j, white); //AND
    for(j = 222; j <= 224 ; j++) strip.SetPixelColor(j, red); //HOT
  }

  if ( APIKEY.length() < 32 || APIKEY == " " || CityID == " " || (String) ambience == "0" )
      {
        for(j = 209; j <= 211 ; j++) strip.SetPixelColor(j, black); //AND
        for(j = 222; j <= 236 ; j++) strip.SetPixelColor(j, black); //clear weather strip
      }
  }
}

//Temp taken from Openweathermap API
void displayTemp(int Temp)
{
  if(Temp_mode == "cel") {
    Knock = Temp;
  }
  if(Temp_mode == "fah") {
    Knock = (Temp * 9) / 5 + 32;
  }
  
  if( APIKEY.length() == 32 || CityID != "" || (String) show_weather == "1" )
  {
    
   if(Knock == 0){
    t_0();
  }
  else if(Knock == 1){
    t_1();
  }
  else if(Knock == 2){
    t_2();
  }
  else if(Knock == 3){
    t_3();
  }
  else if(Knock == 4){
    t_4();
  }
  else if(Knock == 5){
    t_5();
  }
  else if(Knock == 6){
    t_6();
  }
  else if(Knock == 7){
    t_7();
  }
  else if(Knock == 8){
    t_8();
  }
  else if(Knock == 9){
    t_9();
  }
  else if(Knock == 10){
    t_1_();t_0();
  }
  else if(Knock == 11){
    t_1_();t_1();
  }
  else if(Knock == 12){
    t_1_();t_2();
  }
  else if(Knock == 13){
    t_1_();t_3();
  }
  else if(Knock == 14){
    t_1_();t_4();
  }
  else if(Knock == 15){
    t_1_();t_5();
  }
  else if(Knock == 16){
    t_1_();t_6();
  }
  else if(Knock == 17){
    t_1_();t_7();
  }
  else if(Knock == 18){
    t_1_();t_8();
  }
  else if(Knock == 19){
    t_1_();t_9();
  }
  else if(Knock == 20){
    t_2_();t_0();
  }
  else if(Knock == 21){
    t_2_();t_1();
  }
  else if(Knock == 22){
    t_2_();t_2();
  }
  else if(Knock == 23){
    t_2_();t_3();
  }
  else if(Knock == 24){
    t_2_();t_4();
  }
  else if(Knock == 25){
    t_2_();t_5();
  }
  else if(Knock == 26){
    t_2_();t_6();
  }
  else if(Knock == 27){
    t_2_();t_7();
  }
  else if(Knock == 28){
    t_2_();t_8();
  }
  else if(Knock == 29){
    t_2_();t_9();
  }
  else if(Knock == 30){
    t_3_();t_0();
  }
  else if(Knock == 31){
    t_3_();t_1();
  }
  else if(Knock == 32){
    t_3_();t_2();
  }
  else if(Knock == 33){
    t_3_();t_3();
  }
  else if(Knock == 34){
    t_3_();t_4();
  }
  else if(Knock == 35){
    t_3_();t_5();
  }
  else if(Knock == 36){
    t_3_();t_6();
  }
  else if(Knock == 37){
    t_3_();t_7();
  }
  else if(Knock == 38){
    t_4_();t_8();
  }
  else if(Knock == 39){
    t_4_();t_9();
  }
  else if(Knock == 40){
    t_4_();t_0();
  }
  else if(Knock == 41){
    t_4_();t_1();
  }
  else if(Knock == 42){
    t_4_();t_2();
  }
  else if(Knock == 43){
    t_4_();t_3();
  }
  else if(Knock == 44){
    t_4_();t_4();
  }
  else if(Knock == 45){
    t_4_();t_5();
  }
  else if(Knock == 46){
    t_4_();t_6();
  }
  else if(Knock == 47){
    t_4_();t_7();
  }
  else if(Knock == 48){
    t_4_();t_8();
  }
  else if(Knock == 49){
    t_4_();t_9();
  }
  else if(Knock == 50){
    t_5_();t_0();
  }
  else if(Knock == 51){
    t_5_();t_1();
  }
  else if(Knock == 52){
    t_5_();t_2();
  }
  else if(Knock == 53){
    t_5_();t_3();
  }
  else if(Knock == 54){
    t_5_();t_4();
  }
  else if(Knock == 55){
    t_5_();t_5();
  }
  else if(Knock == 56){
    t_5_();t_6();
  }
  else if(Knock == 57){
    t_5_();t_7();
  }
  else if(Knock == 58){
    t_5_();t_8();
  }
  else if(Knock == 59){
    t_5_();t_9();
  }
  else if(Knock == 60){
    t_6_();t_0();
  }
  else if(Knock == 61){
    t_6_();t_1();
  }
  else if(Knock == 62){
    t_6_();t_2();
  }
  else if(Knock == 63){
    t_6_();t_3();
  }
  else if(Knock == 64){
    t_6_();t_4();
  }
  else if(Knock == 65){
    t_6_();t_5();
  }
  else if(Knock == 66){
    t_6_();t_6();
  }
  else if(Knock == 67){
    t_6_();t_7();
  }
  else if(Knock == 68){
    t_6_();t_8();
  }
  else if(Knock == 69){
    t_6_();t_9();
  }
  else if(Knock == 70){
    t_7_();t_0();
  }
  else if(Knock == 71){
    t_7_();t_1();
  }
  else if(Knock == 72){
    t_7_();t_2();
  }
  else if(Knock == 73){
    t_7_();t_3();
  }
  else if(Knock == 74){
    t_7_();t_4();
  }
  else if(Knock == 75){
    t_7_();t_5();
  }
  else if(Knock == 76){
    t_7_();t_6();
  }
  else if(Knock == 77){
    t_7_();t_7();
  }
  else if(Knock == 78){
    t_7_();t_8();
  }
  else if(Knock == 79){
    t_7_();t_9();
  }
  else if(Knock == 80){
    t_8_();t_0();
  }
  else if(Knock == 81){
    t_8_();t_1();
  }
  else if(Knock == 82){
    t_8_();t_2();
  }
  else if(Knock == 83){
    t_8_();t_3();
  }
  else if(Knock == 84){
    t_8_();t_4();
  }
  else if(Knock == 85){
    t_8_();t_5();
  }
  else if(Knock == 86){
    t_8_();t_6();
  }
  else if(Knock == 87){
    t_8_();t_7();
  }
  else if(Knock == 88){
    t_8_();t_8();
  }
  else if(Knock == 89){
    t_8_();t_9();
  }
  else if(Knock == 90){
    t_9_();t_0();
  }
  else if(Knock == 91){
    t_9_();t_1();
  }
  else if(Knock == 92){
    t_9_();t_2();
  }
  else if(Knock == 93){
    t_9_();t_3();
  }
  else if(Knock == 94){
    t_9_();t_4();
  }
  else if(Knock == 95){
    t_9_();t_5();
  }
  else if(Knock == 96){
    t_9_();t_6();
  }
  else if(Knock == 97){
    t_9_();t_7();
  }
  else if(Knock == 98){
    t_9_();t_8();
  }
  else if(Knock == 99){
    t_9_();t_9();
  }
  t_deg();
  
  if ( APIKEY.length() < 32 || APIKEY == " " || CityID == " " || (String) show_weather == "0")
      {
        AllLedsOff();
        //for(j = 108 ; j <= 111 ; j++) strip.SetPixelColor(j, red);
        //strip.SetPixelColor(125, red);
        //strip.SetPixelColor(128, red);
        //strip.SetPixelColor(142, red);
        //strip.SetPixelColor(145, red);
        //for(j = 159 ; j <= 162 ; j++) strip.SetPixelColor(j, red);
        squidgame();
      }
  }
}

//Squid Game
void circle_()
  {
for(j = 74; j <= 77; j++) strip.SetPixelColor(j, red); 
strip.SetPixelColor(90, red);    
strip.SetPixelColor(95, red); 
strip.SetPixelColor(106, red); 
strip.SetPixelColor(113, red);
strip.SetPixelColor(123, red);    
strip.SetPixelColor(130, red); 
strip.SetPixelColor(140, red); 
strip.SetPixelColor(147, red);
strip.SetPixelColor(157, red);    
strip.SetPixelColor(164, red);  
strip.SetPixelColor(175, red); 
strip.SetPixelColor(180, red); 
for(j = 193; j <= 196; j++) strip.SetPixelColor(j, red); 
  }

void triangle_()
  {
strip.SetPixelColor(75, red);    
strip.SetPixelColor(76, red); 
strip.SetPixelColor(108, red); 
strip.SetPixelColor(111, red);
strip.SetPixelColor(141, red); 
strip.SetPixelColor(146, red);
strip.SetPixelColor(174, red);
strip.SetPixelColor(181, red);
for(j = 191; j <= 198; j++) strip.SetPixelColor(j, red); 
  }

void square_()
  {
for(j = 72; j <= 79; j++) strip.SetPixelColor(j, red); 
strip.SetPixelColor(89, red);    
strip.SetPixelColor(96, red); 
strip.SetPixelColor(106, red); 
strip.SetPixelColor(113, red); 
strip.SetPixelColor(123, red);
strip.SetPixelColor(130, red);
strip.SetPixelColor(140, red);
strip.SetPixelColor(157, red);
strip.SetPixelColor(164, red);
strip.SetPixelColor(174, red);
strip.SetPixelColor(181, red);
for(j = 191; j <= 198; j++) strip.SetPixelColor(j, red); 
  }
  
//Space invader sprites
void vader_0()
  {
strip.SetPixelColor(39, this_color);    
strip.SetPixelColor(45, this_color); 
strip.SetPixelColor(56, this_color); 
strip.SetPixelColor(60, this_color);
for(j = 73; j <= 79; j++) strip.SetPixelColor(j, this_color); 
for(j = 88; j <= 89; j++) strip.SetPixelColor(j, this_color);  
for(j = 91; j <= 93; j++) strip.SetPixelColor(j, this_color); 
for(j = 95; j <= 96; j++) strip.SetPixelColor(j, this_color); 
for(j = 105; j <= 115; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(121, this_color);
for(j = 123; j <= 129; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(131, this_color);
strip.SetPixelColor(139, this_color);
strip.SetPixelColor(141, this_color);
strip.SetPixelColor(147, this_color);
strip.SetPixelColor(149, this_color);
for(j = 158; j <= 159; j++) strip.SetPixelColor(j, this_color); 
for(j = 161; j <= 162; j++) strip.SetPixelColor(j, this_color); 
  }
  
void vader_1()
  {   
strip.SetPixelColor(21, this_color);    
strip.SetPixelColor(27, this_color); 
strip.SetPixelColor(40, this_color); 
strip.SetPixelColor(44, this_color);
strip.SetPixelColor(53, this_color);
for(j = 55; j <= 61; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(63, this_color);
for(j = 71; j <= 73; j++) strip.SetPixelColor(j, this_color); 
for(j = 75; j <= 77; j++) strip.SetPixelColor(j, this_color); 
for(j = 79; j <= 81; j++) strip.SetPixelColor(j, this_color); 
for(j = 87; j <= 97; j++) strip.SetPixelColor(j, this_color); 
for(j = 106; j <= 114; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(123, this_color);
strip.SetPixelColor(129, this_color);
strip.SetPixelColor(140, this_color);
strip.SetPixelColor(148, this_color);
  }
  
void vader_2()
  { 
strip.SetPixelColor(5, this_color);    
strip.SetPixelColor(11, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(26, this_color);
for(j = 39; j <= 45; j++) strip.SetPixelColor(j, this_color); 
for(j = 54; j <= 55; j++) strip.SetPixelColor(j, this_color); 
for(j = 57; j <= 59; j++) strip.SetPixelColor(j, this_color); 
for(j = 61; j <= 62; j++) strip.SetPixelColor(j, this_color); 
for(j = 71; j <= 81; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(87, this_color);
for(j = 89; j <= 95; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(97, this_color);
strip.SetPixelColor(105, this_color);
strip.SetPixelColor(107, this_color);
strip.SetPixelColor(113, this_color);
strip.SetPixelColor(115, this_color);
for(j = 124; j <= 125; j++) strip.SetPixelColor(j, this_color); 
for(j = 127; j <= 128; j++) strip.SetPixelColor(j, this_color); 
  }
  
void vader_3()
  {  
strip.SetPixelColor(21, this_color);    
strip.SetPixelColor(27, this_color); 
strip.SetPixelColor(40, this_color); 
strip.SetPixelColor(44, this_color);
strip.SetPixelColor(53, this_color);
for(j = 55; j <= 61; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(63, this_color);
for(j = 71; j <= 73; j++) strip.SetPixelColor(j, this_color); 
for(j = 75; j <= 77; j++) strip.SetPixelColor(j, this_color); 
for(j = 79; j <= 81; j++) strip.SetPixelColor(j, this_color); 
for(j = 87; j <= 97; j++) strip.SetPixelColor(j, this_color); 
for(j = 106; j <= 114; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(123, this_color);
strip.SetPixelColor(129, this_color);
strip.SetPixelColor(140, this_color);
strip.SetPixelColor(148, this_color);
  }
  
void vader_4()
  { 
strip.SetPixelColor(39, this_color);    
strip.SetPixelColor(45, this_color); 
strip.SetPixelColor(56, this_color); 
strip.SetPixelColor(60, this_color);
for(j = 73; j <= 79; j++) strip.SetPixelColor(j, this_color); 
for(j = 88; j <= 89; j++) strip.SetPixelColor(j, this_color); 
for(j = 91; j <= 93; j++) strip.SetPixelColor(j, this_color); 
for(j = 95; j <= 96; j++) strip.SetPixelColor(j, this_color); 
for(j = 105; j <= 115; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(121, this_color);
for(j = 123; j <= 129; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(131, this_color);
strip.SetPixelColor(139, this_color);
strip.SetPixelColor(141, this_color);
strip.SetPixelColor(147, this_color);
strip.SetPixelColor(149, this_color);
for(j = 158; j <= 159; j++) strip.SetPixelColor(j, this_color); 
for(j = 161; j <= 162; j++) strip.SetPixelColor(j, this_color); 
  }
  
void vader_5()
  { 
strip.SetPixelColor(55, this_color);    
strip.SetPixelColor(61, this_color); 
strip.SetPixelColor(74, this_color); 
strip.SetPixelColor(78, this_color);
strip.SetPixelColor(87, this_color);
for(j = 89; j <= 95; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(97, this_color);
for(j = 105; j <= 107; j++) strip.SetPixelColor(j, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
for(j = 113; j <= 115; j++) strip.SetPixelColor(j, this_color); 
for(j = 121; j <= 131; j++) strip.SetPixelColor(j, this_color); 
for(j = 140; j <= 148; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(157, this_color);
strip.SetPixelColor(163, this_color);
strip.SetPixelColor(174, this_color);
strip.SetPixelColor(182, this_color);
  }
  
void vader_6()
  { 
strip.SetPixelColor(73, this_color);    
strip.SetPixelColor(79, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color);
for(j = 107; j <= 113; j++) strip.SetPixelColor(j, this_color); 
for(j = 122; j <= 123; j++) strip.SetPixelColor(j, this_color); 
for(j = 125; j <= 127; j++) strip.SetPixelColor(j, this_color); 
for(j = 129; j <= 130; j++) strip.SetPixelColor(j, this_color); 
for(j = 139; j <= 149; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(155, this_color);
for(j = 157; j <= 163; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(165, this_color);
strip.SetPixelColor(173, this_color);
strip.SetPixelColor(175, this_color);
strip.SetPixelColor(181, this_color);
strip.SetPixelColor(183, this_color);
for(j = 192; j <= 193; j++) strip.SetPixelColor(j, this_color); 
for(j = 195; j <= 196; j++) strip.SetPixelColor(j, this_color); 
  }
  
void vader_7()
  { 
strip.SetPixelColor(89, this_color);    
strip.SetPixelColor(95, this_color); 
strip.SetPixelColor(108, this_color); 
strip.SetPixelColor(112, this_color);
strip.SetPixelColor(121, this_color);
for(j = 123; j <= 129; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(131, this_color);
for(j = 139; j <= 141; j++) strip.SetPixelColor(j, this_color); 
for(j = 143; j <= 145; j++) strip.SetPixelColor(j, this_color); 
for(j = 147; j <= 149; j++) strip.SetPixelColor(j, this_color); 
for(j = 155; j <= 165; j++) strip.SetPixelColor(j, this_color); 
for(j = 174; j <= 182; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(191, this_color);
strip.SetPixelColor(197, this_color);
strip.SetPixelColor(208, this_color);
strip.SetPixelColor(216, this_color);
  }
  
void vader_8()
  { 
strip.SetPixelColor(73, this_color);    
strip.SetPixelColor(79, this_color); 
strip.SetPixelColor(90, this_color); 
strip.SetPixelColor(94, this_color);
for(j = 107; j <= 113; j++) strip.SetPixelColor(j, this_color); 
for(j = 122; j <= 123; j++) strip.SetPixelColor(j, this_color); 
for(j = 125; j <= 127; j++) strip.SetPixelColor(j, this_color); 
for(j = 129; j <= 130; j++) strip.SetPixelColor(j, this_color); 
for(j = 139; j <= 149; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(155, this_color);
for(j = 157; j <= 163; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(165, this_color);
strip.SetPixelColor(173, this_color);
strip.SetPixelColor(175, this_color);
strip.SetPixelColor(181, this_color);
strip.SetPixelColor(183, this_color);
for(j = 192; j <= 193; j++) strip.SetPixelColor(j, this_color); 
for(j = 195; j <= 196; j++) strip.SetPixelColor(j, this_color); 
  }
  
void vader_9()
  { 
strip.SetPixelColor(55, this_color);    
strip.SetPixelColor(61, this_color); 
strip.SetPixelColor(74, this_color); 
strip.SetPixelColor(78, this_color);
strip.SetPixelColor(87, this_color);
for(j = 89; j <= 95; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(97, this_color);
for(j = 105; j <= 107; j++) strip.SetPixelColor(j, this_color); 
for(j = 109; j <= 111; j++) strip.SetPixelColor(j, this_color); 
for(j = 113; j <= 115; j++) strip.SetPixelColor(j, this_color); 
for(j = 121; j <= 131; j++) strip.SetPixelColor(j, this_color); 
for(j = 140; j <= 148; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(157, this_color);
strip.SetPixelColor(163, this_color);
strip.SetPixelColor(174, this_color);
strip.SetPixelColor(182, this_color);
  }
  
void vader_10()
  {  
strip.SetPixelColor(39, this_color);    
strip.SetPixelColor(45, this_color); 
strip.SetPixelColor(56, this_color); 
strip.SetPixelColor(60, this_color);
for(j = 73; j <= 79; j++) strip.SetPixelColor(j, this_color); 
for(j = 88; j <= 89; j++) strip.SetPixelColor(j, this_color); 
for(j = 91; j <= 93; j++) strip.SetPixelColor(j, this_color); 
for(j = 95; j <= 96; j++) strip.SetPixelColor(j, this_color); 
for(j = 105; j <= 115; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(121, this_color);
for(j = 123; j <= 129; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(131, this_color);
strip.SetPixelColor(139, this_color);
strip.SetPixelColor(141, this_color);
strip.SetPixelColor(147, this_color);
strip.SetPixelColor(149, this_color);
for(j = 158; j <= 159; j++) strip.SetPixelColor(j, this_color); 
for(j = 161; j <= 162; j++) strip.SetPixelColor(j, this_color); 
  }
  
void vader_11()
  { 
strip.SetPixelColor(21, this_color);    
strip.SetPixelColor(27, this_color); 
strip.SetPixelColor(40, this_color); 
strip.SetPixelColor(44, this_color);
strip.SetPixelColor(53, this_color);
for(j = 55; j <= 61; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(63, this_color);
for(j = 71; j <= 73; j++) strip.SetPixelColor(j, this_color); 
for(j = 75; j <= 77; j++) strip.SetPixelColor(j, this_color); 
for(j = 79; j <= 81; j++) strip.SetPixelColor(j, this_color); 
for(j = 87; j <= 97; j++) strip.SetPixelColor(j, this_color); 
for(j = 106; j <= 114; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(123, this_color);
strip.SetPixelColor(129, this_color);
strip.SetPixelColor(140, this_color);
strip.SetPixelColor(148, this_color);
  }
  
void vader_12()
  {  
strip.SetPixelColor(5, this_color);    
strip.SetPixelColor(11, this_color); 
strip.SetPixelColor(22, this_color); 
strip.SetPixelColor(26, this_color);
for(j = 39; j <= 45; j++) strip.SetPixelColor(j, this_color); 
for(j = 54; j <= 55; j++) strip.SetPixelColor(j, this_color); 
for(j = 57; j <= 59; j++) strip.SetPixelColor(j, this_color); 
for(j = 61; j <= 62; j++) strip.SetPixelColor(j, this_color); 
for(j = 71; j <= 81; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(87, this_color);
for(j = 89; j <= 95; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(97, this_color);
strip.SetPixelColor(105, this_color);
strip.SetPixelColor(107, this_color);
strip.SetPixelColor(113, this_color);
strip.SetPixelColor(115, this_color);
for(j = 124; j <= 125; j++) strip.SetPixelColor(j, this_color); 
for(j = 127; j <= 128; j++) strip.SetPixelColor(j, this_color); 
  }
  
void vader_13()
  { 
strip.SetPixelColor(21, this_color);    
strip.SetPixelColor(27, this_color); 
strip.SetPixelColor(40, this_color); 
strip.SetPixelColor(44, this_color);
strip.SetPixelColor(53, this_color);
for(j = 55; j <= 61; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(63, this_color);
for(j = 71; j <= 73; j++) strip.SetPixelColor(j, this_color); 
for(j = 75; j <= 77; j++) strip.SetPixelColor(j, this_color); 
for(j = 79; j <= 81; j++) strip.SetPixelColor(j, this_color); 
for(j = 87; j <= 97; j++) strip.SetPixelColor(j, this_color); 
for(j = 106; j <= 114; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(123, this_color);
strip.SetPixelColor(129, this_color);
strip.SetPixelColor(140, this_color);
strip.SetPixelColor(148, this_color);
}

void invaders()
{
  unsigned long currentMillis = millis();                     //  Update current time
  if((currentMillis - patternPrevious) >= patternInterval) {  //  Check for expired time
    patternPrevious = currentMillis;
    patternCurrent++;                                         //  Advance to next pattern
    AllLedsOff();
    if(patternCurrent >= 13)
      patternCurrent = 0;
  }
    switch (patternCurrent) {
      case 0:
        vader_0();
        break;
      case 1:
        vader_1();
        break;
      case 2:
        vader_2();
        break;
      case 3:
        vader_3();
        break;
      case 4:
        vader_4();
        break;
      case 5:
        vader_5();
        break;
      case 6:
        vader_6();
        break;
      case 7:
        vader_7();
        break;
      case 8:
        vader_8();
        break;
      case 9:
        vader_9();
        break;
      case 10:
        vader_10();
        break;
      case 11:
        vader_11();
        break;
      case 12:
        vader_12();
        break;
      case 13:
        vader_13();
        break;
  }
}

void squidgame()
{
  unsigned long currentMillis = millis();                     //  Update current time
  if((currentMillis - patternPrevious) >= squidInterval) {  //  Check for expired time
    patternPrevious = currentMillis;
    patternCurrent++;                                         //  Advance to next pattern
    AllLedsOff();
    if(patternCurrent >= 3)
      patternCurrent = 0;
  }
    switch (patternCurrent) {
      case 0:
        circle_();
        break;
      case 1:
        triangle_();
        break;
      case 2:
        square_();
        break;
  }
}

//Manic Miner sprites (not curently installed as a mode)
void willy_0()
  {
for(j = 2 ; j <= 5 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 27 ; j <= 30 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 35 ; j <= 40 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 61 ; j <= 63 ; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(64, this_color);      
for(j = 70 ; j <= 74 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 95 ; j <= 98 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 105 ; j <= 106 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 129 ; j <= 132 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 137 ; j <= 138 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 140 ; j <= 142 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 162 ; j <= 164 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 166 ; j <= 167 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 171 ; j <= 172 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 174 ; j <= 176 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 196 ; j <= 197 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 199; j <= 201 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 206 ; j <= 209 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 232 ; j <= 233 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 241 ; j <= 242 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 265 ; j <= 267 ; j++) strip.SetPixelColor(j, this_color); 
  }

void willy_1()
  {
for(j = 6 ; j <= 9; j++) strip.SetPixelColor(j, this_color); 
for(j = 23 ; j <= 26 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 39 ; j <= 44 ; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(57, this_color);      
for(j = 59 ; j <= 60 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 74 ; j <= 78 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 91 ; j <= 94 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 109 ; j <= 110 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 125 ; j <= 128 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 141; j <= 146 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 158 ; j <= 163 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 174 ; j <= 177 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 179 ; j <= 181 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 191; j <= 192 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 194 ; j <= 198 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 210 ; j <= 213 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 226; j <= 227; j++) strip.SetPixelColor(j, this_color); 
for(j = 229 ; j <= 231 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 243 ; j <= 244 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 246 ; j <= 248 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 259 ; j <= 261 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 263 ; j <= 265 ; j++) strip.SetPixelColor(j, this_color);
  }

void willy_2()
  {
for(j = 9 ; j <= 12; j++) strip.SetPixelColor(j, this_color); 
for(j = 20 ; j <= 23 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 42 ; j <= 47 ; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(54, this_color);      
for(j = 56 ; j <= 57 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 77 ; j <= 81 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 88 ; j <= 91 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 112 ; j <= 113 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 122 ; j <= 125 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 144; j <= 149 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 154 ; j <= 161 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 176 ; j <= 185 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 187 ; j <= 188 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 190 ; j <= 193 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 195 ; j <= 196 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 213 ; j <= 217 ; j++) strip.SetPixelColor(j, this_color); 
strip.SetPixelColor(221, this_color);    
for(j = 226; j <= 228; j++) strip.SetPixelColor(j, this_color); 
for(j = 245 ; j <= 246 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 256 ; j <= 257 ; j++) strip.SetPixelColor(j, this_color); 
for(j = 261 ; j <= 263 ; j++) strip.SetPixelColor(j, this_color); 
}
  
void manicminer()
{
  unsigned long currentMillis = millis();                     //  Update current time
  if((currentMillis - patternPrevious) >= patternInterval) {  //  Check for expired time
    patternPrevious = currentMillis;
    patternCurrent++;                                         //  Advance to next pattern
    AllLedsOff();
    if(patternCurrent >= 3)
      patternCurrent = 0;
  }
    switch (patternCurrent) {
      case 0:
        willy_0();
        break;
      case 1:
        willy_1();
        break;
      case 2:
        willy_2();
        break;
  }
}
//Global space which holds all modes.
void global()
{ 
  if(digitalRead(touch_sensor_1) == 1) //sensor is touched
    {
      delay(300);
      //add 1 to the current value of the mode variable
      mode += 1;
      if (mode >3) mode = 0;
    }
    
    switch (mode)
    {
      case 0:
      cloq_();
      break;

      case 1:
      outside_temp_();
      break;

      case 2:
      digitalclock_();
      break;

      case 3:
      anim_();
      break;
      default:
      break;
   }
}

 void cloq_()
{
  once_per_second();  //all the things we want to do once per second
  once_per_minute();  //all the things we want to do once per minute
  //once_per_hour();    //all the things we want to do once per hour
}

 void digitalclock_()
{
  once_per_second();  //all the things we want to do once per second
  once_per_digital_minute();  //all the things we want to do once per minute
  //once_per_hour();    //all the things we want to do once per hour
}

 void outside_temp_()
{
  once_per_second();  //all the things we want to do once per second
  outside_temperature();  //all the things we want to do once per minute
  //once_per_hour();    //all the things we want to do once per hour
}

 void temphum_()
{
  once_per_second();  //all the things we want to do once per second
  in_temperature_humidity();  //all the things we want to do once per minute
  //once_per_hour();    //all the things we want to do once per hour
}

 void anim_()
{
  once_per_half_second();
  animations();  //all the things we want to do once per minute
  //once_per_hour();    //all the things we want to do once per hour
}

 void temperature_()
{
  once_per_second();  //all the things we want to do once per second
  outside_temperature();  //all the things we want to do once per minute
  //once_per_hour();    //all the things we want to do once per hour
}
 void in_temperature_()
{
  once_per_second();  //all the things we want to do once per second
  in_temperature_humidity();  //all the things we want to do once per minute
  //once_per_hour();    //all the things we want to do once per hour
}

void AllLedsOff()
{
  for (j=0; j < PixelCount ;j++) strip.SetPixelColor(j, black);
}

void connectWifi()
{
  if (!wifiConnected) connectWLAN(ssid1, pass1); 
  if (!wifiConnected) connectWLAN(ssid2, pass2); 
  if (!wifiConnected) Serial.println("not connected to WIFI");
  if (wifiConnected) reconnection_allowed = true;
}

void connectWLAN(String ssid, String pass) 
{
  bool connection = false;

  Serial.println("");
  Serial.println("connect to: ");
  Serial.println(ssid);
  WiFi.begin(ssid.c_str(), pass.c_str());

  for (j=0 ; j<20 ; j++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("");
      Serial.println("connected");
      Serial.println(WiFi.localIP());
      connection = true;
      break;
    }
    strip.SetPixelColor(270, red);
    strip.Show();
    delay(100);
    strip.SetPixelColor(270, black);
    strip.Show();
    delay(500);
  }
  wifiConnected = connection;
}

void GetConfiguration()
{
  if(ee_read_string(  0) != "config_ok")  //a new ESP does not have the magic word in eeprom
  {
    Serial.println("reset Config");       //so we preset it to valid values
    ee_write_string(  0, "config_ok");

    ee_write_string( 10, "wlan_1_ssid"); //will be overwritten by web config
    ee_write_string( 30, "wlan_1_pwd");
    ee_write_string( 60, "wlan_2_ssid"); //second Wifi credentials for workshop area
    ee_write_string( 90, "wlan_2_pwd");  //no access from outside
    ee_write_string(120, "en");          //language for display de or enwlan_1_ssid
    ee_write_string(125, "green");       //color 
    ee_write_string(135, "auto");        //DST: summer, winter, auto (central Europe)
    ee_write_string(145, "1");           //summer time offset
    ee_write_string(149, "0");           //winter time offset
    ee_write_string(155, "word_clock");  //accesspoint name
    ee_write_string(170, "0");           //weather on/off
    ee_write_string(175, "");            //sensor_timer
    ee_write_string(180, "");            //motion sensor
    ee_write_string(185, "green");       //second color
    ee_write_string(195, "3");           //brightness
    ee_write_string(200, "");            //OpenweatherMap City ID
    ee_write_string(210, "");            //OpenweatherMap API Key
    ee_write_string(245, "");            //LDR Sensor
    ee_write_string(249, "");            //SENSOR mode
    ee_write_string(253, "");            //Temp mode
    ee_write_string(258, "1");           //Ambient Temperature
    ee_write_string(263, "1");           //Seconds status
    ee_write_string(266, "8");           //Cool
    ee_write_string(276, "15");          //Warm
    ee_write_string(286, "25");          //Hot
  }
  
  ssid1             = ee_read_string( 10);
  pass1             = ee_read_string( 30);
  ssid2             = ee_read_string( 60);
  pass2             = ee_read_string( 90);
  language          = ee_read_string(120);
  color             = ee_read_string(125);
  
  DST_mode          = ee_read_string(135);
  summer_offset     = ee_read_string(145).toInt();
  winter_offset     = ee_read_string(149).toInt();
  Accesspoint_Name  = ee_read_string(155);
  show_weather      = ee_read_string(170).toInt();
  sensor_timer      = ee_read_string(175).toInt();
  motionSensor      = ee_read_string(180).toInt();
  scolor            = ee_read_string(185);
  brightness        = ee_read_string(195).toInt();
  CityID            = ee_read_string(200);
  APIKEY            = ee_read_string(210);
  ldrSensor_status  = ee_read_string(245).toInt();
  SENSOR_mode       = ee_read_string(249).toInt();
  Temp_mode         = ee_read_string(253);
  ambience          = ee_read_string(258).toInt();
  seconds_status    = ee_read_string(263).toInt();
  cool              = ee_read_string(266).toInt();
  warm              = ee_read_string(276).toInt();
  hot               = ee_read_string(286).toInt();
  
  if (color == "green")   this_color = green;
  if (color == "red")     this_color = red;
  if (color == "orange")  this_color = orange;
  if (color == "yellow")  this_color = yellow;
  if (color == "blue")    this_color = blue;
  if (color == "cyan")    this_color = cyan;
  if (color == "white")   this_color = white;
  if (color == "pink")    this_color = pink;

  if (scolor == "green")   this_seconds_color = green;
  if (scolor == "red")     this_seconds_color = red;
  if (scolor == "orange")   this_seconds_color = orange;
  if (scolor == "blue")    this_seconds_color = blue;
  if (scolor == "cyan")    this_seconds_color = cyan;
  if (scolor == "white")   this_seconds_color = white;
  if (scolor == "pink")    this_seconds_color = pink;
  if (scolor == "black")   this_seconds_color = black;

  if( Accesspoint_Name == "") Accesspoint_Name = "_CLOQ_"; // in case of config error
  
}

String ee_read_string(int adr)
{
  const int MAXREAD = 32;
  char c;
  String ee_string = "";
    
  for (int j = 0; j < MAXREAD; j++)
  { 
    c = char(EEPROM.read(adr + j));
    if(c == '\0') break;
    ee_string += c; 
  } 
  return(ee_string);
}

void ee_write_string(int adr, String value)
{
  int i;
  for (i = 0; i < (value.length() +1); ++i)
    {
      EEPROM.write(adr +i, value[i]);
    }
  EEPROM.write(adr +i , '\0');
  EEPROM.commit();
}

void StartupAccesspoint()
{
  WiFi.softAPConfig(Accesspoint_IP, Accesspoint_IP, Accesspoint_netmask);
  WiFi.softAP(Accesspoint_Name.c_str());
  
  AP_running = true;
  Serial.println("Accesspoint started");
}

void shutdown_accesspoint(unsigned long runtime)
{
  if (AP_running == true)
  {
    if(millis() >  runtime)
    {
      Serial.println("Accesspoint shutdown");
      WiFi.softAPdisconnect();
      AP_running = false;
    }
  }
}

void StartupNeopixels()
{
  strip.Begin();
  strip.Show();
  Serial.println("Neopixel bus started");
}

void CalculateDST() //Central European Summer Time (CEST)
{
  if(DST_mode == "auto")
  {
    summer = false;
    int thisYear, lastSundayInMarch, lastSundayInOctober;
    String now_Time, Summer_begin, Summer_end;
    
    timeClient.update();
    setTime( timeClient.getEpochTime());
    thisYear = year();
      
    now_Time = String(year(),DEC);
    if(month() < 10) now_Time = now_Time + "0";
    now_Time = now_Time + String(month(),DEC);
    if(day() < 10) now_Time = now_Time + "0";
    now_Time = now_Time + String(day(),DEC);
    if(hour() < 10) now_Time = now_Time + "0";
    now_Time = now_Time + String(hour(),DEC);
    if(minute() < 10) now_Time = now_Time + "0";
    now_Time = now_Time + String(minute(),DEC);
    
    setTime(02, 00, 00, 31, 03, thisYear);  // set to last day in March actual year
    lastSundayInMarch = 31 - weekday() +1;    //last sunday in march
    Summer_begin = String(year(),DEC) + "03" + String(lastSundayInMarch,DEC) + "0200";
    
    setTime(02, 00, 00, 31, 10, thisYear);  // set to last day in October actual year
    lastSundayInOctober = 31 - weekday() +1;      //last sunday in october
    Summer_end = String(year(),DEC) + "10" + String(lastSundayInOctober,DEC) + "0300";
    
    if(now_Time > Summer_begin) summer = true;
    if(now_Time > Summer_end) summer = false;
  }
  if(DST_mode == "summer") summer = true;
  if(DST_mode == "winter") summer = false;
  Serial.println("DST calculated");
}

void GetNtpTime()
{
  int j;
  
  if (wifiConnected)
  {
    for (j = 0; j < 5; j++)
    {
      timeClient.update();

      setTime( timeClient.getEpochTime());
      NTPtime = (String) day() + "." + (String) month() + "." + (String) year()  + " " +  (String) hour() + ":"; 
      if(minute() < 10) NTPtime += "0";
      NTPtime += (String) minute(); 

      NTPSimplytime = (String) hour() + "." + (String) minute();
          
      if(summer)
      {
        setTime( timeClient.getEpochTime() + (summer_offset * SECS_PER_HOUR)); // summer
      }
      else
      {
        setTime( timeClient.getEpochTime() + (winter_offset * SECS_PER_HOUR)); // winter
      }
      if(year() > 2000) break;
      delay(1000);
    }
  
    if(year() < 2000)
      {
        Serial.println("NTP failure");
        last_minute = 99; // retry next minute
      }

    else
      Serial.println("NTP sync done");
      Serial.println(NTPtime);
  }
  else
    Serial.println("not connected, Time not correct");
}
     
      
void ConfigWebServer()
{
  server.on("/",              handleRoot);
  server.on("/factoryreset",  factoryreset); //call factory reset
  server.onNotFound(          handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void factoryreset()
{
  ee_write_string(  0, "resetted"); //force factory reset
  server.send(200, "text/html", restart_HTML());  
  delay(2000);
  ESP.restart();
}

void handleRoot() 
{  
  Serial.println("");

  if(server.client().localIP() == Accesspoint_IP) //we come via CLOQ's access point
  {                                               //////////////////////////////////////
    Serial.println("* handle AP Root *");
    if(server.args() > 0) AP_Posted_Data();
    server.send(200, "text/html", AP_Root_HTML());
  }
    
  if(server.client().localIP() == WiFi.localIP()) //we come via wlan
  {                                               //////////////////
    Serial.println("* handle STA Root *");
    if(server.args() > 0) STA_Posted_Data();
    server.send(200, "text/html", STA_Root_HTML());
  }
}

void AP_Posted_Data()
{
  if(server.arg("ok") == "ok")
  {
    if( (server.arg("ssid1") != "") && (server.arg("pass1") != "")) 
    {
      ee_write_string( 10, server.arg("ssid1"));
      ee_write_string( 30, server.arg("pass1"));

      wifiConnected = false;                    //force connect to wifi

      Serial.println("reset AP and restart");
      WiFi.softAPdisconnect();
      AllLedsOff();
      server.send(200, "text/html", restart_HTML());
      ESP.restart(); //restart all, reset ESP after flashing, otherwise it will not work
    }
  }
}

void STA_Posted_Data()
{
  if(server.arg("ok") == "ok")
  {
    if(server.arg("language")       != "") ee_write_string(120, server.arg("language"));
    if(server.arg("color")          != "") ee_write_string(125, server.arg("color"));
    if(server.arg("DST_mode")       != "") ee_write_string(135, server.arg("DST_mode"));
    if(server.arg("summer_offset")  != "") ee_write_string(145, server.arg("summer_offset"));
    if(server.arg("winter_offset")  != "") ee_write_string(149, server.arg("winter_offset"));
    if(server.arg("show_weather")   != "") ee_write_string(170, server.arg("show_weather"));
    if(server.arg("sensor_timer")   != "") ee_write_string(175, server.arg("sensor_timer"));
    if(server.arg("motionSensor")   != "") ee_write_string(180, server.arg("motionSensor"));
    if(server.arg("scolor")         != "") ee_write_string(185, server.arg("scolor"));
    if(server.arg("brightness")     != "") ee_write_string(195, server.arg("brightness"));
    if(server.arg("APIKEY")         != "") ee_write_string(210, server.arg("APIKEY"));
    if(server.arg("ldrSensor_status")      != "") ee_write_string(245, server.arg("ldrSensor_status"));
    if(server.arg("SENSOR_mode")    != "") ee_write_string(249, server.arg("SENSOR_mode"));
    if(server.arg("Temp_mode")      != "") ee_write_string(253, server.arg("Temp_mode"));
    if(server.arg("ambience")       != "") ee_write_string(258, server.arg("ambience"));
    if(server.arg("seconds_status") != "") ee_write_string(263, server.arg("seconds_status"));
    if(server.arg("cool") != "")           ee_write_string(266, server.arg("cool"));
    if(server.arg("warm") != "")           ee_write_string(276, server.arg("warm"));
    if(server.arg("hot") != "")            ee_write_string(286, server.arg("hot"));

    GetConfiguration(); //maybe we changed something
    last_minute = 99;   //force refresh cloq's display
        
    if((server.arg("Accesspoint_Name") != Accesspoint_Name) && (server.arg("Accesspoint_Name") != ""))
    {
      Serial.println("reset AP and restart");
      WiFi.softAPdisconnect();
      ee_write_string(155, server.arg("Accesspoint_Name"));
      AllLedsOff();
      ESP.restart(); //restart all
    }

    if((server.arg("CityID") != CityID))
    {
      Serial.println("reset CityID and restart");
      WiFi.softAPdisconnect();
      ee_write_string(200, server.arg("CityID"));
      AllLedsOff();
      ESP.restart(); //restart all
    }
  }
}

//CSS//
///////
String CSS = "*,*:before,*:after{box-sizing:border-box}body{height:100%;font-family:Tahoma,Sans-Serif;padding:0;margin:0;background:#020B16;letter-spacing:.5}a{text-decoration:none;color:#fafafa}.container{display:block;position:relative;padding:20px;margin:0 auto;height:auto}@media (min-width:750px){.container{width:600px}}h1{color:#fff;font-weight:400}ul{list-style:none;margin:0;padding:0;overflow:auto}ul li{color:#AAA;display:block;position:relative;float:left;height:100px}ul li input[type=radio]{position:absolute;visibility:hidden}ul li label{display:block;position:relative;font-weight:300;font-size:1.35em;padding:40px 35px 35px 60px;margin:5px auto;height:30px;z-index:9;cursor:pointer;-webkit-transition:all 0.25s linear}ul li:hover label{color:#FFF}ul li .check{display:block;position:absolute;border-radius:2px;height:50px;width:50px;top:30px;left:0;z-index:5;transition:border .25s linear;-webkit-transition:border .25s linear}ul li:hover .check{border:5px solid #fafafa}ul li .check::before{display:block;position:absolute;content:\"\";border-radius:100%;height:15px;width:15px;top:5px;left:5px;margin:auto;transition:background 0.25s linear;-webkit-transition:background 0.25s linear;background:transparent}input[type=radio]:checked~.coral{border:25px solid #27C3C3}.coral__{border:2px solid #27C3C3}.material-textfield{display:inline-block;height:4rem;width:100%;padding-top:2rem}.material-textfield input{background:none;border:none;box-sizing:border-box;display:block;font-weight:300;font-family:Roboto,Arial,sans-serif;font-size:1.35em;padding:.25rem .1rem;width:100%}.material-textfield input:invalid{box-shadow:none}.material-textfield input:focus{outline:none}.material-textfield label{display:inline-block;font-family:Roboto,Arial,sans-serif;font-size:0;pointer-events:none;position:relative;text-transform:uppercase;width:100%}.material-textfield label::before{content:attr(data-content);position:relative;transition:all 0.2s ease;will-change:font-size,top}.material-textfield label::after{bottom:.9rem;content:\"\";height:.3rem;left:50%;position:absolute;transition:all 0.2s ease;width:0;will-change:width,left}.material-textfield label::before,.material-textfield input[required]:focus~label::before{font-size:.75rem;top:-3.25rem}.material-textfield input:focus~label::after{left:0%;width:100%}.material-textfield input:invalid~label::before{font-size:1rem;top:-2rem}.material-textfield.coral input{border-bottom:.1rem solid #229393;color:#fff}.material-textfield.coral label::after{background:#229393}.material-textfield.coral label::before,.material-textfield.coral input[required]:focus~label::before{color:#27c3c3}.material-textfield.coral input:invalid~label::before{color:#828282}.button{border:0;outline:none;border-radius:0;padding:20px 0;font-size:1rem;font-weight:600;text-transform:uppercase;letter-spacing:.1em;background:#229393;color:#fff;transition:all 0.5s ease;-webkit-appearance:none;display:inline;width:calc(50% - 5px)}.button:hover,.button:focus{background:#27C3C3;cursor:pointer}._2{background:#01060C;color:#fff}._2:hover,.button_2:focus{background:#111}.mr-10{margin-right:10px}.mb-20{margin-bottom:20px}p{font-size:19px;line-height:180%;font-weight:300;color:#8b9cb0}p span{color:#fff}h3{color:#fff;font-size:22px}.noUi-handle:after,.noUi-handle:before{background:none}.noUi-background{box-shadow:none;background:#1a3a3f}.noUi-horizontal{height:10px;box-shadow:none}.noUi-target{border-radius:5px;border:none;box-shadow:none}.noUi-tooltip{display:block;position:absolute;border:none;border-radius:3px;background:#197373;color:#fff;padding:7px;text-align:center;white-space:nowrap;font-size:16px;margin-bottom:12px}.c-1-color{background:#0253CC}.c-2-color{background:#0FF}.c-3-color{background:#FFC700}.c-4-color{background:#FF5C00}.c-0-color{background:#1a3a3f}.c-0a-color{background:#229393}.c-cool{color:#0FF}.c-warm{color:#FFC700}.c-hot{color:#FF5C00}.special input[type=text]{width:50px;padding:6px;text-align:center;margin-bottom:20px;outline:none;border:none;color:#35CECA;font-size:20px;font-weight:700;background:none}";

void handleNotFound()
{
  String message = "Page Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  server.send(404, "text/plain", message);  
}

String restart_HTML()
{
  String
  txt  = "<html><head><meta name=viewport content=\"width=device-width, initial-scale=1\"></head>";
  txt += "<body>";
  txt += "Restart now</body></html>"; 
  return txt;
}

String AP_Root_HTML()
{
  String 
  txt  = "<html><head><meta name=viewport content=\"width=device-width, initial-scale=1\">";
  txt += "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/15.6.0/nouislider.css\">";
  txt += "<link rel=\"stylesheet\" href=\"https://drive.google.com/uc?export=download&id=1E6Vg00i0Af7N_sfDcAs40B0V5sHX4T9c\">";
  txt += "</head>";
  txt += "<body>";
  txt += "<style> " + CSS + " ";
  txt += "</style>";
  txt += "<form method=post>";
  txt += "<div class=\"container\">";
  txt += "<h1>Choose your WiFi Network</h1>";
  txt += "<p>CLOQ needs to access the internet for the time. Choose the WiFi Network you want CLOQ to connect to. CLOQ will then reset and spell out your local IP address on the matrix. If you miss it the first time round, just reboot CLOQ and make a note of the numbers and then enter the IP address into your browser (mobile or desktop) to access CLOQ Settings.</p>";
  txt += "</div>";
  txt += "<div class=\"container\">";
  txt += "<ul>";

  byte numSsid = WiFi.scanNetworks();
  String this_ssid;
  for (int thisNet = 0; thisNet<numSsid; thisNet++)
  {
    this_ssid = WiFi.SSID(thisNet);
    
    txt += "<li><input type=radio id=" + this_ssid + " name=ssid1 value =" + this_ssid + " "; if(ssid1 == this_ssid) txt += "checked"; txt += ">";
    txt += "<label for=" + this_ssid + "> " + this_ssid + " </label>";
    txt += "<div class=\"check coral coral__\"></div>";
    txt += "</li>";
  }
  txt += "</ul>";
  txt += "<div class=\"material-textfield coral\">";
  txt += "<input required type=password name=pass1 maxlength=32 value =" + pass1 + ">";
  txt += "<label data-content=\"WiFi Network Password\">WiFi Network Password";
  txt += "</label>";
  txt += "</div>";

  txt += "</div>";
  
  txt += "<div class=\"container\">";
  txt += "<button name=ok type=submit value=ok class=\"button mr-10\">Let's go!</button>";
  txt += "</div>";
  txt += "</form>";
  txt += "</body></html>";

  return txt;
}

String STA_Root_HTML()
{
  String
  txt = "<html><head><meta name=viewport content=\"width=device-width, initial-scale=1\">";
  txt += "<link rel=\"apple-touch-icon\" sizes=\"180x180\" href=\"https://drive.google.com/uc?export=download&id=1MvzTtGSpQhgiILv690Y1UTNZbBunNZez\">";
  txt += "<link rel=\"icon\" type=\"image/png\" sizes=\"32x32\" href=\"https://drive.google.com/uc?export=download&id=12zjKvIbqYfYbahEWgh2-DuqXzzft33q9\">";
  txt += "<link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"https://drive.google.com/uc?export=download&id=1oKGcV6jpzxbU-nohVOhoyIDuX1PQC2on\">";
  txt += "<link rel=\"manifest\" href=\"https://drive.google.com/uc?export=download&id=1NHSW4wPOXakPUe3fkN_1ajCUF_WZXXIX\">";
  txt += "<link rel=\"mask-icon\" href=\"https://drive.google.com/uc?export=download&id=1x2k8ic86bvUrQjTPxDRlxKDN1ytQj_b0\" color=\"#020b16\">";
  txt += "<meta name=\"msapplication-TileColor\" content=\"#00aba9\">";
  txt += "<meta name=\"theme-color\" content=\"#020B16\">";
  txt += "<style>*, *:before, *:after {box-sizing: border-box;}body {margin:0;padding:0;background: #030A15;color:#fff;font-weight:normal;}.box {width: 375px;background: #101721;padding:20px;margin: 0 auto 20px auto;}h2 {font-size:24px;}h3 {font-size:20px;}ul {list-style-type: none;padding:0;}.intro {font-size:18px;color: #53747E;padding-bottom:40px;}.subtitle {font-size:18px;font-weight:bold;}p {font-size:14px;}.tint {color: #53747E;}.grid {display: grid;gap: 10px;grid-template-columns: 1fr 1fr;}.col {padding: 0;border-radius: 10px }@media(max-width: 400px){.grid {grid-template-columns: 1fr;}}.spacer {margin-bottom: 30px;}input[type=text] {width:100%;padding:5px;}.parent {display: flex;align-items: flex-start;flex-wrap: nowrap;}.ch:nth-child(2) {margin-left:15px;margin-right:15px;}.mb {margin-bottom:20px;}button {width:100%;padding:5px;font-size:18px;}:root {--color-primary: #29AEAE;--color-subtle: #b0b0b0;}body {font-family: \"Arial\", sans-serif;}.container {margin: 0 auto;width: max-content;}.control {margin: 24px;}label {display: flex;align-items: center;border-radius: 8px;padding: 8px 0;cursor: pointer;}input[type=\"checkbox\"], input[type=\"radio\"] {appearance: none;height: 24px;width: 24px;background: #fff;border: 2px solid var(--color-subtle);margin: 0 8px 0 0;display: flex;align-items: center;justify-content: center;cursor: pointer;outline-offset: 5px;outline-color: var(--color-subtle);}input[type=\"checkbox\"]:disabled, input[type=\"radio\"]:disabled {opacity: 0.7;background: var(--color-subtle);cursor: not-allowed;}input[type=\"checkbox\"]:after, input[type=\"radio\"]:after {content: \"\";display: block;transform: scale(0);transition: 120ms transform ease-in-out;}input[type=\"checkbox\"]:checked:after, input[type=\"radio\"]:checked:after {transform: scale(1);}input[type=\"checkbox\"] {border-radius: 5px;}input[type=\"checkbox\"]:after {width: 16px;height: 16px;}input[type=\"checkbox\"]:checked {border: none;background-color: var(--color-primary, #dfdfdf);}input[type=\"checkbox\"]:checked:after {content: url(\"data:image/svg+xml, <svg viewBox='0 0 16 15' xmlns='http://www.w3.org/2000/svg'><path fill='white' d='M15.25.847a1.51 1.51 0 0 1 .405 2.096L8.106 14.11a1.944 1.944 0 0 1-2.94.329L.6 10.156a1.51 1.51 0 1 1 2.067-2.202l3.645 3.42 6.841-10.122a1.51 1.51 0 0 1 2.098-.405Z'/></svg>\");}input[type=\"radio\"] {border-radius: 20px;}input[type=\"radio\"]:after {width: 14px;height: 14px;border-radius: 7px;}input[type=\"radio\"]:checked {border-color: var(--color-primary);}input[type=\"radio\"]:checked:after {background-color: var(--color-primary);}</style>";
  txt += "</head>";
  txt += "<body>";
  txt += "<div id=\"preloader\"><div class=\"status\">&nbsp;</div></div>";
  txt += "<form method=post id=settings>";
txt += "<div class=\"box\">";
txt += "<h2>Colours</h2>";
txt += "<div class=\"grid\">";
txt += "<div class=\"col\">";
txt += "<h3>Hrs & Min</h3>";
txt += "<label><input type=radio id=white name=color value=white "; if(color == "white") txt += "checked"; txt += ">White</label>";
txt += "<label><input type=radio id=cyan name=color value=cyan "; if(color == "cyan") txt += "checked"; txt += ">Cyan</label>";
txt += "<label><input type=radio id=green name=color value=green "; if(color == "green") txt += "checked"; txt += ">Green</label>";
txt += "<label><input type=radio id=pink name=color value=pink "; if(color == "pink") txt += "checked"; txt += ">Magenta</label>";
txt += "<label><input type=radio id=orange name=color value=orange "; if(color == "orange") txt += "checked"; txt += ">Orange</label>";
txt += "<label><input type=radio id=red name=color value=red "; if(color == "red") txt += "checked"; txt += ">Red</label>";
txt += "</div>";

txt += "<div class=\"col\">";
txt += "<h3>Seconds</h3>";
txt += "<label><input type=radio id=white_s name=scolor value=white "; if(scolor == "white") txt += "checked"; txt += ">White</label>";
txt += "<label><input type=radio id=cyan_s name=scolor value=cyan "; if(scolor == "cyan") txt += "checked"; txt += ">Cyan</label>";
txt += "<label><input type=radio id=green_s name=scolor value=green "; if(scolor == "green") txt += "checked"; txt += ">Green</label>";
txt += "<label><input type=radio id=pink_s name=scolor value=pink "; if(scolor == "pink") txt += "checked"; txt += ">Magenta</label>";
txt += "<label><input type=radio id=orange_s name=scolor value=orange "; if(scolor == "orange") txt += "checked"; txt += ">Orange</label>";
txt += "<label><input type=radio id=red_s name=scolor value=red "; if(scolor == "red") txt += "checked"; txt += ">Red</label>";
txt += "</div></div>";

txt += "<input type=checkbox name=seconds_status value=" + (String) seconds_status + " "; if( (String) seconds_status == "1")  txt += "checked "; txt += ">";
txt += "<label for=seconds_status>Display Seconds</label>";
txt += "<p>If this is set to OFF, CLOQ will not display seconds.</p>";
txt += "</div>";

txt += "<div class=\"box\">";
txt += "<h2>Auto Brightness</h2>";
txt += "<div class=\"intro\">CLOQ senses the presence or absence of light, and adjusts brightness accordingly.</div>";
txt += "<div class=\"subtitle\">Refine brightness</div>";

txt += "<div style=\"display:flex;\">";
txt += "<label><input type=radio name=brightness value=10 " ; if( (String) brightness == "10") txt += "checked"; txt += ">Low</label>";
txt += "<label><input type=radio name=brightness value=25 " ; if( (String) brightness == "25") txt += "checked"; txt += ">Med</label>";
txt += "<label><input type=radio name=brightness value=50 "; if( (String) brightness == "50") txt += "checked"; txt += ">High</label>";
txt += "</div>";

txt += "<input type=hidden name=ldrSensor_status value=" + (String) ldrSensor_status + "><input type=checkbox name=ldrSensor_status onclick=\"this.previousSibling.value=1-this.previousSibling.value\" "; if( (String) ldrSensor_status == "1")  txt += "checked "; txt += ">";
txt += "<label for=seconds_status>Auto Brightness Sensor</label>";
txt += "<p>If this is set to OFF, CLOQ will not display seconds.</p>";
txt += "</div>";

txt += "<div class=\"box\">";
txt += "<h2>Motion Sensor</h2>";
txt += "<div class=\"intro\">The sensor checks for movement every 1 - 60 minutes. If movement isn't detected, CLOQs' display reverts to standby mode but remains active.</div>";
txt += "<div class=\"subtitle\">Refine sensor</div>";
txt += "<input name=sensor_timer type=text value=" + (String) sensor_timer + " min=1 max=60> mins";
txt += "<p>The lower the setting, the less energy CLOQ will consume.</p>";
txt += "<ul>";
txt += "<li>";
txt += "<input type=hidden name=motionSensor value=" + (String) motionSensor + "><input type=checkbox name=motionSensor onclick=\"this.previousSibling.value=1-this.previousSibling.value\" "; if( (String) motionSensor == "1")  txt += "checked "; txt += ">";
txt += "<label>Motion Sensor</label>";
txt += "<p>If the Motion Sensor is set to OFF, CLOQ will remain lit at all times.</p>";
txt += "</li>";
txt += "</ul>";
txt += "</div>";

txt += "<div class=\"container mb-20\" style=\"background:#0e1f22; margin-top:40px; padding-top:30px;\">";
  txt += "<img class=\"wc_icon\" src=\"https://drive.google.com/uc?export=download&id=1fMdDSsrZVIiFURolqTFtVJxcim9E2eUW\"><h2>Weather Data Service</h2>";
  txt += "<p>Signup for the <span style=\"color:#35CECA;\">FREE</span> <a href=\"https://home.openweathermap.org/users/sign_up\" target=\"_blank\">Openweathermap Service</a> to display temperature and weather conditions on CLOQ.</p>";
  
  txt += "<div class=\"material-textfield coral mb-20\">";
  txt += "<input type=text id=APIKEY name=APIKEY onblur=\"this.value=removeSpaces(this.value);\" maxlength=32 value = " + APIKEY + ">";
  txt += "<label data-content=\"API KEY\">API KEY";
  txt += "</label>";
  txt += "</div>";

  txt += "<div class=\"material-textfield coral\">";
  txt += "<input type=text name=CityID maxlength=8 onblur=\"this.value=removeSpaces(this.value);\" value = " + CityID + ">";
  txt += "<label data-content=\"6 or 7 digit City ID\">6 or 7 digit City ID";
  txt += "</label>";
  txt += "</div>";
  txt += "<p style=\"margin-top:15px; font-size:16px; line-height:170%; color: #ffffff;\">Example of 7 digit City ID: <span style=\"color:#809ca2;\"><i>https://openweathermap.org/city/</i></span>2643743</p>"; 

  txt += "<div class=\"temperature\">Currently <span style=\"color:#27c3c3;\">" + (String) Temp + "&deg;c</span></div>";

  txt += "<div id=\"with_label\">";
  txt += "<ul style=\"min-height:120px;\">";
  txt += "<h5>Temperature Scale</h5>";
  txt += "<li class=\"third\"><input type=radio id=cel name=Temp_mode value = cel "; if(Temp_mode == "cel")  txt += "checked"; txt += ">";
  txt += "<label for=\"cel\">Celsius</label>";
  txt += "<div class=\"check coral coral__\"></div>";
  txt += "</li>";

  txt += "<li class=\"third\"><input type=radio id=fah name=Temp_mode value = fah "; if(Temp_mode == "fah")  txt += "checked"; txt += ">";
  txt += "<label for=\"fah\">Fahrenheit</label>";
  txt += "<div class=\"check coral coral__\"></div>";
  txt += "</li>";
  txt += "</ul>";
  txt += "</div>";

  txt +="<div class=\"slider-wrapper\">";
  txt += "<h5 style=\"margin-bottom:80px\">Ambient Temperature</h5>";
  txt += "<div class=\"p-l-r\" id=\"multi\" style=\"margin-bottom:40px; margin-top:80px;\">";
  txt += "</div>";
  txt += "<input type=\"hidden\" name=\"cool\" id=\"cool\" value=" + (String) cool + ">";
  txt += "<input type=\"hidden\" name=\"warm\" id=\"warm\" value=" + (String) warm + ">";
  txt += "<input type=\"hidden\" name=\"hot\" id=\"hot\" value=" + (String) hot + ">";
  txt += "<div id=\"temp-slider\" class=\"m-b-20\"></div>";
  txt += "</div>";
  txt += "<p style=\"margin:0; font-size:15px; line-height:170%; color: #ffffff;\">Adjust the outside degrees (Celcius) that favour your <span class=\"c-cool\">COOL</span>, <span class=\"c-warm\">WARM</span> and <span class=\"c-hot\">HOT</span> temperature comfort levels. Anything to the left of COOL denotes being COLD.</p>";
  
  txt += "<h5 style=\"margin-bottom:20px margin-top:20px;\">Displays</h5>";
  txt += "<div class=\"switch\" style=\"position:relative;margin-top: 25px;\">";
  txt += "<input type=hidden name=ambience value=" + (String) ambience + "><input type=checkbox name=ambience onclick=\"this.previousSibling.value=1-this.previousSibling.value\" "; if( (String) ambience == "1")  txt += "checked "; txt += ">";
  txt += "<label for=\"ambience\">";
  txt += "</label>";
  txt += "<p style=\"position:absolute; display: inline; margin:0;margin-top: 0;left: 70px; top: 3px; color:#35CECA;\">Display Ambience</p>"; 
  txt += "<p style=\"font-size:15px; padding: 19px 0px 5px; padding-top:25px; margin:0; line-height:170%; color: #809ca2;\">Choose to show or hide the ambient temperature in CLOQ Mode.</p>"; 
  txt += "</div>";
  
  txt += "<div class=\"switch\" style=\"position:relative;margin-top: 25px;\">";
  txt += "<input type=hidden name=show_weather value=" + (String) show_weather + "><input type=checkbox name=show_weather onclick=\"this.previousSibling.value=1-this.previousSibling.value\" "; if( (String) show_weather == "1")  txt += "checked "; txt += ">";
  txt += "<label for=\"show_weather\">";
  txt += "</label>";
  txt += "<p style=\"position:absolute; display: inline; margin:0;left: 70px; top:3px;color:#35CECA;\">Display Weather Conditions</p>"; 
  txt += "<p style=\"font-size:15px; padding: 19px 0px 5px; padding-top:25px; margin:0; line-height:170%; color: #809ca2;\">Choose to show or hide the weather conditions in Weather Mode.</p>"; 
  txt += "</div>";
  txt += "</div>";
txt += "<div class=\"box\">";
txt += "<h2>Daylight Saving Time</h2>";
txt += "<ul>";
txt += "<li>";
txt += "<div class=\"subtitle\">Time Zone Offset</div>";
txt += "<input type=text name=summer_offset id=summer_offset value=" + (String) summer_offset + ">";
txt += "</li>";
txt += "<br>";
txt += "<li>";
txt += "<div class=\"subtitle\">Winter Offset</div>";
txt += "<input type=text name=winter_offset id=winter_offset value=" + (String) winter_offset + ">";
txt += "</li>";
txt += "</ul>";
txt += "</div>";
txt += "<div class=\"box\">";
txt += "<h2>Switch WiFi Network</h2>";
txt += "<div class=\"intro\">First of all reboot CLOQ and then select the IP Address (" + Accesspoint_Name + ") on your Device's WiFi Settings. Once you're connected, navigate to <a href=\"https://192.168.4.1\" target=\"blank_\"><span style=\"color:#ffffff;\">192.168.4.1</span></a> in your browser. This will give you options to reconnect CLOQ to a different WiFi Network.</div>";
txt += "<div class=\"spacer\">";
txt += "<label>CLOQ WIFI NETWORK</label><br>";
txt += "<input disabled type=text name=Accesspoint_Name value = " + Accesspoint_Name + ">";
txt += "</div>";

txt += "<div class=\"spacer\">";
txt += "<label>CURRENT WIFI NETWORK</label><br>";
txt += "<input disabled type=text name=ssid1 value =" + ssid1 + ">";
txt += "</div>";
txt += "</div>";
txt += "<div class=\"box\">";
txt += "<button name=ok type=submit value=ok class=\"button\">Okay</button>";
txt += "</div>";
txt += "</form>";

txt += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/noUiSlider/15.6.0/nouislider.min.js\"></script>";
  txt += "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/wnumb/1.1.0/wNumb.js\"></script>";
  txt += "<script>var tempSlider = document.getElementById('temp-slider');var cool = document.getElementById('cool');var warm = document.getElementById('warm');var hot=document.getElementById('hot');var inputs=[cool, warm, hot];var c = cool.value; var w = warm.value; var h = hot.value; noUiSlider.create(tempSlider, {start: [c, w, h],connect:[true,true,true,true], tooltips: true,format: wNumb({decimals: 0}),range: {min:0,max:40}});var connect = tempSlider.querySelectorAll('.noUi-connect');var classes = ['c-1-color', 'c-2-color', 'c-3-color', 'c-4-color'];for (var i = 0; i < connect.length; i++){connect[i].classList.add(classes[i]);}var blob = tempSlider.querySelectorAll('.noUi-handle');var classes = ['c-2-color', 'c-3-color', 'c-4-color'];for (var i = 0; i < blob.length; i++){blob[i].classList.add(classes[i]);};tempSlider.noUiSlider.on('change', function (values, handle) {if (values[handle] < 2) {tempSlider.noUiSlider.set(2);}});tempSlider.noUiSlider.on('update',function(values, handle){inputs[handle].value = values[handle];});</script>";
  
txt += "</body></html>";

  return txt;
}
