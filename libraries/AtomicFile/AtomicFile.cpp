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

extern void reset();

static void copyFile(const char* in, const char* out) {
  File FDin = SD.open(in, FILE_READ);
  if (!FDin) {
    Serial.println("Error opening file for copy input. Resetting");
    reset();
  }

  if (SD.exists((char*)out)) {
    SD.remove((char*)out);
  }

  File FDout = SD.open(out, FILE_WRITE);
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

AtomicFile::AtomicFile() {};

AtomicFile::AtomicFile(const char* filename, const char* backup) {
  strcpy(fileCurrent, filename);
  strcpy(fileBackup, backup);

  // Roll back any unfinished transaction
  if (SD.exists(fileBackup)) {
    active = true;
    abort();
  }

  touch();
  active = false;
}

void AtomicFile::touch() {
  if (!SD.exists(fileCurrent)) {
    File fd = SD.open(fileCurrent, FILE_WRITE);
    fd.write("");
    fd.close();
  }
}

File AtomicFile::open(int mode) {
  if (mode == FILE_READ) {
    if (active) {
      // In case of read during a transaction open the backup
      return SD.open(fileBackup, mode);
    } else {
      return SD.open(fileCurrent, mode);
    }
  } else {
    // In case of write start a transaction and then return the file
    if (!active) {
      start();
    }
    return SD.open(fileCurrent, mode);
  }
}

void AtomicFile::erase() {
  if (active) {
    abort();
  }

  SD.remove(fileCurrent);
  touch();
}

void AtomicFile::clear() {
  if (!active) {
    start();
  }

  SD.remove(fileCurrent);
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
  SD.remove(fileBackup);

  active = false;
}

void AtomicFile::commit() {
  if (!active) {
    return;
  }

  SD.remove(fileBackup);
  active = false;
}
