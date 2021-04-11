## tBeamLoRa

This the sensor node part of a LoRaWAN network to save environmental data to a remote database. The software reads data from several sensors, namely the SPS30 particulate matter sensor, the SEN0170 anamometer, the BME680 temp, press etc module and the LPPYRA03AV pyranometer. A custom shield was also developed: the schematics and the KiCad project files are available [here](https://github.com/Piruzzolo/tBeamLoRa-LoRaWAN-Environmental-sensor-node).


## Setup

##### 0.0 Prerequisites:
- Install ```kconfig``` with ```sudo apt install kconfig-frontends``` or whatever for your distro
- Install ```kconfiglib``` with ```pip install kconfiglib```
- Install ```PlatformIO Core``` (see below)

##### 0.1 Install PlatformIO Core 
##### 0.1.1 For CLion only:
- Install PlatformIO Core (needed cuz the plugin is still experimental) with
```python3 -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/develop/scripts/get-platformio.py)"```

- In CLion go to Edit->Settings->Plugins->Marketplace and search for "PlatformIO for CLion"
- Install it (and restart CLion as prompted)
- Go to Tools->PlatformIO->Home
- [If CLion complains about paths, run (with ```sudo```)

  ```ln -s ~/.platformio/penv/bin/platformio /usr/local/bin/platformio```
  
  ```ln -s ~/.platformio/penv/bin/pio /usr/local/bin/pio```
  
  ```ln -s ~/.platformio/penv/bin/piodebuggdb /usr/local/bin/piodebuggdb```
  
  This will create links to make CLion believe paths are right.
  Restart CLion. Type "pio run" in CLion Terminal]
- In the webpage it opens, click on "Open Project" and select the project's root folder
- Re-open Clion, click on File->"Close project"
- In the launcher window, click on open an existing project and select the project's root folder
- In CLion, open the Terminal click Alt+F12 (or whatever in OS X) 
- Type in "platformio run"
- Done, follow the steps below. 
- Commands to build, clean, upload etc are under Tools->PlatformIO
- To build the Doxygen documentation, go to Build->"Build project", the docs is in html folder


##### 1. Edit ```lmic_project_config.h``` inside one of .pio library folders and uncomment your frequency

##### 2. Edit ```hal.h``` in ```src/hal/hal.h``` inside one of ```.pio``` library folders and replace in line 25 ```u1_t``` with ```unsigned char```

##### 3. Configuration:
- From a terminal (from the project root) type ```menuconfig``` or ```guiconfig```, edit prefs and save
- Then type ```genconfig``` to generate/update the ```config.h``` header file

##### 4. Edit this project file ```main/credentials.h``` and add the Keys/EUIs

##### 5. Compile and upload. For CLion, see above. Otherwise, use Visual Studio Code with the PlatformIO plugin or the command line

##### 6. Turn on the device and once a GPS lock is acquired, the device will start sending data


## Note for CLion users

- Step ```3``` can be done in CLion's Terminal
- If Monitor (serial monitor) under Tools->PlatformIO doesn't work, please open a terminal and type 
        ```screen /dev/ttyUSB0 115200```. Exit with ```CTRL+a``` ```CTRL+d``` *AND* type ```fuser -k /dev/ttyUSB0```.
  
- You can generate the Doxygen documentation with ```Build``` -> ```Build Project```