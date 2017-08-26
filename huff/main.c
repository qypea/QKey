#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char randomChar() {
    return rand() & 0xff;
}

#include "huff.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        return 1;
    }

    {
        const char * in = argv[1];
        const size_t inLen = strlen(in);
        printf("input: %s\n", in);

        printf("Output: ");
        const size_t outLen = 256;
        unsigned char out[outLen];
        int ret = compress(in, inLen, out, outLen);
        if (ret != 0) {
            printf("Failed to compress: %d\n", ret);
            return 1;
        }

        for (size_t i=0; i<outLen; i++) {
            printf("%02x ", out[i]);
        }
        printf("\n");

        printf("Decompressed: ");
        char dec[inLen+1];
        ret =decompress(out, outLen, dec, inLen);
        if (ret != 0) {
            printf("Failed to decompress: %d\n", ret);
            return 1;
        }
        dec[inLen] = 0;
        printf("%s\n", dec);
    }

    {
        for (int i=0; i<256; i++) {
            unsigned char in = i;
            char out = '\0';
            int ret = decompress(&in, 1, &out, 1);
            if (ret != 0) {
                printf("Failed to decompress: %d\n", ret);
                return 1;
            }
            printf("%d: (%c)\n", i, out);
        }
    }

    return 0;
}
