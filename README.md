# ESP8266 NodeMCU integration with Salesforce IoT (Platform Events) #
Arduino IDE sketches for integrating ESP8266 NodeMCU with Salesforce through publishing and susbcribing to Salesforce Platform Events. 

## Prerequisites ##
If you are new to programming NodeMCU on Arduino IDE, follow these steps.

* Install Arduino IDE from https://www.arduino.cc/en/main/software
* Install the CP2102 drivers for NodeMCU v1.0 from https://www.silabs.com/products/development-tools/software/usb-to-uart-bridge-vcp-drivers
* Install ESP8266 package for Arduino by following these steps
  * Go to Arduino IDE preferences and add the following URL in "Additional Boards Manager URLs" field and save it - http://arduino.esp8266.com/stable/package_esp8266com_index.json
  * Go to Boards Manager and install the package. You should see a package named "esp8266 by ESP8266 Community". You are all set to program NodeMCU v1.0 in Arduino IDE!

## Platform event publisher sketch ## 
Reads sensor data from HC-SR04 ultrasonic sensor and publishes an event to Salesforce Platform Events. This diagram shows the pin connections that was used.

![alt text](../master/platform-event-publisher/esp8266-hcsr04-diagram.png?raw=true)

* Connect NodeMCU Vin pin to Vcc of HC-SR04

* Gnd to Gnd

* D4 to Trig

* D3 to Echo.

The board is powered through micro USB cable.

## Platform event subscriber sketch ##
NodeMCU subscribes to Salesforce platform events via CometD connection (Bayeux protocol). When it receives an event, it turns ON or OFF an LED and an active Buzzer based on the event data.

![alt text](../master/platform-event-subscriber/esp8266-LED-buzzer-diagram.png?raw=true)

* Gnd - Buzzer Gnd

* Gnd - LED Cathode (-ve)

* D2 - Buzzer anode

* D1 - LED anode

## Sketch Instructions ##
* Create a connected App in Salesforce and update the consumer key and consumer secret in sketch files.

* Update the Wifi SSID and Password

* Extract certificate thumbprints for Salesforce Login endpoint and your instance's HTTPS endpoint and update the sketch files.

* Update Salesforce username, password and security token.

* Create Platform Event objects and setup Orchestration in Salesforce IoT. Update the platform event names in sketches. If you are publishing and subscribing to different events then update the same in right sketch files. 

* Compile and upload the sketches to NodeMCU boards using Arduino IDE.
