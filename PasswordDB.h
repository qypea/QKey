#ifndef _QKEY_PASSWORDDB_H_
#define _QKEY_PASSWORDDB_H_

#define PASSLEN 64

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

  void randomize() {
    // TODO: Entropy lib
    strcpy(password, "Random");
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
