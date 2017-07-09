/*
 * AtomicFile.h - Library to aid writing atomically to the SD card so that in
 * case of power loss or crash the file is protected. The file will always be
 * in the old state or the new state, never a half written state.
 * Created by David Winn<dwinn@qypea.com> 2017/07/09
 * Released into the public domain.
 */
#ifndef _ATOMICFILE_H_
#define _ATOMICFILE_H_

#include "Arduino.h"

// Atomic file transactions on the SD card
class AtomicFile {
  public:
    // Construct an atomic file with the specified backup
    AtomicFile(String filename, String backup);

    // Open the file for the given mode
    // Always start a transaction first
    File open(int mode);

    // Start and finish the atomic transaction
    void start();
    void abort();
    void commit();

  private:
    String fileCurrent;
    String fileBackup;
    bool active;
};

#endif
