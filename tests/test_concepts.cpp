#include <bucket/bucket.hpp>
#include <array>
#include <vector>
#include <span>
#include <valarray>
#include <list>
#include <cstdint>

using namespace bucketlib;


/*
A TEST CASE FOR THE DIFFERENT CONCEPTS WE INTRODUCED IN THE MAIN LIBRARY.

*/

// ----------------------------------------------
// Dummy types for invalid cases
struct BadType {};
struct StructWithDataButNoRandomAccess {
  using value_type = int;
  int* data() const;
  std::size_t size() const;
};


// CONSTRUCTION
// ----------------------------------------------
static_assert(std::is_constructible_v<
    bucket<std::vector<double>>,
    std::size_t, std::size_t, const std::vector<double>&
>);
static_assert(std::is_constructible_v<
    bucket<std::vector<float>>,
    std::size_t, std::size_t, const std::vector<float>&
>);
static_assert(std::is_constructible_v<
    bucket<std::vector<int>>,
    std::size_t, std::size_t, const std::vector<int>&
>);
static_assert(std::is_constructible_v<
    bucket<std::span<double>>,
    std::size_t, std::size_t, const std::span<double>&
>);
static_assert(std::is_constructible_v<
    bucket<std::span<float>>,
    std::size_t, std::size_t, const std::span<float>&
>);
static_assert(std::is_constructible_v<
    bucket<std::span<int>>,
    std::size_t, std::size_t, const std::span<int>&
>);
static_assert(std::is_constructible_v<
    bucket<std::array<double, 10>>,
    std::size_t, std::size_t, const std::array<double, 10>&
>);
static_assert(std::is_constructible_v<
    bucket<std::array<float, 10>>,
    std::size_t, std::size_t, const std::array<float, 10>&
>);
static_assert(std::is_constructible_v<
    bucket<std::array<int, 10>>,
    std::size_t, std::size_t, const std::array<int, 10>&
>);

// More tests about the numeric types


static_assert(Numeric<int>);
static_assert(Numeric<float>);
static_assert(Numeric<double>);
static_assert(!Numeric<bool>);
static_assert(!Numeric<char>);
static_assert(!Numeric<wchar_t>);
static_assert(!Numeric<BadType>);

// ----------------------------------------------
// Valid RandomAccessContainers (if generalized later)
static_assert(RandomAccessContainer<std::vector<double>>);

// If you go back to a generalized concept, these will be used again:

static_assert(RandomAccessContainer<std::array<float, 4>>);
static_assert(RandomAccessContainer<std::span<double>>); // C++20
static_assert(!RandomAccessContainer<std::valarray<double>>); // no iterators
static_assert(!RandomAccessContainer<std::list<double>>);     // not random-access
static_assert(!RandomAccessContainer<StructWithDataButNoRandomAccess>);

// ----------------------------------------------
// NRAContainer: combination
static_assert(NRAContainer<std::vector<double>>);
static_assert(NRAContainer<std::vector<float>>);
static_assert(NRAContainer<std::vector<int>>);
static_assert(NRAContainer<std::vector<uint16_t>>);
static_assert(!NRAContainer<std::vector<bool>>);
static_assert(!NRAContainer<std::vector<char>>);
static_assert(!NRAContainer<std::vector<wchar_t>>);
static_assert(!NRAContainer<std::vector<char16_t>>);
static_assert(!NRAContainer<std::vector<char32_t>>);
static_assert(!NRAContainer<std::vector<char8_t>>);
static_assert(!NRAContainer<std::vector<BadType>>);



// ----------------------------------------------
// Type-based compile-time construction
static_assert(std::same_as<typename bucket<std::vector<double>>::value_type, double>);

int main() {}
