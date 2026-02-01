To illustrate the effectivenes of this datastructure we performed a comparison between this datastructure and a sequential implementation. We defined 3 types of benhcmark to make our analysis more robust and present the true potential of this library, as well as its pitfalls. 

For each type of benchmark we used three different problem sizes (i.e size of the main vector): N = 1K, 10K ,100K elements. We did not move above that number because we believe that for more than 100K elements a GPU implementation of exscan is superior.
In addition we iterate over different ROW sizes to showcase the effect on the performance. As you one can see from the figure, it is clear that indeed the best performance is achieved when ROWS ~ sqrt(N).

# Types of Benchmarks

We performed 3 types of benchmark:

## One single change
In this type of benhcmark, we randomly alter one element of the main vector followed by a row update a refresh of the cummulative sum and a random search for upper bound.

## Consecutive changes
In this type of benchmark we chnage 4 consecutive elements of the main vector followed again by row updates a refresh and an upper bound search. The idea is to showcase that this data structure and the accompany algorithms perform equally fast as in the first case if the changes are local.

## Worst case
We alter one element at every row and force a full update. This tries to illustrates that in the worst case scenario this datastructure will perform reasonably well compared to a sequential vector and exscan


# Speedup Benchmarks

Here are the comparison of bucket vs sequential for different problem sizes as indicated by the titles:

![Speedup vs Rows](benchmarks/speedup_1.png)
![Speedup vs Rows](benchmarks/speedup_10.png)
![Speedup vs Rows](benchmarks/speedup_100.png)


## Conclusions
The results of the benchmark show an immediate imporvement on almost all benhchmarks and problem sizes with the exception of the worst case scenario where the whole datastructure has to compute the full exscan. This is expected as a result and of course it is not the use case we had in mind. However, if the application contain such worst case updates which are not systematic then the extra cost of this library might be compensated by the spped up it can offer in more local updates.

It is also clear that the better results we achieved for very local changes and updates as well as for large problem sizes. One can see that for a size of 100K the speed up can be a bit more than 200x compared to the naive implementation.

The type A benhcmark presents no significant difference from type B benchmark indicating that local updates that are in close rows do not add significant overhead during the calculations.


