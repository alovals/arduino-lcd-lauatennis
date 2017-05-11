//lisame emuleerimiseks vajaliku teegi
#include <SoftSerial.h>

//digispargi viigud ei vasta viikude nimetusele plaadil, vajalik vaadata digispark pinouti
int digiRx = 1;
int digiTx = 2;
int joyY = A3;
int joyX = A2;

//loome emuleeriva objekti
SoftSerial BtSerial(digiRx, digiTx);

void setup() {
  //alustame emulatsiooniga
  BtSerial.begin(38400);
}
void loop() {

  //kas on päring saadetud
  if (BtSerial.available() > 0){
    char b = BtSerial.read();

    //kas päring on x-teljelise muutuse kohta?
    if (b== 'x'){
      int x = analogRead(joyX);
      if (x < 200){
        BtSerial.print(1);
      }
      else{
        BtSerial.print(0);
      }
    }
    //kas päring y-teljelise muutuse kohta?
    else if (b== 'y'){
      int y = analogRead(joyY);
      
      //kontrollime juhtkangi liigutust
      if (y<771){
        BtSerial.print(2);
      }
      else if (770<y && y<785){
        BtSerial.print(1);
      }
      else if (784<y){
        BtSerial.print(0);
      }
      
    }
  }
}
