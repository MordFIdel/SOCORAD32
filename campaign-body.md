# Main Title
SOCORAD32, or the ESP32 SOftware COntrolled RADio is a professional grade, Hack-able Walkie-Talkie for amateur radio
exploration, voice, and data communication using simple AT commands. Add a speaker and a battery, then you'll have a
fully functional walkie talkie Radio. It features real standard operations of a walkie talkie. For this, SOCORAD32 employs a
proprietary RF design featuring the RDA1846 IC, This same IC is used in commercial walkie talkies such as in Baofeng,
Motorola and Hytera. Therefore, SOCORAD32 can communicate with the comerical walie talkie when on the same
settings. How is SOCORAD32 different from your commercial walkie talkie? with the included functions, features, and high-level configurabilty, 
it is designed for exploration. 

SOCORAD32 can communicate data, so we can explore the amateur radio frequency for IoT or send a text. The text can
be read via the onboard OLED screen or on the bluetooth connected mobile device. Beyond communication, it is fully
open source and hackable!
Overall, our device makes it interesting for users who want to explore the intricacies of amateur radio, or explore portable
two-way radio transceiver or a long distance audio or data communications similar to LoRa functions, only this time, it is
done using easy to understand AT commands. This project makes all that simple and fun. The frequency range covers the
license free bands for most countries. To make it more handy, we added 18650 battery holder plus charger to the design. We will aslo release 3D
design enclosure files for free very soon.

## **Designed for Exploration**
With the ESP32 bluetooth function, all settings can be done via a connected mobile device using a serial
blueooth app of any choice, or using the dedicated buttons. with the onboard memory of ESP32, user can store as
many channels as possible. For the high level enthusiasts, the RF module can be opened and tinkered with, such as
upgrading the power amplifier, antenna, filter, matching circuit, etc. Unlike using an SDR for amatuer radio operation, SOCORAD32 as an amatuer radio
tailored device makes it simpler. Using simple AT commands via serial or bluetooth, user can configure the audio volume, tone squelching,
CTCSS, CDSS codes, etc as shown in the product features sections.

**_Please place product image here_**

## **Lists of product features and specifications**
Frequency Range 400mhz -470mhz (covers most countries allocated license free Walkie Talkie bands.)
Frequency Step: 5K/6.25K/12.5K/25K (anyhow you want it)
RF Output Power: 2W/0.5W (set to what the local law permits) see video below.
RF Input sensitivity level: -122dBm see video below.
Voice encryption (scrambling): 8 type (Keep your conversion secret. You can toy with this fucntion)
Voice Compression/Expansion: (check it out)
SMS Receive/Transmit: (yes, send and receive SMS! for IoT, send and receive data!)
CTCSS (38 group) + CDCSS (83 group): (plenty choices)
Automatic Tail Elimination: (Mutes/eliminates the squelch burst often heard at the end of a transmission as the PTT is released.)
Volume Adjustable (1-8) (control the speaker volume, your choice)
AT commands for settings: for example, to turn on VOX (Voice Operated Exchange), use AT+DMOVOX=X, where X is the VOX sensitivity level 1 to 8. To change communication parameters, use AT+DMOGRP=RFV, TFV, RXCT, TXCT, FLag, Flag1. AT+DMOGRP=450.02500, 450.02500, 7006, 7006, 0, 0. Where
RFV is receive frequency
TFV is transmit frequency
RXCT is CTCSS/CDCSS coding
TXCT is CTCSS/CDCSS transmit coding
Flag is busy locking/band setting (narrow or wide)
Flag1 is trasmit power settings high(2W) or low(0.5W)

### TX power at 2W
https://user-images.githubusercontent.com/88499684/211207319-9ae97274-20fc-4fe0-8b27-a4741dbacebb.mp4

### Receive sensitivity test at -120dBm!
https://user-images.githubusercontent.com/88499684/211207403-86377388-e196-4a83-b71c-55ff59fd805c.mp4

### SMS
1 ![SMS1](https://user-images.githubusercontent.com/88499684/211209893-82097bb5-60c5-4e75-83d4-df15435ade6c.png)
2 ![SMS2](https://user-images.githubusercontent.com/88499684/211209900-5dd5749b-2c7d-45b9-9fc6-aa434577b3e3.png)
3 ![SMS3](https://user-images.githubusercontent.com/88499684/211209909-269055eb-6a85-431c-8dbd-4b6eecbd22d5.png)

## List of product use-cases
- Walkie talkie
- Intercom
- Pager
- IoT (data transmission module)
- Audio monitor (voice activated)

## Support, Documentation, and Project files
All of these are tucked neatly in our [Githoub repo](https://github.com/MordFIdel/SOCORAD32). To maintain cohesion and organization, we will also use the SOCORAD32 github repo as the forum area for questions, support, collaborations and everything SOCORAD.

## Manufacturing Plan
We are working with PCBWay. They are a trusted PCB manufacturing partner and have been handling our prototype and few units run. We have worked with them for a while and we fully trust their ability and the quality of their work. They are ready to manufacture, test and coordinate the whole production process. They also understand our components and how to properly handle them. All units will be tested right at their factory before shipping.

## Fulfillment & Logistics
All orders will be fulfilled by Crowd Supply’s fulfillment services (run by Mouser Electronics). So backers, be rest assured that you will receive the units in a timely fashion and well-packed. Once manufacturing is complete and the modules have been assembled, flashed and tested, we’ll make a bulk shipment to Mouser Electronics, who will handle distribution to backers worldwide. You can learn more about Crowd Supply’s fulfillment service under Ordering, Paying, and Shipping in their guide.

## Risks & Challenges
Hardware design is a tricky process and there are many potential pitfalls which could lead to a delay in production. We have extensive experience in designing hardware and software and delivering products to customers all over the world. SOCORAD32 hardware is has completed revision. In total, we have built 10 units. We also have a very good relationship with our contract manufacturer which will help make the delivery of this module as smooth as possible.

The semiconductor crisis has made hardware manufacturing a lot more complex with certain ICs becoming impossible to get hold of for the foreseeable future. One missing chip can be enough to stall the entire production. As an anti-semiconductor-crisis move, we have secured all the key components necessary for our initial production run. This includes the RF module and the ESP32 module. Thankfully with a bit of patience we have been able to acquire a decent quantity of these components meaning we will be ready to press go on the final production run before the end of the campaign.

Finally, we have taken our time to identify areas of possible failures and challenges with a side-by-side solutions on how to mitigate those challenges. 
