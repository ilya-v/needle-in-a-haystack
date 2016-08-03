#ifndef _substring__H
#define _substring__H

#include <cstdint>
#include <vector>
#include <deque>
#include <unordered_map>
#include "adler32.h"
#include "substring_discriminator.h"


struct hasher_t { std::size_t operator()(const uint32_t& k) const { return k; } };
using hash_table_t = std::unordered_multimap<uint32_t, unsigned, hasher_t>;


class matcher_t {
    const unsigned              threshold;
    const unsigned              needle_size;
    const uint8_t              *needle;
    const hash_table_t          needle_hash_table;

    uint64_t                    haystack_pos = 0;
    std::deque<uint8_t>         haystack_chunk;
    adler_hash_t                hash;
    substring_discriminator_t   substrings;

    void roll_data(const uint8_t byte);
    std::vector<substring_info_t>  get_new_substring_seeds();

public:
    matcher_t(const uint8_t *needle, const unsigned needle_size, const unsigned threshold)
        : needle(needle), threshold{ threshold }, needle_size{ needle_size },
        needle_hash_table{ init_string_hashes(needle, needle_size, threshold) }
    {}

    void add_byte(const uint8_t byte);
    void signal_end_of_data()       { substrings.finalize_all_substrings(); }

    bool                            is_common_substring_found() const { return !substrings.complete_substrings().empty(); }
    substring_info_t                pop_substring_info();
    uint64_t                        get_current_haystack_pos()  const { return haystack_pos; }

    // made public for testing only
    static hash_table_t init_string_hashes(const uint8_t *string, const size_t size, const size_t threshold);
};

#endif

