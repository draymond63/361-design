#include <SD.h>
#include <SPI.h>
#include <TimeLib.h>
#include <Bounce2.h>

#include "DHT.h"

// Pins
#define BUTTON_PIN 19
#define LSENSOR_PIN A3
#define DHTPIN 21
#define RLED_PIN 16
#define GLED_PIN 15
#define CHIP_SELECT 10

// Constants
#define SAMPLE_PERIOD_MS 1000
#define IS_FAHRENHEIT false

// Instantiate a Bounce object for the buttons
Bounce dButton = Bounce(); 
DHT dht(DHTPIN, DHT11);

// Create a timer object that measures elapsed time in ms
elapsedMillis timer = 0;
// character array to store the current filename
String filename;

/***********************************************************************************************
 * 
 * Setup
 * 
 ***********************************************************************************************/
 
void setup() {
  // Start the Serial Port so we can send messages back and forth
  Serial.begin(115200); // USB is always 12 Mbit/sec on TEENSY - it's SPECIAL
  
  // Define our analog input pin formally for Light Sensor
  pinMode(LSENSOR_PIN,INPUT);
  // Define our Output pins formally for both the LEDs
  pinMode(RLED_PIN,OUTPUT);
  pinMode(GLED_PIN,OUTPUT);
  // Buttons need inputs w/ PULLUP resistors
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  // They are common anode LEDs, so we need to set it LOW to turn it ON, HIGH to turn it OFF
  digitalWrite(RLED_PIN,HIGH);
  digitalWrite(GLED_PIN,HIGH);
  // After setting up the button, setup the Bounce instance:
  dButton.attach(BUTTON_PIN);
  dButton.interval(5); // debounce interval in ms
  
  delay(100);

  // Initialize the SD Card
  Serial.print("Initializing SD card... ");
  // See if the card is present and can be initialized
  if (!SD.begin(CHIP_SELECT)) {
    throwError("Card failed, or not present");
  }
  Serial.println("card initialized.");

  // Initialize DHT Sensor
  Serial.print("Initializing DHT temperature sensor... ");
  dht.begin();
  // Check if any reads failed and exit early (to try again).
  if (isnan(dht.readHumidity())) {
    throwError("Failed to read from DHT sensor!");
  }
  Serial.println("DHT initialized.");

  delay(100);

  // Set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  delay(100);
  if (timeStatus()!= timeSet) {
    throwError("Unable to sync with the RTC");
  } else {
    Serial.println("RTC has set the system time");
  }

  // Define the current datalog file to write to
  startNewLog();
 
  // Turn Green LED on to indicate things are set up!
  digitalWrite(GLED_PIN,LOW);
}


/***********************************************************************************************
 * 
 * Loop
 * 
 ***********************************************************************************************/

void loop() {
  // Update so we know if the state of the button has changed:
  dButton.update();
  
  // If more than 1000ms has passed, record a new sensor value
  if(timer > SAMPLE_PERIOD_MS){
    int light = analogRead(LSENSOR_PIN);
    float humidity = dht.readHumidity();
    float temp = dht.readTemperature();
    float heat = dht.computeHeatIndex(temp, humidity, false);
    writeDataToSD(light, humidity, temp, heat);
    timer = 0;
  }
  // If the BUTTON_PIN has been grounded (i.e. we pressed the button)
  // then the value of dButton.fell() will be TRUE = 1
  // and we'll write the current Data Log file to the Serial Port
  if(dButton.fell()){
    printSDtoSerialPort();
  }
}



/***********************************************************************************************
 * 
 * Helper Functions
 * 
 ***********************************************************************************************/

// Writes the time and parameters to the SD Card
void writeDataToSD(int light, float temp, float humidity, float heatIndex) {
  // make a string for assembling the data to log:
  String dataString = "";

  // Make a string with the given values
  dataString += getTimeStamp();
  dataString += ", ";
  dataString += String(light);
  dataString += ", ";
  dataString += String(temp);
  dataString += ", ";
  dataString += String(humidity);
  dataString += ", ";
  dataString += String(heatIndex);

  writeToSD(dataString);
}

// This is a self defined function to read the datalog file
// on the SD Card and print it to the Serial Port
void printSDtoSerialPort() {
  File dataFile = SD.open(filename.c_str());
  
  // if the file is available, write it to the Serial Port:
  Serial.println();    
  Serial.println(filename);      
  Serial.println("Data Log Output from SD:");
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
    Serial.println("End of File");
    Serial.println();    
  }  // if the file isn't open, pop up an error:
  else {
    throwError("error opening datalog");
  } 
  delay(1000);
}

// Function to create a new datalog filename
void startNewLog() {
  // make a string for assembling the data to log:
  String dataString = "";

  // Make a string with the 
  dataString += String(hour());
  dataString += "_";  
  dataString += formatDigits(minute());
  dataString += "_";
  dataString += formatDigits(second());
  dataString += ".csv";
  filename = dataString;
  Serial.println("New file log:");
  Serial.println(filename);
  Serial.println();

  // write the headers of log
  File dataFile = SD.open(filename.c_str());
  writeToSD("Time, Light, Temperature (C), Humidity (%), Heat Index (C)");
}

void writeToSD(String line) {
  // open the file. Note the .c_str() converts from String to character array
  File dataFile = SD.open(filename.c_str(), FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(line);
    dataFile.close();
    Serial.println(line); // print to the serial port too
  } else {
    // if the file isn't open, pop up an error
    throwError("error opening datalog file - write");
  }
}

// Utility function to get the current time as a string
String getTimeStamp() {
  String timestamp = "";
  timestamp += String(hour());
  timestamp += ":";  
  timestamp += formatDigits(minute());
  timestamp += ":";
  timestamp += formatDigits(second());
  timestamp += " ";
  timestamp += String(day());
  timestamp += "-";
  timestamp += String(month());
  timestamp += "-";
  timestamp += String(year()); 
  return timestamp;
}

// Utility function for digital clock display: Prints leading 0's
String formatDigits(int digits){
  String digitString = "";
  if(digits < 10){
    digitString += '0';    
  }
  digitString += String(digits);
  return digitString;
}

// Function to use the Real Time Clock
time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

// Display Error and quit
void throwError(String message) {
  Serial.print("\nERROR: ");
  Serial.println(message);
  Serial.println("\nEND OF PROGRAM");
  // We're using the RED LED as an error message
  digitalWrite(RLED_PIN, LOW);
  digitalWrite(GLED_PIN, HIGH);
  delay(100);
  digitalWrite(RLED_PIN, HIGH);
  delay(100);    
  digitalWrite(RLED_PIN, LOW);
  exit(0);
}
