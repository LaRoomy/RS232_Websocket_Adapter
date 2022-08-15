# RS232_Websocket_Adapter

This Project could be considered as the next-level development after my accomplishments with the apps for the serial data transmission to/from older EPL2 based machines. There were a few points which bothered me on my recent solutions: First thing is that the computer has to come to the machine, means there hast to be a (short) cable connection. What if I want to send data from the office? Second thing is the limitation to the windows platform, what if I want to use a mac or an android/ios-tablet or something. So this project does accomplish the following:
- Enable data exchange over RS232 interface (especially for EPL2 based machines)
- Exchange this data over a wireless connection in the local network
- Provides a platform independent front-end which can be access via browser

This repository provides all necessary data to build the WebSocket-Adapter.
- CAD data to create the [printed circuit board](CAD/Board)
- CAD data to create the [case](CAD/Case) for the adapter with a 3D printer
- The source code for the [backend/frontend](RS232WebSocketAdapter)

The front-end is accessible via browser from every device in the local network over the url "ncinterface.local".

![front-end view](IMG/html_interface.png)

### Special capabilities of the adapter
- The board contains a switch to select the RX/TX mode. The two lines can be crossed by this switch or used in line.
- The board also has pinheader bars. The level of these bars can be set directly out of the program and indiviually set for each of the hardware-handshake pins of the RS232 interface. This ensures the possibility to make the interface conform to every handshake needs of the machine.

![board specifications](CAD/Board/board_inst.png)

### How it works

The adapter logs in to the local network and advertises its local domain *ncinterface.local* via MDNS. It acts as an webserver. By entering the url *ncinterface.local* the interface website is loaded and remains connected by a websocket. The interface consists of the transmission page and the config page. The RS232 connection parameter can be set using the config page.


### How to config the network

To set the network credentials the serial connection of the adapter must be used. Connect the adapter to a pc via USB to Serial adapter. Make sure all jumper of the handshake-pinheader bars are removed. Otherwise it will not work properly. The config switch of the adapter must be set to the appropriate position. Now connect the power-source and the device will start in config mode. To configure the network every serial terminal program can be used. I recommend to use my free app [*EPL Exchange*](https://epl-exchange.blogspot.com/). The parameters of the config interface are:
- Baudrate: 9600
- Parity: None
- Databits: 8
- Stopbits: 1
- Handshake: None

To set up the SSID (network name): type in "ssid", to set up the password for the network: type in "password" and follow the prompt respectively. Typing in "?" shows instructions to use. To check if the connection works, put to config switch in the off position and press reset. While trying to connect the red led blicks. If the device is connected to the network the blue led is on.

### How get it ready to use

In most modern scenarios the handshake is ignored, known as RS232 3-wire interface, but machines running EPL2 needs specific levels on specific hardware handshake pins on transmission and some pins must be set to GND.








link to Firmware Project

link to Circuit board resources

link to CAD case resources

link to website


1. Description
- platform independent interface
- Access from every device in the private network (subnet)
- platform IO
- ESP8266
- max3232 (rs232 to uart level converter)

<screenshot of the html page>

2. Theory of operation

3. How to assemble/build the whole thing

- esp8266
- max3232 level shifter
- pin header rows to adapt the level of each hardware handshake pin
- parts, switches leds, explanation of funtional parts
- config switch, boot switch
- rx-tx cross switch
- reset switch

3. How to use the adapter

<foto of the adapter plugged in>

- configuration
- data transmission


- notes to compatibility of browsers (edge problem / safari problem)
