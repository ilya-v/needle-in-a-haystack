#include "substring_discriminator.h"
#include <vector>
#include <algorithm>
#include <iterator>

void substring_discriminator_t :: add_haystack_byte(const uint8_t byte, const uint8_t *needle, const unsigned needle_size) {
    for (auto isub = substrings.begin(); isub != substrings.end(); )
        if ( !isub->can_grow(needle_size) || needle[isub->next_needle_pos()] != byte ) {  
            complete.push_back(static_cast<substring_info_t>(*isub));
            complete.back().finalize();
            isub = substrings.erase(isub);            
        } else
           ++isub;
}

void substring_discriminator_t :: finalize_all_substrings() {
    std::copy(substrings.begin(), substrings.end(), std::back_inserter(complete));
    substrings.clear();        
}