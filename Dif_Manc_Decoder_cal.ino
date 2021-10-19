// Differential Manchester decoder
// Zack Nelson
const byte pulsePin = 2; //interrupt input

int byte_count = 0; //count for printing newlines
uint32_t last_ts = 0; //timestamp of prev edge
byte edge_count = 0; //edges per bit
byte bit_count = 0;
byte rec_Byte = 0;

//calibration-----------------------------------
unsigned int hi_threshold = 0; //hi pulse in uS
unsigned int cal_count = 0;
unsigned int cal_ts = 0;
bool lead_in_done = 0;

void setup() {
  pinMode(pulsePin, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(pulsePin), count, CHANGE );
  
  Serial.begin(230400);
  Serial.print("Start. ");
  
  while(!hi_threshold); //Calibration done--------------------------
  Serial.print("High threshold(us): ");
  Serial.println(hi_threshold);
}
  
void loop() { }

void count() { //gets called on every transition of data pin
  if (!hi_threshold){ //Calibration---------------------------------
    if (cal_count == 32) cal_ts = 0; //skip 0-31 readings
    cal_ts += (micros() - last_ts); //average 16 pulses
    if (++cal_count == 48) hi_threshold = cal_ts / 21; //calc 75%
  } else { //Receive data--------------------------------------------
    bool bit_val = ((micros() - last_ts) > hi_threshold); //hi or lo?
  
    //lead in check--------------------------------------------------
    if (!lead_in_done && !bit_val) lead_in_done = 1; //first zero

    if (++edge_count > 2) { //error
      Serial.println("Edge cnt err");
      edge_count = 1;
    }
    
    //low bit = 2 fast pulses, high = 1 slow pulse
    if ((!bit_val && edge_count == 2) || (bit_val && edge_count == 1)){
      if (lead_in_done) bitDone(bit_val); //add bit to byte
      edge_count = 0;
    }
  }
  
  last_ts = micros();
}

void bitDone(bool bit_val) {
  //start bit lo, 8 bits MSB first, stop bit hi
  if (bit_val) rec_Byte |= (0x80 >> (bit_count-1));

  if (bit_count == 0 && bit_val) Serial.println("Start err");
  else if (bit_count == 9 && !bit_val) Serial.println("Stop err");
  
  if (++bit_count == 10) { //complete byte?
    //Uncomment to print hex
    /*if (rec_Byte < 16) Serial.print(0); //leading zero
    Serial.print(rec_Byte, HEX);
    Serial.print(", ");
    if (++byte_count % 16 == 0) Serial.println(""); */
    Serial.print((char)rec_Byte); //print ASCII character
    
    bit_count = 0;
    rec_Byte = 0;
  }
}