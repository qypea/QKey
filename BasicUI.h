#ifndef _QKEY_BASICUI_H_
#define _QKEY_BASICUI_H_

static void readStringSilent(const __FlashStringHelper* prompt,
                             char buffer[PASSLEN]) {
  Serial.print(prompt);
  int read;
  do {
    read = Serial.readBytesUntil('\r', buffer, PASSLEN);
  } while (read == 0);
  buffer[read] = 0;
}

static void readString(const __FlashStringHelper* prompt,
                       char buffer[PASSLEN]) {
  readStringSilent(prompt, buffer);
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
  bool blink = false;
  int count = 0;
  int previous = HIGH;

  Serial.println(F("Are you sure?(Button)"));

  while (count == 0 || previous == LOW) {
    if (digitalRead(buttonPin) == LOW) {
      count++;
      previous = LOW;
    } else {
      previous = HIGH;
    }

    if (blink && count == 0) {
      digitalWrite(LEDSerial, HIGH);
    } else {
      digitalWrite(LEDSerial, LOW);
    }
    blink = !blink;

    delay(100);
  }

  if (count > 10) {
    Serial.println(F("Aborted"));
    return false;
  } else {
    Serial.println(F("Ok"));
    return true;
  }
}

#endif
