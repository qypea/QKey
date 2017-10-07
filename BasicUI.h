#ifndef _QKEY_BASICUI_H_
#define _QKEY_BASICUI_H_

static int readGeneric(const __FlashStringHelper* prompt,
                       char buffer[PASSLEN],
                       bool (*isValid)(int),
                       bool echo) {
  Serial.print(prompt);

  // Read loop
  int i;
  char c;
  for (i=0; i<PASSLEN; ) {
    while (!Serial.available()) {
      // Wait for more input
    }
    c = Serial.read();

    if (c == '\r') {
      // End of string, mark as done
      break;
    } else if (isValid(c)) {
      // Valid char
      buffer[i] = c;
      i++;

      if (echo) {
        Serial.print(c);
      }
    } else if (c == 0x8 && i > 0) {
      // Backspace
      i--;

      if (echo) {
        Serial.print(c);
        // Clear to end of line
        Serial.print((char)0x1b);
        Serial.print("[K");
      }
    } else {
      // Invalid char
      Serial.print("Invalid char: ");
      Serial.print(c, HEX);
      Serial.println();
      return -1;
    }
  }
  int len = i;

  // Ensure buffer is always null terminated, filled with 0
  while (i<PASSLEN) {
    buffer[i] = 0;
    i++;
  }
  buffer[PASSLEN-1] = 0;
  Serial.println();

  return len;
}

static int readString(const __FlashStringHelper* prompt,
                      char buffer[PASSLEN]) {
  return readGeneric(prompt, buffer, isPrintable, true);
}

static int readPassword(const __FlashStringHelper* prompt,
                        char buffer[PASSLEN]) {
  return readGeneric(prompt, buffer, isHuff, false);
}

static int readPosInt(const __FlashStringHelper* prompt) {
  char buffer[PASSLEN];
  int stat = readGeneric(prompt, buffer, isDigit, true);
  if (stat <= 0) {
    return -1;
  }

  int i = 0;
  unsigned int value = 0;
  while (buffer[i] != 0) {
    value = value * 10;
    value = value + buffer[i] - '0';
    i++;
  }
  return value;
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
      if (count > 10) {
        break;
      }
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
