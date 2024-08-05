#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>
#include <utility/Adafruit_MCP23017.h>
#include <EEPROM.h>
#include <avr/eeprom.h>
#include <TimerOne.h>
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

#define UpB button_state & BUTTON_UP
#define DownB button_state & BUTTON_DOWN
#define LeftB button_state & BUTTON_LEFT
#define RightB button_state & BUTTON_RIGHT
#define SelectB button_state & BUTTON_SELECT

int onTime[51] = {};
int offTime[51] = {};
int Levels[51] = {};
String rowOne = "Ground  First   ";
String rowTwo = "Outside Data    ";
int roomNum;
String row1;
String row2;
String TEMP;
String TEMP2;
String SPECIFIC;
int menu[4] = {};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Initialises the serial monitor type
  lcd.begin(16, 2);
  // Initialises the LCD screen type
  Serial.println("ENHANCED:");
  Serial.println("VALID");
  // Prints the descriptors

  for (int i = 0; i < 256; i++) { EEPROM.write(i, 0); }
  // Wipes the EEPROM
  
}

void loop() {
  // put your main code here, to run repeatedly:

  //int numOfItems;  
  static int menuState;
  // Represents the page that the user will see/use
  static int selectedMenuItem = 1;
  // Represents which cell (ranging from 1-4) the user currently has selected in a page
  uint8_t button_state = lcd.readButtons();
  // Reads the state of every LCD button and returns teh result as an 8-bit integer
  
  


  if (Serial.available() > 0) {
    String serialInp = Serial.readString();

    switch(serialInp[0]) {

      case 'S':
        prepareDeviceF(serialInp.substring(2)); 
        // If the first chracter is 'S' then call the prepareDevice function with the entered room data as the parameter
        break;
        
      case 'M':
        if (serialInp.length() == 2) { Serial.print(2000 - freeRam()); Serial.println(" bytes of SRAM are being used."); }
        /* If the first character is 'M' then print the result of calling a funciton which returns the amount of SRAM being used
        The length of the input must also only be 1 character long*/
        break;

      case 'Q':
        if (serialInp.length() == 2) { readEEPROM(); }
        /* If the first chracter is 'Q' then call the function which will print every device and their associated times and values.
        The length of the input must also only be 1 character long*/
        break;
    }
  } 

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

  if (UpB && (selectedMenuItem > 2)) {
    // Checks if the up button is pressed by the user and if the current selected cell is on the second row
    if (menu[selectedMenuItem - 3] == 1) {
      // Checks if the cell the user is trying to move to is available, if so then program flow continues
      selectedMenuItem = selectedMenuItem - 2;
      // Sets the new value for the currently selected menu cell
      clearRows(row1,row2);
      // Resets the values of the rows so checkmarks are not left behind
      setCheck(selectedMenuItem);
      // Adds a checkmark character 'X' to the end of the selected cell to show it is to the user
      
    }
  }

  if (DownB && (selectedMenuItem <= 2)) {
      // Checks if the down button is pressed by the user and if the current selected cell is in the first column
    if (menu[selectedMenuItem + 1] == 1) {
       // Checks if the cell the user is trying to move to is available, if so then program flow continues
      selectedMenuItem = selectedMenuItem + 2;
       // Sets the new value for the currently selected menu cell
      clearRows(row1,row2);
       // Resets the values of the rows so checkmarks are not left behind
      setCheck(selectedMenuItem);
       // Adds a checkmark character 'X' to the end of the selected cell to show it is to the user
    }
  }

  if (LeftB && ((selectedMenuItem % 2) == 0)) {
    // Checks if the left button is pressed by the user and if the current selected cell is in the second column
    if (menu[selectedMenuItem - 2] == 1) {
      // Checks if the cell the user is trying to move to is available, if so then program flow continues
      selectedMenuItem--;
      // Sets the new value for the currently selected menu cell 
      clearRows(row1,row2);
      // Resets the values of the rows so checkmarks are not left behind    
      setCheck(selectedMenuItem);
      // Adds a checkmark character 'X' to the end of the selected cell to show it is to the user
    }
  }

  if (RightB && !((selectedMenuItem % 2) == 0)) {
    if (menu[selectedMenuItem] == 1) {
      selectedMenuItem++;
      // Sets the new value for the currently selected menu cell 
      clearRows(row1,row2);
      // Resets the values of the rows so checkmarks are not left behind    
      setCheck(selectedMenuItem);
      // Adds a checkmark character 'X' to the end of the selected cell to show it is to the user      
    }
  }

  bool nextPage = false;
  // Variable used to indicate whether the user has gone to the next page, which is used later on in the program
 
  switch(menuState) {
  // Using this statement, the program decides which menu screen to display based on the value of menuState, each case represents a different level

    static bool nonUnique = false;
    // Variable that's used to indicate whether a value being checked is already a part of TEMP
    
    case 0:
    // This level makes the user choose between floors or the Data option 
  
      for (int menuSlot = 0; menuSlot < 4; menuSlot++) { menu[menuSlot] = 1; }
      // Sets all menu positions to be available since the home screen is pre-set and its parameters are known
      
      row1 = "Ground  First   ";
      row2 = "Outside Data    ";
      // Sets the values of row1 and row2 to their home screen states

      if (SelectB) {       
      // Checks if the select button is pressed by the user, if so then the following code block is executed

        if (selectedMenuItem == 4) {
        // Checks if the user is selecting the 'Data' option 
          readEEPROM();
          // If they are, then this function is called to return every device and their quantities
        }
        else {
        // Otherwise, the following segment is executed instead
        
        menuState = 1;
        // Sets the new menuState to be 1 so the user will choose between different rooms on the next iteration of loop()
        char floorChoice = charFloorF(selectedMenuItem-1);      
        // Calls the function which returns the character associated with the string passed into it, in this case it is the chosen floor that's the parameter
        TEMP = "";
        // Resets the TEMP string so it can be used to find rooms

        if (SPECIFIC == "") {
          SPECIFIC = SPECIFIC + floorChoice;
          // Adds the chosen floor to the SPECIFIC string
        }
  
        for (int roomCount = 1; roomCount < (roomNum*5); roomCount = roomCount + 5) {
        // Iterates through every device in EEPROM
          
          if (charFloorF(EEPROM.read(roomCount-1)) == floorChoice) {
          // Checks if the floor value in the current device matches that of the chosen one  
            
            for (int uniqueRoom = 0; uniqueRoom < TEMP.length(); uniqueRoom++) {    
            // If so, the program checks if the current room matches any of the ones stored in TEMP at the time
                       
              if (charRoomF(EEPROM.read(roomCount)) == TEMP[uniqueRoom]) {
                nonUnique = true;
                // If there is a match: nonUnique is set to true so it can't be added to TEMP
              } 
            }
            if (nonUnique == false) { TEMP = TEMP + charRoomF(EEPROM.read(roomCount)); }
          } 
          nonUnique = false;
          // Resets nonUnique so it can be used for the next dvice in EEPROM
        } 
        TEMP2 = TEMP; 
        // Sets the value of TEMP2 so the user can scroll through every option in the next menuState
      }
      }
      break;
    
    case 1:
    // This level makes the user choose between rooms
    
      static int rowIndex; 
      // Declares the variable that's used to represent the current set of options being displayed, the user will be able to scroll through them   
      prepareRow(1,rowIndex);
      // Calls the function that displays the rooms to be chosen based on the value of TEMP2
      setCheck(selectedMenuItem);
      // Sets the checkmark
      TEMP = "";
      // Resets TEMP
      nextPage = false;
      // Resets nextPage


      if (SelectB) {
        
        if (selectedMenuItem == 3) {
          //Serial.println("YES");
          SPECIFIC = "";
          // Resets SPECIFIC
          menuState = 0;
          // Resets the menuState
          clearRows("Ground  First   ","Outside Data    ");
          // Sets the LCD display to show the floor selection screen
          setCheck(selectedMenuItem);
          // Sets the checkmark
        }

        if (selectedMenuItem == 4) {
        // If the user presses the 'Next' button the following code is executed
          rowIndex = rowIndex + 2;
          // Scrolls to the next frame to show more options available to the user
          nextPage = true;
          
        }
        
        if (selectedMenuItem == 1 && menu[0] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex];
          // Adds the chosen room to SPECIFIC
          rowIndex = 0;
          // Resets rowIndex
          menuState = 2;
          // Sets the new menuState to be 2 so the user will choose between different types on the next iteration of loop()
        }

        if (selectedMenuItem == 2 && menu[1] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex+1];
          // Adds the chosen room to SPECIFIC
          rowIndex = 0;
          // Resets rowIndex
          menuState = 2;
          // Sets the new menuState to be 2 so the user will choose between different types on the next iteration of loop()
        }

              if (rowIndex >= TEMP2.length()) { rowIndex = 0; Serial.println("RESET");} 

            for (int roomCount = 2; roomCount < (roomNum*5); roomCount = roomCount + 5) {
            // Iterates through every device in EEPROM
              
              if (EEPROM.read(roomCount-2) == FloorF(SPECIFIC[0]) && EEPROM.read(roomCount-1) == RoomF(SPECIFIC[1])) {
              // Checks if the floor and room values in the current device matches that of the chosen ones  
                
                for (int uniqueType = 0; uniqueType < TEMP.length(); uniqueType++) {   
                // If so, the program checks if the current type matches any of the ones stored in TEMP at the time
                            
                  if (charTypeF(EEPROM.read(roomCount)) == TEMP[uniqueType]) {
                    nonUnique = true;
                    // If there is a match: nonUnique is set to true so it can't be added to TEMP
                  }
                }
                if (nonUnique == false) { 
                  
                  TEMP = TEMP + charTypeF(EEPROM.read(roomCount)); 
                  }
              } 
              nonUnique = false;
              // Resets nonUnique so it can be used for the next dvice in EEPROM
            }  
                    if (!nextPage) {
                    // Chceks if nextPage is false
                      TEMP2 = TEMP;  
                      // Sets the value of TEMP2 so the user can scroll through every option in the next menuState      
                      nextPage = false;
                    }

      }

     
      break;

    case 2:
      TEMP = "";     
      nextPage = false;
      prepareRow(2,rowIndex);
      setCheck(selectedMenuItem);

      if (SelectB) {     
        
        if (selectedMenuItem == 3) {
          //Serial.println("YES");
          SPECIFIC = "";
          menuState = 0;
        }

        if (selectedMenuItem == 4) {
          //Serial.println("NEXT");
          rowIndex = rowIndex + 2;
          nextPage = true;          
          
        }
        
        if (selectedMenuItem == 1 && menu[0] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex];
          //Serial.println(SPECIFIC + " SPECIFIC");
           rowIndex = 0;
           menuState = 3;
        }

        if (selectedMenuItem == 2 && menu[1] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex+1];
          //Serial.println(SPECIFIC + " SPECIFIC");
          rowIndex = 0;
          menuState = 3;
        }

              if (rowIndex >= TEMP2.length()) { rowIndex = 0; } 

            for (int roomCount = 3; roomCount < (roomNum*5); roomCount = roomCount + 5) {
              
              if (EEPROM.read(roomCount-3) == FloorF(SPECIFIC[0]) && EEPROM.read(roomCount-2) == RoomF(SPECIFIC[1]) && EEPROM.read(roomCount-1) == TypeF(SPECIFIC[2])) {
    
                //Serial.println(TEMP+" TEST");
                
                for (int uniqueName = 0; uniqueName < TEMP.length(); uniqueName++) {   
                  
                  if (charNameF(EEPROM.read(roomCount)) == TEMP[uniqueName]) {
                    nonUnique = true;
                  }
                }
                if (nonUnique == false) { 
                  
                  TEMP = TEMP + charNameF(EEPROM.read(roomCount)); 
                  }
              } 
              nonUnique = false;
            }  
                    if (!nextPage) {
                      TEMP2 = TEMP;        
                      nextPage = false;
                    }      

      }

    
      break;


    case 3:

      TEMP = "";
      nextPage = false;
      prepareRow(3,rowIndex);
      setCheck(selectedMenuItem);


      if (SelectB) {      
        
        if (selectedMenuItem == 3) {
          //Serial.println("YES");
          SPECIFIC = "";
          menuState = 0;
        }

        if (selectedMenuItem == 4) {
          //Serial.println("NEXT");
          rowIndex = rowIndex + 2;
          nextPage = true;          
          
        }
        
        if (selectedMenuItem == 1 && menu[0] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex];
           rowIndex = 0;
           menuState = 4;
        }

        if (selectedMenuItem == 2 && menu[1] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex+1];        
          rowIndex = 0;
          menuState = 4;
        }

              if (rowIndex >= TEMP2.length()) { rowIndex = 0; }

            for (int roomCount = 4; roomCount < (roomNum*5); roomCount = roomCount + 5) {
              
              if (EEPROM.read(roomCount-4) == FloorF(SPECIFIC[0]) && EEPROM.read(roomCount-3) == RoomF(SPECIFIC[1]) && EEPROM.read(roomCount-2) == TypeF(SPECIFIC[2]) && EEPROM.read(roomCount-1) == NameF(SPECIFIC[3])) {
    
                //Serial.println(TEMP+" TEST");
                
                for (int uniqueQualifier = 0; uniqueQualifier < TEMP.length(); uniqueQualifier++) {   
                  
                  if (charQualifierF(EEPROM.read(roomCount)) == TEMP[uniqueQualifier]) {
                    nonUnique = true;
                  }
                }
                if (nonUnique == false) { 
                  
                  TEMP = TEMP + charQualifierF(EEPROM.read(roomCount)); 
                  }
              } 
              nonUnique = false;
            }  
                    if (!nextPage) {
                      TEMP2 = TEMP;        
                      nextPage = false;
                    }       

      }
      break;


    case 4:
      TEMP = "";
      if (rowIndex >= TEMP2.length()) { rowIndex = 0; }  
      prepareRow(4,rowIndex);
      setCheck(selectedMenuItem);


      if (SelectB) {

        if (selectedMenuItem == 3) {
          //Serial.println("YES");
          SPECIFIC = "";
          menuState = 0;
        }

        if (selectedMenuItem == 4) {
          //Serial.println("NEXT");
          rowIndex = rowIndex + 2;
          
        }
        
        if (selectedMenuItem == 1 && menu[0] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex];
           rowIndex = 0;
           menuState = 5;
           clearRows("On Time OffTime ","Back    Value   ");
           setCheck(selectedMenuItem);
        }

        if (selectedMenuItem == 2 && menu[1] ==  1) {
          SPECIFIC = SPECIFIC + TEMP2[rowIndex+1];
          rowIndex = 0;
          menuState = 5;
          clearRows("On Time OffTime ","Back    Value   ");
          setCheck(selectedMenuItem);
        }

      }
      break;



    case 5:

    row1 = "On Time OffTime ";
    row2 = "Back    Value   ";
    menu[1] = 1;
    int arrayPointer;

    if (SelectB) {

      for (int roomCount = 0; roomCount < (roomNum*5); roomCount = roomCount + 5) {
              
        if (EEPROM.read(roomCount) == FloorF(SPECIFIC[0]) && EEPROM.read(roomCount+1) == RoomF(SPECIFIC[1]) && EEPROM.read(roomCount+2) == TypeF(SPECIFIC[2])
        && EEPROM.read(roomCount+3) == NameF(SPECIFIC[3]) && EEPROM.read(roomCount+4) == QualifierF(SPECIFIC[4])) {

          arrayPointer = roomCount;
          Serial.println(arrayPointer);
        }      
      }
        
        if (selectedMenuItem == 3) {
          //Serial.println("YES");
          SPECIFIC = "";
          Serial.println(SPECIFIC);
          menuState = 0;
          clearRows("Ground  First   ","Outside Data    ");
          setCheck(selectedMenuItem);
        }

        if (selectedMenuItem == 4) {
          
          Serial.println("Please enter a value from 0-100");
          while (Serial.available() == 0) { }
          Serial.println("TEST A");
          setValue(0,arrayPointer);
         
        }
        
        if (selectedMenuItem == 1) {
          Serial.println("Please enter what time the device will go on");
          while (Serial.available() == 0) { }          
          setValue(1,arrayPointer);
        }

        if (selectedMenuItem == 2) {
          Serial.println("Please enter what time the device will go off");
          while (Serial.available() == 0) { }          
          setValue(2,arrayPointer);
        }

      }
      break;
    
  }
    lcd.setCursor(0,0);
    lcd.print(rowOne);
    lcd.setCursor(0,1);
    lcd.print(rowTwo);
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setValue(int mode, int pointer) {

  int valueInput;
  
  switch(mode) {

    case 0:

      valueInput = Serial.parseInt();
      Serial.println(valueInput);
      if (valueInput <= 100 & valueInput > 0) {
        Levels[pointer/5] = valueInput;
      }      
      break;
      
    case 1: {

      String timeOnInput = Serial.readString();
      int i;
      
      while (timeOnInput[i] != '.') { i++; }

      String charHours = timeOnInput.substring(0,i);
      int hours = (atoi(charHours.c_str()))*60;
      String charMinutes = timeOnInput.substring(i+1,i+3);
      int minutes = atoi(charMinutes.c_str());
      onTime[pointer/5] = hours+minutes;
      break; }

    case 2: {

      String timeOffInput = Serial.readString();
      int j;
      
      while (timeOffInput[j] != '.') { j++; }
  
      String charHoursB = timeOffInput.substring(0,j);
      int hoursB = (atoi(charHoursB.c_str()))*60;
      String charMinutesB = timeOffInput.substring(j+1,j+3);
      int minutesB = atoi(charMinutesB.c_str());
      offTime[pointer/5] = hoursB+minutesB;
      break; }
  }
  
}






//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void readEEPROM() {

  
  for (int i = 0; i < (roomNum*5); i = i + 5) {
    Serial.print(selectFloorF(EEPROM.read(i))+"/");
    Serial.print(selectRoomF(EEPROM.read(i+1))+"/");
    Serial.print(selectTypeF(EEPROM.read(i+2))+"/");
    Serial.print(selectNameF(EEPROM.read(i+3))+"/");
    Serial.print(selectQualifierF(EEPROM.read(i+4))+"/");
    Serial.print("On: "+decodeDate(onTime[i/5])+"/");
    Serial.print("Off: "+decodeDate(offTime[i/5])+"/");
    Serial.println("Level: "+(String)Levels[i/5]);
  } 
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void prepareDeviceF(String parameter) {

  String device = "";
 
  for (int i = 0; i < parameter.length(); i++) {

    switch(parameter[i]) {
      
    case ',':

      if (parameter[i-1] != ',') {
        

        setDeviceF(roomNum,device);
        initialiseDeviceSettingsF(roomNum);
        roomNum++;
        device = "";
        }       
      break;

    case '/':

      if (parameter[i+1] == 'n') {
        setDeviceF(roomNum,device);
        initialiseDeviceSettingsF(roomNum);
        roomNum++;
        device = "";
        
        i++;
      }
      break;

    case ' ':
      break;

    case '.':
      setDeviceF(roomNum,device);
      initialiseDeviceSettingsF(roomNum);
      roomNum++;
      device = "";
      return;
      
      default:
        device = device + parameter[i];
    }   
  }
  setDeviceF(roomNum,device.substring(0,(device.length()-1)));
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setDeviceF(int roomNum, String parameter) {

  for (int j = 0; j < parameter.length(); j++) {

    byte byteInput;
    
    switch(j) {
   
      case 0:
        byteInput = FloorF(parameter[j]);
        break;
        
      case 1:
        byteInput = RoomF(parameter[j]);        
        break;
        
      case 2:
        byteInput = TypeF(parameter[j]);
        break;
        
      case 3:
        byteInput = NameF(parameter[j]);
        break;
        
      case 4:
        byteInput = QualifierF(parameter[j]); 
        break;   
        
    }

    EEPROM.write(((roomNum*5)+j), byteInput);
  }
}

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

byte FloorF(char parameter) {

  switch (parameter) {
    case 'G':
      return 0;
    case 'F':
      return 1;
    case 'O':
      return 2;
  };
}



String selectFloorF(byte parameter) {

  switch (parameter) {
    case 0:
      return "Ground";
    case 1:
      return "First";
    case 2:
      return "Outside";
  };
}



char charFloorF(byte parameter) {

  switch (parameter) {
    case 0:
      return 'G';
    case 1:
      return 'F';
    case 2:
      return 'O';
  };
}



byte RoomF(char parameter) {

  switch (parameter) {
    case 'K':
      return 0;
    case 'B':
      return 1;
    case 'L':
      return 2;
    case '1':
      return 3;
    case '2':
      return 4;
    case '3':
      return 5;
    case '4':
      return 6;
    case 'G':
      return 7;
    case 'R':
      return 8;
    case 'P':
      return 9;
    case 'H':
      return 10;
  };
}



String selectRoomF(byte parameter) {

  switch (parameter) {
    case 0:
      return "Kitchen";
    case 1:
      return "B-room";
    case 2:
      return "Lounge";
    case 3:
      return "Room 1";
    case 4:
      return "Room 2";
    case 5:
      return "Room 3";
    case 6:
      return "Room 4";
    case 7:
      return "Garage";
    case 8:
      return "Garden";
    case 9:
      return "P Room";
    case 10:
      return "Hall";
  };
}



char charRoomF(byte parameter) {

  switch (parameter) {
    case 0:
      return 'K';
    case 1:
      return 'B';
    case 2:
      return 'L';
    case 3:
      return '1';
    case 4:
      return '2';
    case 5:
      return '3';
    case 6:
      return '4';
    case 7:
      return 'G';
    case 8:
      return 'R';
    case 9:
      return 'P';
    case 10:
      return 'H';
  };
}



byte TypeF(char parameter) {

  switch (parameter) {
    case 'L':
      return 0;
    case 'A':
      return 1;
    case 'H':
      return 2;
    case 'W':
      return 3;

  };
}



String selectTypeF(byte parameter) {

  switch (parameter) {
    case 0:
      return "Light";
    case 1:
      return "Lamp";
    case 2:
      return "Heat";
    case 3:
      return "Water";

  };
}



char charTypeF(byte parameter) {

  switch (parameter) {
    case 0:
      return 'L';
    case 1:
      return 'A';
    case 2:
      return 'H';
    case 3:
      return 'W';

  };
}



byte NameF(char parameter) {

  switch (parameter) {
    case 'M':
      return 0;
    case 'C':
      return 1;
    case 'D':
      return 2;
    case 'B':
      return 3;
    case 'P':
      return 4;
    case 'W':
      return 5;
    default:
      return 0 ;
  };
}



String selectNameF(byte parameter) {

  switch (parameter) {
    case 0:
      return "Main";
    case 1:
      return "Ceiling";
    case 2:
      return "Desk";
    case 3:
      return "Bed";
    case 4:
      return "CupB";
    case 5:
      return "Wall";
    default:
      return "Main" ;
  };
}



char charNameF(byte parameter) {

  switch (parameter) {
    case 0:
      return 'M';
    case 1:
      return 'C';
    case 2:
      return 'D';
    case 3:
      return 'B';
    case 4:
      return 'P';
    case 5:
      return 'W';
    default:
      return 'M' ;
  };
}



byte QualifierF(char parameter) {

  switch (parameter) {
    case '1':
      return 0;
    case '2':
      return 1;
    case '3':
      return 2;
    case 'L':
      return 3;
    case 'R':
      return 4;
  };
}



String selectQualifierF(byte parameter) {

  switch (parameter) {
    case 0:
      return "one";
    case 1:
      return "two";
    case 2:
      return "three";
    case 3:
      return "left";
    case 4:
      return "right";
  };
}



char charQualifierF(byte parameter) {

  switch (parameter) {
    case 0:
      return '1';
    case 1:
      return '2';
    case 2:
      return '3';
    case 3:
      return 'L';
    case 4:
      return 'R';
  };
}


#ifdef __arm__
// should use uinstd.h to define sbrk but Due causes a conflict
extern "C" char* sbrk(int incr);
#else // __ARM__
extern char *__brkval;
#endif // __arm__

int freeRam () {

char top;
#ifdef __arm__
return &top - reinterpret_cast<char*>(sbrk(0));
#elif defined(CORE_TEENSY) || (ARDUINO > 103 && ARDUINO != 151)
return &top - __brkval;
#else // __arm__
return __brkval ? &top - __brkval : &top - __malloc_heap_start;
#endif // __arm__

}



void initialiseDeviceSettingsF(int roomNum) {
  onTime[roomNum] = 540;
  offTime[roomNum] = 1140;
  Levels[roomNum] = 50;
  
}



void setCheck(int selectedMenuItem) {

  if (selectedMenuItem > 2) { 
    rowTwo[((selectedMenuItem - 2) * 8) - 1] = 'X';
    }
  else {
    rowOne[(selectedMenuItem * 8) - 1] = 'X';
  }
}



void clearRows(String row1, String row2) {

  rowOne = row1;
  rowTwo = row2;
  
}



String decodeDate(int rawDate) {

  String hoursReturn;
  String minutesReturn;

  int hours = rawDate / 60;
  int minutes = rawDate - (60*hours);

  if (hours < 10) { hoursReturn = "0"+(String)hours; }
  else { hoursReturn = (String)hours; }

  if (minutes < 10) { minutesReturn = "0"+(String)minutes; }
  else { minutesReturn = (String)minutes; }
  
  String decodedResult = hoursReturn+"."+minutesReturn;
  return decodedResult;
}



void prepareRow(int mode, int index) {

  static String columnOne = "";
  static String columnTwo = "";
  static String tempColumnTwo = "";

  switch(mode) {

    case 1: 
      columnOne = selectRoomF(RoomF(TEMP2[index]));
      tempColumnTwo = selectRoomF(RoomF(TEMP2[index+1]));
      break;

    case 2: 
      columnOne = selectTypeF(TypeF(TEMP2[index]));
      tempColumnTwo = selectTypeF(TypeF(TEMP2[index+1]));
      break;
      
    case 3:
      columnOne = selectNameF(NameF(TEMP2[index]));
      tempColumnTwo = selectNameF(NameF(TEMP2[index+1]));
      break;
      
    case 4:
      columnOne = selectQualifierF(QualifierF(TEMP2[index]));
      tempColumnTwo = selectQualifierF(QualifierF(TEMP2[index+1]));
      break;
      
  }
      int length1 = columnOne.length();
      for (int Rc1 = 0; Rc1 < (8 - length1); Rc1++) { columnOne = columnOne + " "; }
      
      if (index+1 != TEMP2.length()) {
        columnTwo = tempColumnTwo;
        int length2 = columnTwo.length();
        for (int Rc2 = 0; Rc2 < (8 - length2); Rc2++) { columnTwo = columnTwo + " "; }
        menu[1] = 1;
      } 
      else { 
        columnTwo = "        "; 
        menu[1] = 0;
      }
      row1 = columnOne + columnTwo;
      row2 = "Back    Next    ";
      clearRows(row1,row2);
      
}
