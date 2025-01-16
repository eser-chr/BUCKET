#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#ifdef ENABLE_CHECKS
#define ROW_CHECK(cond, msg) \
    if (!(cond)) throw std::runtime_error(msg);
#define VAL_CHECK(cond, msg) \
    if (!(cond)) throw std::runtime_error(msg);
#else
#define ROW_CHECK(cond, msg)
#define VAL_CHECK(cond, msg)
#endif

template <typename T>
class bucket {
   private:
    size_t _min_row_affecetd, _max_row_affected;

   public:
    size_t _ROWS;
    size_t _COLS;
    size_t _total_size;
    std::span<T> _data;
    std::vector<T> _row_sums;
    std::vector<T> _row_cumsums;
    using value_type = T;

    // bucket(size_t ROWS, size_t COLS, const std::vector<T> &other)
    //     : _ROWS(ROWS), _COLS(COLS), _data(other) {

    //     _total_size = ROWS * _COLS;
    //     _row_sums.resize(_ROWS);
    //     _row_cumsums.resize(_ROWS + 1);
    //     update_sum();
    //     update_cumsum();
    //     _min_row_affecetd = _ROWS;
    //     _max_row_affected = 0;
    // }


    //----------------------------------------------
    void print() {
        for (const T i : _row_cumsums) std::cout << i << ",";
        std::cout << std::endl;
    }

    void updateSum() {
        for (size_t row = 0; row < _ROWS; row++) update_sum_at_row(row);
    }

    void updateSumRow(int row) {
        ROW_CHECK(row >= 0 && row < _ROWS, "Row index out of range");
        _row_sums[row] = static_cast<T>(0);
        for (size_t i = _COLS * row; i < _COLS * (row + 1); i++) {
            _row_sums[row] += _data[i];
        }
        if (row < _min_row_affecetd) _min_row_affecetd = row;
        if (row > _max_row_affected) _max_row_affected = row;
    }

    void updateCumsum() {
        _row_cumsums[0] = static_cast<T>(0);

        for (size_t row = 0; row < _ROWS; row++) {
            _row_cumsums[row + 1] = _row_cumsums[row] + _row_sums[row];
        }
    }

    // void update_cumsum_from_row_on(int row) {
    //     ROW_CHECK(row >= 0 && row < _ROWS, "Row index out of range");
    //     for (size_t l_row = row; l_row < _ROWS; l_row++) {
    //         _row_cumsums[l_row + 1] = _row_cumsums[l_row] + _row_sums[l_row];
    //     }
    // }

    void refreshCumsum() {
        T diff = _row_cumsums[_max_row_affected + 1];
        size_t l_row = _min_row_affecetd;
        for (; l_row < _max_row_affected+1; l_row++) {
            _row_cumsums[l_row + 1] = _row_cumsums[l_row] + _row_sums[l_row];
        }
        diff -= _row_cumsums[_max_row_affected + 1];

        for (; l_row < _ROWS; l_row++) {
            _row_cumsums[l_row + 1] -= diff;
        }
        _min_row_affecetd = _ROWS;
        _max_row_affected = 0;
        
    }

    /*
    Assumes a change of the underlying vector at index that corresponds to a
    specific row and that one has already used the function update_sum_at_row().

     */
    // void update_cumsum_after_single_change_at_row(int row) {
    //     ROW_CHECK(row >= 0 && row < _ROWS, "Row index out of range");

    //     T diff = _row_cumsums[row + 1];
    //     _row_cumsums[row + 1] = _row_cumsums[row] + _row_sums[row];
    //     diff -= _row_cumsums[row + 1];

    //     for (size_t l_row = row + 1; l_row < _ROWS; l_row++) {
    //         _row_cumsums[l_row + 1] = _row_cumsums[l_row] - diff;
    //     }
    // }

    // void update_single_row(int row) {
    //     ROW_CHECK(row >= 0 && row < _ROWS, "Row index out of range");
    //     T diff = _row_sums[row];
    //     update_sum_at_row(row);
    //     diff -= _row_sums[row];

    //     for (size_t l_row = row + 1; l_row < _ROWS; l_row++) {
    //         _row_cumsums[l_row] -= diff;
    //     }
    // }

    // void update_single_row_of_index(int index) { update_single_row(index / _COLS); }

    // int find_upper_bound_in_cumsum(const T &val) const {
    //     auto it = std::upper_bound(_row_cumsums.begin(), _row_cumsums.end(), val);
    //     return static_cast<int>(std::distance(_row_cumsums.begin(), it));
    // }

    int findUpperBound(const T &val) const {
        VAL_CHECK(val > 0, "In upper limit, the value passed is smaller than the first element")
        VAL_CHECK(val < _row_cumsums.back(),
                  "In upper limit, the value passed is "
                  "bigger or equal to the last element")

        int row_index =
            std::distance(_row_cumsums.begin(),
                          std::upper_bound(_row_cumsums.begin(), _row_cumsums.end(), val)) -
            1;

        int index = row_index * _COLS;
        T temp = _row_cumsums[row_index];

        for (; index < (row_index + 1) * _COLS; index++) {
            temp += _data[index];
            if (temp >= val) break;
        }
        if (index >= (row_index + 1) * _COLS) return -1;
        return index;
    }
};
