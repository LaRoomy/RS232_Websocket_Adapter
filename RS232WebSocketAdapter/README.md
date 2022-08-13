## RS232 WebSocket-Adapter Firmware project

This is the firmware for the WebSocket-Adapter.
- MCU: Espressif ESP8266
- Development Platform: [PlatformIO](https://platformio.org/)
- Framework: Arduino
- IDE: [Microsoft Visual Studio Code](https://platformio.org/platformio-ide)

The MCU data consists of two elements: the firmware and the filesystem image. The filesystem image is used to store the files for the WebServer (html, css, js).

---

#### How to build
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



Notes:

- first initialization
- evaluation mode - use serial interface
- no jumper!
- special - setting the OUT pinheader level
