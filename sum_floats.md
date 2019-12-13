How to sum a list of floats
=============================

Summing a list of floats is easy, you can't do much better than this:

~~~
float sum(float* array, int len)
{
  float accu = 0.0f;
  for(int i = 0; i < len; ++i)
  {
    accu += array[i];
  }
  return accu;
}
~~~

Right?

Wrong!
--------
~~~
#define BATCH_SIZE 4 

float batched_sum(float* array, int len)
{
  float accu[BATCH_SIZE] = {0};
  for(int i = 0; i < len; i += BATCH_SIZE)
  {
    for(int j = 0; j < BATCH_SIZE; ++j)
    {
      accu[j] += array[i + j];
    }
  }
  return sum(accu, BATCH_SIZE);
}

float fast_sum(float* array, int len)
{
  int unbatched = len % BATCH_SIZE;
  return sum(array, unbatched) + batched_sum(&array[unbatched], len - unbatched);
}
~~~

"But how can this be faster? It doesn't make sense."

There are two effects at play here: 
  1) in the first code, the result of each summation operation depends on the previous one.
     A modern processor is able to execute multiple operations at a time, if the following operation doesn't depend on the result of a previous one
  2) Summing consequent elements pairwise can be implemented using SSE or similar SIMD instructions. 

How much faster is this?
--------------------------
I tested the functions on my old 2012 Mac Mini. The above code was compiled using Apple clang version 11.0.0.
The test generated exponentially larger input arrays filled with random floats. For each size 100 repetitions of a function
were calculated and the average running time was recorded. One of the runs is with O3 optimizations and the other with O0.

![Results with O0](O0.png)
![Results with O3](O3.png)

In the unoptimized case the gain is modest 21%, but with the optimization turned on, the batched version is 63% faster (takes only 37% of the time). The optimized version is able to benefit from the SIMD vectorization.

This is what Clang 9.0 (On Compiler Explorer) makes of the two loops in batched_sum:
~~~
.LBB2_10:                               # =>This Inner Loop Header: Depth=1
        movups  xmm2, xmmword ptr [rcx + 4*rdx]
        addps   xmm1, xmm2
        add     rdx, 4
        cmp     rdx, rax
        jl      .LBB2_10
~~~
