#include <SPI.h>
#include <SD.h>
#include <avr/wdt.h>

#include <SpritzCipher.h>

#include "PasswordDB.h"

const int LEDSerial = 13;
const int LEDSD = 8;
const int buttonPin = 10;
const int groundPin = 12;

#include "BasicUI.h"

static struct PasswordHeader header;
static struct PasswordRecord record;
static char search[PASSLEN];
static char pass[PASSLEN];
static File fd;
static const char fname[] = "/passwd.db";

static void freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  v = (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
  Serial.print("Free ram: ");
  Serial.println(v);
}

// Reset helper
void reset() {
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

spritz_ctx rng;
const uint8_t rng_init[] = __DATE__ "T" __TIME__;
unsigned long time;

// Random generator
void randomInit() {
  spritz_setup(&rng, rng_init, sizeof(rng_init));

  // TODO: Connect a pin to more random signal
  uint8_t val;
  int i;
  for (i=0; i<4; i++) {
    val = analogRead(i);
    spritz_add_entropy(&rng, &val, 1);
  }

  // Note: this only makes sense because it is after the serial connect
  // Otherwise it would not provide any entropy
  time = millis();
  spritz_add_entropy(&rng, (uint8_t*)&time, sizeof(time));
}

unsigned char randomChar() {
  return spritz_random8(&rng);
}

void setup() {
  pinMode(LEDSerial, OUTPUT);
  pinMode(LEDSD, OUTPUT);
  pinMode(buttonPin, INPUT);
  digitalWrite(buttonPin, HIGH);
  pinMode(groundPin, OUTPUT);
  digitalWrite(groundPin, LOW);

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

  if (readPassword(F("Password: "), (char*)masterKey) <= 0) {
    reset();
  }
  masterKeyLen = strlen((char*)masterKey);

  randomInit();

  if (!SD.exists(fname)) {
    initDB();
  }

  dumpDBHeader();
  digitalWrite(LEDSerial, LOW);

  if (!confirm()) {
    reset();
  }

  Serial.print('>');
}

static void initDB() {
  header.recordCount = 0;
  header.fileCheck.randomize();

  // Erase the DB and write the new header
  digitalWrite(LEDSD, HIGH);
  SD.remove(fname);
  fd = SD.open(fname, FILE_WRITE);
  fd.write((char*)&header, sizeof(header));
  fd.close();
  digitalWrite(LEDSD, LOW);
}

static void dumpDBHeader() {
  digitalWrite(LEDSD, HIGH);
  fd = SD.open(fname, FILE_READ);
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
  digitalWrite(LEDSD, HIGH);
  fd = SD.open(fname, FILE_READ);

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
  if (readString(F("Search term: "), search) > 0) {
    dumpDB(search);
  }
}

static void addRecord() {
  if (readString(F("Description: "), record.description) <= 0) {
    return;
  }
  if (readString(F("Username: "), record.username) <= 0) {
    return;
  }
  record.separator = readChar(F("Separator(t,n): "), "tn");

  char method = readChar(F("Random or Manual password?(r/m) "), "rm");
  if (method == 'r') {
    record.password.randomize();
  } else if (method == 'm') {
    if (readPassword(F("Password: "), pass) <= 0) {
      return;
    }
    record.password.wrap(pass, strlen(pass));
  } else {
    Serial.println(F("Unexpected password method"));
    reset();
  }

  Serial.print(F("Password: "));
  Serial.println(record.password.unwrap());

  if (!confirm()) {
    return;
  }

  // Read in current header
  digitalWrite(LEDSD, HIGH);
  fd = SD.open(fname, FILE_READ);
  fd.read(&header, sizeof(header));
  fd.close();

  // Write record
  fd = SD.open(fname, FILE_WRITE);
  fd.seek(sizeof(struct PasswordHeader)
          + (sizeof(record) * header.recordCount));
  fd.write((char*)&record, sizeof(record));

  // Write length +1
  header.recordCount++;
  fd.seek(0);
  fd.write((char*)&header, sizeof(header));

  // Commit it all
  fd.close();
  digitalWrite(LEDSD, LOW);

  Serial.print("Added as record: ");
  Serial.println(header.recordCount-1);
}

static int readRecordI() {
  // Read in current header
  digitalWrite(LEDSD, HIGH);
  fd = SD.open(fname, FILE_READ);
  fd.read(&header, sizeof(header));
  fd.close();
  digitalWrite(LEDSD, LOW);

  // Read in user input as int
  int target = readPosInt(F("Index: "));
  if (target < 0) {
    return -1;
  }

  // Range check
  if (target >= header.recordCount) {
    Serial.println(F("Invalid record"));
    return -1;
  }
  Serial.println(target);

  // Open, advance
  digitalWrite(LEDSD, HIGH);
  fd = SD.open(fname, FILE_READ);
  fd.seek(sizeof(struct PasswordHeader) + (sizeof(record) * target));

  // Read in, print record
  fd.read(&record, sizeof(record));
  dumpRecordShort(record);

  // Clean up
  fd.close();
  digitalWrite(LEDSD, LOW);

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

  digitalWrite(LEDSD, HIGH);
  // Read in header
  fd = SD.open(fname, FILE_READ);
  fd.read(&header, sizeof(header));
  fd.close();

  int i;
  for (i=target; i<header.recordCount; i++) {
    // Read in record+1
    fd = SD.open(fname, FILE_READ);
    fd.seek(sizeof(struct PasswordHeader) + (sizeof(record) * (i+1)));
    memset(&record, 0, sizeof(record)); // for if read is past end
    fd.read(&record, sizeof(record));
    fd.close();

    // Write out record
    fd = SD.open(fname, FILE_WRITE);
    fd.seek(sizeof(struct PasswordHeader) + (sizeof(record) * i));
    fd.write((char*)&record, sizeof(record));
    fd.close();
  }

  // Write out new header
  header.recordCount--;
  fd = SD.open(fname, FILE_WRITE);
  fd.seek(0);
  fd.write((char*)&header, sizeof(header));
  fd.close();
  digitalWrite(LEDSD, LOW);
}

static void showRecord() {
  int target = readRecordI();
  if (target < 0) {
    return;
  }
  if (!confirm()) {
    return;
  }

  // Open, advance
  digitalWrite(LEDSD, HIGH);
  fd = SD.open(fname, FILE_READ);
  fd.seek(sizeof(struct PasswordHeader) + (sizeof(record) * target));

  // Read in, print record
  fd.read(&record, sizeof(record));
  dumpRecordFull(record);

  // Clean up
  fd.close();
  digitalWrite(LEDSD, LOW);
}

static void enterRecord() {
  int target = readRecordI();
  if (target < 0) {
    return;
  }

  // Open, advance
  digitalWrite(LEDSD, HIGH);
  fd = SD.open(fname, FILE_READ);
  fd.seek(sizeof(struct PasswordHeader) + (sizeof(record) * target));

  // Read
  fd.read(&record, sizeof(record));

  // Clean up
  fd.close();
  digitalWrite(LEDSD, LOW);

  if (!confirm()) {
    return;
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
  int prompt = 1;
  if (Serial.available()) {
    digitalWrite(LEDSerial, HIGH);
    Serial.println();
    prompt = 1;

    char c = Serial.read();
    switch (c) {
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
        prompt = 0;
        // Ignore extra newlines
        break;

      default:
        Serial.print(F("Unknown command: "));
        Serial.println(c);
        // Fall through to help
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
    };

    digitalWrite(LEDSerial, LOW);
    if (prompt) {
      Serial.print('>');
    }
  }
}
