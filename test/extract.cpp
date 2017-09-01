#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <string>
#include <iostream>

#define String std::string
#define min std::min

unsigned char randomChar() {
    return rand() & 0xff;
}

#include "spritz_stub.hpp"
#include "../PasswordDB.h"

int main(int argc, const char** argv) {
    if (argc != 2) {
        printf("Error: No filename supplied\n");
        return 1;
    }
    printf("Database file %s\n", argv[1]);

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        printf("Error: Unable to open file\n");
        return 1;
    }

    printf("Password? ");
    std::string masterPassword;
    std::cin >> masterPassword;
    memcpy(masterKey, masterPassword.data(), masterPassword.size());
    masterKeyLen = masterPassword.size();

    struct PasswordHeader header;
    int stat = read(fd, &header, sizeof(header));
    if (stat < sizeof(header)) {
        printf("Error: Couldn't read header: %d\n", errno);
        return 2;
    }

    printf("Records: %d\n", header.recordCount);
    printf("File check: %s\n", header.fileCheck.unwrap().c_str());

    for (int i=0; i<header.recordCount; i++) {
        printf("Record %d\n", i);
        struct PasswordRecord record;
        stat = read(fd, &record, sizeof(record));
        if (stat < sizeof(record)) {
            printf("Error: Couldn't read record: %d\n", errno);
            return 2;
        }

        printf("  Description: %s\n", record.description);
        printf("  Username: %s\n", record.username);
        printf("  Separator: %c\n", record.separator);
        printf("  Password: %s\n", record.password.unwrap().c_str());
    }

    return 0;
}
