# nodemcu-salesforce-iot
Arduino IDE sketches for integrating ESP8266 NodeMCU with Salesforce through publishing and susbcribing to Salesforce Platform Events.

## Platform-event-publisher.ino 
Reads sensor data from HC-SR04 ultrasonic sensor and publishes an event to Salesforce Platform Events

### Pin connections between ESP8266 NodeMCU and HC-SR04
NodeMCU --  Ultrasonic Sensor
Vin - Vcc
Gnd - Gnd
D4 - Trig
D3 - Echo
NodeMCU is powered through micro USB using an external battery pack.


## platform-event-subscriber.ino
NodeMCU subscribes to Salesforce platform events. When it receives an event, it turns ON/OFF an LED and an active Buzzer.

### Pin connections between NodeMCU, LED and active Buzzer
Gnd - Buzzer Gnd
Gnd - LED Cathode (-ve)
D2 - Buzzer anode
D1 - LED anode
