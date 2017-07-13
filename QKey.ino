#include <SPI.h>
#include <SD.h>

#include <AtomicFile.h>
#include <AES.h>
#include "PasswordDB.h"

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
  Serial.setTimeout(30 * 1000);

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

  dumpDBHeader();

  digitalWrite(LEDSerial, LOW);
}

AtomicFile getDB() {
  digitalWrite(LEDSD, HIGH);
  AtomicFile db("/passwd.db", "/passwd.bak");
  return db;
}

void initDB() {
  struct PasswordHeader header;
  header.recordCount = 0;
  header.fileCheck.randomize();

  // Erase the DB and write the new header
  AtomicFile db = getDB();
  db.erase();
  File fd = db.open(FILE_WRITE);
  fd.write((char*)&header, sizeof(header));
  fd.close();
  db.commit();
  digitalWrite(LEDSD, LOW);
}

void dumpDBHeader() {
  AtomicFile db = getDB();
  File fd = db.open(FILE_READ);
  struct PasswordHeader header;
  fd.read(&header, sizeof(header));
  fd.close();
  digitalWrite(LEDSD, LOW);

  Serial.print("Records: ");
  Serial.println(header.recordCount);
  Serial.print("File check: ");
  Serial.println(header.fileCheck.unwrap());
}

void dumpRecordFull(const PasswordRecord & record) {
  Serial.print("Description: ");
  Serial.println(record.description);
  Serial.print("Username: ");
  Serial.println(record.username);
  Serial.print("Separator: ");
  Serial.println(record.separator);
  Serial.print("Password: ");
  Serial.println(record.password.unwrap());
}

void dumpRecordShort(const PasswordRecord & record) {
  Serial.print(record.description);
  Serial.print(", ");
  Serial.print(record.username);
  Serial.print(", ");
  Serial.println(record.separator);
}

void dumpDB() {
  AtomicFile db = getDB();
  File fd = db.open(FILE_READ);

  struct PasswordHeader header;
  fd.read(&header, sizeof(header));

  struct PasswordRecord record;
  int i;
  for (i = 0; i < header.recordCount; i++) {
    fd.read(&record, sizeof(record));
    Serial.print(i);
    Serial.print(": ");
    dumpRecordShort(record);
  }

  fd.close();
  digitalWrite(LEDSD, LOW);
}

void readString(char buffer[PASSLEN]) {
  int read;
  do {
    read = Serial.readBytesUntil('\n', buffer, PASSLEN);
  } while (read == 0);
  buffer[read] = 0;
}

void addRecord() {
  PasswordRecord record;

  Serial.print("Description: ");
  readString(record.description);
  Serial.println(record.description);

  Serial.print("Username: ");
  readString(record.username);
  Serial.println(record.username);

  Serial.print("Separator(t,n): ");
  do {
    record.separator = Serial.read();
  } while(record.separator != 't' && record.separator != 'n');
  Serial.println(record.separator);

  record.password.randomize();
  Serial.print("Password: ");
  Serial.println(record.password.unwrap());

  Serial.print("Is this ok?(y/n)");
  if (confirm()) {
    Serial.println("Ok");

    AtomicFile db = getDB();

    // Read in current header
    File fd = db.open(FILE_READ);
    struct PasswordHeader header;
    fd.read(&header, sizeof(header));
    fd.close();

    // Write record
    fd = db.open(FILE_WRITE); // Write appends
    fd.write((char*)&record, sizeof(record));

    // Write length +1
    header.recordCount++;
    fd.seek(0);
    fd.write((char*)&header, sizeof(header));

    // Commit it all
    fd.close();
    db.commit();
    digitalWrite(LEDSD, LOW);

  } else {
    Serial.println("Aborted");
  }
}

bool confirm() {
  while (true) {
    while (!Serial.available()) {
      delay(100);
    }
    char c = Serial.read();
    if (c == 'y' || c == 'Y') {
      return true;
    } else if (c == 'n' || c == 'N') {
      return false;
    }
  }
}

// Main loop
void loop() {
  if (Serial.available()) {
    digitalWrite(LEDSerial, HIGH);
    Serial.println();

    char c = Serial.read();
    switch (c) {
      case 'i': // Init the file
        Serial.println("Re-initializing file");
        Serial.print("Are you sure?(y/n)");
        if (confirm()) {
          Serial.println("Ok");
          initDB();
          dumpDBHeader();
        } else {
          Serial.println("Aborted");
        }
        break;

      case 'a': // Add record
        Serial.println("Adding a record");
        addRecord();
        break;

      case 'p': // Print db listing
        Serial.println("DB contents:");
        dumpDB();
        break;

      // TODO: Delete record(by index)
      // TODO: Find, print matching records
      // TODO: Print record including password (by index)
      // TODO: Ghost type a record(by index)

      case '\r':
      case '\n':
        // Ignore extra newlines
        break;

      default:
        Serial.print("Unknown command: ");
        Serial.println(c);
        break;
    };

    digitalWrite(LEDSerial, LOW);
  }
}
