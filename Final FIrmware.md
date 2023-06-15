This software is written in vs code using the [v4.4.4 ESP-IDF](https://github.com/espressif/idf-installer/releases/download/offline-4.4.4/esp-idf-tools-setup-offline-4.4.4.exe)


Once you load this software, you can change the channel and volume with the provided buttons on the PCB. 
The channels are what is stored in the code. You are free to change the channel anyway you want, 
just edit the channel storage section in the code. Up to 50 channels or more can be edited to your
preference. Also you don't need to connect to Bluetooth device to do this. Press and hold the button to change channel and press once to change volume.
Connecting to your bluetooth phone, you can use the bluetooth serial software to also change channel by sendng AT+DMOCHN=8 (for channel 8)
AT+DMOCHN=10 (for channel 10).

To edit channel list, in the zip file, navigate to main, then ui.c You'll find this section at the top part of the code. 
This is same  for editing the ctcss. See screenshots below.

![Channel list](https://github.com/MordFIdel/SOCORAD32/assets/88499684/7b41d1e7-2f0b-41b1-a23a-68fc2770b3ba)

![ctcss list](https://github.com/MordFIdel/SOCORAD32/assets/88499684/a1105bc5-e974-43d5-814f-fc22f31f57cb)


This software allows simple and less complex use of the SOCORAD32 directly without bluetooth connetion. 
Other functions provided by the walkie talkie module are accesssible via bluetooth connections to a mobile device.
The connection process is simple. Using the 'Serial Bluetooth Terminal' android app as example (get from PlayStore), 

1. Firstly, connect the SCRD32 to your phone following your phone standard connection process. Passkey is 123456
2. Press the Menu button
3. Select Devices
4. Select Bluetooth LE
5. Press and hold the listed BLE device and choose edit.
6. Choose custom and enter the details as shown in the image below.

![BLE connect process](https://github.com/MordFIdel/SOCORAD32/assets/88499684/fe236797-c1af-4f8d-872b-68ce362304f2)

7. Save the settings and voila!, your SOCORAD32 is connected. You can now use the AT commands directly.
