/*This sketch subscribes ESP8266 NodeMCU to a Salesforce Platform Event via CometD/Bayeux connection
 * When an event is received, NodeMCU turns ON/OFF an LED and an Active Buzzer. 
 * 
 */
 
#include <ESP8266HTTPClient.h> 
#include <ESP8266WiFi.h> 
#include <ArduinoJson.h>

// Define NodeMCU D2 pin  connect to Piezo Buzzer
#define BUZZER_PIN  D2
#define LED_PIN D1

//WiFi credentials
const char* wifiSSID = "<Your WiFi SSID goes here>"; 
const char* wifiPassword = "<Your WiFi password goes here>";

bool buzzerSwitch = false;
String takeAction = "false";


/* SSL Certificate fingerprints, you need one for login.salesforce.com and one for your instance url.
   Follow the steps here to retrieve them using openssl - https://knowledge.digicert.com/solution/SO28771.html
   Or if you would like a GUI then use Portecle - http://portecle.sourceforge.net/ 
*/
const char*  sfInstanceFingerprint = "BA:69:22:06:CA:B4:1D:B9:0D:01:AB:1F:98:BB:1F:53:37:64:95:98";

//vars to store SF auth token and SF instance url
String sfAuthToken = "";
String sfInstanceURL = "";

//Store cookies
String cookies;
String cometdClientId;

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

void setupCometdConnection(String event) {
  Serial.println("Establishing CometD Connection...");
  
  DynamicJsonBuffer jsonBuffer(2048);
  bool handshakeSuccessful = false;  
  doBayeuxHandshake();
  String connectionResponse = doBayeuxConnect();
  JsonObject& connObject = jsonBuffer.parseObject(connectionResponse);
  
  String subResponse = doBayeuxSubscribe("/event/" + event);
    
  Serial.println("Connected and subscribed to topic " + event);

}

String cleanPayload(String payload) {
  return payload.substring(1, payload.length() - 1);
}

void doBayeuxHandshake(){
  const char * headerKeys[] = {"Set-Cookie"};

  DynamicJsonBuffer jsonBuffer(2048);
  JsonObject& handshake = jsonBuffer.createObject();
  
  handshake["channel"] = "/meta/handshake";
  handshake["version"] = "1.0";
  handshake["minimumVersion"] = "1.0";
  JsonArray& supportedConnectionTypes = handshake.createNestedArray("supportedConnectionTypes");
  supportedConnectionTypes.add("long-polling");

  JsonObject& advice = handshake.createNestedObject("advice");
  advice["timeout"] = 20000;

  http.begin(sfInstanceURL + "/cometd/45.0", sfInstanceFingerprint);
  http.addHeader("Authorization", "OAuth " + (String)sfAuthToken);
  http.addHeader("Content-Type", "application/json");
  http.collectHeaders(headerKeys, 1);
  
  String handshakePOST;
  handshake.printTo(handshakePOST);
  int httpCode = http.POST(handshakePOST);
  cookies = http.header("Set-Cookie");

  JsonObject& handshakeResponse = jsonBuffer.parseObject(cleanPayload(http.getString()));
  String cl = handshakeResponse["clientId"];
  cometdClientId = cl;
    
  cookies.replace(",",";");  
}

String doBayeuxConnect(){
  DynamicJsonBuffer jsonBuffer(2048);
  JsonObject& connectObj = jsonBuffer.createObject();

  connectObj["channel"] = "/meta/connect";
  connectObj["clientId"] = cometdClientId;
  connectObj["connectionType"] = "long-polling";
  
  String connectionPOST;
  connectObj.printTo(connectionPOST);

  String connectionRES = postToCometdServer(connectionPOST);
  return connectionRES;  
}

String doBayeuxSubscribe(String topic){
  DynamicJsonBuffer jsonBuffer(2048);
  JsonObject& subscribeObj = jsonBuffer.createObject();

  subscribeObj["channel"] = "/meta/subscribe";
  subscribeObj["clientId"] = cometdClientId;
  subscribeObj["subscription"] = topic;

  String subscribePOST;
  subscribeObj.printTo(subscribePOST);  

  String subscribeRES = postToCometdServer(subscribePOST);
  return subscribeRES;
}


String doLongPoll() {
  DynamicJsonBuffer jsonBuffer(2048);
  JsonObject& connectObj = jsonBuffer.createObject();
  connectObj["channel"] = "/meta/connect";
  connectObj["clientId"] = cometdClientId;
  connectObj["connectionType"] = "long-polling";
  
  String connectionPOST;
  connectObj.printTo(connectionPOST);
  
  String pollRES = postToCometdServer(connectionPOST);
  return pollRES;
}

String postToCometdServer(String postData){   
  http.begin(sfInstanceURL + "/cometd/45.0", sfInstanceFingerprint);
  http.addHeader("Authorization", "OAuth " + (String)sfAuthToken);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Cookie", cookies);
  
  int httpCode = http.POST(postData);
  String response = http.getString();  
  http.end();
  
  if(httpCode > 299){
    Serial.println("Post to CometD server failed! Http Status code:" + httpCode);
  }
      
  return response;
}

//standard arduino setup method
void setup() {
  // put your setup code here, to run once:
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BUILTIN_LED, OUTPUT); 
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600);
    
  connectToWifi();
  loginToSalesforce();  
  setupCometdConnection("DrainFlowControl__e");  
}

void toggleLedAndBuzzer(){
  if(takeAction == "true"){
    digitalWrite(LED_PIN, LOW);    
    digitalWrite(BUZZER_PIN, LOW);
    delay(250);
    digitalWrite(LED_PIN, HIGH);    
    digitalWrite(BUZZER_PIN, HIGH);
       
  }else{
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  }
}

void loop() {
  digitalWrite(BUILTIN_LED, LOW);
  DynamicJsonBuffer jsonBuffer(2048);   
  //only output if we have data
  String pollResult = doLongPoll();
  if(pollResult.length() > 0) {
    Serial.println("POLL RESULT = " + pollResult);    
    JsonArray& array = jsonBuffer.parseArray(pollResult);    
    const char* switchOn = array[0]["data"]["payload"]["SwitchOn__c"];
    takeAction = switchOn;   
  } 
  digitalWrite(BUILTIN_LED, HIGH);       
  toggleLedAndBuzzer();
}
