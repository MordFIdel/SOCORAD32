# SOCORAD32 - ESP32 SOftware COntrolled RADio

<br/> (3D enclosure design already released)
<br/>
<br />Add a speaker and a battery, then you'll have a fully functional walkie talkie Radio. It shares the same properties as the commercial one. Can be used both for voice and data communication. Because it can communicate data, we can explore the amateur radio frequency for IoT.
<br/>Beyond communication, it is fully open source and hackable!
<br />The central module contains the special purpose RDA1846 IC. This same IC is used in commercial walkie talkies such as in Baofeng and Hytera. <br/>

<br/>Other of its peripherals is an onboard ESP32 for parameter settings (via Bluetooth or WiFi), control and storage. 
<br/>The ESP32 talks to the central module using simple AT commands. The specifications are listed below.

# Specifications
Frequency Range: 400-470mhz covers most countries allocated walkie talkie frequencies, especially the license free ones.
<br/> Frequency Step: 5K/6.25K/12.5K/25K
<br/> RF Output Power: 2W/1W/0.5W
<br/> Voice features, Tone Squelches, SMS cabapility:
<br/> Voice encryption (scrambling): 8 types
<br/> SMS Receive/Transformer, baudrate: 1200
<br/> CTCSS (38 groups) + CDCSS (83 groups)
<br/> Automatic elimination tail
<br/> Volume adjustable (1-8)
<br/> Squelch level adjustable (0-9)
<br/> MIC sensitivity level adjustable (1-8)
<br/> Sleep Mode (0.1μA)
<br/><br/>To talk to the module, a generic Bluetooth serial communication APP is used on the phone to connect to the ESP32 module which in turn communicates with the RF module via UART. Due to the SOCORAD32 rich features, it can be used for the following applications.


# Applications
Walkie Talkie
<br/>Intercom
<br/>Pager
<br/>IoT (data transmission module)
<br/><br/>The AT commands are in the repository. It includes commands for volume control, power output, RX/TX frequencies, voice activated exhange, squelching, etc.
<br/>The device contains a dedicated PTT button and extra buttons for user configurations.

<br/>![SOCORAD32 high rez](https://user-images.githubusercontent.com/88499684/215540777-c825e2d2-a014-41b9-847e-6e92eacf6c23.png)

## First run / Flashing
See [Final Firmware](Final_Firmware.md) for instructions
