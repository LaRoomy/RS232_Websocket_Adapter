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
