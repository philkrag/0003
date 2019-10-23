
/*
// /////////////////////////////////////////////////////////////////////// COPYRIGHT NOTICE
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// You should have received a copy of the GNU General Public License
// along with Pip-Project.  If not, see <http://www.gnu.org/licenses/>.
// /////////////////////////////////////////////////////////////////////// COPYRIGHT NOTICE
// /////////////////////////////////////////////////////////////////////// VERSION CONTROL
// PAGE CREATED BY: Phillip Kraguljac
// PAGE CREATED DATE: 2019-07-23
// DATE       || NAME           || MODIFICATION
// 2019-07-23   || Phillip Kraguljac    || Created.
// 2019-10-23   || Phillip Kraguljac    || Added RTC Reset Function.
// /////////////////////////////////////////////////////////////////////// VERSION CONTROL
*/

/* STILL UNDER CONSTRUCTION */

#include <Wire.h>                                                                   // Used for RTC functions.
#include "RTClib.h"                                                                 // Used for RTC functions.
#include <SPI.h>                                                                    // Used for SD Card functions.
#include <SD.h>                                                                     // Used for SD Card functions.


// /////////////////////////////////////////////////////////////////////// USER DEFINABLE VARIABLES
String Data_Logging_File = "Data.csv";                                              // File to be used for data logging.

// /////////////////////////////////////////////////////////////////////// SYSTEM VARIABLES
File myFile;                                                                        // Create a file object.
RTC_Millis RTC;                                                                     // Create a RTC object.

String Incoming_Buffer = "";                                                        // Temporary storage of data being recieved.
bool stringComplete = false;                                                        // [Possibly Obsolete]
bool Data_Transmitting_Flag = false;                                                // [Possibly Obsolete]
bool Data_Revieced_Flag = false;                                                    // Used for determing when data packets have been sent.
int Serial_Recieve_Indexer = 0;                                                     // Used for checksum feedback.
String Temp_Message_Indexer;                                                        // Used for temporary memory.

void setup() {

Serial.begin(9600);                                                                 // Start serial connection.
RTC.begin(DateTime(__DATE__, __TIME__));                                            // Initiate RTC.
Incoming_Buffer.reserve(200);                                                       // Define allowable memory.
//Serial.print("Initializing SD card...");                                          // For technical support.
if (!SD.begin(4)) {                                                                 // Start SD communication.
//Serial.println("initialization failed!");                                         // For technical support.
return;
}
  
//Serial.println("initialization done.");                                           // For technical support.
myFile = SD.open(Data_Logging_File, FILE_WRITE);                                    // Open SD Card file.
Extract_SD_Data();                                                                  // Display data already recorded.

}

void loop() {

while (Serial.available() > 0) {
Incoming_Buffer = Incoming_Buffer + Serial.read();                                  // When serial is available. 
Data_Transmitting_Flag = true;                                                      // [Possibly Obsolete] Raise flag for data tramission.
}

if(Incoming_Buffer.indexOf("\r") >= 0){Data_Revieced_Flag = true;}                  // When return carriage detected raise flag for data being sent.

if(Data_Revieced_Flag){                                                             // When data has been recieved.
Input_SD_Data(Incoming_Buffer);                                                     // Save data to SD card.
Temp_Message_Indexer = "["+Padding(6,(String)Serial_Recieve_Indexer)+"]";           // Temp external indexer formatting.
Serial.println(Get_Time_Stamp() + Temp_Message_Indexer + " OK");                    // Provide feedback to transmitting device.
Check_Function_Requests(Incoming_Buffer);                                           // Check for externally triggered function request.
Incoming_Buffer = "";                                                               // Reset incoming message buffer.
Data_Revieced_Flag = false;                                                         // Reset data recieved flag.
Serial_Recieve_Indexer++;                                                           // Increment data transmit indexer.
}
}


// ///////////////////////////////////////////////////////////////// FUNCTION(S)

// [FUNCTION] => PULL DATA RECORDED ON SD CARD
void Extract_SD_Data(){
myFile = SD.open(Data_Logging_File);                                                // Open SD card data file.
Serial.println("***EXPORT***");                                                     // Quick reference serial label.
if (myFile) {                                                                       // If file exists.
while (myFile.available()) {Serial.write(myFile.read());}                           // Pull the data from the SD card.
myFile.close();                                                                     // Close the file after completing.
} else { }
}


// [FUNCTION] => INSERT DATA ONTO SD CARD
void Input_SD_Data(String Data_To_Be_Logged){
myFile = SD.open(Data_Logging_File, FILE_WRITE);                                    // Open SD card data file.
if (myFile) {                                                                       // If file exists.
myFile.println(Get_Time_Stamp() + Data_To_Be_Logged);                               // Insert data.
myFile.close();                                                                     // Close the file after completing.
} else {  }
}


// [FUNCTION] => EXTERNAL FUNCTION CALLING
void Check_Function_Requests(String String_To_Be_Checked){
if(String_To_Be_Checked.indexOf("SDFileExport") >= 0){Extract_SD_Data();}           // Export data to serial if requested.
if(String_To_Be_Checked.indexOf("SDFileDelete") >= 0){SD_File_Delete();}            // Delete data file if requested.
if(String_To_Be_Checked.indexOf("RTCSetTime") >= 0){RTC_Set_Time(String_To_Be_Checked);}                // Set RTC Time.
}


// [FUNCTION] => DELETE DATA FILE
void SD_File_Delete(){
Serial.println("***DELETE***");                                                     // Quick reference serial display.
SD.remove(Data_Logging_File);                                                       // Delete data file.
}


// [FUNCTION] => RTC RESET TIME
void RTC_Set_Time(String String_To_Be_Checked){                                     // [0] => Year: [1] => Month: [2] => Day: [3] => Hour: [4] => Minutes: [5] => Seconds
Serial.println("***TIME RESET***");                                                 // Quick reference serial display.
int First_Demlimiter = 0;                                                           // ...
int Next_Delimiter = 0;                                                             // ...
int Last_Delimiter_Memory = 0;                                                      // ...
int String_Length = String_To_Be_Checked.length();                                  // ...
String Individual_Extracted_Item = "";                                              // ...
String Last_String = "";                                                            // ...
int Input_Memory[10];                                                               // ...
bool Completed_Conversion = false;                                                  // ...
for (int i=0; i<10; i++) {                                                          // ...
First_Demlimiter = String_To_Be_Checked.indexOf(";", Last_Delimiter_Memory);        // ...
Next_Delimiter = String_To_Be_Checked.indexOf(";", First_Demlimiter+1);             // ...
if(!Completed_Conversion){                                                          // ...
Individual_Extracted_Item = String_To_Be_Checked.substring(First_Demlimiter+1, Next_Delimiter);     // ...
Last_Delimiter_Memory = Next_Delimiter;                                             // ...
Input_Memory[i] = Individual_Extracted_Item.toInt();                                // ...
}                                                                                   // ...
if(Next_Delimiter<0){Completed_Conversion=true;}                                    // ...
}                                                                                   // ...
RTC.adjust(DateTime(Input_Memory[0],Input_Memory[1],Input_Memory[2],Input_Memory[3],Input_Memory[4],0));    // ...
}                                                                                   // ...


// [FUNCTION] => OBTAIN DATE TIME / STAMP
String Get_Time_Stamp(){
DateTime now = RTC.now();                                                           // Get RTC data.
String Current_Date_Time_Stamp = "[Not Known]";                                     // Initial data value [default]
Current_Date_Time_Stamp = Padding(4,(String)now.year()) + "/" + Padding(2,(String)now.month()) + "/" + Padding(2,(String)now.day())
 + " " + Padding(2,(String)now.hour()) + ":" + Padding(2,(String)now.minute())+ ":" + Padding(2,(String)now.second()) + ";";
  return Current_Date_Time_Stamp;                                                   // Return time date stamp.
}


// [FUNCTION] => APPLY REQUIRED PADDING
String Padding(int Expected_Spaces, String String_To_Modify){
String Return_String = "";                                                          // Initial data value [default]
if(String_To_Modify.length()<Expected_Spaces){                                      // If there is a difference between expected and actual.
int Total_Difference = Expected_Spaces - String_To_Modify.length();                 // Determine difference.
for (int i = 0; i < Total_Difference; i++) {Return_String = Return_String + "0";}   // Add padding requried based on difference.
Return_String = Return_String + String_To_Modify;                                   // Add initial data.
}else{Return_String = String_To_Modify;}                                            // If value passed initially - keep the same.
return Return_String;                                                               // Return formatted value.
}


void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if (inChar != '\n') {
    Incoming_Buffer += inChar;
    }
//    if (inChar == '\n') {stringComplete = true;}
  }
}
