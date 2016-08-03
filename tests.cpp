#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <cstring>
#include <vector>
#include <tuple>
#include <algorithm>
#include "adler32.h"

#define VERIFY(x) do { if (!(x)) bark("Condition failed", #x, __FILE__, __LINE__, __func__); } while(0)

void bark(const char * message, const char *condition, const char *file, const int line, const char *func) {
    fprintf(stderr, "\n%s: %s\n\tat %s:%d, %s\n", message, condition, file, line, func);
}

//-------------------------------------------------------------------------------------------------------------------//

adler_hash_t calculate_adler_hash(const uint8_t *data, const size_t size) {
    adler_hash_t hash;
    for (const uint8_t *p = data; p < data + size; p++)
        hash.add_byte(*p);
    return hash;
}

void test_adler_hash() {
    const unsigned
        total_len = 7 * 1024 * 1024,
        window = 10 * 1024;

    srand(0xAA);
    std::vector<uint8_t> data(total_len);
    std::generate(data.begin(), data.end(), [] { return (uint8_t)rand(); });

    const adler_hash_t final_hash = calculate_adler_hash(data.data() + total_len - window, window);

    adler_hash_t rolling = calculate_adler_hash(data.data(), window);
    for (const uint8_t *p = data.data(); p < data.data() + total_len - window; p++) {
        rolling.roll(p[window], p[0]);
    }

    VERIFY(final_hash == rolling);
}

#include "substring.h"

void test_hash_table_builder() {
    const size_t N = 1024 * 1024;
    std::vector<uint8_t> string;
    for (unsigned n = 0; string.size() < N; n++) {
        string.push_back(n);
        const std::vector<uint8_t> cpy = string;
        string.insert(string.end(), cpy.begin(), cpy.end());
    }
    printf("%d\n", (int)string.size());

    const size_t threshold = 80 * 1024;
    auto table = matcher_t::init_string_hashes(string.data(), string.size(), threshold);
    size_t tt = 0;
    for (const uint8_t *p = string.data(); p + threshold < string.data() + string.size(); p++) {
        const uint32_t hash = calculate_adler_hash(p, threshold).get_value();
        auto matching_indexes = table.equal_range(hash);
        VERIFY(matching_indexes.first != matching_indexes.second);
        auto i_found = std::find_if( matching_indexes.first, matching_indexes.second, 
            [&](auto i){ return i.second == (p - string.data()); } );
        VERIFY(i_found != matching_indexes.second);
        if (p  > tt + string.data()) {
            printf(".");
            tt += string.size() / 200;
        }
    }
    printf("\n");
}

#include <string>

void print(const substring_info_t sub, const char *haystack, const char *needle) {
    printf("H%2d N%2d L%2d %s %s\n", (int)sub.start_in_haystack, (int)sub.start_in_needle, (int)sub.final_length,
        std::string(needle + sub.start_in_needle, sub.final_length).c_str(),
        std::string(haystack + sub.start_in_haystack, sub.final_length).c_str());
}

template<typename Container>
void print_all(const Container &cont, const uint8_t *haystack, const uint8_t *needle) {
    printf("\n---{\n");
    std::for_each(cont.begin(), cont.end(), [&](const auto &sub) { print(sub, (const char*)haystack, (const char*)needle); });
    printf("\n}---\n");
}


std::vector<substring_info_t> trivial_search(const unsigned threshold,
    const uint8_t *haystack, const uint8_t *haystack_end, const uint8_t *needle, const uint8_t *needle_end) {

    std::vector<substring_info_t>  actual_subs;
    uint64_t haystack_pos = 0;

    for (uint8_t *pn = (uint8_t *)needle; pn < needle_end; pn++)
        for (uint8_t *qn = pn + threshold; qn < needle_end; qn++)
        for (auto ipos = std::search((const uint8_t *)haystack, haystack_end, pn, qn); ipos != haystack_end;
    ipos = std::search(ipos + 1, haystack_end, pn, qn)) {
        uint64_t start_in_haystack = &(*ipos) - (const uint8_t *)haystack;
        haystack_pos = (qn - pn) + start_in_haystack;
        actual_subs.emplace_back(substring_info_t(start_in_haystack, pn - (uint8_t *)needle, haystack_pos));
        actual_subs.back().finalize();
    }

    auto pt_included = [](unsigned x, unsigned a, unsigned b) { return x >= a && x <= b; };
    auto int_included = [&](unsigned x, unsigned y, unsigned a, unsigned b) { return pt_included(x, a, b) && pt_included(y, a, b); };

    for (auto s = actual_subs.begin(); s != actual_subs.end(); s++)
        for (auto q = s + 1; q != actual_subs.end(); q++) {
            unsigned
                sh1 = (unsigned)s->start_in_haystack,
                sn1 = s->start_in_needle,
                qh1 = (unsigned)q->start_in_haystack,
                qn1 = q->start_in_needle,
                sh2 = sh1 + s->final_length,
                sn2 = sn1 + s->final_length,
                qh2 = qh1 + q->final_length,
                qn2 = qn1 + q->final_length;

            if (int_included(sh1, sh2, qh1, qh2) && int_included(sn1, sn2, qn1, qn2) && (sh1 - sn1 == qh1 - qn1)) {
                actual_subs.erase(s);
                s = actual_subs.begin();
                q = s;
            }
            else if (int_included(qh1, qh2, sh1, sh2) && int_included(qn1, qn2, sn1, sn2) && (sh1 - sn1 == qh1 - qn1)) {
                actual_subs.erase(q);
                q = s;
            }
        }
    return actual_subs;
}


void test_matcher_banana() {
    const unsigned threshold = 2;
    const char
        needle[] = "banana",
        haystack[] = "b ban anan ana na a ba nax";
    const uint8_t
        *haystack_begin = (const uint8_t *)haystack,
        *haystack_end = haystack_begin + strlen(haystack) + 1,
        *needle_begin = (const uint8_t *)needle,
        *needle_end = needle_begin + strlen(needle) + 1;

    matcher_t M(needle_begin, needle_end - needle_begin, threshold);
    for (auto b = haystack_begin; b < haystack_end; b++)
        M.add_byte(*b);
    M.signal_end_of_data();

    std::vector<substring_info_t>  subs;
    for (; M.is_common_substring_found(); )
        subs.push_back(M.pop_substring_info());

    auto subs_less = [&](substring_info_t x, substring_info_t y)
    { return std::tie(x.start_in_haystack, x.start_in_needle) < std::tie(y.start_in_haystack, y.start_in_needle); };

    std::sort(subs.begin(), subs.end(), subs_less);
    //print_all(subs, haystack_begin, needle_begin);

    auto actual_subs = trivial_search(threshold, haystack_begin, haystack_end, needle_begin, needle_end);
    std::sort(actual_subs.begin(), actual_subs.end(), subs_less);
    //print_all(actual_subs, haystack_begin, needle_begin);

    auto compare_subs = [](const substring_info_t &s1, const substring_info_t &s2) {
        return std::tie(s1.start_in_haystack, s1.start_in_needle, s1.final_length)
            == std::tie(s2.start_in_haystack, s2.start_in_needle, s2.final_length);
    };

    VERIFY(subs.size() == actual_subs.size());
    auto iactual = actual_subs.begin();
    for (auto & sub : subs)
        VERIFY(compare_subs(sub, *iactual++));
}
