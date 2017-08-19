#ifndef _QKEY_BASICUI_H_
#define _QKEY_BASICUI_H_

static void readString(const __FlashStringHelper* prompt, char buffer[PASSLEN]) {
  Serial.print(prompt);
  int read;
  do {
    read = Serial.readBytesUntil('\n', buffer, PASSLEN);
  } while (read == 0);
  buffer[read] = 0;
  Serial.println(buffer);
}

static char readChar(const __FlashStringHelper* prompt, const char * allowed) {
  Serial.print(prompt);
  String allowed_s(allowed);
  char r;

  do {
    while (!Serial.available()) {
      delay(100);
    }
    r = Serial.read();
  } while(allowed_s.indexOf(r) == -1);
  Serial.println(r);

  return r;
}

static bool confirm() {
  char c = readChar(F("Are you sure?(y,n): "), "yYnN");
  if (c == 'y' || c == 'Y') {
    Serial.println(F("Ok"));
    return true;
  } else if (c == 'n' || c == 'N') {
    Serial.println(F("Aborted"));
    return false;
  } else {
    Serial.println(F("Logic error"));
    return false;
  }
}

#endif
