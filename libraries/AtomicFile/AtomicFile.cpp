/*
 * AtomicFile.cpp - Library to aid writing atomically to the SD card so that in
 * case of power loss or crash the file is protected. The file will always be
 * in the old state or the new state, never a half written state.
 * Created by David Winn<dwinn@qypea.com> 2017/07/09
 * Released into the public domain.
 */

#include "Arduino.h"

#include <SPI.h>
#include <SD.h>

#include "AtomicFile.h"


// Reset helper
static void(*reset) (void) = 0;

static void copyFile(const String in, const String out) {
  File FDin = SD.open(in.c_str(), FILE_READ);
  if (!FDin) {
    Serial.println("Error opening file for copy input. Resetting");
    reset();
  }

  if (SD.exists((char*)out.c_str())) {
    SD.remove((char*)out.c_str());
  }

  File FDout = SD.open(out.c_str(), FILE_WRITE);
  if (!FDout) {
    Serial.println("Error opening file for copy output. Resetting");
    reset();
  }

  while (FDin.available()) {
    FDout.write(FDin.read());
  }
  FDout.close();
  FDin.close();
}

AtomicFile::AtomicFile(const String filename, const String backup) {
  fileCurrent = filename;
  fileBackup = backup;

  // Roll back any unfinished transaction
  if (SD.exists((char*)fileBackup.c_str())) {
    active = true;
    abort();
  }

  touch();
  active = false;
}

void AtomicFile::touch() {
  if (!SD.exists((char*)fileCurrent.c_str())) {
    File fd = SD.open(fileCurrent.c_str(), FILE_WRITE);
    fd.write("");
    fd.close();
  }
}

File AtomicFile::open(int mode) {
  if (mode == FILE_READ) {
    if (active) {
      // In case of read during a transaction open the backup
      return SD.open(fileBackup.c_str(), mode);
    } else {
      return SD.open(fileCurrent.c_str(), mode);
    }
  } else {
    // In case of write start a transaction and then return the file
    if (!active) {
      start();
    }
    return SD.open(fileCurrent.c_str(), mode);
  }
}

void AtomicFile::erase() {
  if (active) {
    abort();
  }

  SD.remove((char*)fileCurrent.c_str());
  touch();
}

void AtomicFile::start() {
  if (active) {
    abort();
  }

  copyFile(fileCurrent, fileBackup);
  active = true;
}

void AtomicFile::abort() {
  if (!active) {
    return;
  }
  copyFile(fileBackup, fileCurrent);
  SD.remove((char*)fileBackup.c_str());

  active = false;
}

void AtomicFile::commit() {
  if (!active) {
    return;
  }

  SD.remove((char*)fileBackup.c_str());
  active = false;
}
