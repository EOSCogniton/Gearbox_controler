/**************************************************************************/
/*!
    @file     projectconfig.h
    @author   Bruno Moreira Nabinger and Corentin Lepais (EPSA)
                                                  Ecurie Piston Sport Auto
    
    Config settings and macros in this header file,  for controling the 
    Motor with integrated Controller and CAN interface BG 45 CI.

 
    @section  HISTORY
    
    v0.1 - 10/11/2018 First release ()
*/
/**************************************************************************/

//inclusion guards - ensure this file definitions are only ever included
//once 
#ifndef INC_PROJECTCONFIG_H //if not define symbol INC_PROJECTCONFIG_H
#define INC_PROJECTCONFIG_H 


//--------------  assignation des pins  ------------------
/* PIN Mapping    
     ====================================================================
     PIN  Symbol    Description                                   Default
     ---  ------    --------------------------------------------- ------- 
          AREF      Analog Reference for the analog inputs        -           
          GND                                                     -       
     A13  SCK       Serial Clock - The clock pulses which         -       
                     synchronize data transmission generated by          
                     the master                                          
     A12  MISO      Master In Slave Out - The Slave line for      -      
                     sending data to the master                          
     A11  MOSI      Master Out Slave In - The Master line for     -      
                     sending data to the peripherals,                    
     A10  CS        Chip Select for CAN Controller with SPI       -      
                     Interface  (CANdiy-Shield)                          
          GND                                                            
          VCC                                                            
          RESET     Reset                                                 
                                                                         
      USED
      A1  motorSTATE0  Motor State 1 (Error,Stop,Moving)         Digital INPUT
      A0  motorSTATE1  Motor State 2 (Error,Stop,Moving)         Digital INPUT
                                                                             
       0  motorInput0  Motor Input 0                            Digital OUTPUT
       1  motorInput1  Motor Input 1                            Digital OUTPUT
       2  motorInput2  Motor Input A4                           Digital OUTPUT
       3  motorInput3  Motor Input 3                            Digital OUTPUT
       4  motorInput4  Motor Input 4                            Digital OUTPUT
       7  shiftCut     Cut the injection                        Digital OUTPUT
       A3  gearPot     Inform DTA of the engaged gear           Analog OUTPUT
      8  paletteIncrease Pallete for increasing the speed       Digital INPUT
      12  paletteDencrease Pallete for increasing the speed     Digital INPUT
      10  chipSelect    chip select for can                     Digital INPUT                                                                     
      2  intPinCan      linked to the can                       Digital INPUT
      
      TODO
      Speak we Arthur about intPinCan
      */
const int motorState1 = A1; 
const int motorState2 = A0; 
const int motorInput0 = 0;
const int motorInput1 = 1;
const int motorInput2 = A4;
const int motorInput3 = 3;
const int motorInput4 = 4;
const int shiftCut = 7; 
const int gearPot = A3;
const int paletteIncrease = 8;
const int paletteDecrease = 9;
const int chipSelect = 10;
const int intPinCAN = 2;

#endif  /* inclusion guards - INC_PROJECTCONFIG_H */ 

// SIZER, Ben. Organizing Code Files in C and C++.  
// https://www.gamedev.net/articles/programming/general-and-gameplay-programming/organizing-code-files-in-c-and-c-r3173
// June 8, 2013. Last Access 10 nov 2018.

// TOWNSEND, Kevin. KTOWN's Guide to Readable C Code.
// https://cdn-learn.adafruit.com/downloads/pdf/ktowns-guide-to-readable-c-code.pdf
// Last Access 10 nov 2018.
