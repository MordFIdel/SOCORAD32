
// upload this sketch to facilitate AT command communication betweeen phone and device.
///////////////////////////////////////////////////////////////////////////////////////////////////////

 #include "BluetoothSerial.h"

  #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
  #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
  #endif

  BluetoothSerial SerialBT;

 void setup() {
   Serial2.begin(9600, SERIAL_8N1, 16, 17);
    Serial.begin(115200);
    SerialBT.begin("ESP32MORD"); //Bluetooth device name
   Serial.println("The device started, now you can pair it with bluetooth!");
 }

  void loop() {
   if (Serial2.available()) {
   SerialBT.write(Serial2.read());
   }
   if (SerialBT.available()) {
   Serial2.write(SerialBT.read());
  }
  delay(20);
  }


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
