#include <SPI.h>
#include <SD.h>

#include <AtomicFile.h>
#include <AES.h>
#include "PasswordDB.h"

const int LEDSerial = 13;
const int LEDSD = 8;

// Reset helper
static void(*reset) (void) = 0;

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

  // Initialize keyboard input
  Keyboard.begin();

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
  dumpDB(NULL);

  digitalWrite(LEDSerial, LOW);
}

static AtomicFile getDB() {
  digitalWrite(LEDSD, HIGH);
  AtomicFile db("/passwd.db", "/passwd.bak");
  return db;
}

static void initDB() {
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

static void dumpDBHeader() {
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

static void dumpRecordFull(const struct PasswordRecord & record) {
  Serial.print("Description: ");
  Serial.println(record.description);
  Serial.print("Username: ");
  Serial.println(record.username);
  Serial.print("Separator: ");
  Serial.println(record.separator);
  Serial.print("Password: ");
  Serial.println(record.password.unwrap());
}

static void dumpRecordShort(const struct PasswordRecord & record) {
  Serial.print(record.description);
  Serial.print(", ");
  Serial.print(record.username);
  Serial.print(", ");
  Serial.println(record.separator);
}

static void dumpDB(const char * filter) {
  AtomicFile db = getDB();
  File fd = db.open(FILE_READ);

  struct PasswordHeader header;
  fd.read(&header, sizeof(header));

  struct PasswordRecord record;
  int i;
  for (i = 0; i < header.recordCount; i++) {
    fd.read(&record, sizeof(record));

    // Determine if we should show this record
    bool show = true;
    if (filter != NULL) {
      String description = String(record.description);
      int loc = description.indexOf(filter);
      if (loc == -1) {
        show = false;
      }
    }

    if (show) {
      Serial.print(i);
      Serial.print(": ");
      dumpRecordShort(record);
    }
  }

  fd.close();
  digitalWrite(LEDSD, LOW);
}

static void readString(const char * prompt, char buffer[PASSLEN]) {
  Serial.print(prompt);
  int read;
  do {
    read = Serial.readBytesUntil('\n', buffer, PASSLEN);
  } while (read == 0);
  buffer[read] = 0;
  Serial.println(buffer);
}

static void find() {
  char search[PASSLEN];
  readString("Search term: ", search);
  dumpDB(search);
}

static void addRecord() {
  PasswordRecord record;
  readString("Description: ", record.description);
  readString("Username: ", record.username);

  Serial.print("Separator(t,n): ");
  do {
    record.separator = Serial.read();
  } while(record.separator != 't' && record.separator != 'n');
  Serial.println(record.separator);

  record.password.randomize();
  Serial.print("Password: ");
  Serial.println(record.password.unwrap());

  if (!confirm()) {
    return;
  }

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
}

static int readRecordI() {
  // Read in current header
  AtomicFile db = getDB();
  File fd = db.open(FILE_READ);
  struct PasswordHeader header;
  fd.read(&header, sizeof(header));
  fd.close();
  db.abort();
  digitalWrite(LEDSD, LOW);

  // Read in user input as int
  Serial.print("Index: ");
  int target = Serial.parseInt();

  // Range check
  if (target >= header.recordCount) {
    Serial.println("Invalid record");
    return -1;
  }

  Serial.println(target);
  return target;
}

static void deleteRecord() {
  int target = readRecordI();
  if (target < 0) {
    return;
  }

  if (!confirm()) {
    return;
  }

  AtomicFile db = getDB();
  db.start();
  db.clear();

  // Read in current header
  File r_fd = db.open(FILE_READ);
  struct PasswordHeader header;
  r_fd.read(&header, sizeof(header));

  // Open for write
  File w_fd = db.open(FILE_WRITE); // Write appends
  header.recordCount--;
  w_fd.write((char*)&header, sizeof(header));

  int i;
  struct PasswordRecord record;
  for (i=0; i<header.recordCount+1; i++) {
    r_fd.read(&record, sizeof(record));
    if (i != target) {
      w_fd.write((char*)&record, sizeof(record));
    }
  }

  // Commit it all
  r_fd.close();
  w_fd.close();
  db.commit();
  digitalWrite(LEDSD, LOW);
}

static void showRecord() {
  int target = readRecordI();
  if (target < 0) {
    return;
  }

  // Open, advance
  AtomicFile db = getDB();
  File fd = db.open(FILE_READ);
  struct PasswordRecord record;
  fd.seek(sizeof(struct PasswordHeader) + (sizeof(record) * target));

  // Read in, print record
  fd.read(&record, sizeof(record));
  dumpRecordFull(record);

  // Clean up
  fd.close();
  db.abort();
  digitalWrite(LEDSD, LOW);
}

static void enterRecord() {
  int target = readRecordI();
  if (target < 0) {
    return;
  }

  // Open, advance
  AtomicFile db = getDB();
  File fd = db.open(FILE_READ);
  struct PasswordRecord record;
  fd.seek(sizeof(struct PasswordHeader) + (sizeof(record) * target));

  // Read
  fd.read(&record, sizeof(record));

  // Clean up
  fd.close();
  db.abort();
  digitalWrite(LEDSD, LOW);

  if (!confirm()) {
    return;
  }

  // TODO: Replace this countdown with confirm on button
  int i;
  for (i=0; i<30; i++) {
    delay(500);
    digitalWrite(LEDSerial, LOW);
    delay(500);
    digitalWrite(LEDSerial, HIGH);
  }

  // Send record to keyboard
  Keyboard.print(record.username);
  if (record.separator == 't') {
    Keyboard.print('\t');
  } else if (record.separator == 'n') {
    Keyboard.print('\n');
  } else {
    Serial.println("Unknown separator");
    return;
  }
  Keyboard.print(record.password.unwrap());
  Keyboard.print('\n');
}

static bool confirm() {
  Serial.print("Are you sure?(y/n)");
  while (true) {
    while (!Serial.available()) {
      delay(100);
    }
    char c = Serial.read();
    if (c == 'y' || c == 'Y') {
      Serial.println("Ok");
      return true;
    } else if (c == 'n' || c == 'N') {
      Serial.println("Aborted");
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
      case 'h':
        Serial.println("Help");
        Serial.println(" Find");
        Serial.println(" Add");
        Serial.println(" Delete(i)");
        Serial.println(" Show(i)");
        Serial.println(" Enter(i)");
        Serial.println(" Init(db)");
        Serial.println(" Print(db)");
        Serial.println(" Quit");
        break;

      case 'f': // Find, print matching records
        Serial.println("Find");
        find();
        break;

      case 'a': // Add record
        Serial.println("Add");
        addRecord();
        break;

      case 'd': // Delete record(by index)
        Serial.println("Delete(i)");
        deleteRecord();
        break;

      case 's': // Show record including password (by index)
        Serial.println("Show(i)");
        showRecord();
        break;

      case 'e': // Enter(ghost type)
        Serial.println("Enter(i)");
        enterRecord();
        break;

      case 'i': // Init the file
        Serial.println("Init(db)");
        if (confirm()) {
          initDB();
          dumpDBHeader();
        } else {
        }
        break;

      case 'p': // Print db
        Serial.println("Print(db)");
        dumpDB(NULL);
        break;

      case 'q': // Quit
        Serial.println("Goodbyte");
        // TODO: Clear out password variables in ram
        reset();
        break;

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
