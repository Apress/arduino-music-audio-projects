// Channel set - Mike Cook Feb 2014
#define readInput 5
int channelThreshold[] = {67, 166, 248, 314, 373, 419, 462, 496, 530, 556, 580, 
                          605, 622, 644, 660, 680, 1025}; // mid way between actual readings

void setup(){
  Serial.begin(9600);  
}

void loop(){
  static int lastRead = -3;
  int reading = analogRead(readInput);  // read the MIDI cahnnel switch
  if( abs(lastRead - reading) > 4) {
     Serial.print("Reading ");
     Serial.println(reading);
     Serial.print("Channel ");
     Serial.println(getMIDIchannel(reading));
     lastRead = reading;
  }
}

int getMIDIchannel(int input){
int channel = -1;
    if(input != -1){ 
      channel++;
     while( channelThreshold[channel] < input) channel++;  // convert reading into channel number
    }
    return channel;
}
