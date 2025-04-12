# `bucket` - Efficient Lazy Update for Cumulative Sums and Upper Bound Indices.

A header-only C++ library that efficiently uses the **lazy update protocol**  for cumulative sums of a 1D container such as `std::vector<double>`.

It supports arithmetic operations on primary data types `(double, float, int, uint8_t, ...)` but does not handle custom-defined types with self defined operations. 



## Cumulative Sums (a.k.a Prefix Sums):
---
The cumulative sum of a sequence `a_i` is a sequence defined as 

` sₖ = a₀ + a₁ + ... + aₖ`


Updating or calculating from scratch the cumulative sum is an `O(N)` operation.


# Problem
---
Many algorithms (such as the Gillespie), at their core, have a main loop where both the underlying sequence and its cumulative sum are updated on every iteration based on a random number. If `N>100` this update is the bottleneck in the execution of the program. If `N>1e6`  we might start thinking for letting a GPU handle the cumulative sum.

In this project, however, we tackle another (not-so-rare) scenario found in many real-world applications in which the updates of the underlying data happen at a very small local range. For example at one iteration the underlying data present a change from element 40 to 50. In such a scenario we can use the so called lazy update protocol.


# Lazy update protocol explained:
---
Instead of storing the whole cumulative sum we can imagine as if the vector is in a matrix form  and for each row we calculate the sum of that row. Moreover, we keep a **second-level cumulative sum** over those row sums. Pictographically :


\[
\begin{array}{lcl}
\text{Raw data block} & \longrightarrow & \text{Row sum} \\
\hline
a_0, \ldots, a_{\text{COLS}-1} & \longrightarrow & \text{sum}_0 \\
a_{\text{COLS}}, \ldots, a_{2\cdot\text{COLS}-1} & \longrightarrow & \text{sum}_1 \\
\vdots & \ddots & \vdots \\
a_{(\text{ROWS}-1)\cdot \text{COLS}}, \ldots, a_{\text{ROWS} \cdot \text{COLS} -1} & \longrightarrow & \text{sum}_{\text{ROWS}-1}
\end{array}
\]

\[
\text{Cumulative sum:} \quad \text{cum}_k = \sum_{i=0}^{k-1} \text{sum}_i
\]

where \(sum_{row}\) is the sum of that particular row.


### Constraints:
- You can choose the number of rows and columns for optimal efficiency in your specific case  [see Total Cost Comparison](#-total-cost-comparison).
- The only requirement is that `ROWS x COLS >= size of the data`.



## Update the Data
---
If an element of a particular row is updated:
1) We need to calculate the sum of a ROW `O(COLS)`
2) Adjust the cumulative sum of that row onward.

Extra note: The last part can further be optimised because  in practice we can simply add the difference of the updated sum element to the rest of the elements in the cumulative sum, thus avoiding multiple reads of the sum vector. Still, the operation of the cumulative sum is linear `O(ROWS)`. However, it is linear in terms of the number of ROWS, which for the majority of the relevant applications ROWS should be close to `sqrt(N)`.

## Find upper Bound
---
This operation is crucial for sampling from distributions. Unfortunately, for this operation, we get a penalty for using this data structure. Initially, the upper bound in a sorted vector such as the full cumulative sum is `O(logN)`. Now, we have to:
1) Perform a binary search over the second layer of cumulative sums.
2) A linear scan over the selected row.

The total cost is`O(log(ROWS)+COLS) \approx O(COLS)` vs the traditional `O(logN)`.




## Total Cost Comparison
---
| Operation            | Standard Vector              | Lazy Bucket Version                          |
|----------------------|------------------------------|----------------------------------------------|
| Update full cumsum   | `O(N)`                       | `O(ROWS + COLS)`                             |
| Find upper bound     | `O(log N) `                  | `O(log (ROWS) + COLS)`                       |
| Total per iteration  | Slow if large `N`            | Fast if changes are local                    |


## Choose the correct values for ROWS, COLS
---
From the theoretical expressions shown in the previous section we get the best performace when


`COLS \approx ROWS/3 ... ROWS/2`

In some cases it also worths to use some padding on the underlying vector (add some ghost zeros at the end) to ensure that that the chosen number of columns and rows lead to a better efficiency.



# Installation
---
Since it's header-only, you can simply copy bucket.hpp into your project.

```Cmake```
You can also clone the repo and use on the cmake of your project. 
```
#It is assumed that external libraries are 
#placed in an external folder. Adjustable.

add_subdirectory(<external/bucket>) 
target_link_libraries(your_target PRIVATE bucket)

```

Optional install support is available, simply add the following command to your cmake:
```
set_option(BUCKET_INSTALL ON)
```


# Usage
---
If you have copied the file manually, then add to your script
```
#include "bucket.h"
```

If you used cmake then the inclusion is
```
#include <bucket/bucket.hpp>
```


# Platform Support / Compiler
---
It has been tested only under: 
- Ubuntu 22.04
- GCC and Clang with C++ 20








