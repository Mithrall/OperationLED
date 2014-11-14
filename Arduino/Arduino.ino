#include <Process.h>
#include <String.h>

// Analog pin the microphone uses
const int SENSOR_PIN = A0;

// Number of LED's that are used
const int LED_COUNT = 16;
// Number of PWM sets that are used (3 PWM's per set)
const int PWM_COUNT = 2;

// the more PWM sets used the more LED's 
// can be changed at the same time
const int LOOP_SIZE = LED_COUNT/PWM_COUNT;

// Maximum analog value is 1023
// The microphones max is 704
const int MAX_SENSOR = 704;

// Gets filled in approximately 1 second
const int SENSOR_ELEMENTS_SIZE = 58;
// Gets filled in approximately 1 minute
const int BUFFER_SIZE = 60;

// number of reads that are in sensor_avg
byte sensor_elements = 0;
// sensor_elements == SENSOR_ELEMENTS_SIZE, 
// sensor_avg/sensor_elements will be the 
// average for a second
float sensor_avg = 0;

// the index that is free
byte bufferIndex = 0;
// this buffer contains each sensor_avg reading, one per second
float buffer[BUFFER_SIZE];

// used to serial output
long lastMillis = 0;
long lastWhole = 0;

// this is reused across runs
Process pClient;

void setup() 
{
  Bridge.begin();
  Serial.begin(9600);
  
  // run the local server asynchronously
  Process pServer;
  pServer.begin("/usr/bin/hwserver");
  pServer.runAsynchronously();
  
  lastMillis = millis();
}

void loop() 
{
  float avg = processInputOutput();
  addToBuffers(avg);
  
  // only display once per second
  if (sensor_elements == 0)
  {
    // lastWhole is the number of seconds a complete buffer 0-60 took
    Serial.println("Second: " + String(bufferIndex, DEC) + ", avg: " + String(buffer[bufferIndex-1], DEC) + ", seconds: " + String(lastWhole, DEC));
  }
  
  // once the buffer is full, start sending data to local client
  if (bufferIndex >= BUFFER_SIZE)
  {
    lastWhole = millis() - lastMillis;
    
    // if the process is already running, this skips it in this case and resets the buffer
    if (pClient.running() == false)
    {
      pClient.begin("/usr/bin/hwclient");
      
      for (int i = 0; i < bufferIndex; i++)
      {
        // each seconds reading is added as parameter
        pClient.addParameter(String(buffer[i], DEC));
      }
      
      // runs the client while the sketch continues
      pClient.runAsynchronously();
    }
    
    lastMillis = millis();
    bufferIndex = 0;
  }
}

// Reads the output from the microphone every 2 ms
// This is also where code to update the LED's will be placed later
// returns the average sensor value over LOOP_SIZE * 2 ms
float processInputOutput()
{
  float tmp = 0;
  
  for (int i = 0; i < LOOP_SIZE; i++)
  {
    // TODO: insert code that changes lights here
    
    tmp += analogRead(SENSOR_PIN);
    delay(2);
  }

  // average over LOOP_SIZE*2 ms
  return tmp / LOOP_SIZE;
}

// add the avg to the buffers
void addToBuffers(int avg)
{
  sensor_avg += avg;
  sensor_elements++;
  
  if (sensor_elements >= SENSOR_ELEMENTS_SIZE)
  {
    buffer[bufferIndex++] = (sensor_avg / sensor_elements);
    
    sensor_avg = sensor_elements = 0;
  }
}

