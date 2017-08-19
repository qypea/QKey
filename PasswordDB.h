#ifndef _QKEY_PASSWORDDB_H_
#define _QKEY_PASSWORDDB_H_

#define PASSLEN 64

unsigned char randomChar();
uint8_t masterKey[PASSLEN] = "unset";
size_t masterKeyLen = 5;

static uint8_t temp[PASSLEN];

static const char TokenChars[] =
  "0123456789" // 0..9
  "abcdefghijklmnopqrstuvwxyz"     // 10..35
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"     // 36..61
  "~!@#$%^&*()-_=+[]{}|;:<>?,./ "; // 62..90
#define TOKENBASE 62
#define TOKENEXT 91

struct Token {
  uint8_t cipherText[PASSLEN];
  uint8_t cipherLength;
  uint8_t nonce[PASSLEN];

  size_t unwrap(uint8_t * buffer, size_t bufferLen) const {
    spritz_ctx ctx;
    spritz_setup_withIV(&ctx,
                        masterKey, masterKeyLen,
                        nonce, PASSLEN);
    size_t len = min(cipherLength, bufferLen);
    spritz_crypt(&ctx, cipherText, len, buffer);
    return len;
  }

  String unwrap() const {
    size_t len = unwrap(temp, PASSLEN);
    temp[len] = '\0';
    return String((char*)temp);
  }

  void randomNonce() {
    int i;
    for (i=0; i<PASSLEN; i++) {
      nonce[i] = randomChar();
    }
  }

  void wrap(const uint8_t * buffer, size_t bufferLen) {
    randomNonce();
    spritz_ctx ctx;
    spritz_setup_withIV(&ctx,
                        masterKey, masterKeyLen,
                        nonce, PASSLEN);
    spritz_crypt(&ctx, buffer, bufferLen, cipherText);
    cipherLength = bufferLen;
  }

  void randomize(unsigned int chars) {
    const int length = 16; // All passwords are 16B
    int i;
    for (i=0; i<length; i++) {
      unsigned char c = randomChar();
      temp[i] = TokenChars[c % chars];
    }

    wrap(temp, length);
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
