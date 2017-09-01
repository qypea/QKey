#pragma once

struct spritz_ctx {
};

void spritz_setup_withIV(spritz_ctx * ctx,
                         const uint8_t * masterKey, size_t masterKeyLen,
                         const uint8_t * nonce, size_t nonceLen) {
}

void spritz_crypt(spritz_ctx * ctx,
                  const uint8_t * in, size_t len,
                  uint8_t * out) {
    memcpy(out, in, len);
}

void spritz_state_memzero(spritz_ctx * ctx) {
}
