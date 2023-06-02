//
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
 #include "BluetoothSerial.h"
 #include <SPI.h>
 #include <Wire.h>
 #include <Adafruit_GFX.h>
 #include <Adafruit_SH1106.h>
 //defines the I2C pins to which the display is connected
 #define OLED_SDA 33
 #define OLED_SCL 32

Adafruit_SH1106 display(33, 32);
  #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
  #endif

  BluetoothSerial SerialBT;

 void setup() 
 {
   Serial2.begin(9600, SERIAL_8N1, 16, 17);  //comm between ESP32 and Module
    Serial.begin(115200);  //Serial Monitor
    SerialBT.begin("SOCORAD32"); //Bluetooth Seraial //define Bluetooth name here
   Serial.println("The device started, now you can pair it with bluetooth!");
   
    display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  //set the text size, color, cursor position and displayed text
  display.setTextColor(WHITE);
  display.setCursor(22, 30);
  display.println("Hello SOCORAD32");
  display.display();
 }

  void loop() 
  {
  while (Serial2.available() > 0) //receive to module to esp32 ;receive
   {
   Serial.write(Serial2.read());
   }
   while (Serial.available()>0) //send from esp32 to module ;transmit
   {
   Serial2.write(Serial.read());
   }
  delay(40);
  }


//
// void setup() {
//   Serial2.begin(9600, SERIAL_8N1, 16, 17);
//    Serial.begin(115200);
//   Serial.println("The device started, now you can pair it with bluetooth!");
// }
//
//  void loop() 
//  {
//   while (Serial2.available() > 0)
//   {
//   Serial.write(Serial2.read());
//   Serial2.flush();
//   }
//   while (Serial.available() > 0)
//   {
//   Serial2.write(Serial.read());
//   Serial2.flush();
//   }
//  }

  //#include <esp_bt.h>
//
//void setup()
//{
//  btStop();
//  esp_bt_controller_disable();
//
//}
//
//void loop()
//{}
