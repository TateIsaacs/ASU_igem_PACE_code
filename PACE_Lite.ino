// Software version 1.0 
// Creators: Ryan Crane and Tate Isaacs 

// Notes on the variables used:
// const is a constant variable which does not change at any point during the program. In our code this variable is often used to assign a pin number to a specific variable, something that will not change. 
// int is a variable that can be assigned a value that can change and be changed throughout the program. This variable can only store a 16-bit value or from -32,768 to 32,767 
// unsigned long is a variable that can store a 32-bit variable this allows the variable to store much larger numbers, it can only store positive values so this give the variable a range from 0 to 4,294,967,295
// bool simply means a boolean which is a sort of variable that can only store a true or false value 
// One additional note about unsigned long and the millis() function, this function will overflow after 49 days. While we feel it is unlikely that individuals will be running a PACE experiment for more then 49 days, however we wanted to warn the user that the code will mostly malfunction if you try and run the system for more then 49 days continuously. 

// variable definitions-  Before the setup function we have our variable definitions. This variables are considered to be declared globally, meaning they can be used at any point within the code. 

//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

// Stir motors 
const int stir1 = 5;  // This assigns the stirmotors to pin 6 on the microcontroller 

// Variable for Peristaltic Pump function 
const int pump = 3;                         // This assigns the pump to pin 9 
unsigned long previousintervalpump = 0;     //  This variable is set to 0 in our loop function it is set to time when the pump was last turned on                                     
unsigned long pumponinterval = 1000;        // This variable controls how long the runs once it is turned on. The time is in milliseconds in our case the pump is on for 1 second or 1000 milliseconds 
unsigned long currenttime = 0;              // This variable represents the current time of the system we set it to 0 to start 

// Mid log Set point Variables 
int setpoint = 800;                         // This variable is set to the value of the photoresistor when the bacteria should hit mid log growth. When the photoresistor reading matches this set point the pump will turn on and move media. 
bool setpointhit = false;                   // This is a boolean that tells the code when the set point has been hit we set this to false when the code starts 
unsigned long previousintervalsensor = 0;   // Similar to the variable for the pump function, this variable is the time when the sensor was last read, and is initally set to 0. 
unsigned long setpointdelay = 5000;        // This variable determines the time between when the program checks if the sensor value is equivelent to the set point. The time is in milliseconds so here we have a 10 second delay. 

// Variables for od measurment in Turbidostat 1 
const int sensor1pin = A0;                 // This sets the pin for the photoresistor as A0 on the microcotroller  
int sensor1;                               // This initilizes a variable to store the value of the photoresistor reading 
unsigned long sensordelay = 1000;          // This variable determines the time between when the voltage across the photoresistor is read. ie the time between sensor readings 

//Variables for od measurement in Turbidostat 2 (this is for if your system contains two turbidostats)
//const int sensor_pin2 = A1;
//int sensor2;

// Vatriables for Averaging Function      // To combat potnetial noise in the system, our code averages the sensor values it reads 
int counter = 0;                          // This variable counts the amount of times a sensor reading is taken 
int sum = 0;                              // This variable stores the sum of the sensor reading values 
int average = 0;                          // This variable stores the computed average

//Priming Function Variables              // To ensure the proper flow of media the system first needs to be primed, this varaible allows us to use a button to prime the system 
const int primingbutton = 2;             // This vaiable assigns the button to pin 13 of the micro controller 

//Variables for NEOPixel LEDs 
const int LEDpin = 6;
int numleds = 12;
#include <Adafruit_NeoPixel.h> 
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(numleds,LEDpin, NEO_GRB + NEO_KHZ800);
unsigned long color = pixels.Color(255,255,255);

//Function Declarations                   // Instead of including all of our code in one void loop function we seperated it into multiple different functions to allow for easier interpretation and editing. 
void stirmotors();                        // This function runs the stir motors 
void runsensors();                        // This function tells the arduino to collect data from the photoresistor 
void checksetpoint();                     // This function checks to see if the sensor value has hit the set point 
void primepump();                         // This function allows for the priming of the pump 


//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


void setup()                             // Within the setup function everything is run once at the very begining of the program. This is where we tell the microcontroller weather it should send data and/or power to it's pins or if it should "read" the voltage across it's pins 
{
  //priming function 
  pinMode(primingbutton, INPUT);         // Sets the priming button as an imput, this way the microcontroller can check weather or not the button is pressed. 
  
  //Stirmotor Function 
  pinMode(stir1, OUTPUT);                // Sets the stirmotor pin to an output, so the microcontroller can control when and how fast the stir motors spin. 
  
  //Pump Function 
  pinMode(pump, OUTPUT);                // Sets the pump pin as an output, this allows the microcontroller to control for how long and how fast the pump is moving liquid. 
 
 
  
  // Set up for LEDs 
  pixels.begin();
  for (int i = 0; i < numleds; i++){ 
    pixels.setPixelColor(i, color);
  } 
  pixels.show();

  Serial.begin(9600);                  //This establishes Serial communication with board so that serial monitor can be used
}


//-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

//Notes on the Void Loop Function: 
// The void loop function runs repeatadly for as long as the microcontroller has power
// The analogwrite function allows you to output a range of voltages from your microcontroller's pins. The function analog write accepts the "arguments" ie range of values from 0-255. 

void loop(){
  currenttime = millis();             // This uses the millis(); function to set the variable currenttime to the currenttime 
  stirmotors();                       // Calls the function stirmotors 
  runsensors();                       // Calls the function runsensors 
  checksetpoint();                    // Calls the function checksetpoint 
  primepump();                        // calls the function primepump 

}

// Pump Function 
void runpump(){
 analogWrite(pump, 255);              // This is the maximum output of an analog pin and essential tells the pump to run at full speed. We decided to keep the analog function here instead of using digital write to allow others to easily change the pump speed. 
}

// set point function 
void checksetpoint() { 
if(average <= setpoint && !setpointhit) {     // This says if the sensor value is less then or equal to the setpoint and the boolean setpointhit then execute the code within the {}
  setpointhit = true;                         // This sets the boolean named setpointhit, to true, essentially telling the code that the optical density of mid log growth has been achived 
  previousintervalpump = currenttime;        // This resets the interval variable to ensure that it represents the time when the pump was last turned on  
  Serial.println("Pump is on");
  }

  
  if(setpointhit) {                                                     // This says if the setpointhit boolean is set to true then excecute the code in the {}
    if(currenttime - previousintervalpump < pumponinterval){            // This says if the current time minus the time when the pump was last on is less then the interval run the pump, or you could think of it as continue to run the pump 
     runpump();                                                         // This calls the run pump function 
    } else if (currenttime - previousintervalpump >= pumponinterval){   // This says otherwise if the current time minus the last time the pump was on is greater then or equal to the amount of time the pump should be on then exceute what is in the {}
     analogWrite(pump, 0);                                              // This is the lowest possible value of the analog write function and subsequently turns the pump off
    }
  
    if(currenttime - previousintervalpump >= setpointdelay) {          // This basically asks if the delay or the time between when the program should check the sensor value has been met if that time has been met then excete what is in the {}
      setpointhit = false;                                             // If the time has elapsed, in this case 10 seconds then we no longer know if the setpoint has been hit so we set the variable to false so the program checks the to see if it has been hit again
    }
  } else {
    analogWrite(pump,0);                                               // From what I understand this just makes sure no funny buisness happens, can't really tell you more then that all I know is that it works 
  }
}

// Stirmotor Function 
void stirmotors(){
 analogWrite(stir1, 50);                                    // Powers the Stirmotors with the specified voltage, we try and turn the sitrmotors as slowly as possible to ensure the vortex does not interfer with our OD measurements  
}


// OD Measurement Function  
void runsensors(){
if(currenttime - previousintervalsensor >= sensordelay) {   // If the current time minus the last time the sensor value was read is greater then the sensor delay in this case 3 seconds then excecute what is in the {}
   previousintervalsensor = currenttime;                    // Since the interval has been met or exceeded the sensor value is going to be read this means we need to set the previous interval variable to the current time.
   sensor1 = analogRead(sensor1pin);                        // This uses the analog read command to allow us to read the voltage from the sensor pin this volatage is then coverted to a range by the microcontroller from 0-1023 
    //sensor2 = analogRead(sensor_pin2);                    // read another sensor pin

    Serial.print(sensor1);                                  // This prints out the sensor's reading to the serial monitor  
    //Serial.print(",");                                    // print a comma so that it can be a CSV
    //Serial.print(sensor2);                                // print the second sensor reading
    
  
    //Averaging Function 
    sum = sum + sensor1;                                   // this sets the variable sum to the sum of all the readings of the sensor's readings 
    counter = counter + 1;                                 // this sets the variable counter to what ever the previous value of the variable was plus 1 
  
    if (counter == 10) {                                  // if the sensor has been read is this case 10 times then the rest of the code executes
      average = (sum/10);                                 // the average variable is set to the sum divided by the number of readings taken
      Serial.print(",");                                  // This prints a comma to the serial monitor to make the data in CSV format for easier interpreting later on 
      Serial.print(average);                              // this prints the average variable to the serial monitor 
      sum = 0;                                            // This resets the sum 
      counter = 0;                                        // This resets the counter
    }
    Serial.println();
   }
}
                                     // This prints a new line, by having this outside the if statement then it will print a new line after the second sensor reading but then it will also print a new line if the if statement is executed -- not sure bout this... 
