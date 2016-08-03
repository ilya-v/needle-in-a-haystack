#ifndef _adler32__H
#define _adler32__H

#include <cstdint>
#include <cstddef>
#include <tuple>

struct adler_hash_t {
    static const unsigned ADLER_PRIME = 65521;
    unsigned a = 1, b = 0, n = 0;

    void add_byte(const uint8_t byte) {
        a = (a + byte) % ADLER_PRIME;
        b = (b + a) % ADLER_PRIME;
        n++;
    }

    adler_hash_t & roll(const uint8_t byte_in, const uint8_t byte_out) {
        a = (a + byte_in + ADLER_PRIME - byte_out) % ADLER_PRIME;
        b = (b + a + ADLER_PRIME - (1u + n*byte_out) % ADLER_PRIME) % ADLER_PRIME;  // good enough for n < 2^23
        return *this;
    }

    uint32_t get_value() const {
        return (a << 16) | b;
    }
};

inline bool operator==(const adler_hash_t h1, const adler_hash_t h2) {
    return std::tie(h1.a, h1.b, h1.n) == std::tie(h2.a, h2.b, h2.n);
}

#endif
