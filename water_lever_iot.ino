#include <timer.h>//library timer
#include <timerManager.h>//library multiple timer
#include <FirebaseESP8266.h>  // Install Firebase ESP8266 library
#include <ESP8266WiFi.h>//library wifi
#include <NTPClient.h> // library time server
#include <WiFiUdp.h> //library time server 
#include <LiquidCrystal_I2C.h>// library lcdI2C

LiquidCrystal_I2C lcd(0x27,20,4);
Timer realtimer;
Timer historytimer;
Timer Timerlcd;

#define WIFI_SSID "SSID"
#define WIFI_PASSWORD "PASS"
//========================================================
#define FIREBASE_HOST "URL" //Without http:// or https:// schemes
#define FIREBASE_AUTH "FIREBASE_TOKEN"
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "id.pool.ntp.org");

int trig = 14;//pin d5
int echo = 12;//pin d6

//Define FirebaseESP8266 data object
FirebaseData firebaseData;
FirebaseJson json;
//===============================================================================
String getcurrentdate() {
  //Get a time structure
  unsigned long epochTime = timeClient.getEpochTime();

  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
//  String currentMonthName = months[currentMonth - 1];
  int currentYear = ptm->tm_year + 1900;

  //Print complete date:
  String currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear);
  return currentDate;
}

  float elevasi() {

    unsigned long durasi,jarak;
    float var = 0.19;

    digitalWrite(trig,HIGH);
    delayMicroseconds(10);

    digitalWrite(trig,LOW);
    delayMicroseconds(2);

    durasi = pulseIn(echo,HIGH);
    Serial.print(durasi);Serial.print(" mikrosec  ");

    jarak = durasi/58;
    Serial.print(jarak);Serial.println(" cm");

    float f_jarak = float(jarak);
    float mdpl = (683+var)-(f_jarak/100);
    
    return mdpl;
  }
  String streval(){
    String str_eval = String(elevasi());
    return str_eval;
  }
   
void elevasiRealtimer() {
  
  String formattime = timeClient.getFormattedTime();
  formattime.replace(":","T");
  String value= getcurrentdate()+"|"+formattime+"|"+streval();
  // Check if any reads failed and exit early (to try again).
  if (isnan(elevasi())) {
    Serial.println(F("Failed to read from elevasi sensor!"));
    return;
  }
//  Serial.print(F("Elevasi: "));
  Serial.print(streval());
  if (Firebase.setString(firebaseData,"Realtimedb/elevasi",value)){
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
 }
  void elevasiHistory() {
  String dbtime ="historicaldb/"+getcurrentdate()+"/"+timeClient.getFormattedTime()+"/elevasi";
 
  if (Firebase.setString(firebaseData,dbtime,streval()))
  {
    Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath());
    Serial.println("TYPE: " + firebaseData.dataType());
    Serial.println("ETag: " + firebaseData.ETag());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + firebaseData.errorReason());
    Serial.println("------------------------------------");
    Serial.println();
  }
}

void LcdDisplay(){
  lcd.clear();
  lcd.setCursor(4,0);
  lcd.print(timeClient.getFormattedTime());
  if(WiFi.status()==WL_CONNECTED){
    lcd.setCursor(14,0);
    lcd.print("OK");
  }else{
    lcd.setCursor(14,0);
    lcd.print("CF");
  }
  lcd.setCursor(2,1);
  lcd.print(streval());
  lcd.print("  MDPL");
}
  
//==========================================================================  
void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.backlight();                 // Print a message to the LCD.
  
  Serial.begin(9600);

  pinMode(trig,OUTPUT);
  pinMode (echo,INPUT);
  digitalWrite(trig,LOW);
  delayMicroseconds(2);
  
//  WiFiManager wificonfig;
//  Serial.println("connecting..");
//  wificonfig.autoConnect("ESP8266Config","Admin1234");
//  Serial.println("connected");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Selamat Datang");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    lcd.setCursor(0,0);
    lcd.clear();
    lcd.print("Connecting");
    delay(300);
    for(int i=12;i<=14;i++){
      lcd.setCursor(i,0);
      lcd.print(".");
      delay(200);
      if(WiFi.status()== WL_CONNECTED){
        break;
      }
    }
  }
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  
  //set interval in millisecond
  realtimer.setInterval(5000);
  historytimer.setInterval(60000);
  Timerlcd.setInterval(100);
  //set our callback fuction
  realtimer.setCallback(elevasiRealtimer);
  historytimer.setCallback(elevasiHistory);
  Timerlcd.setCallback(LcdDisplay);
  
  TimerManager::instance().start();
}
  

void loop() {
  TimerManager::instance().update();
  timeClient.update();

  Serial.print(getcurrentdate());
  Serial.print("T");
  String formattedTime = timeClient.getFormattedTime();//current time "HH:MM:SS
  Serial.println(formattedTime);

  Serial.println("");

  delay(1000);

}
