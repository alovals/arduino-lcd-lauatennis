//ILI9341 LCD driver - ekraanil

// !!! WProgram.h error, siis tuleb TimedAction.cpp failis muuta include WProgram.h Arduino.h-ks
#include <TimedAction.h>

#include <SPFD5408_Adafruit_GFX.h>   
#include <SPFD5408_Adafruit_TFTLCD.h> 
#include <SPFD5408_TouchScreen.h>

#define YP A1
#define XM A2
#define YM 5
#define XP 6 

//need väärtused vajalikud vajutuse väärtuse ümbermappimiseks
//vastasel juhul ei kattu tspointi x ja y väärtus tft koordinaatidega
//calibrate näite koodist saab need kätte
#define TS_MINX 133
#define TS_MINY 139
#define TS_MAXX 900
#define TS_MAXY 920

#define MINPRESSURE 30
#define MAXPRESSURE 1000

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//näitekoodist
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//näitekoodist
#define  BLACK   0x0000
#define RED     0xF800
#define GREEN   0x07E0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft (LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

/*
 * //definitsioonid juhtmetega juhtkangide jaoks
#define SW_LEFT 37 // digital pin connected to switch output
#define X_LEFT 13 // analog pin connected to X output
#define Y_LEFT 12 // analog pin connected to Y output

#define SW_RIGHT 44 // digital pin connected to switch output
#define X_RIGHT 10 // analog pin connected to X output
#define Y_RIGHT 9 // analog pin connected to Y output 
 */


//nupud
Adafruit_GFX_Button backButton;
Adafruit_GFX_Button creditsButton;
Adafruit_GFX_Button creditsButton2;
Adafruit_GFX_Button aboutGameButton;
Adafruit_GFX_Button aboutGameButton2;
Adafruit_GFX_Button startGenerating;
Adafruit_GFX_Button stopGenerating;
Adafruit_GFX_Button doRestart;

//uint tüüp ei lähe miinustesse!!
//pinksi palli informatsioon
uint16_t ballRadius = 3;
int ballX;
int ballY;
int ballXold;
int ballYold;
int ballSpeedX;
int ballSpeedY;
//palli liikumiskiirused eri reketi osadelt
int bounceNormalSpeedX = 1;
int bounceNormalSpeedY = 5;
int bounceDiffAreaSpeedX = 3;
int bounceDiffAreaSpeedY = 4;
int racketTipSpeedX = 6;
int racketTipSpeedY = 2;
     
//reketite info
uint16_t racketHeight = 6;
uint16_t racketWidth = 35;
uint16_t racketMargin = 8;
int leftRacketLoc;
int rightRacketLoc;
//old vajalik, et teaksime, kas liigutame vasakule või paremale
int leftRacketLocOld;
int rightRacketLocOld;
uint16_t leftRacketColor = GREEN;
uint16_t rightRacketColor = RED;
uint16_t leftRacketSpeed;
uint16_t rightRacketSpeed;

//reketi erinevate alade jaoks
uint16_t bounceDifferentArea = 7;
uint16_t bounceNormalArea = 21;

//ekraani informatsioon
uint16_t width;
uint16_t height;

// nn "multithreading" globaalsed muutujad, vt. void flipScreen()
// ja void touch()
boolean rotationIsOne = true;
TSPoint tspoint;

//platsi äärmised jooned, et pall ei liiguks skoori ringide peale
uint16_t sideVlineMargin = 11;

//skoori ringide jaoks vajalikud andmed
uint16_t scoreCircleY = 11;
uint16_t scoreCircleX = 5;
uint16_t scoreCircleRadius = 3;

//muutujad skoori jälgmiseks
uint16_t leftRacketScore = 1;
uint16_t rightRacketScore = 1;

//kes alustab????
long rnd;
//loosimisel jätame meelde saadud numbrid
long leftRandomNumber;
long rightRandomNumber;

//värvi valimiseks loosimise eel, et vilgutada 3x0
boolean black = true;

/*
 * //servimiseks vaja lugeda push väärtust - juhtmega versiooni puhul
uint16_t leftJoystickButtonState;
uint16_t rightJoystickButtonState;
 */


//et saaks kasutada globaalselt joystickilt tulenevaid väärtusi, mis on skaalal [-3;3]
int leftJoystickVal;
int rightJoystickVal;

//tagasised andmiseks servimise kohta, vilgutame reketit
int flashColor = WHITE;

void setup() {

    //käivitame jadapordid
    Serial.begin(38400);
    Serial1.begin(38400);

    /*
     * //joysticki push  - juhtme versioon
    pinMode(SW_LEFT, INPUT_PULLUP);
    pinMode(SW_RIGHT, INPUT_PULLUP);
     */
    
    //käivitame ekraani
    tft.begin(0x9341); // SDFP5408

    //selliselt ekraani vaatamisnurk suurem mõlema mängija jaoks
    tft.setRotation(1);

    //kasutamiseks platsil objektide liigutamiseks
    width = tft.width() - 1;
    height = tft.height() - 1;

    //lähtestame pseudo-random numbru generaatori, et hakkaks suvalise kohapealt
    //loeme müra tühjast pesast
    randomSeed(analogRead(A15));
    
    //reketid mõlemalpool keskel
    int rlc = width/2-racketWidth/2;
    rightRacketLoc = rlc;
    leftRacketLoc = rlc;
    
    //taustavärv
    tft.fillScreen(BLACK);

    //kuvame avamenüü
    menu();

    //loosime alustava mängija
    whoStartsScreen();
    
    //liigume edasi mängu, kui kasutaja otsustab seda
    animationToGame();

}
//mängu lõppedes nullime mängu täitmise nullinda reani
void(* resetFunc) (void) = 0; //declare reset function @ address 0

void loop() {
    //uuendame asukohalisi väärtusi
    calcMoves();

    //joonistame objektid uude asukohta
    updateScreen();
}
//funktsioon palli vilgutamiseks mänguplatsilt väljudes
void flashBall(int color){
  tft.fillCircle(ballXold, ballYold, ballRadius, BLACK);
  delay(100);
  tft.fillCircle(ballXold, ballYold, ballRadius, color);
  delay(100);
  tft.fillCircle(ballXold, ballYold, ballRadius, BLACK);
  delay(100);
  tft.fillCircle(ballXold, ballYold, ballRadius, color);
  delay(100);
  tft.fillCircle(ballXold, ballYold, ballRadius, BLACK);
  delay(100);
  tft.fillCircle(ballXold, ballYold, ballRadius, color);
  delay(100);
  tft.fillCircle(ballXold, ballYold, ballRadius, BLACK);
}
//skoori kontrollimine
void checkScore(){
  boolean restartGame = false;
  
  //võitja on vasak mängija
  if (leftRacketScore == 12 && rightRacketScore <= 10){
    tft.setCursor(width/2, height/2);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("vasak");
    restartGame = true;
  }
  //võitja on parem mängija
  else if (rightRacketScore == 12 && leftRacketScore <= 10){
    tft.setCursor(width/2, height/2);
    tft.setTextColor(WHITE);
    tft.setTextSize(3);
    tft.print("parem");
    restartGame = true;
  }
  //viik
  else if (leftRacketScore == 11 && rightRacketScore == 11){
    tft.setCursor(width/2, height/2);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("VIIK");
    restartGame = true;
  }
  if (restartGame){
    doRestart.initButton(&tft, 40, height/2, 60, 30, WHITE, BLACK, WHITE, "edasi", 1);
    doRestart.drawButton(false);
    
    
    while (restartGame){
      //uuendame vajtust
      touch();

      //kontrollime vajutust
      if (tspoint.z > MINPRESSURE && doRestart.contains(tspoint.x, tspoint.y)){
        resetFunc();
      }
    }
  }
}
void calcScore(int racketScore, boolean leftRacketPoint, int color){
  
  //kui tegemist USB pesa poolt vaadatuna vasaku reketiga
  if (leftRacketPoint){
    tft.fillCircle(width-scoreCircleX, height-scoreCircleY*racketScore, scoreCircleRadius, color);
    leftRacketScore += 1;
  }
  //kui tegemist USB pesa poolt vaadatuna parema reketiga
  else{
    tft.fillCircle(scoreCircleX, scoreCircleY*racketScore, scoreCircleRadius, color);
    rightRacketScore += 1;
  }
}
void joystickPushEvent(){
  //juhtmega versioon
  // leftJoystickButtonState = digitalRead(SW_LEFT);
  //-------------------------------------------------BLUETOOTH-vasak-------------------------------------------------

  //saadame digile soovi x telje suhtes
  Serial1.print('x');

  //ootame
  while(Serial1.available()==0){
  }

  //loeme saadetu
  char b = Serial1.read();

  if (b == '0'){
    leftJoystickButtonState = 1;
  }
  else if (b == '1'){
    leftJoystickButtonState = 0;
  }
  //Serial.print(leftJoystickButtonState);
  
  //-------------------------------------------------------------------------------------------------------------
  //juhtmega versiooni jaoks
  //rightJoystickButtonState = digitalRead(SW_RIGHT);
  //-------------------------------------------------BLUETOOTH-parem-------------------------------------------------

  //saadame digile soovi x telje suhtes
  Serial2.print('x');

  //ootame
  while(Serial2.available()==0){
  }

  //loeme saadetu
  char b = Serial2.read();

  if (b == '0'){
    rightJoystickButtonState = 1;
  }
  else if (b == '1'){
    rightJoystickButtonState = 0;
  }

  
  //-------------------------------------------------------------------------------------------------------------
}
//tagasiside servimise õiguse kohta
void changeColor(){
  if (flashColor == WHITE){
    flashColor = BLACK;
  }
  else{
    flashColor = WHITE;
  }
}
//mängijale võimaldame servimise temale vajalikul ajal
void waitForServe(int ballServePosition){
  
  //isendid ajaliseks käsitlemiseks
  TimedAction pushButtonAction = TimedAction(3, joystickPushEvent);
  TimedAction MoveRacketAction = TimedAction(5, calcRacketLoc);
  TimedAction racketColorAction = TimedAction(250, changeColor);

  ballY = ballServePosition;
  while (true){
    
    //kontrollime reketi liigutamist ja uuendame koordinaate, kui vaja
    MoveRacketAction.check();
    //kontrollime, kas nupule on vajutatud ja tahetakse servida
    pushButtonAction.check();
    //servimise kohta tagasiside andmiseks
    racketColorAction.check();

    //asetame palli õige reketi juurde (palli x ja y kiirus nagu skoorides)
    if (ballY > height/2){
      ballX = leftRacketLoc+racketWidth/2;
    }
    else{
      ballX = rightRacketLoc+racketWidth/2;
    }
  
    //uuendame kuva
    //PUHTALT TAGASISIDE MÄNGIJATELE SERVIMISE ÕIGUSE KOHTA----------------------------------------------------------------------------------
    
    //vasak
    if (ballY > height/2){
      tft.drawFastHLine(leftRacketLocOld, height-racketMargin-racketHeight-2, racketWidth, BLACK);
      tft.drawFastHLine(leftRacketLoc, height-racketMargin-racketHeight-2, racketWidth, flashColor);
    }
    //parem
    else{
      tft.drawFastHLine(rightRacketLocOld, racketMargin+racketHeight+2, racketWidth, BLACK);
      tft.drawFastHLine(rightRacketLoc, racketMargin+racketHeight+2, racketWidth, flashColor);
    }
    //---------------------------------------------------------------------------------------------------------------------------------------------------------------
    
    //reketite update ekraanile
    //USB pesa poolt vasak reket
    //(X uus asukoht, X vana asukoht, Y asukoht, värv, kiirus)
    if (leftRacketLoc > leftRacketLocOld){
      moveRacketRight(leftRacketLoc, leftRacketLocOld, height-racketMargin-racketHeight, leftRacketColor, leftRacketSpeed);
    }
    else{
      moveRacketLeft(leftRacketLoc, leftRacketLocOld, height-racketMargin-racketHeight, leftRacketColor, leftRacketSpeed);
    }
    
    //USB pesa poolt parem reket
    if (rightRacketLoc > rightRacketLocOld){
      moveRacketRight(rightRacketLoc, rightRacketLocOld, racketMargin, rightRacketColor, rightRacketSpeed);
    }
    else{
      moveRacketLeft(rightRacketLoc, rightRacketLocOld, racketMargin, rightRacketColor, rightRacketSpeed);
    }
    //------------------------------------------------------------------------------------------------------
    
    //kontrollime servimise tahet, kui vajutus, siis laseme palli taas mängu
     if (ballY > height/2){
      if (leftJoystickButtonState == LOW){
        
        //kustutame servimise indikaatorjoone, kui peaks olema joonistatud
        tft.drawFastHLine(leftRacketLoc, height-racketMargin-racketHeight-2, racketWidth, BLACK);
        
        if (leftJoystickVal < 0){
          ballSpeedX = -bounceDiffAreaSpeedX;
        }
        else if (leftJoystickVal > 0){
          ballSpeedX = bounceDiffAreaSpeedX;
        }
        break;
      }
    }
    else{
      if (rightJoystickButtonState == LOW){

        //kustutame servimise indikaatorjoone, kui peaks olema joonistatud
        tft.drawFastHLine(rightRacketLoc, racketMargin+racketHeight+2, racketWidth, BLACK);
        
        if (rightJoystickVal < 0){
          ballSpeedX = -bounceDiffAreaSpeedX;
        }
        else if (rightJoystickVal > 0){
          ballSpeedX = bounceDiffAreaSpeedX;
        }
        break;
      }
    }
  }
}
//uuendame muutujate väärtusi
void calcMoves(){
    ballXold = ballX;
    ballYold = ballY;
    ballX += ballSpeedX;
    ballY += ballSpeedY;

    //-------------------REKETIST MÖÖDUMINE----------------------------------------------------
    //reketi kõrvalt mööda lennates alustame uuesti vastasmängija reketi juurest
    //kui terve pall liigub ekraanilt välja, alles siis loeme möödunuks reketist
    if (ballY <= 0){

      //reketist möödunud palli vilgutame punkti saaja värvis
      flashBall(leftRacketColor);
      
      //true näitab, et tegemist on vasaku reketi punktiga
      calcScore(leftRacketScore, true, leftRacketColor);

      //kontrollime mängu seisu. Viigi ja võidu korral anname teada ja lõpetame mängu
      checkScore(); 
    
      //laseme vasakul mängijal servida, anname kaasa seritava palli asukoha
      waitForServe(height-racketMargin-racketHeight-ballRadius-2);
    }
    if (ballY >= height){

      //reketist möödunud palli vilgutame punkti saaja värvis
      flashBall(rightRacketColor);

      //false näitab, et tegemist on vasaku reketi punktiga
      calcScore(rightRacketScore, false, rightRacketColor);

      //kontrollime mängu seisu
      checkScore();
      
      //laseme paremal mängijal servida, anname kaasa seritava palli asukoha
      waitForServe(racketMargin+racketHeight+ballRadius+2);
    }
    //-----------------ÄÄREJOONTEST PÕRKAMINE-----------------------------------------------------------------------------------------------
    //pall ei tohi liikuda külgmistest joontest välja, vaid peab "põrkama" ja suunda muutma
    //ülemine külg USB pesa poolt vaadatuna
    if (ballX-ballRadius-(2*abs(ballSpeedX)) <= sideVlineMargin){
      ballSpeedX = ballSpeedX *-1;
    }
    //alumine külg
    if (ballX+ballRadius+(2*abs(ballSpeedX)) >= width-sideVlineMargin){
      ballSpeedX = ballSpeedX *-1;
    }
    //------PAREMALT REKETILT PÕRKAMINE------------------------------------------------------------------------------------------------------
    //kiirused:
    /*
     *  int bounceNormalSpeedX = 2;
        int bounceNormalSpeedY = 3;
        int racketTipSpeedX = 5;
        int racketTipSpeedY = 5;
        int bounceDiffAreaSpeedX = 4;
        int bounceDiffAreaSpeedY = 3;
     */
    //kui pall põrkab vastu reketi äärmisi otsi, siis muudab suunda vastavalt sellele, millise ääre pihta parasjagu põrkas.
    //Reketi keskele põrgates liigub edasi normaalselt põrgates.
    //USB pesa poolt parema reketi ülemine ots
    if (ballX+ballRadius >= rightRacketLoc && ballX+ballRadius <= rightRacketLoc+ballRadius && racketMargin <= ballY+ballRadius && ballY-ballRadius <= racketMargin+racketHeight){
      ballSpeedX = -racketTipSpeedX;
      ballSpeedY = racketTipSpeedY;
    }
    //USB pesa poolt parema reketi bounceDifferentArea ülemine
    if (ballY-ballRadius <= racketMargin+racketHeight && 
        rightRacketLoc <= ballX+ballRadius && ballX <= rightRacketLoc+bounceDifferentArea){
          
      ballSpeedX = -bounceDiffAreaSpeedX;    
      ballSpeedY = bounceDiffAreaSpeedY;
    }
    //USB pesa poolt paremalt reketilt põrkab tagasi ja x liikumissuund jääb samaks, kui tabab reketi keskmist (21px) ala
    if (ballY-ballRadius <= racketMargin+racketHeight && rightRacketLoc+bounceDifferentArea < ballX && ballX < rightRacketLoc+racketWidth-bounceDifferentArea){
      ballSpeedX = ballSpeedX/ballSpeedX * bounceNormalSpeedX;
      ballSpeedY = bounceNormalSpeedY;
    }
    //USB pesa poolt parema reketi bounceDifferentArea alumine
    if (ballY-ballRadius <= racketMargin+racketHeight && 
        rightRacketLoc+bounceDifferentArea+bounceNormalArea <= ballX && 
        ballX-ballRadius <= rightRacketLoc+racketWidth){
      
      ballSpeedX = bounceDiffAreaSpeedX;
      ballSpeedY = bounceDiffAreaSpeedY;
    }
    //USB pesa poolt parema reketi alumine ots
    if (ballX-ballRadius <= rightRacketLoc+racketWidth && ballX-ballRadius >= rightRacketLoc+racketWidth-ballRadius && racketMargin <= ballY+ballRadius && 
        ballY-ballRadius <= racketMargin+racketHeight){
      
      ballSpeedX = racketTipSpeedX;
      ballSpeedY = racketTipSpeedY;
    }
    //--------VASAKULT REKETILT PÕRKAMINE---------------------------------------------------------------------------
    //USB pesa poolt vasaku reketi ülemine ots
    if (ballX+ballRadius >= leftRacketLoc && height-racketMargin <= ballY-ballRadius && 
        ballY+ballRadius <= height-racketMargin-racketHeight){
          
      ballSpeedX = -racketTipSpeedX;
      ballSpeedY = -racketTipSpeedY;
    }
    //USB pesa poolt vasaku reketi bounceDifferentArea ülemine
    if (ballY+ballRadius >= height-racketMargin-racketHeight && 
        leftRacketLoc <= ballX+ballRadius && ballX <= leftRacketLoc+bounceDifferentArea){
      
      ballSpeedX = -bounceDiffAreaSpeedX;
      ballSpeedY = -bounceDiffAreaSpeedY;
    }
    //vasakultlt reketilt põrkab tagasi ja y liikumiskiirus jääb samaks, 
    //kui tabab reketi keskmist (21px) ala
    if (ballY+ballRadius >= height-racketMargin-racketHeight && 
        leftRacketLoc+bounceDifferentArea < ballX && 
        ballX < leftRacketLoc+racketWidth-bounceDifferentArea){
          
          ballSpeedX = ballSpeedX/ballSpeedX * bounceNormalSpeedX;
          ballSpeedY = -bounceNormalSpeedY;
    }
    //USB pesa poolt vasaku reketi bounceDifferentArea alumine
    if (ballY+ballRadius >= height-racketMargin-racketHeight && 
        leftRacketLoc+bounceDifferentArea+bounceNormalArea <= ballX && 
        ballX-ballRadius <= leftRacketLoc+racketWidth){
      
      ballSpeedX = bounceDiffAreaSpeedX;
      ballSpeedY = -bounceDiffAreaSpeedY;
    }
    
    //USB pesa poolt vasaku reketi alumine ots
    if (ballX-ballRadius <= leftRacketLoc+racketWidth && height-racketMargin <= ballY-ballRadius && 
        ballY+ballRadius <= height-racketMargin-racketHeight){
          
      ballSpeedX = racketTipSpeedX;
      ballSpeedY = -racketTipSpeedY;
    }
    //--------REKETI LIIKUMINE---------------------------------------------------------------------------------------------
    calcRacketLoc();
    
}
void calcRacketLoc(){

  //juhtmega versiooni jaoks
  //leftJoystickVal = map(analogRead(Y_LEFT), 0, 1023, 3, -3);
  //-----------------------------------BLUETOOTH---vasak-------------------------------------------------------------

  //saadame digile y koordinaadi soovi
  Serial1.print('y');

  //ootame
  while(Serial1.available()==0){
  }

  //loeme saadetu
  int b = Serial1.read();
  if (b <= 52){
    leftJoystickVal = 6;
  }
  else if(b==53){
    leftJoystickVal = 2;
  }
  else if(b==54){
    leftJoystickVal = 0;
  }
  else if(b==55){
    leftJoystickVal = -2;
  }
  else if(b>=56){
    leftJoystickVal = -6;
  }
 
  
  //---------------------------------------------------------------------------------------------------

  //juhtmega versioon
  //rightJoystickVal = map(analogRead(Y_RIGHT)+11, 0, 1023, -6, 6);

   //-----------------------------------BLUETOOTH---parem-------------------------------------------------------------

  //saadame digile y koordinaadi soovi
  Serial2.print('y');

  //ootame
  while(Serial2.available()==0){
  }

  //loeme saadetu
  int b = Serial2.read();
  if (b <= 52){
    rightJoystickVal = 6;
  }
  else if(b==53){
    rightJoystickVal = 2;
  }
  else if(b==54){
    rightJoystickVal = 0;
  }
  else if(b==55){
    rightJoystickVal = -2;
  }
  else if(b>=56){
    rightJoystickVal = -6;
  }
 
  //---------------------------------------------------------------------------------------------------
  
  
  //vasaku reketi liikumise arvutamine
  leftRacketLocOld = leftRacketLoc;
  //kui reketi liigutamisel liiguks äärejoonest välja, siis ei arvuta uut positsiooni
  if (sideVlineMargin < leftRacketLoc+leftJoystickVal && leftRacketLoc+racketWidth+leftJoystickVal < width-sideVlineMargin){
    leftRacketLoc += leftJoystickVal;
    leftRacketSpeed = abs(leftJoystickVal);
  }
  //parema reketi liikumise arvutamine
  rightRacketLocOld = rightRacketLoc;
  if (sideVlineMargin < rightRacketLoc+rightJoystickVal && rightRacketLoc+racketWidth+rightJoystickVal < width-sideVlineMargin){
    rightRacketLoc += rightJoystickVal;
    rightRacketSpeed = abs(rightJoystickVal);
  }
}
//joonistame liikunud objektid uude asukohta, kustutame vanast ära
void updateScreen(){



    //------------------------pall------------------------------------------------------------------
    //joonistame uude asukohta
    tft.fillCircle(ballX, ballY, ballRadius, WHITE);
    //kustutame vanast asukohast
    tft.fillCircle(ballXold, ballYold, ballRadius, BLACK);
    
    
    
    //USB pesa poolt vasak reket--------------------------------------------------------------
    //(X uus asukoht, X vana asukoht, Y asukoht, värv, kiirus)
    if (leftRacketLoc > leftRacketLocOld){
      moveRacketRight(leftRacketLoc, leftRacketLocOld, height-racketMargin-racketHeight, leftRacketColor, leftRacketSpeed);
    }
    else{
      moveRacketLeft(leftRacketLoc, leftRacketLocOld, height-racketMargin-racketHeight, leftRacketColor, leftRacketSpeed);
    }
    
    //USB pesa poolt parem reket
    if (rightRacketLoc > rightRacketLocOld){
      moveRacketRight(rightRacketLoc, rightRacketLocOld, racketMargin, rightRacketColor, rightRacketSpeed);
    }
    else{
      moveRacketLeft(rightRacketLoc, rightRacketLocOld, racketMargin, rightRacketColor, rightRacketSpeed);
    }
}
//Mugavad funktsioonid Reketite liigutamiseks ja kustutamiseks
void moveRacketLeft(int racketXnew, int racketXold, int racketY, int color, int speedY){
    tft.fillRect(racketXold+racketWidth-speedY, racketY, speedY, racketHeight, BLACK);
    tft.fillRect(racketXnew, racketY, speedY, racketHeight, color);
}
void moveRacketRight(int racketXnew, int racketXold, int racketY, int color, int speedY){
    tft.fillRect(racketXold, racketY, speedY, racketHeight, BLACK);
    tft.fillRect(racketXnew+racketWidth-speedY, racketY, speedY, racketHeight, color);
}
//avamenüü funktsioon
void menu(){
    
    //suur ring
    for (int r = 60; r > 52; r--){
        tft.drawCircle(width/2, height/2, r, WHITE);
    }
    //lisame teksti ringi sisse
    title(WHITE);
    
    //nupp mängu tutvustamiseks, false - ei täida nuppu värviga
    aboutButtons(false, WHITE);
    
    //isend ajaliseks käsitlemiseks
    TimedAction flipScreenAction = TimedAction(2000, flipScreen);
   
    
    //tsükkel ajalise täitmise kontrollimise jaoks
    while (true){
      flipScreenAction.check();
      touch();
      
      //vajutus suurel ringil, siis liigume setup koodis edasi ehk hakkame mängima
      if (tspoint.z > MINPRESSURE && 
          width/2-52 <= tspoint.x && tspoint.x <= width/2+52 && 
          height/2-52 <= tspoint.y && tspoint.y <= height/2+52){
            
        
        //juhul kui tft rotation == 3, siis
        //pöörame ekraani esialgseks, et juhtkangid ei töötaks pööratult
        tft.setRotation(1);
        
        //about nupud tuleb ära kustutada
        aboutButtons(false, BLACK);
        
        break;
      }
      //tuleks kuvada about ekraan
      //aboutGameButton puhul tuleb koordinaadid ümber mappida, kuna tegime nupud, kui
      //rotation == 3 ehk tspoint ja tft  koordinaadid ei ühti enam
      else if (tspoint.z > MINPRESSURE && aboutGameButton.contains(tspoint.x, tspoint.y) ||
                tspoint.z > MINPRESSURE && 
                aboutGameButton2.contains(tspoint.x, tspoint.y)){
          
          aboutScreen();
          
      }
    }
}
//info kuva teksti kuvamine
void aboutScreen(){
  //kustutame pinpong pealkirja
  title(BLACK);
  
  //kasutajale tagasiside jaoks vilgutame nuppu
  aboutButtons(true, WHITE);
  delay(100);
  aboutButtons(false, WHITE);
  delay(100);
  aboutButtons(true, WHITE);
  delay(100);
  aboutButtons(false, WHITE);
  delay(100);
  aboutButtons(true, WHITE);
  delay(100);
  
  //kustutame about nupud
  aboutButtons(true, BLACK);
  
  //kustutame suure ringi
  tft.setRotation(1);
  //suur ring
  for (int r = 60; r > 52; r--){
      tft.drawCircle(width/2, height/2, r, BLACK);
  }
  
  //lisame info
  aboutInfo(WHITE);
  
  //lisame nupu tagasi avaekraanile liikumiseks
  backButtonfunc(false, WHITE);

  //ootame kasutajapoolset tegevust
  while (true){
    //uuendame vajutuse kohta
    touch();
    
    //GFX_Buttonil oma meetod olemas, hea lihtne
    if (backButton.contains(tspoint.x, tspoint.y) && tspoint.z > MINPRESSURE){
      
      //kasutajale tagasiside jaoks vilgutame nuppu
      backButtonfunc(true, WHITE);
      delay(100);
      backButtonfunc(false, WHITE);
      delay(100);
      backButtonfunc(true, WHITE);
      delay(100);
      backButtonfunc(false, WHITE);
      delay(100);
      backButtonfunc(true, WHITE);
      delay(100);
      
      //kustutame 
      backButtonfunc(false, BLACK);
      aboutInfo(BLACK);

      //lisame tagasi avamenüü objektid
      //suur ring
      for (int r = 60; r > 52; r--){
        tft.drawCircle(width/2, height/2, r, WHITE);
      }

      //about nupud
      aboutButtons(false, WHITE);
      
      break;
    }
  }
}
void aboutInfo(int color){
  
  //nimi
  tft.setCursor(width/2-50, height/2-100);
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.print("Alo Vals");
  
  //töö pealkiri
  tft.setCursor(width/2-100, height/2-60);
  tft.setTextSize(3);
  tft.print("Arduino LCD");
  tft.setCursor(width/2-80, height/2-35);
  tft.print("lauatennis");
  
  //juhendajad
  tft.setCursor(width/2, height/2+10);
  tft.setTextSize(1);
  tft.print("Juhendajad: Alo Peets");
  tft.setCursor(width/2+72, height/2+25);
  tft.print("Taavi Duvin");
  tft.setCursor(width/2+73, height/2+40);
  tft.print("Anne Villems");
  
  //tegemisaasta
  tft.setCursor(width/2-5, height-15);
  tft.setTextSize(1);
  tft.print("2017");

}
//selliselt, kuna mugav nii vilgutada nuppu tagasisideks kasutajale
void backButtonfunc(boolean invertColor, int color){
  backButton.initButton(&tft, 20, height-20, 30, 30, color, BLACK, color, "<<", 2);
  backButton.drawButton(invertColor);
}
void aboutButtons(boolean invertColor, int color){
  
  //getRotation() == 1 vaatenurgast vaadatuna alumine vasak nupp
  aboutGameButton.initButton(&tft, 20, height-20, 30, 30, color, BLACK, color, "?", 2);
  aboutGameButton.drawButton(invertColor);
  
  //getRotation() == 1 vaatenurgast vaadatuna ülemine parem nupp
  tft.setRotation(3);
  aboutGameButton2.initButton(&tft, 20, height-20, 30, 30, color, BLACK, color, "?", 2);
  aboutGameButton2.drawButton(invertColor);
  tft.setRotation(1);
  
}
//avamenüü pealkirja pöörav funktsioon
void flipScreen(){
    if (rotationIsOne){
      title(BLACK);
      tft.setRotation(3);
      title(WHITE);
      rotationIsOne = false;
    }
    else{
      title(BLACK);
      tft.setRotation(1);
      title(WHITE);
      rotationIsOne = true;
    }
}
//välakutsumisel uuendatakse vajutuskoha objekti vastavaid välju
void touch(){
    tspoint = ts.getPoint(); 
  
    pinMode(XM, OUTPUT); //Pins configures again for TFT control
    pinMode(YP, OUTPUT);
    
    //see kõik, kuna kasutan ekraani pikali olles
    //vajalik, kuna getPoint() tagastab teistsugusel skaalal väärtused
    //ja sellepärast ei ühti tft koordinaatidega
    int x_cord = tspoint.x;
    int y_cord = tspoint.y;
    
    //põhivaade getRotation() == 1, siis tspoint koordinaadid
    //algavad alumisest vasakust nurgast ja telje nimetused on vahetuses
    //tft koordinaadid algavad aga ülemisest vasakust nurgast
    if (tft.getRotation() == 1){
      tspoint.x = map(y_cord, TS_MINY, TS_MAXY, 0, width);
      tspoint.y = map(x_cord, TS_MINX, TS_MAXX, height, 0);
    }
    else{
      tspoint.x = map(y_cord, TS_MINY, TS_MAXY, width, 0);
      tspoint.y = map(x_cord, TS_MINX, TS_MAXX, 0, height);
    }
    
}
//Mängijatele loosiarvu genereerimise funktsioon
void lotoFunction(int rotation){
  //kontrollime, kummale mängijale kuvame
  if (rotation == 1){
    tft.setRotation(1);
  }
  else{
    tft.setRotation(3);
  }
  
  //anname teada milleks ekraan kuvatakse
  lotoScreenInfo(WHITE);

  //vajuta: start
  //kuvame nupu
  startGeneratingButton(WHITE);

  //isend numbrite vilgutamiseks kui mängija ei ole veel genereerimist alustavat nuppu vajutanud
  TimedAction noNumberAction = TimedAction(500, beforeGeneratingFlashing);

  //ootame mängija tegevust
  while (true){

    //uuendame vajutuse kohta
    touch();
    //kontrollime, kas on ülesande täitmise aeg
    noNumberAction.check();

    //kas on vajutatud nupule
    if (tspoint.z > MINPRESSURE && startGenerating.contains(tspoint.x, tspoint.y)){
      //kustutame start nupu
      startGeneratingButton(BLACK);
      
      break;
      }
  }
  
  //vajuta: stop
  //kuvame stop nupu
  stopGeneratingButton(WHITE);
  
  //kuvame esimese random numbri
  rnd = random(500);
  //kuvame esialgse numbri
  tft.setCursor(width/2-25, height/2+5);
  tft.setTextColor(BLACK);
  tft.setTextSize(3);
  tft.print(rnd);

  TimedAction randomNumberAction = TimedAction(4, printRandomNumber);
  
  while (true){
    touch();
    randomNumberAction.check();

    if (tspoint.z > MINPRESSURE && stopGenerating.contains(tspoint.x, tspoint.y)){
      //kustutame stop nupu
      stopGeneratingButton(BLACK);
      
      break;
      }
  }
  //kustutame teksti
  lotoScreenInfo(BLACK);

  //juhtkangide normaalseks toimimiseks seadistame tagasi rotationi 1e peale
  tft.setRotation(1);

  //jätame meelde saadud numbri
  if (rotation == 1){
    leftRandomNumber = rnd;
  }
  else{
    rightRandomNumber = rnd;
  }
}
//loosikuval toimuva loogika
void whoStartsScreen(){
  //täidame suure ringi
  tft.fillCircle(width/2, height/2, 60, WHITE);

  //algul loosime vasaku mängija random nr-i USB pesa poolt vaadatuna
  lotoFunction(1);
  //seejärel paremale mängijale
  lotoFunction(3);

  //anname tagasisidet mängijatele, kumb alustab
  //vilgutame vasakut numbrit
  if (rightRandomNumber > leftRandomNumber){
    tft.setRotation(3);

    tft.setTextSize(3);
    for (int i = 0; i < 5; i++){
      tft.setCursor(width/2-25, height/2+5);
      tft.setTextColor(rightRacketColor);
      tft.print(rightRandomNumber);
      delay(200);
  
      tft.setCursor(width/2-25, height/2+5);
      tft.setTextColor(BLACK);
      tft.print(rightRandomNumber);
      delay(200);
    }
    tft.setRotation(1);
  }
  //vilgutame paremat numbrit
  else{
    tft.setTextSize(3);
    for (int i = 0; i < 5; i++){
      tft.setCursor(width/2-25, height/2+5);
      tft.setTextColor(leftRacketColor);
      tft.print(leftRandomNumber);
      delay(200);
  
      tft.setCursor(width/2-25, height/2+5);
      tft.setTextColor(BLACK);
      tft.print(leftRandomNumber);
      delay(200);
    }
  }
}
void startGeneratingButton(int color){
  startGenerating.initButton(&tft, 40, height/2, 60, 30, color, BLACK, color, "start", 1);
  startGenerating.drawButton(false);
}
void stopGeneratingButton(int color){
  stopGenerating.initButton(&tft, width-40, height/2, 60, 30, color, BLACK, color, "stop", 1);
  stopGenerating.drawButton(false);
}
void lotoScreenInfo(int color){
  //info mängijale
  tft.setCursor(width/2-110, 20);
  tft.setTextColor(color);
  tft.setTextSize(2);
  tft.println("suurem arv servib");
  tft.setCursor(width/2-25, height-30);
  tft.setTextSize(1);
  tft.println("Good luck!");
}
//3me 0-i vilgutamiseks
void beforeGeneratingFlashing(){
  if (black){
    //kuvame
    tft.setCursor(width/2-25, height/2+5);
    tft.setTextSize(3);
    tft.setTextColor(BLACK);
    tft.print("000");
    black = false;
  }
  else{
    //kustutame
    tft.setCursor(width/2-25, height/2+5);
    tft.setTextSize(3);
    tft.setTextColor(WHITE);
    tft.print("000");
    black = true;
  }
}
//numbrite kiireks kerimiseks genereerimises
void printRandomNumber(){
  
  //kustutame eelmise
  tft.setCursor(width/2-25, height/2+5);
  tft.setTextColor(WHITE);
  tft.print(rnd);

  rnd = random(500);

  //kuvame uue
  tft.setCursor(width/2-25, height/2+5);
  tft.setTextColor(BLACK);
  tft.print(rnd);
}
//mänguplatsi ettevalmistus kui genereerimisega on ühele poole saadud
void animationToGame(){
  
  //kustutame suure ringi
  tft.fillCircle(width/2, height/2, 60, BLACK);

  //joonistame muu mänguvälja elemendid
  //äärte jooned
  tft.drawFastVLine(sideVlineMargin, 0, height, WHITE);
  tft.drawFastVLine(width-sideVlineMargin, 0, height, WHITE);

  //skoori ringid mõlemale mängijale
  int circleY = scoreCircleY;
  for (int i = 1; i < 12; i++){
    tft.drawCircle(scoreCircleX, circleY, scoreCircleRadius, WHITE);
    tft.drawCircle(width-scoreCircleX, height-circleY, scoreCircleRadius, WHITE);
    circleY += scoreCircleY;
  }

  //joonistame uue asukohaga vasaku reketi (vaadates USB pesa poolt)
  tft.fillRect(leftRacketLoc, height-racketMargin-racketHeight, racketWidth, racketHeight, leftRacketColor);
  
  //joonistame uue asukohaga parema reketi
  tft.fillRect(rightRacketLoc, racketMargin, racketWidth, racketHeight, rightRacketColor);
  
  //kontrollime, kes alustab mängu ja laseme servida seejärel----------------------------------------------------------------------
  //USB pesa poolt vasaku mängija alustada
  if (leftRandomNumber > rightRandomNumber){
    //igaksjuhuks tuleb määrata ka X kiirus pallile, kui ei suunata joystickiga palli lendu. Muidu lendab pall otse
    //anname kiiruse võidunumbri alusel
    ballSpeedX = bounceNormalSpeedX;
    if (leftRandomNumber < 250){
      ballSpeedX = -bounceNormalSpeedX;
    }
    //määrame palli edasi lendamise kiiruse
    ballSpeedY = -bounceNormalSpeedY;
    
    //laseme mängijal servida
    waitForServe(height-racketMargin-racketHeight-ballRadius-2);
  }
  //USB pesa poolt parema mängija alustada
  else{
    //igaksjuhuks tuleb määrata ka X kiirus pallile, kui ei suunata joystickiga palli lendu. Muidu lendab pall otse
    //anname kiiruse võidunumbri alusel
    ballSpeedX = bounceNormalSpeedX;
    if (rightRandomNumber < 250){
      ballSpeedX = -bounceNormalSpeedX;
    }
    ballSpeedY = bounceNormalSpeedY;
    waitForServe(racketMargin+racketHeight+ballRadius+2);
  }
}
//mängu nimi, mida hakatakse pöörama iga 2 sekundi tagant
void title(int color) {
  //tekst suure ringi sisse
    tft.setCursor(width/2-33, height/2-30);
    tft.setTextColor(color);
    tft.setTextSize(3);
    tft.println("LAUA-");
    
    tft.setCursor(width/2-33, height/2+10);
    tft.setTextColor(color);
    tft.setTextSize(3);
    tft.println("TENNIS");
}
