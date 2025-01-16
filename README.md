A header only library that effectively uses the Lazy update protocol to update a cummulative sum of an underlying vector.

Currently it supports arithmetic operations on primary data types and no custom made types with self defined operations. 



#Cummulative sum or prefix sum:
===============================
The cummulative sum of a sequence $a_i$ is a sequence defined as 
[latex]
s_k = \sum\limits_0^k a_k
[]

That means algorithmically updating or calculating from scratch the cummulative sum is a linear operation O(N).


#Problem
===============================
Many algorithms such as the Gillespie, at their core, have a main loop where the underlying sequence and its respective cummulative sum are updated at every iteration based on a random number. If N is large (~ >100) then in most cases the bottleneck of the algorithm is the calculation of the cummulative sum.

Of course, an algorithm should first be correct and then fast. This saddly means that we have to accept the slow update of the cummulative sum. If N is even larger (>10^6) we might start thinking for moving this part to a GPU.

Here, however, we tackle another (not so rare) scenario. The case where the updates of the underlying data happen at a very small local range of the vector(we use the word vector in the C++ manner). For example at one iteration the underlying data present a change from item 40 to 50. In such a scenario we can use the so called lazy update protocol


#Lazy update protocol explained:
================================
According to this protocol we do not need to store the whole cummulative sum. We can imagine as if the vector is stored as a matrix and for each row we calculate the sum of that row. Pictographically 

a0|....a_{COLS-1}                               sum_0
a_{COLS}|...|a_{2*COLS-1}                       sum_1    
.                                               .
.                                               .    
.                                               .    
a_{(ROWS-1)*COLS} | ... | a_{(ROWS-1)*COLS-1}   sum_{ROWS-1}


where sum_row is the sum of that particular row.

ROWS, and COLS are for now undefined and the user can choose these values wisely by balancing the tradeoffs which we will show later.
For now the only constraint is that ROWS*COLS = N. Also, note that the length of sum is ROWS.

Based on that second sequence we can calculate a cummulative sum, which for practical reasons is a combination of an internal and external cummulative sum. That means it has a length of ROWS+1, the first element is always 0 and the last element is the total sum of the vector.


## Update
===========
Now if an element of a particular row is updated we need to calculate the sum of a ROW (O(COLS)) and calculate the cumsum of the partial sums. The last part can further be optimised because we can simply subtract the difference of before-after and practically add a constant to the elements that follow that row. It is still (O(ROWS)) but now the underlying operation is much faster and given that cumsum is smaller than the underlying vector, we can assume that fetching this part from memory will be much faster.

## Find upper bound
===================
This operation is the next most important operation of an algortihm such as Gillespie. Here, saddly we get a penalty for using this data structure. Initially the upper bound in a sorted vector such as the full cummulative sum is O(log N). Now, it is O(log(ROWS) + COLS)~O(COLS), because we have to first find the upper bound in the cummulative sum, go one row before and in that row start adding the underlying elements one by one till we find the upper bound.



## TOTAL COST
===================
If we add the above operations, we get a complexity for a pair of update+find upper bound of

O(COLS+ROWS).

A bit more precisely, we have at worst case:
~(2*COLS+ROWS)*additions+COLS if statements.

Compared to:
log(ROWS) if statements
N*additions

We can see now where this datastructure can improve our code significantly.


## Choose the correct values for ROWS, COLS
===========================================
From this theoretical expression (2*COLS+ROWS)*additions+COLS * ifs
but also from practical tests on this specific problem
[[hyperlink]]

COLS must be 1/2 to 1/3 of ROWS so that we obtain best results.

In some cases it also worths to use some padding on the underlying vector (add some ghost zeros at the end).



# Installation
===========================================
Since it a header-only library one has the option to simply copy paste the content of the file to a proper location and reference accordingly.

Cmake
The use of cmake is also available and this library supports also the fetch command of cmake. Thus one can either clone the repository in their project under an external or thirdparty folder and add to the main cmake the command

add_subdirectory(external/bucket)
target_include_link(bucket)


We have added the option to install the library in the main headerfiles. 
Simply add to the main CMakeLists.txt the following command

set_option(BUCKET_INSTALL)



# How to use
============================================
If one has simply copy-pasted the header file then they can use 
#include "bucket.h"

If one has used cmake then the inclusion is
#include "bucket/bucket.h"
or
#include <bucket/bucket.h>
if they allowed for proper installation




# OS
===================================
Currently it is been tested only under Ubuntu 








