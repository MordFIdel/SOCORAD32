#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
//defines the I2C pins to which the display is connected
#define OLED_SDA 33
#define OLED_SCL 32

Adafruit_SH1106 display(33, 32);

void setup()   {
  Serial.begin(115200);
  //define the type of display used and the I2C address
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  //set the text size, color, cursor position and displayed text
  display.setTextColor(WHITE);
  display.setCursor(22, 30);
  display.println("Hello SOCORAD32");
  display.display();
}
void loop() {
}
