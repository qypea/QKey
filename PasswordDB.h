#ifndef _QKEY_PASSWORDDB_H_
#define _QKEY_PASSWORDDB_H_

unsigned char randomChar();
#include "huff.h"

#define PASSLEN 32
uint8_t masterKey[PASSLEN] = "unset";
size_t masterKeyLen = 5;

static uint8_t temp[PASSLEN];
static char tempStr[PASSLEN+1];

static spritz_ctx ctx;

struct Token {
  uint8_t cipherText[PASSLEN];
  uint8_t cipherLength;
  uint8_t nonce[PASSLEN];

  size_t unwrap(char * buffer, size_t bufferLen) const {
    spritz_setup_withIV(&ctx,
                        masterKey, masterKeyLen,
                        nonce, PASSLEN);
    spritz_crypt(&ctx, cipherText, PASSLEN, temp);
    spritz_state_memzero(&ctx);
    size_t len = min(bufferLen, (size_t)cipherLength);
    decompress(temp, PASSLEN, buffer, len);
    return len;
  }

  const char* unwrap() const {
    size_t len = unwrap(tempStr, PASSLEN);
    tempStr[len] = '\0';
    return tempStr;
  }

  void randomNonce() {
    int i;
    for (i=0; i<PASSLEN; i++) {
      nonce[i] = randomChar();
    }
  }

  void wrap(const char * buffer, size_t bufferLen) {
    randomNonce();
    compress(buffer, bufferLen, temp, PASSLEN);
    spritz_setup_withIV(&ctx,
                        masterKey, masterKeyLen,
                        nonce, PASSLEN);
    spritz_crypt(&ctx, temp, PASSLEN, cipherText);
    spritz_state_memzero(&ctx);
    cipherLength = bufferLen;
  }

  void randomize() {
    cipherLength = 16; // All passwords are 16B
    int i;
    for (i=0; i<PASSLEN; i++) {
      cipherText[i] = randomChar();
    }
  }

} __attribute__((packed));

struct PasswordRecord {
  char description[PASSLEN];
  char username[PASSLEN];
  char separator;
  struct Token password;
} __attribute__((packed));

struct PasswordHeader {
  uint8_t recordCount;
  struct Token fileCheck;
} __attribute__((packed));

#endif
