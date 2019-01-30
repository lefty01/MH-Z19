 /* 
    For the library to calculate different values from a single response array, 
    the response is stored into 1 of 4 arrays. However, some function share the
    same response data, therefore the option has be created for any valid
    function (see header file) to request or not request a new response. This 
    reduces the chance of over saturation and communication errors.        
      
    Format example; getCO2ppm(New Request, isUnlimited)
        
    New Request:
    (true)  Request is sent and response verified .
    (false) Request is not sent or verified.
    (true)  Default.       
      
    The second bool argument selects which command is used, unlimited or limited for 
    CO2. Although is not seen in an obvious way, it changes which command is sent
    and but more importantly which response array is written to.
        
    isUnlimited:
    (true)  Use unlimimted CO2 / command 133. 
    (false) Use limied CO2 / command 134.        
    (true)  Default.

    Note: getTemperature("new response array", " is unlimited type") also uses
    the same functionality and should be considered in conjunction. Most other
    commands share a seperate array.
*/

#include <Arduino.h>
#include "MHZ19.h"                                                                   

#define RX_PIN 10                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 11                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Native to the sensor (do not change)

MHZ19 myMHZ19;                                             // Constructor for MH-Z19 class
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // Constructor for Stream class *change for HardwareSerial, i.e. ESP32 ***

//HardwareSerial mySerial(1);                              // ESP32 Example 
unsigned long getDataTimer = 0;                                                        

void setup()
{
    Serial.begin(9600);

    mySerial.begin(BAUDRATE);                               // Begin Stream with MHZ19 baudrate

    //mySerial.begin(BAUDRATE, SERIAL_8N1, RX_PIN, TX_PIN); // ESP32 Example 

    myMHZ19.begin(mySerial);                                // *Imporant, Pass your Stream reference
 
    myMHZ19.printCommunication(true, true);                 // *Shows communication between MHZ19 and Device.
                                                            // use printCommunication(true, false) to print as HEX

    myMHZ19.autoCalibration(false);                         // Turn Auto Calibration OFF

    myMHZ19.getCO2();                                       // *Holds last response from command 134 (0x86)
}                                                           //  in library (see description below)

void loop()
{
    if (millis() - getDataTimer >= 2000)                                                // Check if interval has elapsed
    {

        /******* functionality only deominstration as a software alarm *******/

        Serial.print("CO2 (ppm): ");
        Serial.println(myMHZ19.getCO2(false));           

        /* Command 134 is limited by background CO2 (it will not show less than 400ppm) or
           greater than the defined range - using these thresholds. */

        if (myMHZ19.getCO2(false) - myMHZ19.getCO2(false, false) >= 10)             // *Check if stored CO2 reading is 10pp greater than 
        {                                                                           // the limited CO2 reading stored.
            Serial.println("Verifying CO2 Threshold");
            myMHZ19.getCO2(true, false);                                            // Update command 134 response

            if (myMHZ19.getCO2(false) - myMHZ19.getCO2(false, false) < 10)          // Has the difference been closed?
                Serial.println("Increase Verified");                                       
            else                                                                    // Value must be limited
            {
                Serial.println("Alert! CO2 out of range and could not be verified");
                Serial.print(myMHZ19.getCO2(false, false));                          // Some alarm action
                Serial.println(" threshold has been passed!");   
     /* Sanity check vs Raw CO2 (has Span/Zero failed) or straight to your Alarm code */
            }
        }

        Serial.print("Temperature (C): ");                                            // *Request Temperature (as Celsius) without a new
        Serial.println(myMHZ19.getTemperature(false, true));                          //  request format: getTempeature(New Request,isUnlimied)                         
                                                                                      //  where isUnlimied == true uses command 133.                                                                                           
        myMHZ19.getCO2();                                                             // *Requset new response for command 134 (0x86) 
                                                                                      //  (default is true so not entered).
        getDataTimer = millis();                                                      
    }                                                                               
}   