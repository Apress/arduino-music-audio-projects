/*
  WiFi UDP Send and Receive OSC
  Mike Cook Jan 2014
 
 This sketch wait an OSC UDP packet on localPort using a WiFi shield.
 For use with page 1 & 3 of the "simple.touchosc" layout
 
 based on an example by dlf (Metodo2 srl)
 LEDs pins 3,5 & 6 common anode RGB (sink), pin 9 single LED (source)
 */

#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>
#include <OSCBoards.h>


int status = WL_IDLE_STATUS;
char ssid[] = "yourNetwork";   //  your network SSID (name) 
char pass[] = "yourPassword";  // your network password (use for WPA, or use as key for WEP)

IPAddress iPad(192, 168, 1, 102);  // address of iPad running OSC
unsigned int inPort = 8000;  // local port to listen on needs to match OSC's outgoing port
unsigned int outPort = 8001; // local port to talk to on needs to match OSC's incoming port

byte leds[] = {3, 3, 5, 6, 9};  // first entry just a dummy

WiFiUDP Udp;  // instance of the WiFi handler

void setup() {
  //Initialise serial and wait for port to open:
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println(" ");
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present"); 
    // don't continue:
    while(true); // hold here
  } 
  
  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) { 
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);
  
    // wait 10 seconds for connection:
    delay(10000);
  } 
  Serial.println("Connected to wifi");
  printWifiStatus();
  
  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(inPort); 
  for(int i=1 ; i<5; i++) {
  pinMode(leds[i], OUTPUT);
  digitalWrite(leds[i],HIGH);
     }
  delay(500);
  digitalWrite(leds[4],LOW);
}

void loop() {
  checkAnalogue(); // look at real analog ports 
  // if there's data available, read a packet
  OSCMessage messageIN;
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {  
      while(packetSize--)
       messageIN.fill(Udp.read());
       if(!messageIN.hasError())
       printMessage(messageIN);
     {
        messageIN.route("/1", routeScreen1); // add others for other screens
        messageIN.route("/3", routeScreen3);
     }

 }
}

void checkAnalogue(){
  static int lastAnalogueValue;
  int currentAnalogueReading;
    currentAnalogueReading = analogRead(0);
    if(abs(currentAnalogueReading - lastAnalogueValue) > 2){
      lastAnalogueValue = currentAnalogueReading;
      OSCMessage msg("/1/fader5");
      msg.add((float)currentAnalogueReading / 1023.0);
      // now send the message to the outPort
   // Udp.beginPacket(Udp.remoteIP(),outPort); 
     Udp.beginPacket(iPad,outPort); 
    msg.send(Udp);
    Udp.endPacket();
    msg.empty(); // empty the bundle ready to use for new messages
   }  
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

//incoming  messages

/**
 * Route Screen 1 - OSC 5 sliders and 4 toggle switches
 * 
 * called when address matched "/1"
 * expected format:
 * /1/toggle(N) 
 * and 
 * /1/fader(N)
 *   (N) = is 1 to 4
 *   (value) = 1.0 or 0.0 or any value in between for a fader
 */

void routeScreen1(OSCMessage &msg, int addrOffset ){
  //match input
  int match = -1;
  
  for(int i=1; i<5; i++){
   if(msg.match(prefixPulsNumOSCAddress("/toggle",i), addrOffset) != 0) match = i;
  }
  if(match != -1) { // if we have a match for a /toggle address
  float data = msg.getFloat(0);
  if(match == 4)
  if(data == 1.0) analogWrite(leds[match], 255); else analogWrite(leds[match], 0);
  else
  if(data == 1.0) analogWrite(leds[match], 0); else analogWrite(leds[match], 255);
  }
  else { // if not a toggle look for a fader
      for(int i=1; i<5; i++){  // don't look for fader 5
       if(msg.match(prefixPulsNumOSCAddress("/fader",i), addrOffset) != 0) match = i;
      }
      if(match != -1) { // if we have a match for a /fader address
        float data = msg.getFloat(0);
          if(match == 4)
          analogWrite(leds[match], data * 255); // set brightness
          else
          analogWrite(leds[match], 255 - (data * 255)); // set brightness
      }
  }
}

/**
 * Route Screen 3 - XY pad and 4 toggle switches (not matched for)
 * 
 * called when address matched "/1"
 * expected format:
 * /3/xy 
 *   (value) = two floats between 1.0 and 0.0
 *  Results printed out in the serial monitor
 */
void routeScreen3(OSCMessage &msg, int addrOffset ){
  char pad[] = "/xy";
    if(msg.match(pad, addrOffset) ){
       float data = msg.getFloat(0);
       Serial.print("X = ");
       Serial.print(data);
       float data2 = msg.getFloat(1);
       Serial.print("  Y = ");
       Serial.println(data2);
       analogWrite(leds[3], data * 255); // set brightness
       analogWrite(leds[4], data2 * 255); // set brightness
    }
}

char * prefixPulsNumOSCAddress( char * prefix, int num){
    static char s[12]; // space to construct the string
    int i = 11; // last location in the string
    int len = 0;
    while(prefix[len] != '\0') len++; // find the length of the prefix char array
	
    s[i--]= '\0';  // add a null at the end
	do
        {
		s[i] = "0123456789"[num % 10];
                --i;
                num /= 10;
        }
    while(num && i); // keep on going until num or i drop to zero 
    i++;  // compensate for last --i
    for(int j=0; j<len; j++){ // add the prefix string backwards
      s[i - len + j] = prefix[j];
      }
    return &s[i-len]; // return char array and point to first byte
}
void printMessage(OSCMessage &msg){
  char address[255];
  int len = msg.getAddress(address, 0);
  for(int i=0; i<len; i++)
    Serial.print(address[i]);
    Serial.print(" with data ");
    float data = msg.getFloat(0);
    Serial.print(data);
  Serial.println(" ");
}
