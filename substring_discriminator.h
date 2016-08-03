#ifndef _substring_discriminator__H
#define _substring_discriminator__H

#include <cstdint>
#include <unordered_set>
#include <deque>
#include <tuple>
#include <functional>

struct substring_info_t {
    std::reference_wrapper<const uint64_t> last_haystack_pos;

    uint64_t start_in_haystack = 0;
    unsigned start_in_needle = 0;
    unsigned final_length = 0;
    
    substring_info_t(const uint64_t start_in_haystack, const unsigned start_in_needle, const uint64_t & current_haystack_pos)
        : last_haystack_pos(current_haystack_pos), start_in_haystack(start_in_haystack), start_in_needle(start_in_needle)
    {}

    size_t   offset()                       const { return (size_t)(start_in_haystack - start_in_needle); }
    unsigned next_needle_pos()              const { return start_in_needle + (unsigned)(last_haystack_pos - start_in_haystack); }
    bool     can_grow(unsigned needle_size) const { return next_needle_pos() < needle_size; }    
    void     finalize()                           { final_length = (unsigned)(last_haystack_pos - start_in_haystack); }
    bool operator==(const substring_info_t &other) const { return next_needle_pos() == other.next_needle_pos(); }
};

namespace std {
    template <> struct hash<substring_info_t> {
        size_t operator()(const substring_info_t &t) const { return (size_t)t.offset(); };
    };
}

class substring_discriminator_t {
    std::unordered_set<substring_info_t>    substrings;
    std::deque<substring_info_t>            complete;
public:
    void add_haystack_byte(const uint8_t byte, const uint8_t *needle, const unsigned needle_size);
    void add_substring(const substring_info_t & substring) { 
        // auto t = 
        substrings.insert(substring);
        //printf("insert: %2d %2d %s\n", (int)substring.start_in_haystack, (int)substring.start_in_needle, t.second? "ok":"reject");

    }
    const std::deque<substring_info_t> & complete_substrings() const { return complete; }
          std::deque<substring_info_t> & complete_substrings()       { return complete; }
    void finalize_all_substrings();
};

#endif
