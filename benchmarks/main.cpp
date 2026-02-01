// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN 0
// #include <doctest/doctest.h>

#include "timer.hpp"
#include <bucket/bucket.hpp>
#include <chrono>
#include <random>
#include <vector>

using bucketlib::bucket;
// using clock_t = std::chrono::steady_clock;

static volatile std::size_t sink; // prevent optimization

/*
A) Benchmark of a random access.
Radnomly change something, update row, refresh and get upper bound from random
again.

B) Random modify 4 consecutive entries, and then as above.

For each test use different ROWS, COLS
For each test compare with a sequential approach.

C) Modify first and last entry. Worst case scenario for both implementations.

*/

std::size_t sequential_upper_bound(const std::vector<double> &data, double val)
{
  // Compute prefix sums first
  std::vector<double> prefix(data.size() + 1, 0.0);
  std::partial_sum(data.begin(), data.end(), prefix.begin() + 1);

  // Use standard upper_bound
  auto it = std::upper_bound(prefix.begin(), prefix.end(), val);
  if (it == prefix.end())
    return std::numeric_limits<std::size_t>::max();
  return std::distance(prefix.begin(), it) -
         1; // -1 because prefix has extra 0 at start
}

void benchmark_A(std::size_t ROWS, std::size_t COLS, std::size_t iterations)
{
  const std::size_t N = ROWS * COLS;

  std::mt19937 rng(42);
  std::uniform_int_distribution<std::size_t> idx_dist(0, N - 1);
  std::uniform_real_distribution<double> val_dist(0.0, 1.0);

  std::vector<double> data(N);
  for (auto &x : data)
    x = val_dist(rng);

  bucket<std::vector<double>> b(ROWS, COLS, data);

  //---------------------------
  MyTimer t{};
  for (std::size_t i = 0; i < iterations; ++i)
  {
    std::size_t idx = idx_dist(rng);
    data[idx] = val_dist(rng);

    std::size_t row = idx / COLS;
    b.update_sum_at_row(row);
    b.refresh_cumsum();

    double q = val_dist(rng) * b.get_cumsums().back();
    sink = b.find_upper_bound(q);
  }
  auto duration = t.get();

  //---------------------------
  MyTimer seq{};
  std::vector<double> prefix(data.size() + 1, 0.0);
  for (std::size_t i = 0; i < iterations; ++i)
  {
    std::size_t idx = idx_dist(rng);
    data[idx] = val_dist(rng);
    std::partial_sum(data.begin(), data.end(), prefix.begin() + 1);

    double q = val_dist(rng) * prefix[N-1];
    auto it = std::upper_bound(prefix.begin(), prefix.end(), q);
    if (it == prefix.end())
      sink = std::numeric_limits<std::size_t>::max();
    sink = std::distance(prefix.begin(), it) - 1;
  }

  auto seq_duration = seq.get();

  std::cout << "A," << ROWS << "," << COLS << "," << duration << ","
            << seq_duration << std::endl;
}

void benchmark_B(std::size_t ROWS, std::size_t COLS, std::size_t iterations)
{
  const std::size_t N = ROWS * COLS;

  std::mt19937 rng(1337);
  std::uniform_int_distribution<std::size_t> idx_dist(0, N - 4);
  std::uniform_real_distribution<double> val_dist(0.0, 1.0);

  std::vector<double> data(N);
  for (auto &x : data)
    x = val_dist(rng);

  bucket<std::vector<double>> b(ROWS, COLS, data);

  MyTimer t{};
  for (std::size_t i = 0; i < iterations; ++i)
  {
    std::size_t idx = idx_dist(rng);

    std::size_t first_row = idx / COLS;
    std::size_t last_row = (idx + 3) / COLS;

    for (std::size_t j = 0; j < 4; ++j)
      data[idx + j] = val_dist(rng);

    for (std::size_t r = first_row; r <= last_row; ++r)
      b.update_sum_at_row(r);

    b.refresh_cumsum();

    double q = val_dist(rng) * b.get_cumsums().back();
    sink = b.find_upper_bound(q);
  }
  auto duration = t.get();

  MyTimer seq{};
  std::vector<double> prefix(data.size() + 1, 0.0);
  for (std::size_t i = 0; i < iterations; ++i)
  {
    std::size_t idx = idx_dist(rng);
    data[idx] = val_dist(rng);
    std::partial_sum(data.begin(), data.end(), prefix.begin() + 1);

    double q = val_dist(rng) * prefix[N-1];
    auto it = std::upper_bound(prefix.begin(), prefix.end(), q);
    if (it == prefix.end())
      sink = std::numeric_limits<std::size_t>::max();
    sink = std::distance(prefix.begin(), it) - 1;
  }

  auto seq_duration = seq.get();

  std::cout << "B," << ROWS << "," << COLS << "," << duration << ","
            << seq_duration << std::endl;
}

void benchmark_C(std::size_t ROWS, std::size_t COLS, std::size_t iterations)
{
  const std::size_t N = ROWS * COLS;

  std::mt19937 rng(1337);
  std::uniform_int_distribution<std::size_t> idx_dist(0, N - 4);
  std::uniform_real_distribution<double> val_dist(0.0, 1.0);

  std::vector<double> data(N);
  for (auto &x : data)
    x = val_dist(rng);

  bucket<std::vector<double>> b(ROWS, COLS, data);

  MyTimer t{};
  for (std::size_t i = 0; i < iterations; ++i)
  {
    std::size_t idx = idx_dist(rng);

    std::size_t first_row = 0;
    std::size_t last_row = ROWS - 1;

    for (std::size_t j = 0; j < ROWS; ++j)
    {

      data[0 + COLS * j] = val_dist(rng);
    }

    for (std::size_t r = first_row; r <= last_row; ++r)
      b.update_sum_at_row(r);

    b.refresh_cumsum();

    double q = val_dist(rng) * b.get_cumsums().back();
    sink = b.find_upper_bound(q);
  }
  auto duration = t.get();

  MyTimer seq{};
  std::vector<double> prefix(data.size() + 1, 0.0);
  for (std::size_t i = 0; i < iterations; ++i)
  {
    std::size_t idx = idx_dist(rng);
    data[idx] = val_dist(rng);
    std::partial_sum(data.begin(), data.end(), prefix.begin() + 1);

    double q = val_dist(rng) * prefix[N-1];
    auto it = std::upper_bound(prefix.begin(), prefix.end(), q);
    if (it == prefix.end())
      sink = std::numeric_limits<std::size_t>::max();
    sink = std::distance(prefix.begin(), it) - 1;
  }

  auto seq_duration = seq.get();

  std::cout << "C," << ROWS << "," << COLS << "," << duration << ","
            << seq_duration << std::endl;
}

int main()
{
  constexpr std::size_t ITER = 100'000;

  std::cout << "benchmark_type,rows,cols,bucket_duration,seq_duration"
            << std::endl;

  std::size_t N = 1'000;
  std::array<std::size_t, 4> rows{10, 20, 50, 100};

  for (std::size_t rep{}; rep < 5UL; rep++)
  {

    for (auto const ROWS : rows)
    {
      auto const COLS = N / ROWS;
      benchmark_A(ROWS, COLS, ITER);
      benchmark_B(ROWS, COLS, ITER);
      benchmark_C(ROWS, COLS, ITER);
    }
  }
}

// int main()
// {

//   std::size_t const N = 10'000;
//   std::size_t const ROWS = 100;
//   std::size_t const COLS = 100;
//   std::size_t const MODIFICATIONS = 1'000'000;

//   std::mt19937 rng{std::random_device{}()};
//   std::uniform_real_distribution<double> dist(0.0, 1.0);

//   std::vector<double> data;
//   data.resize(N);
//   std::generate(data.begin(), data.end(), [&] { return dist(rng); });

//   assert(data.size() == N); // Just to be sure

//   bucket<std::vector<double>> b(ROWS, COLS, data);

//   // Main benchmark
//   {
//     for (std::size_t i{}; i < MODIFICATIONS; ++i)
//     {
//       u
//     }
//   }
// }