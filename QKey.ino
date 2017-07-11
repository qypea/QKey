#include <SPI.h>
#include <SD.h>

#include <AtomicFile.h>
#include <AES.h>

const int LEDSerial = 13;
const int LEDSD = 8;

// Reset helper
void(*reset) (void) = 0;

void setup() {
  pinMode(LEDSerial, OUTPUT);
  pinMode(LEDSD, OUTPUT);

  // Open serial communications and wait for port to open:
  digitalWrite(LEDSerial, HIGH);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Serial connected");

  // Open SD Card
  Serial.print("Initializing SD card...");
  digitalWrite(LEDSD, HIGH);
  if (!SD.begin(4)) {
    digitalWrite(LEDSD, LOW);
    Serial.println("failed! Resetting in 5s");
    delay(5*1000);
    reset();
    return;
  }
  Serial.println("done.");
  digitalWrite(LEDSD, LOW);

  digitalWrite(LEDSerial, LOW);
}

// Main loop
void loop() {
  String val;
  File fd;

  if (Serial.available()) {
    digitalWrite(LEDSerial, HIGH);
    digitalWrite(LEDSD, HIGH);
    AtomicFile db("/passwd.db", "/passwd.bak");
    digitalWrite(LEDSD, LOW);

    char c = Serial.read();
    switch (c) {
      case 'w':
        Serial.println("Writing file");
        val = Serial.readStringUntil('\n');
        Serial.print("Input string: (");
        Serial.print(val);
        Serial.println(")");

        digitalWrite(LEDSD, HIGH);
        fd = db.open(FILE_WRITE);
        fd.seek(0);
        fd.print(val);
        fd.close();
        db.commit();
        digitalWrite(LEDSD, LOW);
        break;

      case 'x':
        Serial.println("RWing file");
        val = Serial.readStringUntil('\n');
        Serial.print("Input string: (");
        Serial.print(val);
        Serial.println(")");

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

        Serial.print("Read string: (");
        Serial.print(val);
        Serial.println(")");
        break;

      case 'r':
        Serial.println("Reading file");
        digitalWrite(LEDSD, HIGH);
        fd = db.open(FILE_READ);
        val = fd.readString();
        fd.close();
        db.abort();
        digitalWrite(LEDSD, LOW);

        Serial.print("Read string: (");
        Serial.print(val);
        Serial.println(")");
        break;

      case 'e':
        Serial.println("Erasing file");
        SD.remove("/passwd.db");
        break;

      case 'c':
        Serial.println("Corrupting file");
        val = Serial.readStringUntil('\n');
        Serial.print("Input string: (");
        Serial.print(val);
        Serial.println(")");

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
        Serial.println("Unknown entry");
        break;
    };

    digitalWrite(LEDSerial, LOW);
  }
}
