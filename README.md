# Upvotes challenge from Quora.

**Upvotes**

At Quora, we have aggregate graphs that track the number of upvotes we get each day.

As we looked at patterns across windows of certain sizes, we thought about ways to track trends such as non-decreasing and non-increasing subranges as efficiently as possible.

For this problem, you are given N days of upvote count data, and a fixed window size K. For each window of K days, from left to right, find the number of non-decreasing subranges within the window minus the number of non-increasing subranges within the window.

A window of days is defined as contiguous range of days. Thus, there are exactly N-K+1 windows where this metric needs to be computed. A non-decreasing subrange is defined as a contiguous range of indices [a,b], a<b, where each element is at least as large as the previous element. A non-increasing subrange is similarly defined, except each element is at least as large as the next. There are up to K(K-1)/2 of these respective subranges within a window, so the metric is bounded by [-K(K-1)/2,K(K-1)/2].

**Constraints**

- 1≤N≤100,000

- 1≤K≤N days

**Input Format**\
Line 1: Two integers, N and K

Line 2: NN positive integers of upvote counts, each integer less than or equal to 109109.

**Output Format**\
Line 1..: N-K+1 integers, one integer for each window's result on each line

**Sample Input**

1. 5  3

2. 1  2  3  1  1

**Sample Output**

1. 3

2. 0

3. -2

**Explanation**\
For the first window of [1, 2, 3], there are 3 non-decreasing subranges and 0\
non-increasing, so the answer is 3. For the second window of [2, 3, 1], there is 1 non-decreasing subrange and 1 non-increasing, so the answer is 0. For the third window of [3, 1, 1], there is 1 non-decreasing subrange and 3 non-increasing, so the answer is -2.
