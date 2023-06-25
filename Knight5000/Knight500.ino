#include <M5StickC.h>
#include <WiFi.h> 
#include <Update.h> 
#include <pgmspace.h>
#include <time.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include "header_defs.h"
#include "WiFi.h" 
extern "C" {
#include "esp_wifi.h"
  esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
  esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
}
const char *ssid_standalone = "SmartWatch";
const char *password = ""; 
const int BTNA = 39;
const int BTNB = 37; 
char emptySSID[32];
char str[32]; 
uint8_t channelIndex = 0;
uint8_t macAddr[6];
uint8_t wifi_channel = 1;
uint32_t currentTime = 0;
uint32_t packetSize = 0;
uint32_t packetCounter = 0;
uint32_t attackTime = 0;
uint32_t packetRateTime = 0;
int color_iteration = 0; 
WiFiServer server(80); 
RTC_TimeTypeDef RTC_TimeStruct;
RTC_DateTypeDef RTC_DateStruct;
int lastDrawTime = 0;
int last_value = 0; 
int current_value = 0; 
int menuIndex = 0;
int menuCount = 6;
int WatchfacemenuIndex = 0; 
int WatchfacemenuCount = 6; 
int subMenuIndex = 0; 
int subMenuCount = 7;  
int dot[6][6][2] {
  {{35,35}},
  {{15,15},{55,55}},
  {{15,15},{35,35},{55,55}},
  {{15,15},{15,55},{55,15},{55,55}},
  {{15,15},{15,55},{35,35},{55,15},{55,55}},
  {{15,15},{15,35},{15,55},{55,15},{55,35},{55,55}},
  };

float accX = 0;
float accY = 0;
float accZ = 0;
byte red = 31;
unsigned int redcolor = red << 11;  
unsigned long targetTime = 0; 
unsigned int colorprofile(int value) {
  byte red = 0;
  byte green = 0;
  byte blue = 0;
  byte quad = value / 32;

  if (quad == 0) {
    blue = 31; 
    green = 2 * (value & 32); 
    red = 0; 
  }
  if (quad == 1) {
   blue = 31 - (value % 32); 
   green = 53; 
   red = 0;   
  }
  if (quad == 2) {
   blue = 0; 
   green = 67; 
   red = value & 32;  
  }
  if (quad == 3) {
  blue = 0; 
  green = 67 - 2 * (value % 32); 
  red = random(32);   
  }
  if (quad == 4) {
    blue = 64 - 12 * (value % 32);
    green = 13 * (value % 32); 
    red = 14 * (value % 32); 
  }
  if (quad == 5) {
    blue = 12 * (value % 32); 
    red = value * 32.13;
    green = 13 * (value % 32); 
  }
  return (red << 5) + (green << 5) + blue; 
}

unsigned long runTime = 0;
uint8_t sx[NSTARS] = {};
uint8_t sy[NSTARS] = {};
uint8_t sz[NSTARS] = {};
uint16_t x0 = 0, x1 = 0, yy0 = 0, yy1 = 0;
uint8_t za, zb, zc, zx;
inline uint8_t __attribute__((always_inline)) rng() {
  zx++;
  za = (za^zc^zx);
  zb = (zb+za);
  zc = (zc+((zb>>1)^za));
  return zc;
}
int angle = 0;
unsigned long delay_time = random(1200, 9988); 
unsigned long delay_time_2 = random(1700, 4500); 
unsigned long delay_time_3 = random(2200, 4800); 
unsigned long delayStart = 0; 
bool delayRunning = false; 

// Our menu titles:    
String menuTitle[] = { " HOME-Network", " Battery info", " Sleep Mode", " Firmware ver", " RTC TIME", " Exit" };
String submenuTitle[] = { " Hacking Tools"," ESSID RTC "," Roll-Dice"," Sleep Mode", " Help?"," Setup Menu", " Watchfaces" };
String WatchfacemenuTitle[] = {"  Basic watchface","  Starfield watchface", "  Spaceinvider watchface", "  Mandelbrot watchface", "  Yin-Yang watchface", "  fader watchface"};

// Sprite data 
TFT_eSprite img = TFT_eSprite(&M5.Lcd); 

void setup() {
targetTime = millis() + 1000; 
randomSeed(analogRead(0));   
pinMode(red_ledpin, OUTPUT); 
digitalWrite(red_ledpin, HIGH); 
M5.begin(); 
M5.Lcd.setRotation(3); 
M5.Lcd.fillScreen(BLACK);
WiFi.softAP(ssid_standalone, password);
IPAddress myIP = WiFi.softAPIP();
server.begin();
M5.IMU.Init(); 
delayStart = millis(); 
delayRunning = true; 
M5.Lcd.fillScreen(BLACK);  
}

void run_setup_menu() {
M5.Lcd.setRotation(3); 
M5.Lcd.fillScreen(BLACK); 
for (int x=0; x<1500; x++) {
setup_menu();  
M5.update(); 
 M5.Lcd.setCursor(0, 4 * 3); 
  for (int i=0; i<menuCount; i++){
    if(menuIndex==i){
      M5.Lcd.setTextColor(RED, BLACK);
      M5.Lcd.println(">"); 
      delay(10); 
    } else {
      M5.Lcd.setTextColor(BLACK, BLACK);
      M5.Lcd.println(" ");
      delay(10);   
    }
  }
  current_value=digitalRead(BTNB); 
  if(current_value != last_value) {
    if(current_value==0) {
      menuIndex++;
      menuIndex=menuIndex % menuCount;
      M5.Lcd.fillScreen(BLACK); 
      delay(30);
    }
    last_value=current_value; 
  }
  current_value=digitalRead(BTNA);
  if(current_value != last_value) {
    if(current_value==0) {
      switch(menuIndex) {
        case 0:
             watch_sync(); 
             display_time();
             break;
        case 1:
             battery_info(); 
             break;
        case 2:
    for(int x=0; x<1500; x++) {
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.drawCentreString("Sleep mode 35s", 80, 30, 1);
    delay(200);
        M5.Lcd.drawCentreString("sleep mode (35s wakeup)", 80, 30, 1);
        delay(3000);

        // close tft lcd 
        M5.Axp.SetLDO2(false);
        // 6MA
        M5.Axp.LightSleep(SLEEP_SEC(35));
        // open tft lcd
        M5.Axp.SetLDO2(true); 

        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.drawCentreString("Week up from light sleep", 80, 30, 1);
        delay(3000);

        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.drawCentreString("Press BtnB for Power-off", 80, 20, 1);
        M5.Lcd.drawCentreString("Press BtnA for,", 80, 30, 2);
        M5.Lcd.drawCentreString("Deep sleep..", 80, 40, 3);

        delay(3000);
        // 2 - 3 MA
        M5.Axp.DeepSleep(SLEEP_SEC(20));

    if(M5.BtnB.wasPressed())
    {
        // will close esp32, can`t wakeup by timer
        M5.Axp.PowerOff();
    }

    delay(20);
    }
             break;   
        case 3: 
             firmware_about(); 
             break;    
        case 4: 
             rtc_info();
             mydisplay_time(); 
             break;    
        case 5: 
             M5.Axp.PowerOff();
             break;       
        case 6: 
            break; 
      default: 
            break;                        
      }
    }
    last_value=current_value;
  }
}
}

void setup_menu() {
M5.Lcd.setCursor(2,2); 
M5.Lcd.setTextSize(1); 
M5.Lcd.setTextColor(WHITE, BLUE); 
M5.Lcd.println(" SETUP MENU ");
M5.Lcd.drawRect(1,1, 75, 9,WHITE); 
M5.Lcd.setCursor(0, 4 * 3); 
  M5.Lcd.setTextColor(WHITE, BLACK); 
for (int i=0; i < menuCount; i++) { 
  M5.Lcd.printf("%s\n", menuTitle[i].c_str());  
}
 
M5.Lcd.setTextColor(GREEN, BLACK);
M5.Lcd.setCursor(0, 80-16);
M5.Lcd.print("BUTTON A: ");
M5.Lcd.setTextColor(RED, BLACK);  
M5.Lcd.println("Mode"); 
M5.Lcd.setTextColor(GREEN, BLACK); 
M5.Lcd.print("BUTTON B: ");
M5.Lcd.setTextColor(RED, BLACK); 
M5.Lcd.println("Select"); 
M5.Lcd.drawXBitmap(88, 1, logo, logoWidth, logoHeight, random(0xFF00));
Server_client(); 
}

void loop() {
     if(color_iteration == 128) { color_iteration = 0; } 
     else {
     color_iteration++;
     }
     M5.IMU.getAccelData(&accX,&accY,&accZ);
    if (accX > 1.5 ||  accY > 1.5 ) {
     M5.Lcd.fillScreen(BLACK); 
    Accel_time();
    if (delayRunning == true) { delayRunning = false; }
    if (delayRunning == false) { delayRunning = true; }
    for (int x=0; x<7; x++) { invader_effect(); } 
    }
     M5.IMU.getAccelData(&accX,&accY,&accZ); 
    if (accZ > 1.5 || accX > 1.5 ) {
     M5.Lcd.fillScreen(BLACK);      
    for (int x=0; x<2; x++) { invader_effect(); } 
    }
    checkToggle(); 
    Server_client(); 
    submenu_logic();
}

void Server_client() {
  WiFiClient client = server.available();
  if (client) {    
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        if (c == '\n') {
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html; charset=utf-8;");
            client.println();

            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />");
            client.println("<title>M5StickC - Smart Watch -</title>"); 
            client.println("<script>");

            client.println("function clickBtn(){");
            client.println("  var jikan= new Date();");
            client.println("  document.time.year.value = jikan.getFullYear();");
            client.println("  document.time.mon.value  = jikan.getMonth()+1;");
            client.println("  document.time.day.value  = jikan.getDate();");
            client.println("  document.time.week.value  = jikan.getDay();");
            client.println("  document.time.hour.value = jikan.getHours();");
            client.println("  document.time.min.value  = jikan.getMinutes();");
            client.println("  document.time.sec.value  = jikan.getSeconds();");
            client.println("}");

            client.println("</script>");


            client.println("</head>");
            client.println("<body>");
            client.println("<center><h1>Set Date/Time</h1>"); 
            client.println("<form method=\"get\" name=\"time\">");
            client.println("<table>");        
            client.println("<tr><th>year</th><td><input type=\"text\" name=\"year\" value=\"1900\" />1990-2099</td></tr>");
            client.println("<tr><th>mon</th><td><input type=\"text\" name=\"mon\" value=\"1\" />1-12</td></tr>");
            client.println("<tr><th>day</th><td><input type=\"text\" name=\"day\" value=\"1\" />1-31</td></tr>");
            client.println("<tr><th>week</th><td><input type=\"text\" name=\"week\" value=\"0\" />0-6</td></tr>");
            client.println("<tr><th>hour</th><td><input type=\"text\" name=\"hour\" value=\"0\" />0-23</td></tr>");
            client.println("<tr><th>min</th><td><input type=\"text\" name=\"min\" value=\"0\" />0-59</td></tr>");
            client.println("<tr><th>sec</th><td><input type=\"text\" name=\"sec\" value=\"0\" />0-59</td></tr>");
            client.println("<tr><th></th><td><input type=\"button\" value=\"Get current RTC time\" onclick=\"clickBtn()\" /></td></tr>");
            client.println("<tr><th></th><td><input type=\"submit\" value=\"Adjust RTC time\" /></td></tr>");
            client.println("</table>");
            client.println("</form>");
            client.println("</body>");
            client.println("</html>");
            client.println("</center>");
            break;
          } else if (currentLine.indexOf("GET /?") == 0) {
            int pos1 = 0;
            int pos2 = 0;
            int val = 0;

            // Set RTC time
            RTC_TimeTypeDef TimeStruct;
            RTC_DateTypeDef DateStruct;

            // year
            pos1 = currentLine.indexOf('year=', pos2);
            pos2 = currentLine.indexOf('&', pos1);
            val = currentLine.substring(pos1 + 1, pos2).toInt();
            DateStruct.Year = val;

            pos1 = currentLine.indexOf('mon=', pos2);
            pos2 = currentLine.indexOf('&', pos1);
            val = currentLine.substring(pos1 + 1, pos2).toInt();
            DateStruct.Month = val;

            pos1 = currentLine.indexOf('day=', pos2);
            pos2 = currentLine.indexOf('&', pos1);
            val = currentLine.substring(pos1 + 1, pos2).toInt();
            DateStruct.Date = val;

            pos1 = currentLine.indexOf('week=', pos2);
            pos2 = currentLine.indexOf('&', pos1);
            val = currentLine.substring(pos1 + 1, pos2).toInt();
            DateStruct.WeekDay = val;

            pos1 = currentLine.indexOf('hour=', pos2);
            pos2 = currentLine.indexOf('&', pos1);
            val = currentLine.substring(pos1 + 1, pos2).toInt();
            TimeStruct.Hours = val;

            pos1 = currentLine.indexOf('min=', pos2);
            pos2 = currentLine.indexOf('&', pos1);
            val = currentLine.substring(pos1 + 1, pos2).toInt();
            TimeStruct.Minutes = val;

            pos1 = currentLine.indexOf('sec=', pos2);
            pos2 = currentLine.indexOf(' ', pos1);
            val = currentLine.substring(pos1 + 1, pos2).toInt();
            TimeStruct.Seconds = val;

            M5.Rtc.SetTime(&TimeStruct);
            M5.Rtc.SetData(&DateStruct);

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html; charset=utf-8;");
            client.println();

            client.println("<!DOCTYPE HTML>");
            client.println("<html>");
            client.println("<head>");
            client.println("<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\" />");
            client.println("</head>");
            client.println("<body>");
            client.println("You have updated the RTC clock on your M5 Smart-watch!<br />");
            client.println("[<a href=\"/\">HOME</a>]<br />");

            break;
          } else {
            currentLine = "";
          }
        } else if (c != '\r') {
          currentLine += c;
        }
      }
    }

    client.stop();
  }  
}

void subMenu() {
M5.Lcd.setCursor(2,2); 
M5.Lcd.setTextSize(1); 
M5.Lcd.setTextColor(WHITE, RED); 
M5.Lcd.println(" WATCH MENU ");
M5.Lcd.drawRect(1,1, 75, 9,WHITE); 
M5.Lcd.setCursor(0, 4 * 3); 
M5.Lcd.setTextColor(WHITE, BLACK); 
for (int i=0; i < subMenuCount; i++) { 
  M5.Lcd.printf("%s\n", submenuTitle[i].c_str());  
}
 
M5.Lcd.setTextColor(GREEN, BLACK);
M5.Lcd.setCursor(0, 80-16);
clear_all(); 
M5.Lcd.drawXBitmap(88, 1, logo, logoWidth, logoHeight, random(0xFF00));
Server_client(); 
}

void submenu_logic() {
  M5.update(); 
  M5.Lcd.setCursor(0, 4 * 3); 
  for (int i=0; i<subMenuCount; i++){
    if(subMenuIndex==i){
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.println(">"); 
      delay(10); 
    } else {
      M5.Lcd.setTextColor(BLACK, BLACK);
      M5.Lcd.println(" ");
      delay(10);   
    }
  }
  current_value=digitalRead(BTNB); 
  if(current_value != last_value) {
    if(current_value==0) {
      subMenuIndex++;
      subMenuIndex=subMenuIndex % subMenuCount;
      M5.Lcd.fillScreen(BLACK); 
      delay(30);
    }
    last_value=current_value; 
  }
  current_value=digitalRead(BTNA);
  if(current_value != last_value) {
    if(current_value==0) {
      switch(subMenuIndex) {
        case 0:
             hacker_tools(); 
             break;
        case 1:
             rtc_info(); 
             mydisplay_time(); 
             break;
        case 2:
             roll_dice(); 
             break;   
        case 3:
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(1,1); 
        M5.Lcd.setTextColor(random(0xFFF0), RED); 
        M5.Lcd.println(" Sleep Mode"); 
        M5.Lcd.println("  (35 SEC)");
        delay(1000);  
        M5.Axp.SetLDO2(false);
        M5.Axp.LightSleep(SLEEP_SEC(15)); 
        M5.Axp.DeepSleep(SLEEP_SEC(20));   
        M5.Axp.SetLDO2(true);
        M5.Lcd.fillScreen(BLACK);
             break;
        case 4:
             help_text();
             break;
        case 5:     
             menuIndex = 6;  // case 6 is a break statment. 
             run_setup_menu();
             break;       
        case 6: 
             WatchfacemenuIndex = 10; 
             run_watchface_menu(); 
             break;                
      }
    }
    last_value=current_value;
  }
  subMenu();
}

void help_text() {
M5.Lcd.fillScreen(BLACK);   
M5.Lcd.setCursor(1,20); 
M5.Lcd.setTextColor(RED, BLACK); 
M5.Lcd.print("BUTTON A: ");
M5.Lcd.setTextColor(GREEN, BLACK); 
M5.Lcd.print("MODE"); 
M5.Lcd.setCursor(1,30);
M5.Lcd.setTextColor(RED, BLACK); 
M5.Lcd.print("BUTTON B: ");
M5.Lcd.setTextColor(GREEN, BLACK);  
M5.Lcd.print("SLECT"); 
M5.Lcd.setTextColor(WHITE, BLACK); 
String text_buffer = " M5StickC smart watch quick help ... - BUTTON A: MODE - BUTTON B: SLECT - Please Note: the glitch invader is totaly random and may malfunction form time to time. LOLZ!";
const int width = 24;
for (int j=0; j<3; j++) { 
for (int offset = 0; offset < text_buffer.length(); offset++) {
String t=""; 
for (int i=0; i<width; i++) {
t+=text_buffer.charAt((offset+i)% text_buffer.length());
M5.Lcd.setCursor(1,1);
M5.Lcd.print(t); 
delay(9);    
Server_client(); 
}
}
}
M5.Lcd.fillScreen(BLACK); 
}
void rtc_info() {
M5.Lcd.setTextSize(1);   
M5.Lcd.setCursor(1,0); 
M5.Lcd.fillScreen(TFT_BLACK); 
M5.Lcd.setTextColor(WHITE, BLACK); 
M5.Lcd.setRotation(3);
M5.Lcd.println("RTC AP SET");
WiFi.softAP(ssid_standalone, password);
IPAddress myIP = WiFi.softAPIP();
server.begin();
M5.Lcd.print("AP : " );
M5.Lcd.println(ssid_standalone);
M5.Lcd.print("IP : " );
M5.Lcd.println(myIP);
delay(3900); 
M5.Lcd.fillScreen(TFT_BLACK); 
}

void Accel_time() {
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setTextSize(1); 
  M5.Lcd.setTextColor(random(0xFFF0), BLACK);

    int action = random(0, 5); 
    switch(action) {
    case 0:    
    M5.Lcd.drawXBitmap(88, 20, logo3, logoWidth, logoHeight, random(0xFFFF));
    break; 
    case 1: 
    M5.Lcd.drawXBitmap(88, 20, logo2, logoWidth, logoHeight, random(0x0FFF));
    break; 
    case 2: 
    M5.Lcd.drawXBitmap(88, 20, logo, logoWidth, logoHeight, random(0x00F0)); 
    break;  
    case 3: 
    M5.Lcd.drawXBitmap(88, 20, logo4, logoWidth, logoHeight, random(0xF0F0)); 
    break;  
    case 4:
    M5.Lcd.drawXBitmap(88, 20, logo5, logoWidth, logoHeight, random(0xF4F5)); 
    break;   
    }
     
  for (int x=0; x<1000; x++) {
    M5.Lcd.setTextSize(1); 
    M5.Lcd.setCursor(1,1); 
    static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Rtc.GetData(&RTC_DateStruct);
    M5.Lcd.printf("%04d-%02d-%02d(%s)\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, wd[RTC_DateStruct.WeekDay]);
    M5.Lcd.setTextSize(2); 
    M5.Lcd.printf("%02d:%02d:%02d\n", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    Server_client(); 
  }
  M5.Lcd.fillScreen(BLACK); 

}

void mydisplay_time() {
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setTextSize(1); 
  M5.Lcd.setTextColor(random(0xFFF0), BLACK); 
  for (int x=0; x<1000; x++) {
    M5.Lcd.setCursor(1,1); 
    static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Rtc.GetData(&RTC_DateStruct);
    M5.Lcd.printf("Data: %04d-%02d-%02d(%s)\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, wd[RTC_DateStruct.WeekDay]);
    M5.Lcd.printf("Time: %02d : %02d : %02d\n", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
  }
  M5.Lcd.fillScreen(BLACK); 
}

void watch_sync() {
  const char* ssid = "Teddylove1230";
  const char* passwd = "Teddylove1230"; 
  const char* ntpServer = "north-america.pool.ntp.org"; 
  const long gmtOffset_sec = -6; 
  const int daylightOffset_sec = 3600; 
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setTextColor(WHITE,BLACK); 
  M5.Lcd.setCursor(0,1); 
  M5.Lcd.println("Accessing home WiFi network..");
  M5.Lcd.printf("\nConnecting to,\n%s", ssid); 
  WiFi.begin(ssid,passwd); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print("."); 
      current_value=digitalRead(BTNB); 
  if(current_value != last_value) { M5.Lcd.print("<HALT>"); break; }
          current_value=digitalRead(BTNA); 
  if(current_value != last_value) { M5.Lcd.print("<HALT>"); break; }
  }
  M5.Lcd.println("\nCONNECTED!"); 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  delay(1000); 
  printlocaltime(); 
  WiFi.disconnect(true); 
  WiFi.mode(WIFI_OFF); 
  delay(20); 
}

void display_time() { 
  M5.Lcd.fillScreen(BLACK); 
  for(int x=0; x<100; x++) {
    delay(1000); 
    M5.Lcd.setTextColor(random(0xFFFF), BLACK); 
    M5.Lcd.setTextSize(1); 
    M5.Lcd.setCursor(0,25); 
    printlocaltime(); 
  }
  M5.Lcd.setTextSize(1); 
  M5.Lcd.fillScreen(BLACK); 
}

void printlocaltime() {
  struct tm timeinfo; 
  if (!getLocalTime(&timeinfo)) {
    M5.Lcd.println("Failed to obtain time"); 
    delay(2000); 
    M5.Lcd.fillScreen(BLACK); 
    return; 
  }
  M5.Lcd.println(&timeinfo, "%A, %B %d %Y");
  M5.Lcd.setTextSize(2); 
  M5.Lcd.setTextColor(WHITE, BLUE); 
  M5.Lcd.println(&timeinfo,"%H:%M:%S");
  M5.Lcd.setTextSize(1); 
}

void battery_info() {    
  if(M5.Axp.GetWarningLevel())  {
            M5.Lcd.fillScreen(WHITE);
            M5.Lcd.setCursor(0, 20, 2);
            M5.Lcd.setTextColor(RED, WHITE);
            M5.Lcd.printf("Warning: low battery");
            delay(3000);
    
                M5.Axp.SetSleep();
            
        }
        else
        {
    M5.Lcd.fillScreen(BLACK); 
    M5.Lcd.setCursor(0, 0, 1);
    M5.Lcd.setTextColor(RED, BLACK); 
    M5.Lcd.print("Battery Voltage:"); 
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("%.3fV\r\n", M5.Axp.GetBatVoltage());
    M5.Lcd.setTextColor(RED, BLACK); 
    M5.Lcd.print("APS Voltage:"); 
    M5.Lcd.setTextColor(WHITE, BLACK); 
    M5.Lcd.printf("%.3fV\r\n", M5.Axp.GetAPSVoltage());
    M5.Lcd.setTextColor(RED, BLACK); 
    M5.Lcd.print("Battery Level:"); 
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("%d\r\n", M5.Axp.GetWarningLevel());
    delay(1500); 
    M5.Lcd.fillScreen(BLACK); 
    }
}

void firmware_about() {
             M5.Lcd.fillScreen(BLACK);
             M5.Lcd.setCursor(2,2); 
             M5.Lcd.setTextSize(1); 
             M5.Lcd.setTextColor(WHITE, BLUE); 
             M5.Lcd.print("Firmware version 2.9\n");
             M5.Lcd.setTextColor(BLUE, BLACK); 
             M5.Lcd.print("OS NAME:");
             M5.Lcd.setTextColor(YELLOW, BLACK);  
             M5.Lcd.print("Knight5000\n"); 
             M5.Lcd.setTextColor(BLUE, BLACK); 
             M5.Lcd.print("NTP POOL SERVER NAME:\n");
             M5.Lcd.setTextColor(YELLOW, BLACK);  
             M5.Lcd.print("north-america.pool.ntp.org\n");
             M5.Lcd.setTextColor(BLUE, BLACK);  
             M5.Lcd.print("Stand-Alone Server ESSID:\n");
             M5.Lcd.setTextColor(YELLOW, BLACK);  
             M5.Lcd.print("SmartWatch\n");
             M5.lcd.setTextColor(BLUE, BLACK); 
             IPAddress myIP = WiFi.softAPIP();
             M5.Lcd.print("IP : " );
             M5.Lcd.setTextColor(YELLOW, BLACK); 
             M5.Lcd.println(myIP);
             M5.Lcd.setTextColor(WHITE, RED); 
             M5.Lcd.print("HACKING EDITION"); 
             delay(longdelay); 
             M5.Lcd.fillScreen(BLACK); 
             for(int x=0; x<20; x++) {
              Server_client(); 
              M5.Lcd.drawXBitmap(88, 1, logo2, logoWidth, logoHeight, BLACK);
              M5.Lcd.drawXBitmap(88, 1, logo, logoWidth, logoHeight, random(0xFF00));
              delay(240); 
              M5.Lcd.drawXBitmap(88, 1, logo, logoWidth, logoHeight, BLACK);
              M5.Lcd.drawXBitmap(88, 1, logo2, logoWidth, logoHeight, random(0xFF00));
              delay(250); 
             }
             delay(1000); 
             M5.Lcd.fillScreen(BLACK); 
}

void roll_dice() {
  M5.Lcd.setRotation(3); 
  M5.Lcd.fillScreen(TFT_GREEN);  
  M5.Lcd.setTextColor(TFT_WHITE);  // Adding a background colour erases previous text automatically
  M5.Lcd.setCursor(10, 30);  
  M5.Lcd.setTextSize(3);
  M5.Lcd.print("SHAKE ME");  
      while(1) {  // pause break at shake 
    M5.IMU.getAccelData(&accX,&accY,&accZ);
    if (accX > 1.5 ||  accY > 1.5 ) {
      break;
    }
  }
    M5.Lcd.setRotation(1); 
    M5.Lcd.fillScreen(TFT_BLACK);
    int number = random(0, 6);
    draw_dice(5,5,number);
    number = random(0, 6);
    draw_dice(85,5,number);
    while(1) {  // pause break on button M5 push 
            current_value=digitalRead(BTNB); 
  if(current_value != last_value) { break; } 
  }
M5.Lcd.setRotation(3); 
M5.Lcd.fillScreen(BLACK); 
}

void draw_dice(int16_t x, int16_t y, int n) {
  M5.Lcd.fillRect(x, y, 73, 73, random(0xFFF0));
  M5.Lcd.fillRect(x, y, 70, 70, WHITE);  
  for(int d = 0; d < 6; d++) {
    if (dot[n][d][0] > 0) {
        M5.Lcd.fillCircle(x+dot[n][d][0], y+dot[n][d][1], DOT_SIZE, random(0xFFF0));
    }
  }  
}

void invader_effect() {
    int action = random(0, 4);
    int wee = random(1, 41);
    int colors = colorprofile((12*color_iteration+64)%128);   
    switch(action) {
    case 0:
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo3, logoWidth, logoHeight, colors);  
    delay(wee); 
    break; 
    case 1: 
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo2, logoWidth, logoHeight, colors);  
    delay(wee); 
    break; 
    case 2: 
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo, logoWidth, logoHeight, colors); 
    delay(wee); 
    break;  
    case 3: 
    M5.Lcd.drawXBitmap(88, 1, logo4, logoWidth, logoHeight, colors); 
    delay(wee); 
    break;  
    case 4:
    M5.Lcd.drawXBitmap(88, 1, logo5, logoWidth, logoHeight, colors); 
    delay(wee); 
    break;  
    default: 
    clear_all(); 
    break; 
    }

}

void invader_sec2() {    
    int action = random(0, 4);
    int wee = random(1, 41);
    int colors = colorprofile((12*color_iteration+64)%129);   
 
    switch(action) {   
    case 0: 
    clear_all();
    M5.Lcd.drawXBitmap(88, 1, logo6, logoWidth, logoHeight, colors); 
    delay(wee); 
    break;    
    case 1: 
    M5.Lcd.drawXBitmap(88, 1, logo7, logoWidth, logoHeight, colors); 
    delay(wee); 
    break; 
    case 2:
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo8, logoWidth, logoHeight, colors); 
    delay(wee);     
    break; 
    case 3: 
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo9, logoWidth, logoHeight, colors); 
    delay(wee);     
    break; 
    case 4: 
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo10, logoWidth, logoHeight, colors); 
    delay(wee);
    break;
    default: 
    clear_all(); 
    break;
    }

}

void invader_sec3() {
    int action = random(0, 4);
    int wee = random(1, 41);
   
    int colors = colorprofile((12*color_iteration+64)%130);   
      
    switch(action) { 
    case 0:   
    clear_all();
    M5.Lcd.drawXBitmap(88, 1, logo11, logoWidth, logoHeight, colors); 
    delay(wee);
    break;
    case 1:   
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo12, logoWidth, logoHeight, colors); 
    delay(wee);
    break; 
    case 2: 
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo11, logoWidth, logoHeight, colors); 
    delay(wee);  
    M5.Lcd.drawXBitmap(88, 1, logo10, logoWidth, logoHeight, colors); 
    delay(wee);  
    break;
    case 3: 
    clear_all();    
    M5.Lcd.drawXBitmap(88, 1, logo10, logoWidth, logoHeight, colors); 
    delay(wee);
    break; 
    case 4: 
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo11, logoWidth, logoHeight, colors); 
    delay(wee);
    break;
    
    default: 
    clear_all();   
    M5.Lcd.drawXBitmap(88, 1, logo10, logoWidth, logoHeight, colors); 
    delay(wee); 
    clear_all(); 
    M5.Lcd.drawXBitmap(88, 1, logo11, logoWidth, logoHeight, colors); 
    delay(wee);
    break;
    }
}
   
void clear_all() {
          for (int x=0; x<12; x++) { 
          M5.Lcd.drawXBitmap(88, 1, logo+x, logoWidth, logoHeight, BLACK);        
          }        
}

void checkToggle() {
  if (delayRunning && ((millis() - delayStart) >= delay_time)) {
    delayStart += delay_time; 
    invader_effect(); 
  }
  if (delayRunning && ((millis() - delayStart) >= delay_time_2)) {
    delayStart += delay_time_2;
    invader_sec2();
  }
    if (delayRunning && ((millis() - delayStart) >= delay_time_3)) {
    delayStart += delay_time_3;
    invader_sec3();
  }

}

void watchface_menu_logic() {
  M5.update(); 
  M5.Lcd.setCursor(0, 4 * 3); 
  for (int i=0; i<WatchfacemenuCount; i++){
    if(WatchfacemenuIndex==i){
    int RGB_profile = random(0, 3); 
    switch(RGB_profile) {
    case 0: 
    M5.Lcd.setTextColor(RED, BLACK); 
    break;
    case 1: 
    M5.Lcd.setTextColor(BLUE, BLACK); 
    break;
    case 2: 
    M5.Lcd.setTextColor(GREEN, BLACK); 
    break;     
    default: 
      M5.Lcd.setTextColor(YELLOW, BLACK);
      break;
    }
      M5.Lcd.println(">>"); 
      delay(10); 
    } else {
      M5.Lcd.setTextColor(BLACK, BLACK);
      M5.Lcd.println(" ");
      delay(10);   
    }
  }
  current_value=digitalRead(BTNB); 
  if(current_value != last_value) {
    if(current_value==0) {
      WatchfacemenuIndex++;
      WatchfacemenuIndex = WatchfacemenuIndex % WatchfacemenuCount;
      M5.Lcd.fillScreen(BLACK); 
      delay(30);
    }
    last_value=current_value; 
  }
  current_value=digitalRead(BTNA);
  if(current_value != last_value) {
    if(current_value==0) {
      switch(WatchfacemenuIndex) {
        case 0:
             basic_time(); 
             break;
        case 1:
             stars_time(); 
             break;
        case 2:
             glitch_time(); 
             break;   
        case 3: 
             Mandle_time(); 
             break; 
        case 4: 
             yinyang_time(); 
             break;           
        case 5: 
             xfactor_time();      
             break;   
      }
    }
    last_value=current_value;
  }
  draw_watchface_menu(); 
}

void run_watchface_menu() {
  for (int x=0; x<1000; x++) {
  Server_client(); 
  watchface_menu_logic(); delay(50); 
  Server_client(); 
  }
  M5.Lcd.fillScreen(BLACK); 
}

void draw_watchface_menu() {
M5.Lcd.setRotation(3);
M5.Lcd.fillScreen(BLACK); 
M5.Lcd.setTextColor(WHITE, RED);   
M5.Lcd.setCursor(2,2); 
M5.Lcd.setTextSize(1); 
M5.Lcd.fillRect(2,2, 50, 8, RED); 
M5.Lcd.setTextColor(WHITE, RED); 
M5.Lcd.println("WATCH MENU CONTINUED");
M5.Lcd.drawRect(1,1, 122, 9, WHITE);
M5.Lcd.setCursor(0, 4 * 3); 
M5.Lcd.setTextColor(WHITE, BLACK); 
for (int i=0; i < WatchfacemenuCount; i++) { 
  M5.Lcd.printf("%s\n", WatchfacemenuTitle[i].c_str());  
}
}

void basic_time() {
  M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setTextSize(1); 
  M5.Lcd.setTextColor(random(0xFFF0), BLACK); 
  for (int x=0; x<10; x++) {
    M5.Lcd.fillScreen(BLACK); 
    M5.Lcd.setTextSize(2); 
    M5.Lcd.setTextColor(random(0xFFF0), BLACK);
    M5.Lcd.setCursor(1,1); 
    static const char *wd[7] = {"Sunday", "Monday", "Tuesday", "Wedsday", "Thrsday", "Friday", "Saturday"};
    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Rtc.GetData(&RTC_DateStruct);
    for (int x=0; x<1000; x++) {
    M5.Lcd.setCursor(1,1);   
    M5.Lcd.printf("%02d\n", RTC_DateStruct.Year); 
    M5.Lcd.setCursor(1,20); 
    M5.Lcd.printf(" %02d / %02d", RTC_DateStruct.Month, RTC_DateStruct.Date); 
    M5.Lcd.printf("\n%s\n",wd[RTC_DateStruct.WeekDay]);
    M5.Lcd.drawLine(M5.Lcd.height(), M5.Lcd.width(), random(1, 260), random (1, 260), random(0xFFF0));
    M5.Lcd.drawLine(M5.Lcd.width(), M5.Lcd.height(), random(1, 360), random (1, 360), random(0xFFF0));  
    } 
    M5.Lcd.setTextColor(random(0xFFFF), BLACK);    
    M5.Lcd.fillScreen(BLACK);
    for (int x=0; x<1000; x++) {
    M5.Lcd.setCursor(1,30);        
    M5.Lcd.drawLine(M5.Lcd.width(), M5.Lcd.height(), random(1, 100), random (1, 100), random(0xFFFF));  
     M5.Lcd.setTextSize(3);  
    M5.Lcd.printf("%02d:%02d.%02d\n", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
    if (x == 100) { M5.Lcd.fillScreen(BLACK); }
    if (x == 500) { M5.Lcd.fillScreen(BLACK); }
    if (x == 800) { M5.Lcd.fillScreen(BLACK); }
    if (x == 900) { M5.Lcd.fillScreen(BLACK); }
    }
        Server_client();  
  }
  M5.Lcd.fillScreen(BLACK); 
}

void stars_time() {
  za = random(256);
  zb = random(256);
  zc = random(256);
  zx = random(256);
  M5.Lcd.setRotation(3);
for (int x=0; x<4000; x++) {   
      Server_client(); 
      M5.Lcd.fillScreen(TFT_BLACK);
      M5.Lcd.setTextColor(random(0xFFF0), BLACK);  
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(4);  
      M5.Lcd.printf("%02d:", RTC_TimeStruct.Hours);
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
   
    unsigned long t0 = micros();
  uint8_t spawnDepthVariation = 256;
  for(int i = 0; i < NSTARS; ++i)
  {
    if (sz[i] <= 1)
    {
      sx[i] = 160 - 120 + rng();
      sy[i] = rng();
      sz[i] = spawnDepthVariation--;
    }
    else
    {
      int old_screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
      int old_screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 190;
   
      M5.Lcd.drawPixel(old_screen_x, old_screen_y,TFT_BLACK);
      M5.Lcd.drawPixel(old_screen_x, old_screen_y, TFT_BLACK);  
      sz[i] -= 2;
      if (sz[i] > 1)
      {
       int screen_x = ((int)sx[i] - 160) * 256 / sz[i] + 160;
       int screen_y = ((int)sy[i] - 120) * 256 / sz[i] + 190;
        if (screen_x >= 0 && screen_y >= 0 && screen_x < 190 && screen_y < 160)
        {
          uint8_t r, g, b;
          r = g = b = random(255) - sz[i];
          M5.Lcd.drawPixel(screen_x, screen_y, M5.Lcd.color565(r,g,b));
          M5.Lcd.setTextColor(M5.Lcd.color565(r,g,b), BLACK); 
        }
        else
          sz[i] = 0;
      }
    }
  }
  unsigned long t1 = micros();
  Server_client(); 
}
M5.Lcd.fillScreen(BLACK);
}

void glitch_time() {
   M5.Lcd.fillScreen(BLACK); 
  M5.Lcd.setTextSize(1); 
  M5.Lcd.setTextColor(random(0xFFF0), BLACK); 
  for (int x=0; x<100; x++) {
    Server_client(); 
    M5.Lcd.fillScreen(BLACK); 
    M5.Lcd.setTextSize(1); 
    M5.Lcd.setTextColor(random(0xFFF0), BLACK);
    M5.Lcd.setCursor(1,1); 
    static const char *wd[7] = {"Sunday", "Monday", "Tuesday", "Wedsday", "Thrsday", "Friday", "Saturday"};
    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Rtc.GetData(&RTC_DateStruct);
    M5.Lcd.printf("%02d\n", RTC_DateStruct.Year); 
    M5.Lcd.setCursor(1,20); 
    M5.Lcd.printf(" %02d/%02d", RTC_DateStruct.Month, RTC_DateStruct.Date); 
    M5.Lcd.printf(" %s\n",wd[RTC_DateStruct.WeekDay]);
    M5.Lcd.setCursor(1,40);
    M5.Lcd.setTextColor(random(0xFFFF), BLACK);    
    M5.Lcd.setTextSize(3);  
    M5.Lcd.printf("%02d:%02d", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes);
  int wee = random(1, 100);  
  int effects_slect = random(0, 3); 
  switch(effects_slect) {
  case 0: 
  clear_all(); 
  invader_sec2(); 
  delay(wee); 
  break;   
  case 1: 
  clear_all(); 
  invader_sec3();
  delay(wee);  
  break;
  case 2: 
  clear_all(); 
  invader_effect();
  delay(wee);  
  break;
  }
  }
  M5.Lcd.fillScreen(BLACK); 
}

void Mandle_time() {
   runTime = millis(); 
  randomSeed(millis());
  int rz = random(100, 560);
  randomSeed(rz);
  for (int x=0; x<1000; x++) {
  for (int px = 1; px < 320; px++)
  {
    for (int py = 0; py < 240; py++)
    {
      float x0 = (map(px, 0, rz, -250000/2, -242500/2)) / 100000.0; //scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
      float yy0 = (map(py, 0,rz , -75000/4, -61000/4)) / 100000.0; //scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
      float xx = 0.0;
      float yy = 0.0;
      int iteration = 0;
      int max_iteration = 289;
      while ( ((xx * xx + yy * yy) < 4)  &&  (iteration < max_iteration) )
      {
        float xtemp = xx * xx - yy * yy + x0;
        yy = 2 * xx * yy + yy0;
        xx = xtemp;
        iteration++;
      } 
      int altColor = rainbow((4*iteration+70)%160);
      int color = rainbow((3*iteration+64)%128);
      yield();
      M5.Lcd.drawPixel(px,py, color);
       yield();
       M5.Lcd.drawPixel(py, px, altColor);
       yield(); 
      M5.Lcd.drawPixel(py,py+px, color); 
      M5.Lcd.drawPixel(px+py,px, color);  
    }
  }
      M5.Lcd.setTextColor(WHITE, BLACK);  
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(4);  
      M5.Lcd.fillRect(5,15, 144, 38, BLACK); 
      M5.Lcd.printf("%02d:", RTC_TimeStruct.Hours);
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
      M5.Lcd.drawRect(1,15, 150, 37, WHITE); 
      delay(988); 
  swipe_Mandlebrot();
      M5.Lcd.setTextColor(WHITE, BLACK);  
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(4);  
      M5.Lcd.fillRect(5,15, 144, 38, BLACK); 
      M5.Lcd.printf("%02d:", RTC_TimeStruct.Hours);
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
      M5.Lcd.drawRect(1,15, 150, 37, WHITE); 
      delay(988); 
  swipe_Mandlebrot2();    
      M5.Lcd.setTextColor(WHITE, BLACK);  
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(4);  
      M5.Lcd.fillRect(5,15, 144, 38, BLACK); 
      M5.Lcd.printf("%02d:", RTC_TimeStruct.Hours);
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
      M5.Lcd.drawRect(1,15, 150, 38, WHITE); 
      delay(988); 
  }
}
void swipe_Mandlebrot2() {
  int rz = random(256,560);
  randomSeed(rz); 
  rz = random(256,560);
  for (int px = 1; px < 320; px++)
  {
    for (int py = 0; py < 240; py++)
    {
      float x0 = (map(px, 0, rz, -250000/2, -242500/2)) / 100000.0; //scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
      float yy0 = (map(py, 0,rz , -75000/4, -61000/4)) / 100000.0; //scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
      float xx = 0.0;
      float yy = 0.0;
      int iteration = 5;
      int max_iteration = 256;
      while ( ((xx * xx + yy * yy) < 4)  &&  (iteration < max_iteration) )
      {
        float xtemp = xx * xx - yy * yy + x0;
        yy = 2 * xx * yy + yy0;
        xx = xtemp;
        iteration++;
      } 
      int color = rainbow(2^(3*iteration+74)%122);
      yield();
      M5.Lcd.drawPixel(py,px, color);  
            yield(); 
      M5.Lcd.drawPixel(py,py+px, color); 
      M5.Lcd.drawPixel(px+py,px, color);  
    }
  }
      M5.Lcd.setTextColor(WHITE, BLACK);  
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(4);  
      M5.Lcd.fillRect(5,15, 144, 38, BLACK); 
      M5.Lcd.printf("%02d:", RTC_TimeStruct.Hours);
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
      M5.Lcd.drawRect(1,15, 150, 38, WHITE); 
      delay(988); 
  for (int px = 1; px < 320; px++)
  {
    for (int py = 0; py < 240; py++)
    {
      float x0 = (map(px, 0, rz, -250000/2, -242500/2)) / 100000.0; //scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
      float yy0 = (map(py, 0,rz , -75000/4, -61000/4)) / 100000.0; //scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
      float xx = 0.0;
      float yy = 0.0;
      int iteration = 0;
      int max_iteration = 289;
      while ( ((xx * xx + yy * yy) < 4)  &&  (iteration < max_iteration) )
      {
        float xtemp = xx * xx - yy * yy + x0;
        yy = 2 * xx * yy + yy0;
        xx = xtemp;
        iteration++;
      } 
      int altColor = rainbow((4*iteration+70)%160);
      yield();
       M5.Lcd.drawPixel(py, px, altColor);
      yield(); 
      M5.Lcd.drawPixel(py,py+px, altColor); 
      M5.Lcd.drawPixel(px+py,px, altColor);  
    }
  }
     
      M5.Lcd.setTextColor(WHITE, BLACK);  
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(4);  
      M5.Lcd.fillRect(5,15, 144, 38, BLACK); 
      M5.Lcd.printf("%02d:", RTC_TimeStruct.Hours);
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
      M5.Lcd.drawRect(1,15, 150, 38, WHITE); 
      delay(988); 
  for (int px = 1; px < 320; px++)
  {
    for (int py = 0; py < 240; py++)
    {
      float x0 = (map(px, 0, rz, -250000/2, -242500/2)) / 100000.0; //scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
      float yy0 = (map(py, 0,rz , -75000/4, -61000/4)) / 100000.0; //scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
      float xx = 0.0;
      float yy = 0.0;
      int iteration = 0;
      int max_iteration = 1220;
      while ( ((xx * xx + yy * yy) < 4)  &&  (iteration < max_iteration) )
      {
        float xtemp = xx * xx - yy * yy + x0;
        yy = 2 * xx * yy + yy0;
        xx = xtemp;
        iteration++;
      } 
      int color = rainbow((5*iteration+64)%180);
      yield();
      M5.Lcd.drawPixel(px,py, color);
      yield(); 
      M5.Lcd.drawPixel(py,py+px, color); 
      M5.Lcd.drawPixel(px+py,px, color);  
    }
  }
      M5.Lcd.setTextColor(WHITE, BLACK);  
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(4);  
      M5.Lcd.fillRect(5,15, 144, 38, BLACK); 
      M5.Lcd.printf("%02d:", RTC_TimeStruct.Hours);
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
      M5.Lcd.drawRect(1,15, 150, 38, WHITE); 
      delay(988);    
}

void swipe_Mandlebrot() {
  int rz = random(256,560);
  randomSeed(rz); 
  rz = random(256,560);
  for (int px = 1; px < 220; px++)
  {
    for (int py = 0; py < 220; py++)
    {
       float x0 = (map(px, 0, rz, -250000/2, -242500/2)) / 100000.0; //scaled x coordinate of pixel (scaled to lie in the Mandelbrot X scale (-2.5, 1))
      float yy0 = (map(py, 0,rz , -75000/4, -61000/4)) / 100000.0; //scaled y coordinate of pixel (scaled to lie in the Mandelbrot Y scale (-1, 1))
    
      float xx = 0.0;
      float yy = 0.0;
      int iteration = 0;
      int max_iteration = 249;
      while ( ((xx * xx + yy * yy) < 4)  &&  (iteration < max_iteration) )
      {
        float xtemp = xx * xx - yy * yy + x0;
        yy = 2 * xx * yy + yy0;
        xx = xtemp;
        iteration++;
      } 
      int color = rainbow((5*iteration+64)%180);
      yield();
      M5.Lcd.drawPixel(px,py, color);
      M5.Lcd.drawPixel(py,px, color); 
      yield(); 
      M5.Lcd.drawPixel(py,py+px, color); 
      M5.Lcd.drawPixel(px+py,px, color);    
    }
  }
  
}

unsigned int rainbow(int value)
{

  byte red = 0; 
  byte green = 0;
  byte blue = 0;
  
  byte quadrant = value / 32;

  if (quadrant == 0) {
    blue = 31;
    green = 2 * (value % 32);
    red = 0;
  }
  if (quadrant == 1) {
    blue = 31 - (value % 32);
    green = 63;
    red = 0;
  }
  if (quadrant == 2) {
    blue = 0;
    green = 63;
    red = value % 32;
  }
  if (quadrant == 3) {
    blue = 0;
    green = 63 - 2 * (value % 32);
    red = 31;
  }
  if (quadrant == 4) {
    blue = 53 - 2 * (value % 32); 
    green = 0; 
    red = value %32; 
  }
  if (quadrant == 5) {
   blue = 0; 
   green = value++; 
   red = value+32;  
  }
  if (quadrant == 6) {
   blue = value++;
   green = value++; 
   red = 2*(value % 32);  
  }
  return (red << 11) + (green << 5) + blue;
}


void yinyang_time() {
  img.setColorDepth(COLOR_DEPTH); 
  img.createSprite(RADIUS*2+1, RADIUS*2+1);
  img.fillSprite(BLACK); 
  M5.Lcd.fillScreen(BLACK); 
  for (int y=0; y<8; y++) {
  for (int x=0; x<360; x++) {
    yinyang(RADIUS, RADIUS, angle, RADIUS); 
    img.setBitmapColor(WHITE, BLACK); 
    img.pushSprite(M5.Lcd.width()/2 - RADIUS, 0);
    angle+=3; 
    if (angle > 356) { angle = 0; }
    delay(WAIT);
  }
  M5.Lcd.fillScreen(BLACK); 
  for (int x=0; x<30; x++) { 
      M5.Lcd.setCursor(1,20);
      M5.Rtc.GetTime(&RTC_TimeStruct);
      M5.Lcd.setTextSize(5);  
      M5.Lcd.setTextColor(WHITE, BLACK);  
      M5.Lcd.printf("%02d", RTC_TimeStruct.Hours);
      int action = random(0, 4);
      int wee = random(10, 100); 
      set_ACTION(action, wee); 
      M5.Lcd.printf(":");  
      M5.Lcd.setTextColor(WHITE, BLACK); 
      M5.Lcd.printf("%02d", RTC_TimeStruct.Minutes);
      M5.Lcd.setTextSize(2); 
      delay(WAIT+wee); 
      set_ACTION(action, wee); 
      M5.Lcd.setCursor(65,50); 
      M5.Lcd.setTextColor(WHITE, BLACK); 
      M5.Lcd.printf("%02d\n", RTC_TimeStruct.Seconds);
      delay(WAIT+wee);  
      for (int x=0; x<6; x++) {
      M5.Lcd.setCursor(80, 50); 
      set_ACTION(action, wee); 
      M5.Lcd.print("  ."); 
      set_ACTION(action, wee); 
      M5.Lcd.print(".");
      set_ACTION(action, wee); 
      M5.Lcd.print(".");
      set_ACTION(action, wee); 
      M5.Lcd.print(".");
      }
  }
  M5.Lcd.fillScreen(BLACK); 
  }
}
void set_ACTION(int action, int wee) {
       switch(action) { 
        case 0:
          M5.Lcd.setTextColor(YELLOW);
          delay(WAIT+wee); 
          break;
       case 1:   
          M5.Lcd.setTextColor(WHITE);
          delay(WAIT+wee); 
          break;  
       case 2:
          M5.Lcd.setTextColor(YELLOW);
          delay(WAIT+wee); 
          break;
       case 3:   
          M5.Lcd.setTextColor(WHITE);
          delay(WAIT+wee); 
          break;       
       default: 
          M5.Lcd.setTextColor(WHITE); 
          delay(WAIT+wee); 
          break;         
      }
}
int yinyang(int x, int y, int start_angle, int r)
{
  int x1 = 0; 
  int y1 = 0;
  getCoord(x, y, &x1, &y1, r/2, start_angle);
  img.fillCircle( x1,  y1, r/2, WHITE);
  img.fillCircle( x1,  y1, r/8, BLACK);

  getCoord(x, y, &x1, &y1, r/2, start_angle + 180);
  img.fillCircle( x1,  y1, r/2, BLACK);
  img.fillCircle( x1,  y1, r/8, WHITE);
  
  img.drawCircle(x, y, r, WHITE);
}
void getCoord(int x, int y, int *xp, int *yp, int r, int a)
{
  float sx1 = cos( (a-90) * RAD2DEG );    
  float sy1 = sin( (a-90) * RAD2DEG );
  *xp =  sx1 * r + x;
  *yp =  sy1 * r + y;
}

void xfactor_time() {
  int xpos = 1; 
  int ypos = 1;
  int font = 2; 
  byte red = 31; 
  byte blue = 0;
  byte green = 0; 
  byte state = 0; 
  byte value = 30 * 4; 

  M5.Lcd.fillScreen(TFT_BLACK); 
  img.createSprite(160,128); 
  img.fillSprite(TFT_BLACK); 
  for (int x=0; x<993700; x++) {
    
  if (targetTime < millis()) {
    targetTime = millis() + 100;

     for (int i = 0; i < 160; i++) {
      img.drawFastVLine(i, 0, img.height(), redcolor);
      switch (state) {
        case 0:
          green ++;
          if (green == 64) {
            green = 63;
            state = 1;
          }
          break;
        case 1:
          red--;
          if (red == 255) {
            red = 0;
            state = 2;
          }
          break;
        case 2:
          blue ++;
          if (blue == 32) {
            blue = 31;
            state = 3;
          }
          break;
        case 3:
          green --;
          if (green == 255) {
            green = 0;
            state = 4;
          }
          break;
        case 4:
          red ++;
          if (red == 32) {
            red = 31;
            state = 5;
          }
          break;
        case 5:
          blue --;
          if (blue == 255) {
            blue = 0;
            state = 0;
          }
          break;
      }
      redcolor = red << 11 | green << 5 | blue;
    }
    img.setTextColor(TFT_BLACK); 
    img.setCursor(12,5);
    img.drawCentreString("THE TIME:", xpos, ypos, font);
    img.pushSprite(0, 0);
    M5.Lcd.setTextSize(1); 
    M5.Lcd.setCursor(1,20); 
    static const char *wd[7] = {"Sun", "Mon", "Tue", "Wed", "Thr", "Fri", "Sat"};
    M5.Lcd.setTextColor(TFT_BLACK); 
    M5.Rtc.GetTime(&RTC_TimeStruct);
    M5.Rtc.GetData(&RTC_DateStruct);
    M5.Lcd.printf("%04d-%02d-%02d(%s)\n", RTC_DateStruct.Year, RTC_DateStruct.Month, RTC_DateStruct.Date, wd[RTC_DateStruct.WeekDay]);
    M5.Lcd.setTextSize(2); 
    M5.Lcd.printf("%02d:%02d:%02d\n", RTC_TimeStruct.Hours, RTC_TimeStruct.Minutes, RTC_TimeStruct.Seconds);
   
    delay(1200);
  }
}
  img.deleteSprite();
} 

void nextChannel() {
  if (sizeof(channels) > 1) {
    uint8_t ch = channels[channelIndex];
    channelIndex++;
    if (channelIndex > sizeof(channels)) channelIndex = 0;

    if (ch != wifi_channel && ch >= 1 && ch <= 14) {
      wifi_channel = ch;
      //wifi_set_channel(wifi_channel);
      esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
    }
  }
}

void randomMac() {
  for (int i = 0; i < 6; i++)
    macAddr[i] = random(256);
}

// Attacks and hacking tools
void attack2_ESSIDS(){
M5.Lcd.fillScreen(BLACK); 
M5.Lcd.setTextSize(1); 
M5.Lcd.setTextColor(RED, BLACK);
M5.Lcd.setCursor(1,1); 
M5.Lcd.println("ESSID attack \n[Alpha numeric]\n COUNTER:\n"); 
WiFi.disconnect(); 
  for (int i = 0; i < 32; i++) {
    emptySSID[i] = ' ';
  }
  randomSeed(1);
  packetSize = sizeof(beaconPacket);
  if (wpa2) {
    beaconPacket[34] = 0x31;

  } else {
    beaconPacket[34] = 0x21;
    packetSize -= 26;
  }
  randomMac();
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);
  Serial.println("\n\nSSIDs:");
  int i = 0;
  int len = sizeof(ssids);
  while (i < len) {
    Serial.print((char) pgm_read_byte(ssids + i));
    i++;
  }
    nextChannel2();
    for(int x=0; x<2600; x++) {
  M5.Lcd.fillScreen(BLACK);   
        invader_sec3();
  M5.Lcd.setTextSize(1); 
M5.Lcd.setTextColor(RED, BLACK);
M5.Lcd.setCursor(1,1); 
M5.Lcd.println("ESSID attack \nQuad payload\n COUNTER:\n");       
  M5.Lcd.setCursor(2,26);
  M5.Lcd.setTextColor(WHITE, BLACK); 
  M5.Lcd.print(x);
  M5.Lcd.setCursor(2,40); 
  M5.Lcd.setTextColor(YELLOW, BLACK); 
  M5.Lcd.println("MAX: 2600"); 
  M5.Lcd.setTextColor(GRAY, BLACK); 
  M5.Lcd.print("channel #");
  M5.Lcd.setTextColor(ORANGE, BLACK); 
  M5.Lcd.println(wifi_channel); 
  M5.Lcd.setTextColor(GRAY, BLACK);
  invader_effect(); 
  invader_sec3();
  invader_effect();   
  currentTime = millis();
  for (int q=0; q<20; q++) {
  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false;

    // go to next channel
    nextChannel();
    
    // read out next SSID
    while (i < ssidsLen) {
      j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);

      // Alpha Numeric Random ssid pulse 28/s 
      memset(str, '\0', sizeof(str)); 
      uint8_t cnt = 0;
      while (cnt != sizeof(str) - 1) {
        str[cnt] = random(0, 0x7F);
      if (str[cnt] == 0) { break; }
      if (isAlphaNumeric(str[cnt]) == true) { cnt++; }
      else {
        str[cnt] = '\0';
      }
      memcpy(&beaconPacket[38], str, 32);
      memcpy_P(&beaconPacket[38], &str, 32);
      }
      beaconPacket[82] = wifi_channel;
      // send packet
      for (int k = 0; k < 3; k++) {
        packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
        delay(2); // 1
      }

     i += j;
    }
    }
  }
  for (int q=0; q<20; q++) { payload(q); }
  for (int q=0; q<20; q++) { payload_B(q); }
  for (int q=0; q<20; q++) { payload_C(q); }
    } 
    M5.Lcd.fillScreen(BLACK); 
   
}


void payload_C(int q) {
   currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false;

    // go to next channel
    nextChannel();

    // read out next SSID
    while (i < ssidsLen) {
      j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);

      // Alpha Numeric Random ssid pulse 28/s 
      memset(str, '\0', sizeof(str)); 
      uint8_t cnt = 0;
      while (cnt != sizeof(str) - 1) {
        str[cnt] = random(0, 0x7F);
      if (str[cnt] == 0) { break; }
      if (isAlpha(str[cnt]) == true) { cnt++; }
      else {
        str[cnt] = '\0';
      }
      memcpy(&beaconPacket[38], str, 32);
      memcpy_P(&beaconPacket[38], &str, 32);
      }
      beaconPacket[82] = wifi_channel;
      // send packet
      for (int k = 0; k < 3; k++) {
        packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
        delay(2); // 1
      }

     i += j;
    }
  }

  // show packet-rate
  if (currentTime - packetRateTime > 3000) {
    packetRateTime = currentTime;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s"); 
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s \n[Playload C]");
    M5.Lcd.setCursor(1,20); 
    M5.Lcd.println("[ATTACK ACTIVE]");  
    M5.Lcd.setCursor(2,28);
    M5.Lcd.setTextColor(WHITE, BLACK); 
    M5.Lcd.print(q);
    M5.Lcd.print(" of 20 loops");
    M5.Lcd.setCursor(2,40); 
    M5.Lcd.setTextColor(YELLOW, BLACK); 
    M5.Lcd.println("MAX: 2600"); 
    M5.Lcd.setTextColor(GRAY, BLACK); 
    M5.Lcd.print("channel # ");
    M5.Lcd.setTextColor(ORANGE, BLACK); 
    M5.Lcd.println(wifi_channel); 
  packetCounter = 0;
  } 
}

void payload_B(int q) {
   currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false;

    // go to next channel
    nextChannel();

    // read out next SSID
    while (i < ssidsLen) {
      j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);

      // Alpha Numeric Random ssid pulse 28/s 
      memset(str, '\0', sizeof(str)); 
      uint8_t cnt = 0;
      while (cnt != sizeof(str) - 1) {
        str[cnt] = random(0, 0x7F);
      if (str[cnt] == 0) { break; }
      if (isdigit(str[cnt]) == true) { cnt++; }
      else {
        str[cnt] = '\0';
      }
      memcpy(&beaconPacket[38], str, 32);
      memcpy_P(&beaconPacket[38], &str, 32);
      }
      beaconPacket[82] = wifi_channel;
      // send packet
      for (int k = 0; k < 3; k++) {
        packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
        delay(2); // 1
      }

     i += j;
    }
  }

  // show packet-rate
  if (currentTime - packetRateTime > 3000) {
    packetRateTime = currentTime;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s"); 
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s \n[Playload B]");
    M5.Lcd.setCursor(1,20); 
    M5.Lcd.println("[ATTACK ACTIVE]");  
    M5.Lcd.setCursor(2,28);
    M5.Lcd.setTextColor(WHITE, BLACK); 
    M5.Lcd.print(q);
    M5.Lcd.print(" of 20 loops");
    M5.Lcd.setCursor(2,40); 
    M5.Lcd.setTextColor(YELLOW, BLACK); 
    M5.Lcd.println("MAX: 2600"); 
    M5.Lcd.setTextColor(GRAY, BLACK); 
    M5.Lcd.print("channel #");
    M5.Lcd.setTextColor(ORANGE, BLACK); 
    M5.Lcd.println(wifi_channel); 
    M5.Lcd.setTextColor(GRAY, BLACK);
  packetCounter = 0;
  } 
}

void payload(int q) {
   currentTime = millis();

  // send out SSIDs
  if (currentTime - attackTime > 100) {
    attackTime = currentTime;

    // temp variables
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false;

    // go to next channel
    nextChannel();

    // read out next SSID
    while (i < ssidsLen) {
      j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;

      // set MAC address
      macAddr[5] = ssidNum;
      ssidNum++;

      // write MAC address into beacon frame
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);    
     
     // reset SSID 
      memcpy(&beaconPacket[38], emptySSID, 32);

      // write new SSID into beacon frame
      memcpy_P(&beaconPacket[38], &ssids[i], ssidLen);

      // set channel for beacon frame
      beaconPacket[82] = wifi_channel;

      // send packet
      for (int k = 0; k < 3; k++) {
        packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
        delay(2); // 1
      }

      i += j;
    }
  }
   
  // show packet-rate
  if (currentTime - packetRateTime > 3000) {
    packetRateTime = currentTime;
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s"); 
    M5.Lcd.setTextColor(WHITE, RED); 
    M5.Lcd.setRotation(3);
    M5.Lcd.setCursor(1,1);
    M5.Lcd.print(packetCounter); 
    M5.Lcd.println(" packets/s \n[Playload A]");
    M5.Lcd.setCursor(1,20); 
    M5.Lcd.println("[ATTACK ACTIVE]");  
    M5.Lcd.setCursor(2,28);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.print(q);  
    M5.Lcd.print(" of 20 loops");
    M5.Lcd.setCursor(2,40); 
    M5.Lcd.setTextColor(YELLOW, BLACK); 
    M5.Lcd.println("MAX: 2600"); 
    M5.Lcd.setTextColor(GRAY, BLACK); 
    M5.Lcd.print("channel #");
    M5.Lcd.setTextColor(ORANGE, BLACK); 
    M5.Lcd.println(wifi_channel); 
    M5.Lcd.setTextColor(GRAY, BLACK);
    packetCounter = 0;
  } 
}

void attack_ESSIDS() {
M5.Lcd.fillScreen(BLACK); 
M5.Lcd.setTextSize(1); 
M5.Lcd.setTextColor(RED, BLACK);
M5.Lcd.setCursor(1,1); 
M5.Lcd.println("ESSID Spamming.\n COUNTER:\n"); 
WiFi.disconnect(); 

  for (int i = 0; i < 32; i++)
    emptySSID[i] = ' ';
  randomSeed(1);
  packetSize = sizeof(beaconPacket);
  if (wpa2) {
    beaconPacket[34] = 0x31;
  } else {
    beaconPacket[34] = 0x21;
    packetSize -= 26;
  }
  randomMac();
  WiFi.mode(WIFI_MODE_STA);
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE);
  
  for (int q=0; q<15600; q++) {
  invader_sec3();
  M5.Lcd.setCursor(2,20);
  M5.Lcd.setTextColor(WHITE, BLACK); 
  M5.Lcd.print(q);
  M5.Lcd.setCursor(2,40); 
  M5.Lcd.setTextColor(YELLOW, BLACK); 
  M5.Lcd.println("MAX: 15600"); 
  M5.Lcd.setTextColor(GRAY, BLACK); 
  M5.Lcd.println("channels: \n1,3,5,6,7,11");
  invader_effect(); 
  currentTime = millis();
   if (currentTime - attackTime > 100) {
    attackTime = currentTime;
    int i = 0;
    int j = 0;
    int ssidNum = 1;
    char tmp;
    int ssidsLen = strlen_P(ssids);
    bool sent = false; 
    nextChannel();  
    while (i < ssidsLen) { 
   j = 0;
      do {
        tmp = pgm_read_byte(ssids + i + j);
        j++;
      } while (tmp != '\n' && j <= 32 && i + j < ssidsLen);

      uint8_t ssidLen = j - 1;
      macAddr[5] = ssidNum;
      ssidNum++;
      memcpy(&beaconPacket[10], macAddr, 6);
      memcpy(&beaconPacket[16], macAddr, 6);
      memcpy(&beaconPacket[38], emptySSID, 32);
      memcpy_P(&beaconPacket[38], &ssids[i], ssidLen);
      beaconPacket[82] = wifi_channel;
       if (appendSpaces) {
        for (int k = 0; k < 3; k++) {
          //packetCounter += wifi_send_pkt_freedom(beaconPacket, packetSize, 0) == 0;
          //M5.Lcd.printf("size: %d \n", packetSize);
          packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, beaconPacket, packetSize, 0) == 0;
          delay(1);
        }
      }
     else {
        uint16_t tmpPacketSize = (109 - 32) + ssidLen; 
        uint8_t* tmpPacket = new uint8_t[tmpPacketSize]; 
        memcpy(&tmpPacket[0], &beaconPacket[0], 37 + ssidLen); 
        tmpPacket[37] = ssidLen; 
        memcpy(&tmpPacket[38 + ssidLen], &beaconPacket[70], 39); 
   for (int k = 0; k < 3; k++) {
          //packetCounter += wifi_send_pkt_freedom(tmpPacket, tmpPacketSize, 0) == 0;
          packetCounter += esp_wifi_80211_tx(ESP_IF_WIFI_STA, tmpPacket, tmpPacketSize, 0) == 0;
          delay(1);
        }

        delete tmpPacket;
          }

      i += j;
    }
  }
    if (currentTime - packetRateTime > 1000) {
    packetRateTime = currentTime;
    packetCounter = 0;
  }
}
WiFi.disconnect(); 
WiFi.softAP(ssid_standalone, password);
IPAddress myIP = WiFi.softAPIP();
server.begin();
}

int toolsmenuIndex = 0;
int toolsmenuCount = 3; 
String toolsmenuTitle[] = { " Essid Spammer", " Essid jammer", " " };

void hacker_tools() {
randomSeed(256);  
M5.Lcd.setRotation(3);
M5.Lcd.fillScreen(BLACK);
tools_menu(); 
delay(200); 
int toolmenuIndex = 6;
int o=0;
for(int x=0; x<37000; x++) {
  tools_logic();
  int color[17] = {
                 0xF800, 0xFFFF, 0x001F, // 1st color is lime green. 
                 0xF800, 0xFFFF, 0x001F,
                 0x07FF, 0x07E0, 0x07FF, 
                 0xFA60, 0x8410, 0x04FF,
                 0xF8FF, 0xFFE0, 0xF81F, 
                 0x001F, 0xF800
                };   
    M5.Lcd.drawXBitmap(88, 1, logo, logoWidth, logoHeight, color[o++]);
    delay(10); 
    M5.Lcd.drawXBitmap(88, 1, logo, logoWidth, logoHeight, color[o++]);
    delay(10); 

   if (x < 2501 | x < 2501) {  
   if (x == 1500 | x > 1500) {          
      for (int j=0; j<5; j++){ 
      delay(5);   
      M5.Lcd.drawXBitmap(88, 1, logo10, logoWidth, logoHeight, color[random(0,3)]);
      delay(5);  
      }   
   }
   if (x == 2500 | x > 2500) {
    for (int j=0; j<5; j++){
      delay(5); 
          M5.Lcd.drawXBitmap(88, 1, logo11, logoWidth, logoHeight, color[random(0,3)]);
      delay(5);   
   }
  }
   } else if (o == 17) { o = 0;  } // reset color loop.

}
}
 
void tools_menu() {
M5.Lcd.setCursor(2,2); 
M5.Lcd.setTextSize(1); 
M5.Lcd.setTextColor(WHITE, RED); 
M5.Lcd.println("HACKING TOOLS"); 
M5.Lcd.setCursor(0, 4 * 3); 
M5.Lcd.setTextColor(WHITE, BLACK); 
for (int i=0; i < toolsmenuCount; i++) { 
  M5.Lcd.printf("%s\n", toolsmenuTitle[i].c_str());  
}
 
M5.Lcd.setTextColor(GRAY, BLACK);
M5.Lcd.setCursor(0, 80-16);
M5.Lcd.print("BUTTON A:");
M5.Lcd.setTextColor(LIME, BLACK); 
M5.Lcd.println(" Mode");
M5.Lcd.setTextColor(GRAY, BLACK);  
M5.Lcd.print("BUTTON B:");
M5.Lcd.setTextColor(LIME, BLACK); 
M5.Lcd.println(" Select");
}

void tools_logic() {
  M5.update(); 
  M5.Lcd.setCursor(0, 4 * 3); 
  for (int i=0; i<toolsmenuCount; i++){
    if(toolsmenuIndex==i){
      int xs = random(0, 4); 
      switch(xs) {
      case 0:   
      M5.Lcd.setTextColor(WHITE, BLACK);
      M5.Lcd.print(">");  
      delay(10);
      break;
      case 1:
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.print(">");  
      delay(10);
      break;
      case 2:
      M5.Lcd.setTextColor(LIME, BLACK);
      M5.Lcd.print(">");  
      delay(10);
      break;
      case 3:
      M5.Lcd.setTextColor(GREEN, BLACK);
      M5.Lcd.print(">");  
      delay(10);
      break;
      } 
    } else {
      M5.Lcd.setTextColor(BLACK, BLACK);
      M5.Lcd.println(" ");
      delay(10);   
    }
  }
  current_value=digitalRead(BTNB); 
  if(current_value != last_value) {
    if(current_value==0) {
      toolsmenuIndex++;
      toolsmenuIndex=toolsmenuIndex % toolsmenuCount;
      M5.Lcd.fillScreen(BLACK); 
      delay(30);
    }
    last_value=current_value; 
  }
  current_value=digitalRead(BTNA);
  if(current_value != last_value) {
    if(current_value==0) {
      switch(toolsmenuIndex) {
        case 0:
             attack_ESSIDS();
             break;
        case 1:
             attack2_ESSIDS(); 
             break;
        case 2:
             break;          
      }
    }
    last_value=current_value;
  }
  tools_menu(); 
} 


void nextChannel2() {
  if (sizeof(channelr) < 2) {
    return;
  }

  uint8_t ch = channelr[channelIndex];

  channelIndex++;
  if (channelIndex > sizeof(channelr)) {
    channelIndex = 0;
  }

  if (ch != wifi_channel && ch >= 1 && ch <= 14) {
    wifi_channel = ch;

    esp_wifi_set_channel(wifi_channel, WIFI_SECOND_CHAN_NONE);
  }
}
