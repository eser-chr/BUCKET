#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN 0
#include <doctest/doctest.h>

#include <bucket/bucket.hpp>
#include <vector>

using bucketlib::bucket;

TEST_CASE("Basic functionality of bucket")
{
  std::vector<double> data = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9};

  bucket<std::vector<double>> b(3, 3, data);

  SUBCASE("Size and shape min_max_row")
  {
    CHECK(b.get_rows() == 3);
    CHECK(b.get_cols() == 3);
    CHECK(b.get_size() == 9);
    CHECK(b.get_min_row_affected() == 3);
    CHECK(b.get_max_row_affected() == 0);
    b.update_sum_at_row(1);
    CHECK(b.get_min_row_affected() == 1);
    CHECK(b.get_max_row_affected() == 1);
    b.update_cumsum();
    CHECK(b.get_min_row_affected() == 3);
    CHECK(b.get_max_row_affected() == 0);
    b.update_sum_at_row(1);
    CHECK(b.get_min_row_affected() == 1);
    CHECK(b.get_max_row_affected() == 1);
    b.refresh_cumsum();
    CHECK(b.get_min_row_affected() == 3);
    CHECK(b.get_max_row_affected() == 0);
  }

  SUBCASE("Sum per row")
  {
    auto sums = b.get_sums();
    CHECK(sums[0] == doctest::Approx(0.6));
    CHECK(sums[1] == doctest::Approx(1.5));
    CHECK(sums[2] == doctest::Approx(2.4));
  }

  SUBCASE("Cumulative sums")
  {
    auto cumsums = b.get_cumsums();
    CHECK(cumsums[0] == doctest::Approx(0.0));
    CHECK(cumsums[1] == doctest::Approx(0.6));
    CHECK(cumsums[2] == doctest::Approx(2.1));
    CHECK(cumsums[3] == doctest::Approx(4.5));
  }

  SUBCASE("Upper bound lookup")
  {
    CHECK(b.find_upper_bound(0.1) == 0);
    CHECK(b.find_upper_bound(0.7) == 3); // Should be inside 2nd row
    CHECK(b.find_upper_bound(2.2) == 6); // Should be in last row
    CHECK(b.find_upper_bound(4.4) == 8);
  }

  SUBCASE("Index validity check")
  {
    CHECK(b.is_valid_index(0));
    CHECK(b.is_valid_index(8));
    CHECK_FALSE(b.is_valid_index(bucket<std::vector<double>>::NOT_FOUND));
  }

  SUBCASE("Underlying changes")
  {
    data[0] = 1.0;
    b.update_sum_at_row(0);
    b.update_cumsum();
    CHECK(b.get_sums()[0] == doctest::Approx(1.5));
    CHECK(b.get_cumsums()[1] == doctest::Approx(1.5));
    CHECK(b.get_cumsums()[2] == doctest::Approx(3.0));
    CHECK(b.get_cumsums()[3] == doctest::Approx(5.4));
    data[0] = 0.1;
    b.update_sum_at_row(0);
    b.update_cumsum();
    auto cumsums = b.get_cumsums();
    CHECK(cumsums[0] == doctest::Approx(0.0));
    CHECK(cumsums[1] == doctest::Approx(0.6));
    CHECK(cumsums[2] == doctest::Approx(2.1));
    CHECK(cumsums[3] == doctest::Approx(4.5));
  }

  SUBCASE("Underlying changes + refresh")
  {
    data[0] = 1.0;
    b.update_sum_at_row(0);
    b.refresh_cumsum(); // instead of update_cumsum()

    CHECK(b.get_sums()[0] == doctest::Approx(1.5));
    CHECK(b.get_cumsums()[1] == doctest::Approx(1.5));
    CHECK(b.get_cumsums()[2] == doctest::Approx(3.0));
    CHECK(b.get_cumsums()[3] == doctest::Approx(5.4));
  }
}
