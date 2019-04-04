# ESP8266 NodeMCU integration with Salesforce IoT (Platform Events) #
Arduino IDE sketches for integrating ESP8266 NodeMCU with Salesforce through publishing and susbcribing to Salesforce Platform Events. 

## Sketch: platform-event-publisher.ino ##
Reads sensor data from HC-SR04 ultrasonic sensor and publishes an event to Salesforce Platform Events

Pin connections between ESP8266 NodeMCU and HC-SR04

Connect NodeMCU Vin pin to Vcc of HC-SR04

Gnd to Gnd

D4 to Trig

D3 to Echo.

NodeMCU is powered through micro USB using an external battery pack.


## Sketch: platform-event-subscriber.ino ##
NodeMCU subscribes to Salesforce platform events via CometD connection (Bayeux protocol). When it receives an event, it turns ON or OFF an LED and an active Buzzer based on event data.

Pin connections between NodeMCU, LED and active Buzzer

Gnd - Buzzer Gnd

Gnd - LED Cathode (-ve)

D2 - Buzzer anode

D1 - LED anode


## Sketch Instructions ##
1. Create a connected App in Salesforce and update the consumer key and consumer secret in sketch files.

2. Update the Wifi SSID and Password

3. Extract certificate thumbprints for Salesforce Login endpoint and your instance's HTTPS endpoint and update the sketch files.

4. Update Salesforce username, password and security token.

5. Create Platform Event objects and setup Orchestration in Salesforce IoT. Update the platform event names in sketches. If you are publishing and subscribing to different events then update the same in right sketch files. 

6. Compile and upload the sketches to NodeMCU boards using Arduino IDE.
