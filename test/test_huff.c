#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned char randomChar() {
    return rand() & 0xff;
}

#include "../huff.h"

int testCompressDecompress(const char * in, const size_t inLen)
{
    printf("input: %s\n", in);

    const size_t outLen = 256;
    unsigned char out[outLen];
    int ret = compress(in, inLen, out, outLen);
    if (ret != 0) {
        printf("Failed to compress: %d\n", ret);
        return 1;
    }

    printf("Output: ");
    for (size_t i=0; i<outLen; i++) {
        printf("%02x ", out[i]);
    }
    printf("\n");

    char dec[inLen+1];
    ret = decompress(out, outLen, dec, inLen);
    if (ret != 0) {
        printf("Failed to decompress: %d\n", ret);
        return 1;
    }
    dec[inLen] = 0;
    printf("Decompressed: ");
    printf("%s\n", dec);
    return 0;
}

#if 1
int main(int argc, char** argv) {
    int ret;
    if (argc == 2) {
        ret = testCompressDecompress(argv[1], strlen(argv[1]));
    } else {
        ret = testCompressDecompress(TokenChars, TOKENEXT);
    }
    if (ret != 0) {
        return 1;
    }
#else
int main() {
#endif

#if 1
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
#endif

    for (int i=0; i<TOKENEXT; i++) {
        char in = TokenChars[i];
        unsigned char com = 0;
        char out = '\0';

        printf("%c: ", in);

        int ret = compress(&in, 1, &com, 1);
        if (ret != 0) {
            printf("Failed to compress: %d\n", ret);
            return 1;
        }
        printf("0x%02x, ", com);

        ret = decompress(&com, 1, &out, 1);
        if (ret != 0) {
            printf("Failed to decompress: %d\n", ret);
            return 1;
        }
        printf("%c\n", out);
        if (in != out) {
            printf("Input and output don't match\n");
            return 1;
        }
    }

    return 0;
}
