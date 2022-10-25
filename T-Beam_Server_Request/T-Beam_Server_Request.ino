#include <SPI.h>
#include <LoRa.h>
#include <elapsedMillis.h>

#define BAND 915E6
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26

elapsedMillis sendMillis;
unsigned long sendInterval = 2000;

String target1 = "L1";
String target2 = "L2";
String target3 = "L3";
String target4 = "L4";

byte turn;

String 
moist1, pH1, nitro1, phos1, kal1,
moist2, pH2, nitro2, phos2, kal2,
moist3, pH3, nitro3, phos3, kal3,
moist4, pH4, nitro4, phos4, kal4;

void setup()
{
  Serial.begin(9600);
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND))
  {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  turn = 1;
}

void loop()
{
  if (sendMillis >= sendInterval)
  {
    if (turn == 1)
    {
      sendRequest (target1);
      waitResponse ();
      turn = 2;
    }
    else if (turn == 2)
    {
      sendRequest (target2);
      waitResponse ();
      turn = 3;
    }
    else if (turn == 3)
    {
      sendRequest (target3);
      waitResponse ();
      turn = 4;
    }
    else if (turn == 4)
    {
      sendRequest (target4);
      waitResponse ();
      turn = 1;
    }
    sendMillis = 0;
  }
}

void sendRequest (String target)
{
  LoRa.beginPacket ();
  LoRa.print (target);
  LoRa.print ("#");
  LoRa.endPacket ();
}

void waitResponse()
{
  String area, moist, pH, nitro, phos, kal;
  int packetSize = LoRa.parsePacket();
  if (packetSize)
  {
    while (LoRa.available())
    {
      area = LoRa.readStringUntil ('#');
      moist = LoRa.readStringUntil ('#');
      pH = LoRa.readStringUntil ('#');
      nitro = LoRa.readStringUntil ('#');
      phos = LoRa.readStringUntil ('#');
      kal = LoRa.readStringUntil ('#');
      sensorMap (area, moist, pH, nitro, phos, kal);
    }
  }
}

void sensorMap (String field, String sensor1, String sensor2, String sensor3, String sensor4, String sensor5)
{
  if (field == target1)
  {
    moist1  = sensor1;
    pH1     = sensor2;
    nitro1  = sensor3;
    phos1   = sensor4;
    kal1    = sensor5;
  }
  else if (field == target2)
  {
    moist2  = sensor1;
    pH2     = sensor2;
    nitro2  = sensor3;
    phos2   = sensor4;
    kal2    = sensor5;
  }
  else if (field == target3)
  {
    moist3  = sensor1;
    pH3     = sensor2;
    nitro3  = sensor3;
    phos3   = sensor4;
    kal3    = sensor5;
  }
  else if (field == target4)
  {
    moist4  = sensor1;
    pH4     = sensor2;
    nitro4  = sensor3;
    phos4   = sensor4;
    kal4    = sensor5;
  }
}
