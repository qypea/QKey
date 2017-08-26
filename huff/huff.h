

static const char TokenChars[] =
  "0123456789" // 0..9
  "abcdefghijklmnopqrstuvwxyz"     // 10..35
  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"     // 36..61
  "~!@#$%^&*()-_=+[]{}|;:<>?,./";  // 62..89
#define TOKENBASE 62
#define TOKENEXT 90

static const unsigned char HuffValues[] = {
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
    0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
    0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x4c, 0x4d,
    0x4e, 0x4f, 0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f, 0x60, 0x61,
    0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6a, 0x6b,
    0x6c, 0x6d, 0x6e, 0x6f, 0x70, 0x71, 0x72, 0x73, 0x74, 0x75,
    0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f};

static const unsigned char HuffLengths[] = {
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7};

#define minLength 6
#define maxLength 7

size_t findIndex(char v) {
    // Find the offset in the tables
    int i;
    for (i=0; i<TOKENEXT; i++) {
        if (TokenChars[i] == v) {
            return i;
        }
    }
    return -1;
}

#define pushBits(v, len) do { \
    unsigned char val = v; \
    printf("push 0x%02x(%u) before=0x%04x(%lu)  ", val, len, temp, bitsUsed); \
    temp |= val << (16 - bitsUsed - len); \
    bitsUsed += len; \
    printf("after=0x%04x(%lu)\n", temp, bitsUsed); \
    } while(0)

#define peekBits(len) (temp >> (16 - len))

#define popBits(len) do { \
    printf("pop %u before=0x%04x(%lu)  ", len, temp, bitsUsed); \
    temp = temp & ((1<<(16 - len)) - 1); \
    temp = temp << len; \
    bitsUsed -= len; \
    printf("after=0x%04x(%lu)\n", temp, bitsUsed); \
    } while (0)


int compress(const char * in, size_t inLength,
              unsigned char * out, size_t outLength) {
    unsigned int temp = 0x0000;
    size_t bitsUsed = 0;
    size_t inI = 0;
    size_t outI = 0;
    int i;

    while (outI < outLength) {
        if (inI < inLength) {
            // Find this char in the table
            i = findIndex(in[inI]);
            inI++;
            if (i == -1) {
                // Invalid char
                printf("Invalid char: %c\n", in[inI]);
                return -1;
            }

            // Add it to temp
            pushBits(HuffValues[i], HuffLengths[i]);
        } else {
            // Add a random byte
            //pushBits(randomChar(), 8);
            pushBits(0xff, 8);
        }

        // Pop byte if we're there
        while (bitsUsed >= 8) {
            out[outI] = peekBits(8);
            printf("peeked: 0x%02x(8)\n", out[outI]);
            popBits(8);
            outI++;
        }
    }

    // Buffers must be sized so we always finish in
    if (inI < inLength) {
        return -2;
    }
    return 0;
}

int decompress(const unsigned char * in, size_t inLength,
               char * out, size_t outLength) {
    unsigned int temp = 0x0000;
    size_t bitsUsed = 0;
    size_t inI = 0;
    size_t outI = 0;
    size_t i;

    while (outI < outLength && inI < inLength) {
        // Add bits if needed
        while (bitsUsed < maxLength && inI < inLength) {
            pushBits(in[inI], 8);
            inI++;
        }

        // Search for the pattern we have
        for (i=0; i<TOKENEXT; i++) {
            unsigned char pattern = peekBits(HuffLengths[i]);
            printf("peeked: 0x%02x(%u)\n", pattern, HuffLengths[i]);
            if (pattern == HuffValues[i]) {
                popBits(HuffLengths[i]);
                out[outI] = TokenChars[i];
                outI++;
                break;
            }
        }
        if (i == TOKENEXT) {
            printf("Can't find pattern: 0x%02x\n", temp);
            out[outI] = '\0';
            printf("String so far: %s\n", out);
            return -1;
        }
    }

    // Buffers must be sized so we don't run past the end of in
    if (inI > inLength) {
        return -2;
    }
    return 0;
}
