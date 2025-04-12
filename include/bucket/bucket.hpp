#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <numeric>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <vector>

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

/**
 * @brief The main namespace for the bucketlib library.
 *
 * Contains the `bucket` class and all supporting concepts for compile-time
 * validation.
 */
namespace bucketlib
{

template <typename Container> struct is_supported_container : std::false_type
{
};

template <typename T>
struct is_supported_container<std::vector<T>> : std::true_type
{
};

template <typename T, std::size_t N>
struct is_supported_container<std::array<T, N>> : std::true_type
{
};

template <typename T>
struct is_supported_container<std::span<T>> : std::true_type
{
};

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

/**
 * @brief A 2D manager abstraction for efficient cumulative operations and
 * upper-bound lookup when the underlying data is modified locally.
 *
 * This class partitions a flat container (most commonly a `std::vector`) into
 * logical rows and columns, enabling:
 *  - Row-wise sum updates into a local vector `_p_sums`
 *  - Cumulative sum updates of the row sums into `_p_cum_sums`
 *
 * ### Example:
 * Given a flat vector:
 * ```
 * {1, 2, 3, 4, 5, 6, 7, 8, 9}
 * ```
 * and dimensions: **ROWS = 3**, **COLS = 3**
 *
 * The internal vectors are:
 * - `_p_sums` = {6, 15, 24}
 * - `_p_cum_sums` = {0, 6, 21, 45}
 *
 * Which represent the structure:
 * ```
 *             | vector values | _p_sums | _p_cum_sums
 * ------------|---------------|---------|-------------
 * Row 0       | 1 , 2 , 3     |    6    |      6
 * Row 1       | 4 , 5 , 6     |   15    |     21
 * Row 2       | 7 , 8 , 9     |   24    |     45
 *             |               |         |     ↑ 0 is prepended
 * ```
 *
 * In addition, the class supports:
 *  - Efficient incremental updates to `_p_sums` and `_p_cum_sums`
 *  - Fast inverse transform sampling via `find_upper_bound(val)`
 *
 * @tparam Container Must be a supported contiguous random-access container:
 *         - `std::vector<T>`
 *         - `std::array<T, N>`
 *         - `std::span<T>`
 *
 * @note The container is passed **by reference** and must outlive the `bucket`
 * object.
 * @note Values are assumed to be **non-negative**. This is **not enforced for
 * performance reasons**, but is expected when using cumulative sum logic and
 * upper-bound search.
 */
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
  /// @brief Sentinel index returned when an upper bound is not found.
  static constexpr std::size_t NOT_FOUND =
      std::numeric_limits<std::size_t>::max();
  /**
   * @brief Constructs a bucket with a logical ROWS × COLS view over the input
   * container.
   *
   * @param ROWS Number of rows to partition the container
   * @param COLS Number of columns (per row)
   * @param other Reference to the flat container (not copied)
   *
   * @pre `other.size() <= ROWS * COLS` (an assertion guards this)
   * @post Initializes per-row sums and cumulative sums.
   */
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
  /// @brief Returns the total number of elements in the 2D view. ROWS × COLS.
  /// Not to be confused with the size of the underlying container.
  [[nodiscard]] std::size_t get_size() const noexcept { return _size; }
  /// @brief Returns the number of rows.
  [[nodiscard]] std::size_t get_rows() const noexcept { return _ROWS; }
  /// @brief Returns the number of columns.
  [[nodiscard]] std::size_t get_cols() const noexcept { return _COLS; }
  /// @brief Returns the index of the first row that was modified since last
  /// refresh.
  [[nodiscard]] std::size_t get_min_row_affected() const noexcept
  {
    return _min_row_affected;
  }
  /// @brief Returns the index of the last row that was modified since last
  /// refresh.
  [[nodiscard]] std::size_t get_max_row_affected() const noexcept
  {
    return _max_row_affected;
  }
  /// @brief Returns the current per-row sums.
  [[nodiscard]] const std::vector<value_type> &get_sums() const noexcept
  {
    return _p_sums;
  }
  /// @brief Returns the current cumulative sums across rows.
  [[nodiscard]] const std::vector<value_type> &get_cumsums() const noexcept
  {
    return _p_cum_sums;
  }
  /// @brief Prints the cumulative sums to the standard output.
  void print() const noexcept
  {
    for (const value_type &i : _p_cum_sums)
      std::cout << i << ",";
    std::cout << std::endl;
  }

  /**
   * @brief Updates all per-row sums.
   *
   * Useful when the entire container may have changed.
   * Otherwise, you can use the update_sum_at_row() method for efficiency.
   */
  void update_sum() const
  {
    for (std::size_t row = 0; row < _ROWS; row++)
      update_sum_at_row(row);
  }

  /**
   * @brief Updates the sum of a single row and marks it as affected.
   *
   * @param row The index of the row to update (0-based)
   * @throws std::runtime_error if row is out of range and ENABLE_CHECKS is
   * defined
   */
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

  /**
   * @brief Fully recomputes cumulative sums across all rows.
   *
   * Strongly recommended after calling `update_sum()` or when initialization is
   * needed.
   */
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

  /**
   * @brief Partially refreshes the cumulative sums only for modified rows.
   *
   * This is more efficient than `update_cumsum()` when only a few rows have
   * changed.
   *
   * You can update the underlying structure, update the sums at single rows and
   * then call this method, once the updates have been done.
   */
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
  /**
   * @brief Returns whether a given index is a valid result (not NOT_FOUND).
   */
  [[nodiscard]] bool is_valid_index(std::size_t index) const noexcept
  {
    return index != NOT_FOUND;
  }
  /**
   * @brief Returns the index in the container where the cumulative sum reaches
   * or exceeds a threshold.
   *
   * @param val The target value (must be ≥ 0 and less than the total sum)
   * @return Index into the container, or NOT_FOUND if `val` is out of bounds
   *
   * @throws std::runtime_error if ENABLE_CHECKS is defined and `val` is out of
   * range
   */
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