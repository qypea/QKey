#ifndef _QKEY_PASSWORDDB_H_
#define _QKEY_PASSWORDDB_H_

#define PASSLEN 64

unsigned char randomChar();

static const char TokenChars[] =
  "0123456789" // 0..9
  "abcdefghijklmnopqrstuvwxyz"     // 10..35
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"     // 36..61
  "~!@#$%^&*()-_=+[]{}|;:<>?,./ "; // 62..90
#define TOKENBASE 62
#define TOKENEXT 91

struct Token {
  char password[PASSLEN];

  String unwrap() const {
    // TODO: Decrypt
    return String(password);
  }

  void wrap(String pass) {
    // TODO: Encrypt
    strcpy(password, pass.c_str());
  }

  void randomize(unsigned int chars) {
    const int length = 16; // All passwords are 16B
    int i;
    for (i=0; i<length; i++) {
      unsigned char c = randomChar();
      password[i] = TokenChars[c % chars];
    }
    password[length] = '\0';
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
