/*
OSC Keypad - Mike Cook Jan 2014
*/
#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <OSCBundle.h>
#include <OSCBoards.h>
#define TONE_PIN 8

int notes[] = {262, 262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494,
               523, 554, 587, 622, 659, 698, 740, 784, 831, 880, 932, 988 } ;
               
int status = WL_IDLE_STATUS;
char ssid[] = "yourNetwork"; //  your network SSID (name) 
char pass[] = "yourPassword";    // your network password (use for WPA, or use as key for WEP)

IPAddress iPad(192, 168, 1, 102);  // address of iPad running OSC
unsigned int inPort = 8000;  // local port to listen on needs to match OSC's outgoing port
unsigned int outPort = 8001; // local port to talk to on needs to match OSC's incoming port

WiFiUDP Udp;  // instance of the WiFi handler

void setup() {
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW); // LED off
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
  digitalWrite(9,HIGH); // show connected
  Udp.begin(inPort); 
}

void loop() {
  // if there's data available, read a packet
  OSCMessage messageIN;
  int packetSize = Udp.parsePacket();
  if(packetSize)
  {  
      while(packetSize--)
       messageIN.fill(Udp.read());
       if(!messageIN.hasError())
     {
        messageIN.route("/1", routeScreen1);
     }
 }
}

/**
 * Route Screen 1 - OSC /push(n)
 * 
 * called when address matched "/1"
 * expected format:
 * /1/push(N) 
 *   (N) = is 1 to 4
 *   (value) = 1.0 for a push or 0.0 for release
 */

void routeScreen1(OSCMessage &msg, int addrOffset ){
  //match input
  int match = -1;
  
  for(int i=1; i<24; i++){
   if(msg.match(prefixPulsNumOSCAddress("/push",i), addrOffset) != 0) match = i;
  }
  if(match != -1) { // if we have a match for a /push address
  float data = msg.getFloat(0);
  if(data == 0.0) {
    noTone(TONE_PIN); // stop playing
  }
   else {
    tone(TONE_PIN, notes[match]); // Play note
    }
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
