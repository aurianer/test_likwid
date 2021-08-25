#include <string>
#include <type_traits>
#include <vector>
#include <fstream>
#include <cmath>
#include <experimental/simd>
#include <random>
#include <iostream>

// header for likwid marker API
#include "likwid.h"

template <typename Iter>
bool is_aligned(Iter const& it)
{
    typedef typename std::iterator_traits<Iter>::value_type value_type;
    return (reinterpret_cast<std::uintptr_t>(std::addressof(*it)) &
                (std::experimental::memory_alignment_v<std::experimental::native_simd<value_type>> - 1)) == 0;
}

template <typename Iter>
void test(Iter first, Iter last)
{
    while (!is_aligned(first) && first != last)
    {
        auto temp = *first;
        for (int i = 0; i < 100; i++)
        {
            temp = std::sin(temp);
        }
        *first = temp;
        first++;
    }

    typedef typename std::iterator_traits<Iter>::value_type value_type;
    std::experimental::native_simd<value_type> tmp;
    std::size_t size = tmp.size();
    Iter const lastV = last - (size + 1);

    while (first < lastV)
    {
        tmp.copy_from(std::addressof(*first), std::experimental::vector_aligned);
        for (int i = 0; i < 100; i++)
        {
            tmp = std::experimental::sin(tmp);
        }
        tmp.copy_to(std::addressof(*first), std::experimental::vector_aligned);
        std::advance(first, size);
    }

    while (first != last)
    {
        auto temp = *first;
        for (int i = 0; i < 100; i++)
        {
            temp = std::sin(temp);
        }
        *first = temp;
        first++;
    }
}

template <typename Iter>
void test_seq(Iter first, Iter last)
{
    while (first != last)
    {
        auto temp = *first;
        for (int i = 0; i < 100; i++)
        {
            temp = std::sin(temp);
        }
        *first = temp;
        first++;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Arguments expected : number_elements" << std::endl;
        return 1;
    }
    int number_elements = std::stoi(argv[1]);

    // first, initialize marker in the serial region
    LIKWID_MARKER_INIT;

    std::vector<double> source(number_elements, 3);
    std::vector<double> source2(number_elements, 3);
    LIKWID_MARKER_THREADINIT;

    // start of interesting code region
    LIKWID_MARKER_START("simd-kernel");
        test(source.begin(), source.end());
    LIKWID_MARKER_STOP("simd-kernel");
    for (int i = 0; i < number_elements; i++)
    {
        if (source[i] > 100)
            std::cout << source[i] << "\t";
    }
    std::cout << "\n";

    LIKWID_MARKER_START("seq-kernel");
        test_seq(source2.begin(), source2.end());
    LIKWID_MARKER_STOP("seq-kernel");
    for (int i = 0; i < number_elements; i++)
    {
        if (source2[i] > 100)
            std::cout << source2[i] << "\t";
    }
    std::cout << "\n";
    LIKWID_MARKER_CLOSE;

    return 0;
}
