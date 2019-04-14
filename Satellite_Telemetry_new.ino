#include <SPI.h>
#include <LoRa.h>

int stat[32];
boolean debug = false;
short telemetryMode= 1;
short runtimeHours= 0;
short runtimeDays=0;
long mils=0;
/*
 * Telemetry modes:
 * 1: all values
 * 2: System time, Voltages, Power and RSSI
 * 3: System time, Voltaes, RSSI
 * 4: System time, RSSI and Status
 */
//pin declarations...........................................................
int batPin = 34;//input only
int x3v3Pin = 35;//input only
int x5vPin = 36;//input only
int x12vPin = 37;//input only
int PV = 38;
int DL_ss = 5;
int DL_DIO0 = 17;
int DL_rst = 16;

//pin declarations ends here.................................................

float batteryVoltage=0; //Battery Voltage
float DCDC3v3; //op voltage of 3v3 converter
float DCDC5v; //op voltage of 5v converter
float DCDC12v; //op voltage of 12v converter
float PVVoltage=0; //op voltage of PV cell


float batteryTemp=0; //Battery Temperature
float onboardTemp_MC=0, onboardTemp_APP=0;//onboard temperatures for OBC
float commTemp_UL=0, commTemp_DL;//temperatures from communication module
float EPSTemp_MPPT=0, EPSTemp_DCDC=0, EPSTemp_ChgCnt=0;//temperatures from EPS

float commCurrent_UL=0, commCurrent_DL=0;//Current Consumed by Comm modules
float MCCurrent=0; float APPCurrent=0;

int UL_RSSI=0; //Uplink signal strength

int DL_Power_dB=20;


void setup() {
  Serial.begin(115200);
  while (!Serial);

  //downlink Configuration...
  LoRa.setPins(DL_ss, DL_rst, DL_DIO0); //(ss,rst,DIO0)
  LoRa.setTxPower(DL_Power_dB); //Transmission power
  //LoRa.setSPIFrequency(20E6);
  //uplink Configuration...

  //
  Serial.println("NHCE-SAT Satellite Booting...");

  if (!LoRa.begin(433E6))
  {
    Serial.println("Downlink Communication module Error!");
    while (1);
  }
  Serial.println("NHCE-SAT Satellite Booted Succesfully, Preliminary System Test Complete");
  Serial.println("System Test Report:");
  Serial.println("");

}

void loop()
{
  Serial.println("Sending packet...");
  //String up = systemUptime();
  updateVoltage();
  sendData(); //sends Telemetry data
  delay(1000);
}


void sendData()
{
  //send packet
  LoRa.beginPacket();
  LoRa.println();
  LoRa.print("System time:");
  LoRa.println(systemUptime());
  LoRa.print("Health Status:");
  //LoRa.println(healthStatus()); //uncomment when function complete
  LoRa.endPacket();


  if(telemetryMode==1)//sends temperature data
  {
      LoRa.beginPacket();
      LoRa.println();
      LoRa.println("Temps:");
      LoRa.print("Bat:");
      LoRa.print(batteryTemp);
      LoRa.print(" MC:");
      LoRa.print(onboardTemp_MC);
      LoRa.print(" AP:");
      LoRa.print(onboardTemp_APP);
      LoRa.print(" EPS:");
      LoRa.print((EPSTemp_DCDC+EPSTemp_MPPT+EPSTemp_ChgCnt)/3);
      LoRa.print(" COM:");
      LoRa.print((commTemp_UL+commTemp_DL)/2);
      LoRa.endPacket();
  }

  if(telemetryMode<=2)//sends power usage data
  {
    //add power usage calculations and construct packet
  }
  if(telemetryMode<=3)
  {
    //Add voltage data
  }
  if(telemetryMode<=4)
  {
    LoRa.beginPacket();
    LoRa.println();
    LoRa.println("Voltages:");
    LoRa.print("Bat: ");
    LoRa.print(batteryVoltage);
    LoRa.print(" 3v3: ");
    LoRa.print(DCDC3v3);
    LoRa.print(" 5v: ");
    LoRa.print(DCDC5v);
    LoRa.print(" 12v: ");
    LoRa.print(DCDC12v);
    LoRa.print(" PV: ");
    LoRa.print(PVVoltage);
    LoRa.endPacket();
  }

}

void updateVoltage()
{
  //update all voltages
  batteryVoltage=maping(analogRead(batPin),0,4095,0.15,3.3)*11; //Battery Voltage
  DCDC3v3=maping(analogRead(x3v3Pin),0,4095,0.15,3.3)*11; //op voltage of 3v3 converter
  DCDC5v=maping(analogRead(x5vPin),0,4095,0.15,3.3)*11; //op voltage of 5v converter
  DCDC12v=maping(analogRead(x12vPin),0,4095,0.15,3.3)*11; //op voltage of 12v converter
  PVVoltage=maping(analogRead(PV),0,4095,0.15,3.3)*11; //op voltage of PV cell
}
void updataPower()
{
  //calculate power usage
}
void updateTemp()
{
  //update all temp data
}
String healthStatus()
{
  //determine system health and generate health code
  //stat[]=0;
  //Monitor battery status
  if(batteryVoltage>=3.3)
  {
    stat[0]=1;
  }
  if(batteryTemp<=50)
  {
    stat[1]=1;
  }
  //Monitor OBC temperatures
  if(onboardTemp_MC<=100)
  {
    stat[2]=1;
  }
  if(onboardTemp_APP<=100)
  {
    stat[3]=1;
  }
  //Monitor EPS temperatures
  if(EPSTemp_MPPT<=100)
  {
    stat[4]=1;
  }
  if(EPSTemp_DCDC<=100)
  {
    stat[5]=1;
  }
  if(EPSTemp_ChgCnt<=100)
  {
    stat[6]=1;
  }
  //Monitor COMM temperatures
  if(commTemp_DL<=100)
  {
    stat[7]=1;
  }
  if(commTemp_UL<=100)
  {
    stat[8]=1;
  }
  //Monitor Supply Voltages
  if(DCDC3v3>3&&DCDC3v3<=3.5)
  {
    stat[9]=1;
  }
  if(DCDC5v>4.5&&DCDC5v<=5.2)
  {
    stat[10]=1;
  }
  if(DCDC12v>4.5&&DCDC12v<=5.2)
  {
    stat[11]=1;
  }
  if(PVVoltage>0.5)
  {
    stat[12]=1;
  }
  //
  return " ";
}


int systemUptime()
{
  
  //measure system uptime
  runtimeHours = ((millis()-mils)/3600000);

  if(runtimeHours >= 24)
  {
    mils=millis();
    runtimeHours = 0;
    runtimeDays++;
  }
  int ut = ((runtimeDays*100)+runtimeHours);//ddhh
  return ut;
}


float maping(float x, float in_min, float in_max, float out_min, float out_max)//range mapingping in float
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
