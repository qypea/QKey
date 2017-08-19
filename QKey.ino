#include <SPI.h>
#include <SD.h>
#include <avr/wdt.h>

#include <AtomicFile.h>
#include <SpritzCipher.h>

#include "PasswordDB.h"
#include "BasicUI.h"

const int LEDSerial = 13;
const int LEDSD = 8;

static struct PasswordHeader header;
static struct PasswordRecord record;
static AtomicFile db;
static char search[PASSLEN];
static char pass[PASSLEN];

// Reset helper
static void reset() {
  // Clear key material
  int i;
  for (i=0; i<PASSLEN; i++) {
    masterKey[i] = 0;
    search[i] = 0;
    pass[i] = 0;
    temp[i] = 0;
  }

  // Reset after a time delay
  Serial.println(F("Resetting in 4s. Close serial ports"));
  digitalWrite(LEDSerial, LOW);
  delay(4*1000);
  wdt_enable(WDTO_60MS);
  while(1){};
}

// Random generator
unsigned char randomChar() {
  return random(256); // TODO: Stronger entropy!
}

void setup() {
  pinMode(LEDSerial, OUTPUT);
  pinMode(LEDSD, OUTPUT);

  // Seed RNG initially
  randomSeed(analogRead(0));

  // Open serial communications and wait for port to open:
  digitalWrite(LEDSerial, HIGH);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println(F("Serial connected"));
  Serial.setTimeout(30 * 1000);

  // Initialize keyboard input
  Keyboard.begin();

  // Open SD Card
  Serial.print(F("Initializing SD card..."));
  digitalWrite(LEDSD, HIGH);
  if (!SD.begin(4)) {
    digitalWrite(LEDSD, LOW);
    Serial.println(F("failed!"));
    reset();
    return;
  }
  Serial.println(F("done."));
  digitalWrite(LEDSD, LOW);

  readString(F("Password: "), (char*)masterKey);
  masterKeyLen = strlen((char*)masterKey);

  dumpDBHeader();
  dumpDB(NULL);

  digitalWrite(LEDSerial, LOW);
}

static void getDB() {
  digitalWrite(LEDSD, HIGH);
  db = AtomicFile(F("/passwd.db"), F("/passwd.bak"));
}

static void initDB() {
  header.recordCount = 0;
  header.fileCheck.randomize(TOKENEXT);

  // Erase the DB and write the new header
  getDB();
  db.erase();
  File fd = db.open(FILE_WRITE);
  fd.write((char*)&header, sizeof(header));
  fd.close();
  db.commit();
  digitalWrite(LEDSD, LOW);
}

static void dumpDBHeader() {
  getDB();
  File fd = db.open(FILE_READ);
  fd.read(&header, sizeof(header));
  fd.close();
  digitalWrite(LEDSD, LOW);

  Serial.print(F("Records: "));
  Serial.println(header.recordCount);
  Serial.print(F("File check: "));
  Serial.println(header.fileCheck.unwrap());
}

static void dumpRecordFull(const struct PasswordRecord & record) {
  Serial.print(F("Description: "));
  Serial.println(record.description);
  Serial.print(F("Username: "));
  Serial.println(record.username);
  Serial.print(F("Separator: "));
  Serial.println(record.separator);
  Serial.print(F("Password: "));
  Serial.println(record.password.unwrap());
}

static void dumpRecordShort(const struct PasswordRecord & record) {
  Serial.print(record.description);
  Serial.print(F(", "));
  Serial.print(record.username);
  Serial.print(F(", "));
  Serial.println(record.separator);
}

static void dumpDB(const char * filter) {
  getDB();
  File fd = db.open(FILE_READ);

  fd.read(&header, sizeof(header));

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
      Serial.print(F(": "));
      dumpRecordShort(record);
    }
  }

  fd.close();
  digitalWrite(LEDSD, LOW);
}

static void find() {
  readString(F("Search term: "), search);
  dumpDB(search);
}

static void addRecord() {
  readString(F("Description: "), record.description);
  readString(F("Username: "), record.username);
  record.separator = readChar(F("Separator(t,n): "), "tn");

  char method = readChar(F("Random or Manual password?(r/m) "), "rm");
  if (method == 'r') {
    char allowed = readChar(F("Allowed Chars(b,e): "), "be");
    if (allowed == 'b') {
      record.password.randomize(TOKENBASE);
    } else if (allowed == 'e') {
      record.password.randomize(TOKENEXT);
    } else {
      Serial.println(F("Unexpected allowed chars"));
      reset();
    }
  } else if (method == 'm') {
    readString(F("Password: "), pass);
    record.password.wrap((uint8_t*)pass, strlen(pass));
  } else {
    Serial.println(F("Unexpected password method"));
    reset();
  }

  Serial.print(F("Password: "));
  Serial.println(record.password.unwrap());

  if (!confirm()) {
    return;
  }

  getDB();

  // Read in current header
  File fd = db.open(FILE_READ);
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
  getDB();
  File fd = db.open(FILE_READ);
  fd.read(&header, sizeof(header));
  fd.close();
  db.abort();
  digitalWrite(LEDSD, LOW);

  // Read in user input as int
  Serial.print(F("Index: "));
  int target = Serial.parseInt();

  // Range check
  if (target >= header.recordCount) {
    Serial.println(F("Invalid record"));
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

  getDB();
  db.start();
  db.clear();

  // Read in current header
  File r_fd = db.open(FILE_READ);
  r_fd.read(&header, sizeof(header));

  // Open for write
  File w_fd = db.open(FILE_WRITE); // Write appends
  header.recordCount--;
  w_fd.write((char*)&header, sizeof(header));

  int i;
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
  getDB();
  File fd = db.open(FILE_READ);
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
  getDB();
  File fd = db.open(FILE_READ);
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
    Serial.println(F("Unknown separator"));
    return;
  }
  Keyboard.print(record.password.unwrap());
  Keyboard.print('\n');
}

// Main loop
void loop() {
  if (Serial.available()) {
    digitalWrite(LEDSerial, HIGH);
    Serial.println();

    char c = Serial.read();
    switch (c) {
      case 'h':
        Serial.println(F("Help"));
        Serial.println(F(" Find"));
        Serial.println(F(" Add"));
        Serial.println(F(" Delete(i)"));
        Serial.println(F(" Show(i)"));
        Serial.println(F(" Enter(i)"));
        Serial.println(F(" Init(db)"));
        Serial.println(F(" Print(db)"));
        Serial.println(F(" Quit"));
        break;

      case 'f': // Find, print matching records
        Serial.println(F("Find"));
        find();
        break;

      case 'a': // Add record
        Serial.println(F("Add"));
        addRecord();
        break;

      case 'd': // Delete record(by index)
        Serial.println(F("Delete(i)"));
        deleteRecord();
        break;

      case 's': // Show record including password (by index)
        Serial.println(F("Show(i)"));
        showRecord();
        break;

      case 'e': // Enter(ghost type)
        Serial.println(F("Enter(i)"));
        enterRecord();
        break;

      case 'i': // Init the file
        Serial.println(F("Init(db)"));
        if (confirm()) {
          initDB();
          dumpDBHeader();
        } else {
        }
        break;

      case 'p': // Print db
        Serial.println(F("Print(db)"));
        dumpDB(NULL);
        break;

      case 'q': // Quit
        Serial.println(F("Goodbyte"));
        reset();
        break;

      case '\r':
      case '\n':
        // Ignore extra newlines
        break;

      default:
        Serial.print(F("Unknown command: "));
        Serial.println(c);
        break;
    };

    digitalWrite(LEDSerial, LOW);
  }
}
