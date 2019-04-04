/* This Arduino sketch for ESP8266 NodeMCU reads data from HC-SR04 ultrasonic sensor 
 * and publishes it as a platform event to Salesforce. The code is self-explanatory 
 * with inline comments.
*/

#include <ESP8266HTTPClient.h> 
#include <ESP8266WiFi.h> 
#include <ArduinoJson.h>

//WiFi credentials
const char* wifiSSID = "<Your WiFi SSID goes here>"; 
const char* wifiPassword = "<Your WiFi password goes here>";

//HC-SR04 pin definitions, this could vary depending upon the board.
const int TRIG_PIN = 2;  //D4
const int ECHO_PIN = 0;  //D3

//vars to store SF auth token and SF instance url
String sfAuthToken = "";
String sfInstanceURL = "";

//Stor cookies
String cookies;

HTTPClient http;

void connectToWifi(){
   WiFi.begin(wifiSSID, wifiPassword);

  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {    
    delay(250);
  }  
  Serial.println("Connected to WiFi");
}

// Login to Salesforce using OAuth2
void loginToSalesforce() {	
  String username = "<your Salesforce username>";
  String password = "<Salesforce password>";
  String token = "<Your Salesforce security token>";
  String consumerKey = "<consumer key from Connected App settings>";
  String consumerSecret = "<Consumer Secret from Connected app settings>";

  /* SSL Certificate fingerprints, you need one for login.salesforce.com and one for your instance url.
   Follow the steps here to retrieve them using openssl - https://knowledge.digicert.com/solution/SO28771.html
   Or if you would like a GUI then use Portecle - http://portecle.sourceforge.net/ 
  */
  String fingerprint = "0B:33:19:AC:6D:9E:C1:5F:08:AB:93:17:2A:FE:F9:E0:90:69:C7:9A";

  //Your Developer Edition URL. This can also be https://test.salesforce.com
  http.begin("https://login.salesforce.com/services/oauth2/token", fingerprint);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postContent = "?&grant_type=password";
  postContent += "&client_id=" + consumerKey;
  postContent += "&client_secret=" + consumerSecret;
  postContent += "&username=" + username;
  postContent += "&password=" + password + token;

  int httpCode = http.POST(postContent);
  String payload = http.getString();

  http.end();

  DynamicJsonBuffer jsonBuffer(http.getString().length());
  JsonObject& auth = jsonBuffer.parseObject(payload); 

  if (httpCode == 200) {
    Serial.println("Successfully logged in!");
    String token = auth["access_token"];
    String url = auth["instance_url"];
    sfAuthToken = token;
    sfInstanceURL = url;
  } else {
    Serial.println("An error occured, not logged in!");
  }

}

String buildTelemetryData(String level){
   String telemetryData = "{\"DeviceSerialNumber__c\": \"234156\", \"Level__c\": \"";
  telemetryData +=  level + "\"}";
  return telemetryData;
}

void publishEvent(String eventName, String telemetryData){
  String  sfdcInstanceCertFingerprint = "BA:69:22:06:CA:B4:1D:B9:0D:01:AB:1F:98:BB:1F:53:37:64:95:98";
  http.begin(sfInstanceURL + "/services/data/v45.0/sobjects/" + eventName + "/", sfdcInstanceCertFingerprint);
  http.addHeader("Authorization", "OAuth " + (String)sfAuthToken);
  http.addHeader("Content-Type", "application/json");
  
 
  int httpCode = http.POST(telemetryData);
  String payload = http.getString();

  http.end();
  
  DynamicJsonBuffer jsonBuffer(http.getString().length());
  JsonObject& auth = jsonBuffer.parseObject(payload);
 
  if (httpCode >= 200 && httpCode < 300) {
    Serial.println("Successfully published event to Salesforce!");   
  } else {
    Serial.println("An error occured, event not published!");
  }
}

//Arduino setup method
void setup() {
  pinMode(BUILTIN_LED, OUTPUT); //Set the builtin LED in NodeMCU, this can act as a status indicator
  pinMode(TRIG_PIN, OUTPUT); // Sets the trigPin on HC-SR04 as an Output
  pinMode(ECHO_PIN, INPUT); // Sets the echoPin on HC-SR04 as an Input
  
  Serial.begin(9600);
 
  connectToWifi();  
  loginToSalesforce();  
}


int getHCSR04Data(){
  long duration;
  int distance;
   // Clears the trig Pin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  //Sets the trig Pin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  //Reads the echo Pin, returns the sound wave travel time in microseconds
  duration = pulseIn(ECHO_PIN, HIGH);
  
  //Calculating the distance
  distance= duration*0.034/2;

  return distance;
}


//Arduino loop method
void loop() {
	int i;  
	int distance;  
   
  distance = getHCSR04Data();
	  
  // Prints the distance on the Serial Monitor 
  Serial.println("Distance: " + distance);
   
  char snum[5];
  itoa(distance, snum, 10);

  if(distance > 0){
    String telemetryData = buildTelemetryData(snum);
    //Publish this as a platform event to Salesforce
     publishEvent("SewerTelemetryData__e", telemetryData); 
  }   

  //Make the builtin LED to act as Status indicator
  for(i=0;i<20;i++)
  {
    digitalWrite(BUILTIN_LED, LOW);
    delay(200);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(200);
  }
}
