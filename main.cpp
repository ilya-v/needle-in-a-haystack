#include <cstdio>
#include <cstdlib>

int main(const int argc, const char *argv[]) {
    //void test_adler_hash();           test_adler_hash();
    //void test_hash_table_32bit();     test_hash_table_32bit();
    //void test_hash_table_builder();   test_hash_table_builder();
    //void test_matcher_banana();       test_matcher_banana();

    void match_files(const unsigned, const char*, const char*, const char*);
    match_files(
        argc > 1? strtoul(argv[1], nullptr, 0) : 50,
        argc > 2? argv[2] : "haystack.txt",
        argc > 3? argv[3] : "needle.txt",
        argc > 4? argv[4] : nullptr );
}

//-------------------------------------------------------------------------------------------------------------------//

#include <algorithm>
#include <cstdint>
#include <vector>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

#include "substring.h"

void match_files(const unsigned threshold, const char *haystack_fname, const char *needle_fname, const char *result_fname) {
    using input_t  = std::istream_iterator<uint8_t>;
    using output_t = std::ostream_iterator<uint8_t>;

    const auto              end_of_input = input_t();
    std::ifstream           haystack_f{ haystack_fname, std::ios::binary };
    std::ifstream           needle_f  { needle_fname,   std::ios::binary };
    std::ofstream           out_f;
    std::ostream         &  result_f  = (result_fname? out_f.open(result_fname, std::ios::binary), out_f : std::cout);
    std::vector<uint8_t>    needle    { input_t(needle_f >> std::noskipws), end_of_input };
    output_t                result{ result_f << std::noskipws };

    matcher_t               matcher { needle.data(), (unsigned)needle.size(), threshold };    
    unsigned                match_count = 0;   

    if (!haystack_f.is_open()) {
        fprintf(stderr, "Cannot open input file %s, bailing out\n", haystack_fname);
        return;
    }
    
    //result_f.flush();
    if (!result_f) {
        fprintf(stderr, "Cannot open output file %s, bailing out\n", result_fname? result_fname : "stdout");
        return;
    }

    for (input_t haystack(haystack_f >> std::noskipws);  haystack != end_of_input;  ++haystack)
        for (matcher.add_byte(*haystack);  matcher.is_common_substring_found();) {
            substring_info_t sub = matcher.pop_substring_info();

            const std::string header = 
                    "Common substring of length "   + std::to_string(sub.final_length) 
                +   " found at haystack position "  + std::to_string(sub.start_in_haystack)
                +   " and needle position "         + std::to_string(sub.start_in_needle) + ":\n\n<<";

            std::copy(header.begin(), header.end(), result);
            std::copy(  needle.begin() + sub.start_in_needle, 
                        needle.begin() + sub.start_in_needle + sub.final_length, 
                        result);

            const std::string footer = ">>\n\n";
            std::copy(footer.begin(), footer.end(), result);
            match_count++;
        }

    fprintf(stderr, "\n%u matches\n", match_count);
    if (!out_f.is_open())  {
        fprintf(stderr, "\nPress ENTER\n");
        (void)getc(stdin);
    }
}

