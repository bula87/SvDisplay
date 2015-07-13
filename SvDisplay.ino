// Current Version v0.9
/* Changelist:
/  v0.9 - New special charakter fonts (I,N,Unknown char) - Compatible with SV_BT_Logger_v2.apk
/  v0.8 - Added support for 2 modules: AQ Voltage sensor and Temperature and Humidity sensor. New OLED screen layout - Compatible with SV_BT_Logger_v2.apk
/  v0.7 - Improved BT logging functionality, manual gear voltage settings from Android devices via BT, triger to start Auto Learn from Android devices via BT - Compatible with SV_BT_Logger_v2.apk
/  v0.5 - New functional flag -> Blutooth module HC-05, Added bt logging functionality
/  v0.4 - New manual flag (MANUAL_RANGES), manual voltage comparison functionality, checking manual definition parameters, added idle state to display some info before engine start 
/  v0.3 - New OLED lib
*/

#include <EEPROMex.h>
#include <EEPROMVar.h>
#include <Wire.h>
#include <SvOLED.h>
#include <Timer.h>
#include "config.h"
#include "bitmaps.h"

// Defines
#define HIGH_GEAR 4.0
#define IDLE_STATE 0.8
#define GEAR_COUNT 6
#define GEAR_PIN A0

// Variables
float gearsTab[GEAR_COUNT] = {0};
float TMPgearsTab[GEAR_COUNT] = {0};

#if defined(TEMP_HUM_MODULE) && defined(AQ_VOLTAGE_MODULE)
#define MODULES gearFrame3
#elif defined(AQ_VOLTAGE_MODULE)
#define MODULES gearFrame2
#elif defined(TEMP_HUM_MODULE)
#define MODULES gearFrame1
#else
#define MODULES gearFrame0
#endif

#if defined(SHOW_V) && defined(MANUAL_DEF)
#error "Only one manual definition is allowed!"
#elif defined(SHOW_V) && defined(MANUAL_RANGES)
#error "Only one manual definition is allowed!"
#elif defined(MANUAL_DEF) && defined(MANUAL_RANGES)
#error "Only one manual definition is allowed!"
#endif

#ifdef BT_SERIAL_MODULE
 #include "SV_BT.h"
 #include <SoftwareSerial.h>
#endif

#ifdef TEMP_HUM_MODULE
 #include <DHT.h>
 #define DHTTYPE DHT11 
 #define DHTPIN 8 // D8 pin
 DHT dht(DHTPIN, DHTTYPE);
 Timer tTempHum;
 int temp_value = 0;
 int hum_value = 0;
 int loop_count = 0;
 float sum_t = 0;
 float sum_h = 0;
#endif

#ifdef AQ_VOLTAGE_MODULE
 #define AQ_PIN A2
 Timer tVoltage;
#endif

void initPins()
{
  pinMode(0, OUTPUT); digitalWrite(0, LOW);
  pinMode(1, OUTPUT); digitalWrite(1, LOW);
  pinMode(2, OUTPUT); digitalWrite(2, LOW);
  pinMode(3, OUTPUT); digitalWrite(3, LOW);
  pinMode(4, OUTPUT); digitalWrite(4, LOW);
  pinMode(5, OUTPUT); digitalWrite(5, LOW);
  pinMode(6, OUTPUT); digitalWrite(6, LOW);
  pinMode(7, OUTPUT); digitalWrite(7, LOW);
#ifndef TEMP_HUM_MODULE
  pinMode(8, OUTPUT); digitalWrite(8, LOW); // -> Temperature and Humidity Module Pin
#endif
  pinMode(9, OUTPUT); digitalWrite(9, LOW);
#ifndef BT_SERIAL_MODULE
  pinMode(10, OUTPUT); digitalWrite(10, LOW); // -> Bluetooth RX Pin
  pinMode(11, OUTPUT); digitalWrite(11, LOW); // -> Bluetooth TX Pin
#endif
  pinMode(12, OUTPUT); digitalWrite(12, LOW);
  pinMode(13, OUTPUT); digitalWrite(13, LOW);
//  pinMode(14, INPUT); digitalWrite(14, LOW); // -> Gear Voltage Pin
  pinMode(15, INPUT_PULLUP); digitalWrite(15, HIGH);
#ifndef AQ_VOLTAGE_MODULE
  pinMode(16, INPUT_PULLUP); digitalWrite(16, HIGH); // -> AQ Voltage Module Pin
#endif
  pinMode(17, INPUT_PULLUP); digitalWrite(17, HIGH);
//  pinMode(18, INPUT_PULLUP); digitalWrite(18, HIGH); // -> OLED SDA Pin
//  pinMode(19, INPUT_PULLUP); digitalWrite(19, HIGH); // -> OLED SSCL Pin
  pinMode(A6, INPUT_PULLUP); digitalWrite(A6, HIGH);
  pinMode(A7, INPUT_PULLUP); digitalWrite(A7, HIGH);
}

void setup()
{
  display.init(0x3C);  //initialze OLED display
  pinMode(GEAR_PIN, INPUT);  // Set A0 like analog input - ADC 
#ifdef AQ_VOLTAGE_MODULE
  pinMode(AQ_PIN, INPUT);  // Set A2 like analog input - ADC for AQ voltage read   
  tVoltage.every(1000, readAQvoltage);
#endif
  initPins();
  
#ifdef BT_SERIAL_MODULE
  BT_init();
#endif

#ifndef MANUAL_DEF
  float val = 0;
  
  val = getGearVoltage();
  if ( val > HIGH_GEAR ) 
  {
    learnGears();
  }
  else 
  {
    loadFromEEPROM();
  }
  
#else  
  hardcodeGears();
#endif
  display.drawGearFrame(MODULES);
#ifdef TEMP_HUM_MODULE  
  tTempHum.every(1000, getTempHum);
  dht.begin();

  float h = readHum();
  float t = readTemp();
  if (!isnan(t) && !isnan(h)) 
  {
    showTempHum(t,h);
  }
  else
  {
    showTempHum(0,0);
  }
#endif 

}

void loop()
{   
  static char lastGear = 0, currGear = 0;
  float val = 0.0;
  
#ifdef BT_SERIAL_MODULE
  int BT_command = -1;
  String message = "";
  String tmpMessage = "";
#endif

  val = getGearVoltage();

#ifndef SHOW_V
  // Match voltage to gear
  currGear = checkGear(val,lastGear);
              
  // If gear has changed -> refresh display
  if( currGear != lastGear )
  { 
    lastGear = currGear;
    
#ifdef BT_SERIAL_MODULE
    tmpMessage = BT_SHOW_GEAR;
    tmpMessage += currGear;
    tmpMessage += ";";
    BT_prepareToSend(tmpMessage);
#endif

    drawGear(currGear);           
    delay(10);
  }
#else
  drawString("Value:",5,3);
  drawFloat(val,6,4);
  delay(100);
#endif

#ifdef BT_SERIAL_MODULE
  BT_command = BT_Receive(message);
  if(BT_command == BT_SHOW_LOG_CMD)
  {
    tmpMessage = BT_SHOW_LOG;
    tmpMessage += "RV:";
    tmpMessage += val;
    tmpMessage += ";LG:";
    tmpMessage += lastGear;
    tmpMessage += ";CG:";
    tmpMessage += currGear;
    tmpMessage += ';';
    BT_prepareToSend(tmpMessage);
  }else if(BT_command == BT_LEARN_GEAR_CMD)
  {
    BTshowGears();
    learnGears();
    BTshowGears();
    display.drawGearFrame(MODULES);
    lastGear = ' ';
    delay(500);
  }else if(BT_command == BT_LOAD_GEAR_CMD)
  {
    tmpMessage = BT_LOAD_GEAR;
    tmpMessage = String(tmpMessage+gearsTab[0]);
    tmpMessage = String(tmpMessage+";");
    tmpMessage = String(tmpMessage+gearsTab[1]);
    tmpMessage = String(tmpMessage+";");
    tmpMessage = String(tmpMessage+gearsTab[2]);
    tmpMessage = String(tmpMessage+";");
    tmpMessage = String(tmpMessage+gearsTab[3]);
    tmpMessage = String(tmpMessage+";");
    tmpMessage = String(tmpMessage+gearsTab[4]);
    tmpMessage = String(tmpMessage+";");
    tmpMessage = String(tmpMessage+gearsTab[5]);
    tmpMessage = String(tmpMessage+";");
    BT_prepareToSend(tmpMessage);
  }else if(BT_command == BT_SAVE_GEAR_CMD)
  {
    BTsetGearsTab(message);
  }
  else if(BT_command == BT_SAVE_EEPROM_CMD)
  {
    EEPROM.updateBlock(0, gearsTab);
  }
  
  BT_Send();
#endif

#ifdef AQ_VOLTAGE_MODULE   
  tVoltage.update();
#endif

#ifdef TEMP_HUM_MODULE    
  tTempHum.update();
#endif

  delay(100);
}

char checkGear(float value, char lastGear)
{
  char gear = '0';
#ifndef MANUAL_RANGES
  if (value <= IDLE_STATE)  
  {
    gear = 'I';
  }
    else if (value <= gearsTab[0]+((gearsTab[1]-gearsTab[0])/2))  
    {
      //if('2' != lastGear)  
      gear = '1';
    }
      else if (value <= gearsTab[1]+((gearsTab[2]-gearsTab[1])/2))  
      {
        //if('1' != lastGear)  
        gear = '2';
      }
        else if (value <= gearsTab[2]+((gearsTab[3]-gearsTab[2])/2))  
        {
          gear = '3';
        }
          else if (value <= gearsTab[3]+((gearsTab[4]-gearsTab[3])/2))  
          {
            gear = '4';
          }
            else if (value <= gearsTab[4]+((gearsTab[5]-gearsTab[4])/2))  
            {
              gear = '5';
            }
              else if (value <= gearsTab[5]+0.30)  
              {
                gear = '6';
              }
                else  
                {
                  if('2' == lastGear || '1' == lastGear)
                  {
                    gear = 'N';
                  }
                }
#else
  if (value <= IDLE_STATE)  
  {
    gear = 'I';
  }
    else if (value <= G1_UP)  
    {
      //if('2' != lastGear)  
      gear = '1';
    }
      else if (value <= G2_UP)  
      {
        //if('1' != lastGear)  
        gear = '2';
      }
        else if (value <= G3_UP)  
        {
          gear = '3';
        }
          else if (value <= G4_UP)  
          {
            gear = '4';
          }
            else if (value <= G5_UP)  
            {
              gear = '5';
            }
              else if (value <= G6_UP)  
              {
                gear = '6';
              }
                else  
                {
                  if('2' == lastGear || '1' == lastGear)
                  {
                    gear = 'N';
                  }
                }
#endif
  return gear;
}

void loadFromEEPROM()
{
  float check = 0;
  EEPROM.readBlock(0, gearsTab);
  for(int i=0;i<GEAR_COUNT;i++)
  {
    check += gearsTab[i];
  }
  if( 0 == check ) learnGears();
}

void hardcodeGears()
{
  gearsTab[0] = GEAR_1;
  gearsTab[1] = GEAR_2;
  gearsTab[2] = GEAR_3;
  gearsTab[3] = GEAR_4;
  gearsTab[4] = GEAR_5;
  gearsTab[5] = GEAR_6;
}

void learnGears()
{
  int gCount = 0;
  int check = 0;
  float val = 0.0;
  int BT_command = -1;
  String message = "";
  
  reset();
  
  for(int i=0;i<GEAR_COUNT;i++)
  {
    TMPgearsTab[i]=0;
  }
  
  while( gCount < GEAR_COUNT )
  {
    val = getGearVoltage();
    if(val > 0.0 && val < 5.0)
    {
      check = 0;
      for(int i=0;i<GEAR_COUNT;i++)
      {
        if( 0 != TMPgearsTab[i] && ( val <= TMPgearsTab[i]+0.25 && val >= TMPgearsTab[i]-0.25 ))
        {
          check = 1;
          break;
        }
      }
      if( 0 == check )
      {
        TMPgearsTab[gCount]=val;
        gCount++;
        showGears(TMPgearsTab);
      }

    }
    else
    {
      delay(500);
    }
#ifdef BT_SERIAL_MODULE
    BT_command = -1;
    message = "";
    int timeout = 10;
    while(!is_slotOpen() && timeout>0)
    {
      BT_command = BT_Receive(message);
      timeout--;
      delay(100);
    }
    BTshowGears();
    BT_Send();
#endif
  }
  sort(TMPgearsTab,GEAR_COUNT);
  
  for(int i=0;i<GEAR_COUNT;i++)
  {
    gearsTab[i] = TMPgearsTab[i];
  }
  
  EEPROM.updateBlock(0, gearsTab);
  reset();
}

void showGears(float *TabToShow)
{
  display.setCursorXY(1,1);
  display.print("1:");  
  display.print(TabToShow[0]);   
  display.setCursorXY(1,3);
  display.print("2:"); 
  display.print(TabToShow[1]);   
  display.setCursorXY(1,5);
  display.print("3:"); 
  display.print(TabToShow[2]);   
  display.setCursorXY(9,1);
  display.print("4:"); 
  display.print(TabToShow[3]);   
  display.setCursorXY(9,3);
  display.print("5:"); 
  display.print(TabToShow[4]);   
  display.setCursorXY(9,5);
  display.print("6:"); 
  display.print(TabToShow[5]);     
}

void sort(float a[], int size)
{
  for(int k=0; k<(size-1); k++)
  {
    for(int o=0; o<(size-(k+1)); o++)
    {
      if( a[o] > a[o+1] )
      {
        float t = a[o];
        a[o] = a[o+1];
        a[o+1] = t;
      }
    }
  }
}

float getGearVoltage()
{
  float sumVal = 0;
  int minVal = 9999, raw = 0;
  for(int i=0; i< 100; i++)
  {
    for (int j=0; j < 10; j++){
        raw = analogRead(GEAR_PIN);
        if(minVal>raw) minVal = raw;
    }
    sumVal += minVal * (5.0 / 1023.0);
  }
  return (sumVal / 100);
}

void reset()
{ 
  display.clearDisplay();
}

void drawGear(char arg)
{
  if(arg < 48 || arg > 58)
  {
    display.printSpecialChar(arg,7,1);
  }
  else
  {
    display.printBigNumber(arg,6.8,1);
  }
}

void drawString(String arg, int x, int y)
{
  display.setCursorXY(x,y);
  display.print(arg);
}

void drawFloat(float arg, int x, int y)
{
  display.setCursorXY(x,y);
  display.print(arg);
}

#ifdef BT_SERIAL_MODULE
void BTshowGears()
{ 
  String tmpMessage = BT_LEARN_GEAR;
  tmpMessage = String(tmpMessage+TMPgearsTab[0]);
  tmpMessage = String(tmpMessage+";");
  tmpMessage = String(tmpMessage+TMPgearsTab[1]);
  tmpMessage = String(tmpMessage+";");
  tmpMessage = String(tmpMessage+TMPgearsTab[2]);
  tmpMessage = String(tmpMessage+";");
  tmpMessage = String(tmpMessage+TMPgearsTab[3]);
  tmpMessage = String(tmpMessage+";");
  tmpMessage = String(tmpMessage+TMPgearsTab[4]);
  tmpMessage = String(tmpMessage+";");
  tmpMessage = String(tmpMessage+TMPgearsTab[5]);
  tmpMessage = String(tmpMessage+";");
  BT_prepareToSend(tmpMessage);
}

void BTsetGearsTab(String message)
{
    int firstIndex = message.indexOf(';', 0 );
    gearsTab[0] = (message.substring(firstIndex+1,message.indexOf(';', firstIndex+1 ))).toFloat();
    
    message = message.substring(firstIndex+1);
    firstIndex = message.indexOf(';', 0 );
    gearsTab[1] = (message.substring(firstIndex+1,message.indexOf(';', firstIndex+1 ))).toFloat();
    
    message = message.substring(firstIndex+1);
    firstIndex = message.indexOf(';', 0 );
    gearsTab[2] = (message.substring(firstIndex+1,message.indexOf(';', firstIndex+1 ))).toFloat();
    
    message = message.substring(firstIndex+1);
    firstIndex = message.indexOf(';', 0 );
    gearsTab[3] = (message.substring(firstIndex+1,message.indexOf(';', firstIndex+1 ))).toFloat();
    
    message = message.substring(firstIndex+1);
    firstIndex = message.indexOf(';', 0 );
    gearsTab[4] = (message.substring(firstIndex+1,message.indexOf(';', firstIndex+1 ))).toFloat();
    
    message = message.substring(firstIndex+1);
    firstIndex = message.indexOf(';', 0 );
    gearsTab[5] = (message.substring(firstIndex+1,message.indexOf(';', firstIndex+1 ))).toFloat();
}
#endif

#ifdef TEMP_HUM_MODULE  
float readHum()
{
  return dht.readHumidity();
}

float readTemp()
{
  return dht.readTemperature();
}

void showTempHum(float temp, float hum)
{
  String str;
  str=String((int)temp)+"*C";
  drawString(str,1,3);
  
  str=String((int)hum)+"%";
  drawString(str,12,3);
}

void getTempHum()
{
  if(loop_count>=60)
  {
    float res_t = sum_t/loop_count;
    float res_h = sum_h/loop_count;
    showTempHum(res_t,res_h);
    
    loop_count = 0;
    sum_t = 0;
    sum_h = 0;
  }
  float h = readHum();
  float t = readTemp();
  if (!isnan(t) && !isnan(h)) 
  {
      sum_t += t;
      sum_h += h;
      loop_count ++;
  }
}
#endif

#ifdef AQ_VOLTAGE_MODULE
void showAQvoltage(float voltage)
{
  String str;
  str=String(voltage)+"V";
  //drawString("Vol:",0,7);
  drawString(str,6,7);
}

void readAQvoltage()
{
  float sumVal = 0;
  int minVal = 9999, raw = 0;
  for(int i=0; i< 1000; i++)
  {
    raw = analogRead(AQ_PIN);
    sumVal += raw * (5.0 / 1023.0) * 5;
  }
  sumVal = sumVal / 1000;
  showAQvoltage(sumVal);
  Serial.println("temp=");
  Serial.println(sumVal);
}
#endif

