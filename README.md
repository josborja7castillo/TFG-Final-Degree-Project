# Final Degree Project: Development of a data visualization and acquisition system for automotive electronics

[![Foo](https://github.com/josborja7castillo/TFG-Final-Degree-Project/blob/master/Hardware/image5.jpg)]

Final Degree Project as part of my Electronic Systems B. E at [Universidad de Málaga](https://www.uma.es/etsi-de-telecomunicacion/).
Also, here you can get more documentation in [Spanish](https://github.com/josborja7castillo/TFG-Final-Degree-Project/tree/Spanish).


This repository will be divided into three sections: 
- One for documentation and snapshots.
- One for Microcontroller's code.
- The last one for the GUI.

## Goals
The aim was to achieve a fully Open Source, completely functional "full-stack" prototype which can expect to receive in car's diagnostic data and treat them like a motorsport based dashboard.

## Current state
Lower two sections are functional, currently working to improve GUI smoothness.
Expecting to add CAN Bus software capability and porting GUI to Android soon.

## MEL: Minimum Equipment List
To further develop and improve this project, it is advisable to have at least, these items:
- Function generator.
- Power supply.
- Soldering station.
- Digital oscilloscope with serial decoding (not mandatory but highly recommended).
- A previously routed PCB (Gerbers included [here](https://github.com/josborja7castillo/TFG-Final-Degree-Project/tree/master/Hardware/PCBs)).
- Linux based PC equipped with Bluetooth.
- An ISO 14230 compliant vehicle.
- Bluetooth to UART module (HC-05 in our case).

## BOM: Bill of Materials
- EK-TM4C123GXL evaluation board.
- DB9 male connector.
- OBDII interface to DB9 female plug.
- 1206 510 Ohm resistors.
- 1206 60 Ohm resistors.
- 4.7 nF capacitor.
- 5 decoupling capacitors.
- 2 ST L9637D transceivers.
- 1 TCAN332 transceiver.
- 1 LM1117-3.3 voltage regulator.
- 4 pin right angle jumper.
- Standard straight jumpers.

## Author
- José Borja Castillo Sánchez, under the direction of Mr. Gabriel Valencia Miranda.

## License
 This project is covered by [GPL](http://www.gnu.org/licenses/quick-guide-gplv3.html).