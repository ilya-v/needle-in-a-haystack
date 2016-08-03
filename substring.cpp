#include <deque>
#include "adler32.h"
#include "substring.h"


hash_table_t matcher_t::init_string_hashes(const uint8_t *string, const size_t size, const size_t threshold) {
    hash_table_t table;
    if (threshold > size) 
        return table;
    const unsigned n_buckets_magic = 2 * 1024 * 1024;
    table.rehash(n_buckets_magic);

    adler_hash_t hash;
    for (const uint8_t *p = string; p < string + threshold; p++)
        hash.add_byte(*p);

    table.emplace(hash.get_value(), 0u);
    for(const uint8_t *p = string; p + threshold < string + size; p++)
        table.emplace( hash.roll(p[threshold], p[0]).get_value(), p - string + 1 );
     return table;
};

void matcher_t :: add_byte(const uint8_t byte) {
    substrings.add_haystack_byte(byte, needle, needle_size);
    roll_data(byte);
    haystack_pos++;

    if (haystack_chunk.size() == threshold)
        for (auto & substring : get_new_substring_seeds())
            substrings.add_substring(substring);

}

void  matcher_t:: roll_data(const uint8_t byte) {
    haystack_chunk.push_back(byte);
    if (haystack_chunk.size() <= threshold)
        hash.add_byte(byte);
    else {
        hash.roll(byte, haystack_chunk.front());
        haystack_chunk.pop_front();
    }
}

std::vector<substring_info_t>  matcher_t :: get_new_substring_seeds() {
    static std::vector<substring_info_t> results;
    results.clear();

    auto candidate_positions = needle_hash_table.equal_range(hash.get_value());
    for (auto ipos = candidate_positions.first; ipos != candidate_positions.second; ++ipos) {
        const unsigned candidate_pos = ipos->second;
        if (std::equal( haystack_chunk.begin(), haystack_chunk.end(), 
                        needle + candidate_pos, needle + candidate_pos + threshold))
            results.push_back(substring_info_t{haystack_pos - threshold, candidate_pos, haystack_pos});
    }
    return results;
}

substring_info_t matcher_t::pop_substring_info() {
    const auto res = substrings.complete_substrings().front();
    substrings.complete_substrings().pop_front();
    return res;
}


