/*
  * LORA32 to Soil Moisture Sensor
  * VCC = 3.3V
  * Data = 25
  * GND = GND
*/

/*
  * LORA32 to pH Sensor
  * Data = 4
  * GND = GND
*/

/*
  * LORA32 to MAX485 
  * VCC = 5V
  * GND = GND
  * (Through Logic-Level Converter)
  * DI (HRX CH1) = TX (LTX CH1)
  * DE (HRX CH2) = 32 (LRX CH2)
  * RE (HTX CH2) = 33 (LTX CH2)
  * RO (HTX CH1) = RX (LRX CH1)
*/

/*
  * MAX485 to NPK Sensor
  * VCC = 9 V (Diff)
  * GND = GND
  * A = A
  * B = B
*/

#include <SPI.h>
#include <LoRa.h>
#include <elapsedMillis.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BAND  915E6
#define SCK   5
#define MISO  19
#define MOSI  27
#define SS    18
#define RST   14
#define DIO0  26

#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 16
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64 

#define soil        25
#define pH          4
#define DE_MAX485   32
#define RE_MAX485   33

elapsedMillis readSensorMillis;
elapsedMillis OLEDPrintMillis;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);

const byte nitro[]  = {0x01,0x03, 0x00, 0x1e, 0x00, 0x01, 0xe4, 0x0c};
const byte phos[]   = {0x01,0x03, 0x00, 0x1f, 0x00, 0x01, 0xb5, 0xcc};
const byte kal[]    = {0x01,0x03, 0x00, 0x20, 0x00, 0x01, 0x85, 0xc0};
byte values[11];

int moistAnalog, moistPercent;
int pHAnalog; float pHValue;
byte NValue, PValue, KValue;

String area = "L4";
String msgArea, msgMoist, msgPH, msgNitro, msgPhos, msgKal;

unsigned long readSensorInterval = 2000;
unsigned long OLEDPrintInterval = 3000;

int x, minX;
char message[] = "29M LstWyPt, 345M StPt, rec#89";

void setup()
{
  Serial.begin(4800);
  
  pinMode (OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  initLoRa();
  initSensor();
  initOLED();
}
void loop() 
{
  int packetSize = LoRa.parsePacket();
  if (packetSize) 
  {
    while (LoRa.available()) 
    {
      msgArea = LoRa.readStringUntil ('#');
      if (msgArea == area)
      {
        onResponse(msgArea, msgMoist, msgPH, msgNitro, msgPhos, msgKal);
      }
    }
  }

  if (readSensorMillis >= readSensorInterval)
  {
    pHRead();
    moistRead();
    NPKRead();
    readSensorMillis = 0;
  }

  if (OLEDPrintMillis >= OLEDPrintInterval)
  {
    OLEDPrint();
    OLEDPrintMillis = 0;
  }  
}

void initLoRa()
{
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DIO0);
  if (!LoRa.begin(BAND))
  {
    while (1);
  }
}

void initSensor()
{
  pinMode (soil, INPUT);
  pinMode (pH, INPUT);
  pinMode (DE_MAX485, OUTPUT);
  pinMode (RE_MAX485, OUTPUT);
}

void initOLED()
{
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false))
  { 
    for (;;);
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LoRa Lahan 4");
  display.display();
  x = display.width();
  minX = -12 * strlen(message);
}

void onResponse(String field, String sensor1, String sensor2, String sensor3, String sensor4, String sensor5)
{
  LoRa.beginPacket ();
  LoRa.print (field);
  LoRa.print ("#");
  LoRa.print (sensor1);
  LoRa.print ("#");
  LoRa.print (sensor2);
  LoRa.print ("#");
  LoRa.print (sensor3);
  LoRa.print ("#");
  LoRa.print (sensor4);
  LoRa.print ("#");
  LoRa.print (sensor5);
  LoRa.print ("#");
  LoRa.endPacket ();
}

void pHRead()
{
  pHAnalog  = analogRead (pH);
  pHValue   = -(-0.0693 * pHAnalog )+ 7.3855;
  msgPH     = String (pHValue);
}

void moistRead()
{
  moistAnalog   = analogRead (soil);
  moistPercent  = map (moistAnalog, 0, 4095, 100, 0);
  msgMoist      = String (moistPercent);
}

void NPKRead()
{
  NValue = NRead();
  delay (200);
  PValue = PRead();
  delay (200);
  KValue = KRead();
  delay (200);
  msgNitro  = String (NValue);
  msgPhos   = String (PValue);
  msgKal    = String (KValue);
}

byte NRead()
{
  digitalWrite (DE_MAX485, HIGH);
  digitalWrite (RE_MAX485, HIGH);
  delay (10);
  if (Serial.write (nitro, sizeof (nitro)) == 8);
  {
    digitalWrite (DE_MAX485, LOW);
    digitalWrite (RE_MAX485, LOW);
    for (byte i=0; i<7; i++)
    {
      values[i] = Serial.read();
    }
  }
  return values[4];
}

byte PRead()
{
  digitalWrite (DE_MAX485, HIGH);
  digitalWrite (RE_MAX485, HIGH);
  delay (10);
  if (Serial.write (phos, sizeof (phos)) == 8);
  {
    digitalWrite (DE_MAX485, LOW);
    digitalWrite (RE_MAX485, LOW);
    for (byte i=0; i<7; i++)
    {
      values[i] = Serial.read();
    }
  }
  return values[4];
}

byte KRead()
{
  digitalWrite (DE_MAX485, HIGH);
  digitalWrite (RE_MAX485, HIGH);
  delay (10);
  if (Serial.write (kal, sizeof (kal)) == 8);
  {
    digitalWrite (DE_MAX485, LOW);
    digitalWrite (RE_MAX485, LOW);
    for (byte i=0; i<7; i++)
    {
      values[i] = Serial.read();
    }
  }
  return values[4];
}

void OLEDPrint()
{
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Area D");
  display.setCursor(0, 10);
  display.print("N : ");
  display.setCursor(0, 20);
  display.print("P : ");
  display.setCursor(0, 30);
  display.print("K : ");
  display.setCursor(0, 40);
  display.print("pH : ");
  display.setCursor(0, 50);
  display.print("Soil : ");

  display.setCursor(40, 10);
  display.print(NValue);
  display.setCursor(40, 20);
  display.print(PValue);
  display.setCursor(40, 30);
  display.print(KValue);
  display.setCursor(40, 40);
  display.print(pHValue);
  display.setCursor(40, 50);
  display.print(moistPercent);

  Serial.println(pHValue);
  Serial.println(NValue);
  Serial.println(PValue);
  Serial.println(KValue);

  display.display();
/*
  * Dynamic Variables:
  * moistPercent
  * pHValue
  * NValue
  * PValue
  * KValue
 */
}
