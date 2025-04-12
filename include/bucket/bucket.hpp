#pragma once

#include <algorithm>
#include <cstddef>
#include <cassert>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include <span>
#include <array>

#ifdef ENABLE_CHECKS
#define ROW_CHECK(cond, msg)                                                   \
  if (!(cond))                                                                 \
    throw std::runtime_error(msg);
#define VAL_CHECK(cond, msg)                                                   \
  if (!(cond))                                                                 \
    throw std::runtime_error(msg);
#else
#define ROW_CHECK(cond, msg) static_cast<void>(0);
#define VAL_CHECK(cond, msg) static_cast<void>(0);
#endif

namespace bucketlib
{


template <typename Container>
struct is_supported_container : std::false_type {};

template <typename T>
struct is_supported_container<std::vector<T>> : std::true_type {};

template <typename T, std::size_t N>
struct is_supported_container<std::array<T, N>> : std::true_type {};

template <typename T>
struct is_supported_container<std::span<T>> : std::true_type {};

template <typename Container>
concept RandomAccessContainer = is_supported_container<Container>::value;


template <typename T>
concept ConvertibleToSizeT =
    std::is_integral_v<T> && std::is_convertible_v<T, std::size_t>;

template <typename T>
concept Numeric = std::is_arithmetic_v<T> && !std::is_same_v<T, bool> &&
                  !std::is_same_v<T, char> && !std::is_same_v<T, wchar_t> &&
                  !std::is_same_v<T, char8_t> && !std::is_same_v<T, char16_t> &&
                  !std::is_same_v<T, char32_t>;


// NRA stands for Numeric Random Access Container
template <typename Container>
concept NRAContainer =
    RandomAccessContainer<Container> && Numeric<typename Container::value_type>;

template <NRAContainer Container> class bucket
{
public:
  using value_type = typename Container::value_type;

private:
  mutable std::size_t _min_row_affected, _max_row_affected;
  std::size_t _ROWS;
  std::size_t _COLS;
  std::size_t _size;
  const Container &_vector;
  mutable std::vector<value_type> _p_sums;
  mutable std::vector<value_type> _p_cum_sums;

public:
  static constexpr std::size_t NOT_FOUND =
      std::numeric_limits<std::size_t>::max();

  explicit constexpr bucket(ConvertibleToSizeT auto ROWS,
                            ConvertibleToSizeT auto COLS,
                            const Container &other)
      : _ROWS(ROWS), _COLS(COLS), _vector(other)
  {

    _size = _ROWS * _COLS;
    assert(other.size() <= ROWS * COLS);
    _p_sums.resize(_ROWS);
    _p_cum_sums.resize(_ROWS + 1);
    update_sum();
    update_cumsum();
    _min_row_affected = _ROWS;
    _max_row_affected = 0;
  }

  //------- GETTERS -------//
  [[nodiscard]] std::size_t get_size() const noexcept { return _size; }

  [[nodiscard]] std::size_t get_rows() const noexcept { return _ROWS; }

  [[nodiscard]] std::size_t get_cols() const noexcept { return _COLS; }

  [[nodiscard]] std::size_t get_min_row_affected() const noexcept
  {
    return _min_row_affected;
  }

  [[nodiscard]] std::size_t get_max_row_affected() const noexcept
  {
    return _max_row_affected;
  }

  [[nodiscard]] const std::vector<value_type> &get_sums() const noexcept
  {
    return _p_sums;
  }

  [[nodiscard]] const std::vector<value_type> &get_cumsums() const noexcept
  {
    return _p_cum_sums;
  }

  void print() const noexcept
  {
    for (const value_type &i : _p_cum_sums)
      std::cout << i << ",";
    std::cout << std::endl;
  }

  void update_sum() const
  {
    for (std::size_t row = 0; row < _ROWS; row++)
      update_sum_at_row(row);
  }

  void update_sum_at_row(std::size_t row) const
  {
    ROW_CHECK(row < _ROWS, "Row index out of range");

    auto begin = _vector.begin() + row * _COLS;
    auto end = begin + _COLS;
    _p_sums[row] = std::accumulate(begin, end, static_cast<value_type>(0));

    if (row < _min_row_affected)
      _min_row_affected = row;
    if (row > _max_row_affected)
      _max_row_affected = row;
  }

  void update_cumsum() const
  {
    _p_cum_sums[0] = static_cast<value_type>(0);

    for (std::size_t row = 0; row < _ROWS; row++)
    {
      _p_cum_sums[row + 1] = _p_cum_sums[row] + _p_sums[row];
    }
    _min_row_affected = _ROWS;
    _max_row_affected = 0;
  }

  void refresh_cumsum() const
  {
    value_type diff = _p_cum_sums[_max_row_affected + 1];
    std::size_t l_row = _min_row_affected;
    for (; l_row < _max_row_affected + 1; l_row++)
    {
      _p_cum_sums[l_row + 1] = _p_cum_sums[l_row] + _p_sums[l_row];
    }
    diff -= _p_cum_sums[_max_row_affected + 1];

    for (; l_row < _ROWS; l_row++)
    {
      _p_cum_sums[l_row + 1] -= diff;
    }
    _min_row_affected = _ROWS;
    _max_row_affected = 0;
  }

  [[nodiscard]] bool is_valid_index(std::size_t index) const noexcept
  {
    return index != NOT_FOUND;
  }

  [[nodiscard]] std::size_t find_upper_bound(const value_type &val) const
  {
    VAL_CHECK(
        val > 0,
        "In upper limit, the value passed is smaller than the first element")
    VAL_CHECK(val < _p_cum_sums.back(), "In upper limit, the value passed is "
                                        "bigger or equal to the last element")

    std::size_t row_index =
        std::distance(
            _p_cum_sums.begin(),
            std::upper_bound(_p_cum_sums.begin(), _p_cum_sums.end(), val)) -
        1;

    std::size_t index = row_index * _COLS;
    value_type temp = _p_cum_sums[row_index];

    auto begin = _vector.begin() + index;
    auto end = begin + _COLS;

    for (; begin != end; ++begin, ++index)
    {
      temp += *begin;
      if (temp >= val)
        return index;
    }

    return NOT_FOUND;
  }
};
}; // namespace bucketlib