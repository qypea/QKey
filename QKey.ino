#include <AES.h>

#include <SPI.h>
#include <SD.h>

#define LEDSerial 13
#define LEDSD 8

// Reset helper
void(*reset) (void) = 0;

// Serial wrappers to blink led when active
void print(String str) {
  digitalWrite(LEDSerial, HIGH);
  Serial.print(str);
  digitalWrite(LEDSerial, LOW);
}

void println(String str) {
  digitalWrite(LEDSerial, HIGH);
  Serial.println(str);
  digitalWrite(LEDSerial, LOW);
}

void setup() {
  pinMode(LEDSerial, OUTPUT);
  pinMode(LEDSD, OUTPUT);

  // Open serial communications and wait for port to open:
  digitalWrite(LEDSerial, HIGH);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  println("Serial connected");
  digitalWrite(LEDSerial, LOW);

  // Open SD Card
  print("Initializing SD card...");
  digitalWrite(LEDSD, HIGH);
  if (!SD.begin(4)) {
    digitalWrite(LEDSD, LOW);
    println("failed! Resetting in 5s");
    delay(5*1000);
    reset();
    return;
  }
  println("done.");
  digitalWrite(LEDSD, LOW);
}

// Main loop
void loop() {
}
