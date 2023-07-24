#include "globalConfig.h"
#include "wifiLib.h"
#include <ESP8266HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

//WIND D7 - PIN 13
//TEMPERATURE D5 - PIN 14
//RAIN D1 - PIN 5

#define ONE_WIRE_BUS 14  //pin14 = D5
OneWire oneWire(ONE_WIRE_BUS);  //temperatureSensorPin
DallasTemperature ds18b20(&oneWire);           
DeviceAddress tempDeviceAddress;
int  resolution = 12;
float lastTemperatureRead = 0.0;


//attenzione: upload del codice via wifi chiede la psw di mdsplus che è salvata nel file globalConfig.h


void ICACHE_RAM_ATTR windInterrupt()
{
  noInterrupts();
  //detachInterrupt(windSensorPin); //20230721
  if((long)(micros() - wind_last_micros) >= wind_debouncing_time * 1000) {
    wind_counter++;
    wind_last_micros = micros();
  }
  //attachInterrupt(windSensorPin, windInterrupt, RISING); //20230721
  interrupts();
}

void ICACHE_RAM_ATTR rainInterrupt()
{
  noInterrupts();  
  //detachInterrupt(rainSensorPin); //20230721
  if((long)(micros() - rain_last_micros) >= rain_debouncing_time * 1000) {
    rain_counter++;
    rain_last_micros = micros();
  }
  //attachInterrupt(rainSensorPin, rainInterrupt, RISING); //20230721
  interrupts();  
}

void setup() {

  if(debugOutput)
  {
    Serial.begin(115200);
    delay(10);
  }

  rebootTimeInterval = 1000*60*60*24*7;  //1000ms*60sec*60min*24ore*7giorni   @reboot ogni settimana
  rebootTime  = millis() + rebootTimeInterval; //reboot time used in loop

  wifiCheckInterval = 1000*60*30;  //check wifi status @1000ms*60sec*30min = 30 min , if not connected reboot

  ds18b20.begin();  //init temperature sensor
  ds18b20.getAddress(tempDeviceAddress, 0);  //index0 because only one sensor is present
  ds18b20.setResolution(tempDeviceAddress, resolution);
  ds18b20.setWaitForConversion(false);
  ds18b20.requestTemperatures();
  temperatureUpdateTime  = millis() + temperatureUpdateInterval;

  pinMode(windSensorPin, INPUT);  
  pinMode(rainSensorPin, INPUT); 
  //pinMode(windSensorPin, INPUT_PULLUP); //DA TOGLIERE SU SCHEDA CON PULLUP ESTERNO 
  //pinMode(rainSensorPin, INPUT_PULLUP);  

  wifiSetup();  
  otaUpdSetup();

  //20230721
  //attachInterrupt(windSensorPin, windInterrupt, RISING); 
  //attachInterrupt(rainSensorPin, rainInterrupt, RISING);  
  attachInterrupt(digitalPinToInterrupt(windSensorPin), windInterrupt, FALLING); 
  attachInterrupt(digitalPinToInterrupt(rainSensorPin), rainInterrupt, FALLING);    
}


void loop() {

  ArduinoOTA.handle();  //manage OTA updates
  
  if (millis() > rebootTime)    //regular reboot ESP - millis() can save about 50 days time from powerup
  { 
    ESP.restart();
  }

  if (millis() > wifiCheckTime)   //reboot ESP if not connected to wifi every XXmin/hours
  { 
    wifiCheckTime = millis() + wifiCheckInterval;    
    wifiCheckReboot(); 
  }

  yield();  //not sure is necessary

  if (millis() > temperatureUpdateTime) //async read temperature sensor without blocking
  {
    temperatureUpdateTime  = millis() + temperatureUpdateInterval; //with 12bit resolution wait at least 750ms
    lastTemperatureRead = ds18b20.getTempC(tempDeviceAddress);
    ds18b20.setWaitForConversion(false);
    ds18b20.requestTemperatures(); 
  }

  if (millis() > windUpdateTime)    
  { 
    //detachInterrupt(windSensorPin);
    windUpdateTime  = millis() + windUpdateInterval;
    windUpdateCounter++; 
    float rps=0.0;

    noInterrupts(); //20230721 new to improve code, read on forum to use a copy of the variable minimize the code inside no interrupts-
    wind_counter_copy=wind_counter;
    wind_counter = 0;    
    interrupts(); 

    if(wind_counter_copy == 0)
    {
      wind = 0.0;
    }
    else
    {
      rps = (wind_counter_copy/number_reed)/(windUpdateInterval/1000.0); //computing rounds per second 
      wind = 3.769 * rps + 3.45;  //vedi foglio excel con calcoli sui dati ricevuti via email da SEAV per anemometro installato
    }
    if(last_wind - wind > 1.0 || last_wind - wind < -1.0 || windUpdateCounter >= windUpdateCounterThr) //pubblico solo con isteresi o dopo windUpdateCounterThr
    { 
//      detachInterrupt(windSensorPin);
      httpUpdateReq++;
      windUpdateCounter = 0;
      last_wind = wind;  
//      attachInterrupt(windSensorPin, windInterrupt, RISING);
      if(debugOutput)
      {
        Serial.println("Wind: " +  String(wind,1) + " km/h");
      }
    } 
    //attachInterrupt(windSensorPin, windInterrupt, RISING);
  }
  

  if (httpUpdateReq > 0 ) //sync temperature with wind sensor
  {
    temperature = lastTemperatureRead;
    if(debugOutput)
    {
      Serial.print("temperature: ");
      Serial.println(temperature);
    }   
  }


  if (httpUpdateReq > 0 ) //sync rain with wind sensor 
  { 
//    detachInterrupt(rainSensorPin);
    noInterrupts();
    rain_counter_copy=rain_counter;
    rain_counter = 0; 
    interrupts(); 
    rain=rain_counter_copy * rain_mmOnePulse; 
//    attachInterrupt(rainSensorPin, rainInterrupt, RISING);      

    if(debugOutput)
    {
        Serial.println("Rain: " +  String(rain,1) + " mm");
    }
  }

  if (httpUpdateReq > 0 ) //send http data when new data is available 
  {
    httpUpdateReq = 0;

    WiFiClient client;
    HTTPClient http;

    String serverPath = "http://" + serverAddr + serverPage;
    http.begin(client, serverPath.c_str());  
    http.addHeader("Content-Type", "application/json"); 
 
    //String httpPostString = "{\"wind\":\"" + String(wind,1) + "\"}";  //json string. wind formatted with 1 decimal place 
    String httpPostString = "{\"psw\":\"FM84\", \"wind\":\"" + String(wind,1) +"\", \"temperature\":\"" + String(temperature,2) +"\", \"rain\":\"" + String(rain,2) + "\"}";  //json string. wind formatted with 1 decimal place

    if(debugOutput)
    {
      Serial.print("[HTTP] POST: "); 
      Serial.println(httpPostString.c_str());
    }
    int httpCode = http.POST(httpPostString.c_str());  //poi passare variabile!

    if (httpCode > 0) // httpCode will be negative on error
    {
      // HTTP header has been send and Server response header has been handled
      if(debugOutput)
      {
       Serial.printf("[HTTP] POST... code: %d\n", httpCode);
      }
      // file found at server
      if (httpCode == HTTP_CODE_OK) 
      {
        const String& payload = http.getString();
        if(debugOutput)
        {
          Serial.println("received payload:\n<<");
          Serial.println(payload);
          Serial.println(">>");
        }
      }
    } 
    else
    {
      if(debugOutput)
      {
        Serial.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }
    }
    http.end();
  }//if 
}//LOOP
