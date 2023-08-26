Remember the changes we promised. It's here. The old software has too much bugs and so we decided to make this new one, using Bluetooth Low Energy. once you receive your unit. Just follow the simple steps below.

## ESP-IDF

This software is written in vs code using the [v4.4.4 ESP-IDF](https://github.com/espressif/idf-installer/releases/download/offline-4.4.4/esp-idf-tools-setup-offline-4.4.4.exe)

## Channels

Once you load this software, you can change the channel and volume with the provided buttons on the PCB. 
The channels are what is stored in the code. You are free to change the channel anyway you want, 
just edit the channel storage section in the code. Up to 50 channels or more can be edited to your
preference. 
To change the channel, Press and hold the button. To change volume, press once to change volume.
Connecting to your bluetooth phone, you can use the bluetooth serial software to also change channel by sendng AT+DMOCHN=8 (for channel 8)
AT+DMOCHN=10 (for channel 10).

### Edit Default Channels

To edit channel list, in the zip file, navigate to main, then ui.c You'll find this section at the top part of the code. 
This is same  for editing the ctcss. See screenshots below.

![Channel list](https://github.com/MordFIdel/SOCORAD32/assets/88499684/7b41d1e7-2f0b-41b1-a23a-68fc2770b3ba)

![ctcss list](https://github.com/MordFIdel/SOCORAD32/assets/88499684/a1105bc5-e974-43d5-814f-fc22f31f57cb)

## Bluetooth connection

This software allows simple and less complex use of the SOCORAD32 directly without bluetooth connection. 
Other functions provided by the walkie talkie module are accessible via bluetooth connections to a mobile device.
The connection process is simple. Using the 'Serial Bluetooth Terminal' android app as example (get from PlayStore), 

1. Firstly, connect the SCRD32 to your phone following your phone standard connection process. Passkey is 123456
2. Press the Menu button
3. Select Devices
4. Select Bluetooth LE
5. Press and hold the listed BLE device and choose edit.
6. Choose custom and enter the details as shown in the image below.

![BLE connect process](https://github.com/MordFIdel/SOCORAD32/assets/88499684/fe236797-c1af-4f8d-872b-68ce362304f2)

7. Save the settings and voila!, your SOCORAD32 is connected. You can now use the AT commands directly. Input your commands through the app and view response from the SOCORAD32.

## Firmware

Link to FIrmware https://github.com/MordFIdel/SOCORAD32/blob/main/SCRD32%20firmware%20official.zip

## Beginners Guide For Windows

You have to load the firmware to the radio when you get it.
- Download the [ESP-IDF](#ESP-IDF) linked and install it.
- Download the [firmware zip](#firmware), "SCRD32 firmware official.zip", and - Extract the files to a folder with no spaces in name. For example you might extract it to "C:/SCRD32"
- Open the ESP-IDF powershell program
- Navigate to the folder you saved the firmware in, and type ```idf.py build``` This will build the firmware.
- Once the build is complete connect the radio and type ```idf.py -p [Port] flash```

- Change [Port] to the COM port of the socorad I. E. ```idf.py -p COM3 flash``` 

The firmware should now be loaded on the radio.

## Linux / Mac

Useful but optionnal dependency: ccache

### Install esp-idf
```
mkdir -p ~/esp
cd ~/esp
git clone --recursive --branch v4.4.4 https://github.com/espressif/esp-idf.git
cd ~/esp/esp-idf
./install.sh esp32
```

### Clone repo
```
cd ~/
git clone https://github.com/MordFIdel/SOCORAD32.git
```
### Build & Flash
```
cd ~/SOCORAD32/SCRD32_firmware_official/
. $HOME/esp/esp-idf/export.sh (Run it everytime before you want to use esp-idf)
idf.py build
idf.py flash
```

You can directly use the esp-idf extension in Visual Code. Take care to utilize the correct version of ESP-IDF.

Our you could follow the [guide for installing the tool directly on Linux / Mac](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/linux-macos-setup.html#)
