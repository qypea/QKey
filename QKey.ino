#include <SPI.h>
#include <SD.h>

#include <AtomicFile.h>
#include <AES.h>

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
  String val;
  File fd;

  if (Serial.available()) {
    digitalWrite(LEDSD, HIGH);
    AtomicFile db("/passwd.db", "/passwd.bak");
    digitalWrite(LEDSD, LOW);

    digitalWrite(LEDSerial, HIGH);
    char c = Serial.read();
    digitalWrite(LEDSerial, LOW);
    switch (c) {
      case 'w':
        println("Writing file");
        digitalWrite(LEDSerial, HIGH);
        val = Serial.readStringUntil('\n');
        digitalWrite(LEDSerial, LOW);
        print("Input string: (");
        print(val);
        println(")");

        digitalWrite(LEDSD, HIGH);
        fd = db.open(FILE_WRITE);
        fd.seek(0);
        fd.print(val);
        fd.close();
        db.commit();
        digitalWrite(LEDSD, LOW);
        break;

      case 'x':
        println("RWing file");
        digitalWrite(LEDSerial, HIGH);
        val = Serial.readStringUntil('\n');
        digitalWrite(LEDSerial, LOW);
        print("Input string: (");
        print(val);
        println(")");

        digitalWrite(LEDSD, HIGH);
        fd = db.open(FILE_WRITE);
        fd.seek(0);
        fd.print(val);
        fd.close();

        // Before committing read out old value
        fd = db.open(FILE_READ);
        val = fd.readString();
        fd.close();

        db.commit();
        digitalWrite(LEDSD, LOW);

        print("Read string: (");
        print(val);
        println(")");
        break;

      case 'r':
        println("Reading file");
        digitalWrite(LEDSD, HIGH);
        fd = db.open(FILE_READ);
        val = fd.readString();
        fd.close();
        db.abort();
        digitalWrite(LEDSD, LOW);

        print("Read string: (");
        print(val);
        println(")");
        break;

      case 'e':
        println("Erasing file");
        SD.remove("/passwd.db");
        break;

      case 'c':
        println("Corrupting file");
        digitalWrite(LEDSerial, HIGH);
        val = Serial.readStringUntil('\n');
        digitalWrite(LEDSerial, LOW);
        print("Input string: (");
        print(val);
        println(")");

        digitalWrite(LEDSD, HIGH);
        fd = db.open(FILE_WRITE);
        fd.seek(0);
        fd.print(val);
        fd.close();
        digitalWrite(LEDSD, LOW);
        break;

      case '\r':
      case '\n':
        // Ignore extra newlines
        break;

      default:
        println("Unknown entry");
        break;
    };
  }
}
