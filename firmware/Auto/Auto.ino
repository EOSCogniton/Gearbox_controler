/**************************************************************************/
/*!
    @file     main_carte.cpp
    @author   Bruno Moreira Nabinger and Corentin Lepais (EPSA)
                                                  Ecurie Piston Sport Auto
    
    Main code for control the Motor with integrated Controller and CAN 
    interface BG 45 CI

 
    @section  HISTORY
    v0.7 - 17/04/2019 Bob Aubouin change a lot of things to associate the code with the real word
    v0.6 - 06/04/2019 Add the sending of the speed to the DTA
    v0.5 - 13/02/2019 Modifications linked to the class CAN
    v0.4 - 06/02/2019 Add fonction to can_interface
    v0.3 - 10/11/2018 Comments of code and add of the file 
           "projectconfig.h", with the definitions of the pins numbers.
    v0.2 - 17/10/2018 Management of pallets and beginning of creations of 
                      the associated functions
    v0.1 - 10/10/2018 First release (previously code of Pedro)
*/
/**************************************************************************/
#include "projectconfig.h"

//Ajout des fonctions
#include "fonct_palette_homing.h"
#include "fonct_mot.h"
#include "can_interface.h"

//initialisation Canbus
#include <SPI.h>

//Definition of key-word
#define ERREUR 1 //Use to transmit that the motor is in error

//Definition of used variables 
boolean statePaletteIncrease; 
boolean statePaletteIncreaseBefore;
boolean statePaletteDecrease;
boolean statePaletteDecreaseBefore;
int positionEngager; // Contain what motor position is currently engaged
int wantedPosition;// Contain the motor position wanted so the speed rapport of the bike
boolean stateHoming; // Will contain the state of the homing button
boolean stateHomingBefore;
boolean outMotor1; //Info return by the motor
boolean outMotor2;//Info return by the motor
boolean neutreState; // Will contain the state of the neutral button
boolean stateNeutreBefore;
boolean error;
const int neutrePosition = 2;
const int homingPosition=1;
boolean positionReached=true;
unsigned long T_Descente; //To avoid two gera change on the same push on the palette we use a little delay, this variable will contain the time of the last down shift
unsigned long T_Montee; // and this one the last up shift
int valAnalog[7];

//Mode Auto
boolean Auto; // If false we are in normal mode, else we are in mode auto and we shift up at the right RPM
unsigned long T_Auto; //will contain the time of the state change 0 - 1 when the pilote press the two palette on the same time
boolean stateTwoPalette;
boolean stateTwoPaletteBefore;
int RPM;
int ShiftRPM[2];

//Table which will contain the combination of the motor input for each speed
boolean motorPosition[16][4];//We use only 4 input motor to command it. The 5 is always 0

//Initialization of CANBUS
can_interface CAN;

void setup() 
{ 
  
  //CAN Init
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_16MHZ) == CAN_OK){
      Serial.println("CAN Init Successfully!");
      digitalWrite(13,HIGH);
      }
  else{
      Serial.println("CAN Init Failure");}
  // Set operation mode to normal so the MCP2515 sends acks to received data
  CAN0.setMode(MCP_NORMAL);                  
  // Configuring pin for /INT input
  pinMode(CAN0_INT, INPUT); 

    
  //Serial.begin(115200);
  //Initialization of the pins
  pinMode(motorState1, INPUT);
  pinMode(motorState2, INPUT);
  pinMode(motorInput0, OUTPUT);
  pinMode(motorInput1, OUTPUT);
  pinMode(motorInput2, OUTPUT);
  pinMode(motorInput3, OUTPUT);
  pinMode(motorInput4, OUTPUT);
  pinMode(shiftCut, OUTPUT); 
  pinMode(gearPot, OUTPUT);
  pinMode(pinLED, OUTPUT); 

  pinMode(paletteIncrease, INPUT);
  pinMode(paletteDecrease, INPUT);
  
  digitalWrite(motorInput0, LOW);
  digitalWrite(motorInput1, LOW);
  digitalWrite(motorInput2, LOW);
  digitalWrite(motorInput3, LOW);
  digitalWrite(motorInput4, LOW);
  digitalWrite(shiftCut, HIGH);
  digitalWrite(pinLED,LOW);

  //Initialization of the variables
  statePaletteIncreaseBefore = LOW; //The pallets mode is INPUT but there is a PULL Down resistor on the shield so the inactive state is low
  statePaletteDecreaseBefore = LOW;
  
  stateNeutreBefore = false; //on this buttons there is an INPUT PULLUP so the inactive state is HIGH
  stateHomingBefore = false;
  
  positionEngager = 0; // We don't know but this normaly not the homming position
  wantedPosition = 1; // First we want to do the homming of the motor
  error = false; // we suppose that there is no error 
  T_Descente = millis();
  T_Montee = millis();
  
  //Auto
  Auto = false; // we always start the car in normal mode
  T_Auto = 0; 
  stateTwoPaletteBefore = 0;
  RPM = 0;
  ShiftRPM[1] = 1200; //de 1 vers 2
  ShiftRPM[2] = 1200; //de 2 vers 3
 
    //mappage des valeurs de tensions envoyés au DTA en fonction de la vitesse (0->0.2V .... 6->5V)
  valAnalog[0] = 10;
  valAnalog[1] = 51;
  valAnalog[2] = 92;
  valAnalog[3] = 133;
  valAnalog[4] = 174;
  valAnalog[5] = 215;
  valAnalog[6] = 255;
  
    //Initialization of the table. We use only the position 1-6, clear error and start Homing
    
    //Clear error and Stop
    motorPosition[0][0] = 0;
    motorPosition[0][1] = 0;
    motorPosition[0][2] = 0;
    motorPosition[0][3] = 0; 

    //Start Homing
    motorPosition[1][0] = 1;
    motorPosition[1][1] = 0;
    motorPosition[1][2] = 0;
    motorPosition[1][3] = 0;

    //Position 1: Neutre
    motorPosition[2][0] = 0;
    motorPosition[2][1] = 1;
    motorPosition[2][2] = 0;
    motorPosition[2][3] = 0;

    //Position 2 : vitesse 1 de la moto
    motorPosition[3][0] = 1;
    motorPosition[3][1] = 1;
    motorPosition[3][2] = 0;
    motorPosition[3][3] = 0;

    //Position 3 : vitesse 2
    motorPosition[4][0] = 0;
    motorPosition[4][1] = 0;
    motorPosition[4][2] = 1;
    motorPosition[4][3] = 0;

    //Position 4 : vitesse 3
    motorPosition[5][0] = 1;
    motorPosition[5][1] = 0;
    motorPosition[5][2] = 1;
    motorPosition[5][3] = 0;

    //Position 5 : vitesse 4
    motorPosition[6][0] = 0;
    motorPosition[6][1] = 1;
    motorPosition[6][2] = 1;
    motorPosition[6][3] = 0;

    //Position 6 : vitesse 5
    motorPosition[7][0] = 1;
    motorPosition[7][1] = 1;
    motorPosition[7][2] = 1;
    motorPosition[7][3] = 0;

    //Position 7 : vitesse 6
    motorPosition[8][0] = 0;
    motorPosition[8][1] = 0;
    motorPosition[8][2] = 0;
    motorPosition[8][3] = 1;

    //Position 8
    motorPosition[9][0] = 1;
    motorPosition[9][1] = 0;
    motorPosition[9][2] = 0;
    motorPosition[9][3] = 1;

    //Position 9
    motorPosition[10][0] = 0;
    motorPosition[10][1] = 1;
    motorPosition[10][2] = 0;
    motorPosition[10][3] = 1;

    //Position 10
    motorPosition[11][0] = 1;
    motorPosition[11][1] = 1;
    motorPosition[11][2] = 0;
    motorPosition[11][3] = 1;

    //Position 11
    motorPosition[12][0] = 0;
    motorPosition[12][1] = 0;
    motorPosition[12][2] = 1;
    motorPosition[12][3] = 1;
    
    //Position 12
    motorPosition[13][0] = 1;
    motorPosition[13][1] = 0;
    motorPosition[13][2] = 1;
    motorPosition[13][3] = 1;

    //Position 13 
    motorPosition[14][0] = 0;
    motorPosition[14][1] = 1;
    motorPosition[14][2] = 1;
    motorPosition[14][3] = 1;

    //Position 14
    motorPosition[15][0] = 1;
    motorPosition[15][1] = 1;
    motorPosition[15][2] = 1;
    motorPosition[15][3] = 1;

  //Homming du motored
   EngageVitesse(wantedPosition);
   outMotor1 = digitalRead(motorState1);
   outMotor2 = digitalRead(motorState2);
    while (!PositionReachedOrHomingDone(outMotor1,outMotor2))
    {
         outMotor1 = digitalRead(motorState1);
         outMotor2 = digitalRead(motorState2);
    }
    positionEngager = wantedPosition; 
    wantedPosition = 2; //on débute au neutre comme ça pas de surprise
   analogWrite(gearPot, 255);
  

}

void loop() 
{ 
  CAN.Recieve();//MAJ of the can attributs by recieving the last datas
  CAN.Transmit(positionEngager-2, 0,Auto);
  //Control of pallet+
  statePaletteIncrease = digitalRead(paletteIncrease);
  //If we only test if the state of the palette have changed or if we only test if the state is now 1, the test is going to be true 2 times:
  // when the pilot presses the palette and when it is released. We want to change the speed only  one time for each press

  //0 -> 1 = Detect the rising edge of signal statePaletteIncrease;
  if (statePaletteIncrease != statePaletteIncreaseBefore)// Check if state have changed
  {
  
    if (statePaletteIncrease) // Check if state changed to 1, so we have the rising edge 0 -> 1 
    {
      Auto = false; //any action on the pilote will stop the auto mode
      if(PassageVitesseIsPossible(positionEngager+1)) 
      {
        if ((millis() - T_Montee) > 200)
        {
        wantedPosition = positionEngager+1;
        T_Montee = millis();
        }
      }
    }
    statePaletteIncreaseBefore = statePaletteIncrease;
  }
  //Control of pallet -
  statePaletteDecrease = digitalRead(paletteDecrease);
  //If we only test if the state of the palette have changed or if we only test if the state is now 1, the test is going to be true 2 times:
  // when the pilot presses the palette and when it is released. We want to change the speed only  one time for each press
  //0 -> 1 = Detect the rising edge of signal statePaletteDecrease;
  if (statePaletteDecrease != statePaletteDecreaseBefore)// Check if state have changed
  {
    if (statePaletteDecrease) // Check if state changed to 1, so we have the rising edge 0 -> 1
    {
      Auto = false; //any action on the pilote will stop the auto mode
      if(PassageVitesseIsPossible(positionEngager-1)) //on vérifie que la vitesse à laquelle on veut passer est atteignable
      {
        if ((millis() - T_Descente) > 200)
        {
          wantedPosition = positionEngager-1;
          T_Descente = millis();
        }
      }
    }
    statePaletteIncreaseBefore = statePaletteIncrease;
  }
  //Gestion du neutre

  neutreState=CAN.getNeutreState();
  if(neutreState != stateNeutreBefore) //if state change
  {
    if(neutreState) //if state is 1
    {
      Auto = false; //any action on the pilote will stop the auto mode
      wantedPosition = neutrePosition;
    }
    stateNeutreBefore = neutreState;
  }
  
  //Gestion bouton homing
  stateHoming=CAN.getHomingState(); //We have the state of the homing thank to the can attribut
  if(stateHoming != stateHomingBefore)
  {
    if(stateHoming) //if state is true 
    {
      //Auto = false; //any action on the pilote will stop the auto mode
      //wantedPosition=homingPosition;
    }
    stateHomingBefore = stateHoming;
  }

  // Passage auto

  //Le mode s'active lorsqu'on appuie pendant 3 seconde sur les deux palettes en même temps
  stateTwoPalette = (statePaletteIncrease && statePaletteDecrease);
  if (stateTwoPalette != stateTwoPaletteBefore)
    {
      if (stateTwoPalette)
      {
        T_Auto = millis();
      }
    }

   if (stateTwoPalette && (millis() - T_Auto > 3000))
   {
    Auto=true; // activation 
   }

   if (Auto);
   {
    RPM=CAN.getRPM();
    wantedPosition = CalculAuto(positionEngager,RPM);
   }
  

  // Si changement :
  if (wantedPosition!=positionEngager) //We try to change speed only if the pilot demands it
  {
    digitalWrite(shiftCut, LOW);//Close the injection
    EngageVitesse(wantedPosition);
    outMotor1 = digitalRead(motorState1);
    outMotor2 = digitalRead(motorState2);
    while(MotorIsTurning(outMotor1,outMotor2)) //while the motor is turning
    {
      outMotor1 = digitalRead(motorState1);
      outMotor2 = digitalRead(motorState2);
      if (MotorIsLost(outMotor1,outMotor2)) //error
      {
        CAN.Transmit(0, ERREUR,Auto);
      }
    }
    if (PositionReachedOrHomingDone(outMotor1,outMotor2)) //We change the current speed only if we have reached the position
    {
      positionEngager=wantedPosition; //We save the engaged position
    }
    digitalWrite(shiftCut, HIGH);//Open the injection
    TransmetToDTATheGear(positionEngager-2); // We send to the DTA the engaged gear
  }
}

void EngageVitesse(int wantedPosition) //Function which pass the speed
{
  digitalWrite(motorInput0,LOW);
  digitalWrite(motorInput1, motorPosition[wantedPosition][0]);
  digitalWrite(motorInput2, motorPosition[wantedPosition][1]);
  digitalWrite(motorInput3, motorPosition[wantedPosition][2]);
  digitalWrite(motorInput4, motorPosition[wantedPosition][3]);
}

void TransmetToDTATheGear(int rapportEngage)
{
  analogWrite(gearPot,valAnalog[rapportEngage]);
}

int CalculAuto(int pos,int RPM)
{
  for (int i=0; i <= 2; i++){
    if (pos==i)
    {
      if (RPM > ShiftRPM[i]){return(i+1);}
      else {return(i);}
    }
  }
  if (pos==3){return(3);}
}
  
