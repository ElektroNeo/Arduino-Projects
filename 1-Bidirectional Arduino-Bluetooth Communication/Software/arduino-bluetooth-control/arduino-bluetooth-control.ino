#include <SoftwareSerial.h>
#include <Adafruit_AHTX0.h>

// Pins
#define LED           2
#define IN1           4
#define IN2           5
#define EN12          6

// Modes
#define RESET_MODE    -2
#define READ_MODE     -1
#define SELECT_MODE   0
#define LED_MODE      1
#define DIR_MODE      2
#define SPD_MODE      3
#define TMP_MODE      4
#define HUM_MODE      5

// Variables
sensors_event_t humidity, temp;
int mode = -2, counter = 0;
int dataStr[10];
String str = "";
char ledOnOff = 0, motorDir = 0, motorSpeed = 0, tempSend = 0, humSend = 0;

SoftwareSerial bluetooth(9, 10); // TX of Bluetooth, RX of Bluetooth
Adafruit_AHTX0 aht;

void setup() {
  // Set baud rates
  Serial.begin(9600);
  bluetooth.begin(38400);

  // Set pin modes
  pinMode(LED, OUTPUT); // LED
  pinMode(IN1, OUTPUT); // IN1
  pinMode(IN2, OUTPUT); // IN2
  pinMode(EN12, OUTPUT); //EN12

  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(10);
  }
  Serial.println("AHT10 or AHT20 found");
}

void loop () {
  // Get Bluetooth data
  if (bluetooth.available())
  {
    // Read avaliable data
    char data = bluetooth.read();
    // Check for start character
    if (mode == RESET_MODE) {
      if (data == '$') {
        str = "";
        mode = READ_MODE;
        counter = 0;
      }
    }
    // Read mode
    else if (mode == READ_MODE) {
      // Check for end character
      if (data == '#' && counter == 3) {
        mode = SELECT_MODE;
        // Reset counter
        counter = 0;
      }
      // Get mode characters and add to str
      else {
        str += data;
        counter++;
      }
      // Select mode
      if (mode == SELECT_MODE) {
        if (str == "LED")
          mode = LED_MODE;
        else if (str == "DIR")
          mode = DIR_MODE;
        else if (str == "SPD")
          mode = SPD_MODE;
        else if (str == "TMP")
          mode = TMP_MODE;
        else if (str == "HUM")
          mode = HUM_MODE;
        else
          mode = RESET_MODE;
      }
    } else if (mode == LED_MODE) {
      ledOnOff = data;
      mode = RESET_MODE;
    } else if (mode == DIR_MODE) {
      motorDir = data;
      mode = RESET_MODE;
    } else if (mode == SPD_MODE) {
      motorSpeed = data;
      mode = RESET_MODE;
    } else if (mode == TMP_MODE) {
      tempSend = 1;
      mode = RESET_MODE;
    } else if (mode == HUM_MODE) {
      humSend = 1;
      mode = RESET_MODE;
    } else {
      mode = RESET_MODE;
    }
  }

  // LED control
  if (ledOnOff == 1) {
    digitalWrite(LED, HIGH);
  }
  else {
    digitalWrite(LED, LOW);
  }

  // Motor direction control
  if (motorDir == 1) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  }
  else if (motorDir == 2) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  }
  else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  // Motor speed control
  analogWrite(EN12, motorSpeed);
  //Serial.println(motorSpeed);

  // Temperature send mode
  if (tempSend == 1) {
    // Read and calculate temperature
    aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
    //Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");

    int tempNum = round(temp.temperature);
    bluetooth.write(tempNum & 0xFF);
    // Clear temperature send mode
    tempSend = 0;
  }

  // Humidity send mode
  if (humSend == 1) {
    // Read and calculate humidity
    aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
    //Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

    int humNum = round(humidity.relative_humidity);
    bluetooth.write(humNum & 0xFF);
    // Clear humidity send mode
    humSend = 0;
  }
}
