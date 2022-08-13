## RS232 WebSocket-Adapter Firmware project

This is the firmware for the WebSocket-Adapter.
- MCU: Espressif ESP8266
- Development Platform: [PlatformIO](https://platformio.org/)
- Framework: Arduino
- IDE: [Microsoft Visual Studio Code](https://platformio.org/platformio-ide)

---

### How to build
- Download [Visual Studio Code](https://platformio.org/platformio-ide)
- Install the PlatformIO IDE Extension
- Extensions for code management are also necessary. We need support for C/C++ and html/css/javascript
- Clone this Repository to your local machine
- Open the sub-folder *RS232WebSocketAdapter* of the Repository in VSCode. Or use the PlatformIO Extension and select *Open Project*. Anyway the opened folder must contain the *platformio.ini* file
- Click on the PlatformIO icon on the left pane to open the project tasks.
- Make sure to enter the boot mode of the device by switching the boot switch in the right position and then press reset to enter boot mode
- Select *Build Filesystem Image* and when finished select *Upload Filesystem Image*
- Open the *root.h* file in the editor.

Here are two defines:
```C++
// if set, this enables some functions which are only intended for testing purposes
#define     EVALUATION_MODE

// if set, this initializes the eeprom persistent data, only intended for initial startup of the device
#define     INITIAL_MODE 
```
If the evaluation-mode define is set, the device outputs log messages on the serial interface. This is helpful while developing, but is not allowed if the device is used as intended since the serial interface is used to transmit data to the rs232 endpoint.

When first uploading the firmware to the device, the initial-mode define should be set to init the (pseudo)eeprom data segment with valid values. Otherwise the config parameter are loaded with invalid values. But if this is done, the define must be commented out and the firmware must be overridden with the new binary.

- Select *Build* in the project tasks list on the left
- When finished, select *Upload*
- If this was the first build, disable the initial-mode define value and click *Upload* again

### Sequence of the program
- On startup the device checks the configuration switch
- If it is set, the device enteres the configuration mode and the network credentials can be configured via serial interface
- Otherwise the device tries to connect to saved ssid/password combination
- If the connection succeeded, the device enteres data transmission mode and the interface can be accessed via *ncinterface.local* inside of the local network
- If the connection fails, the device enters the configuration mode

### Source Files
The MCU data consists of two elements: the firmware and the filesystem-image.

The filesystem image is used to store the files for the WebServer (html, css, js):
- **webSocket_nc_Transmission.html**  (front-end code for the transmission page of the interface)
- **webSocket_nc_Configuration.html**  (front-end code for the configuration page of the interface)
- **exStyles.css**  (provides the styles for the html pages)
- **exec_script.js**  (implements the back-end websocket functionality, and page behavior)

The C++ source files are used to generate the binary for the MCU:
- **root.h / root.cpp**  (implements the main functionality)
- **serial.h / serial.cpp**  (implements the serial interface communication)
- **stringtable.h**  (provides the strings for the user-interface)
- **main.cpp**  (entry-point of the WebSocket-Adapter firmware):

```C++
#include "root.h"

// create root instance
RootComponent rootC;

// valid access to root instance from all locations
RootComponent* getRootClass(){
  return &rootC;
}

void setup(){
  // initialize component
  rootC.init();
}

void loop(){
  // run
  rootC.onLoop();
}
```
As you can see, the whole functionality is coverd by the instance of the RootComponent object.

---

### Note:
With the pinheaders on the board it is possible to set each hardware handshake pin to a specific level (HIGH/LOW). Place a jumper bridge either to the GND pinheader or to the OUT pinheader. The OUT pinheader can be controlled from the firmware to customize the levels of the pin on a specific operation.

**But when programming or configuring the device, all jumper must be removed to ensure a reliable operation.**
