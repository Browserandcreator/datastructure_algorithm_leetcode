"""
LeetCode 436. Find Right Interval [Medium]
Link: https://leetcode.cn/problems/find-right-interval (source: leetcode.cn)
Tags: Array, Binary Search, Sorting

Problem:
给你一个区间数组 intervals ，其中&nbsp;intervals[i] = [starti, endi] ，且每个&nbsp;starti 都 不同 。

区间 i 的 右侧区间&nbsp;是满足 startj&nbsp;&gt;= endi，且 startj 最小&nbsp;的区间 j。注意 i 可能等于 j 。

返回一个由每个区间 i&nbsp;对应的 右侧区间 下标组成的数组。如果某个区间 i 不存在对应的 右侧区间 ，则下标 i 处的值设为 -1 。
&nbsp;

示例 1：


输入：intervals = [[1,2]]
输出：[-1]
解释：集合中只有一个区间，所以输出-1。


示例 2：


输入：intervals = [[3,4],[2,3],[1,2]]
输出：[-1,0,1]
解释：对于 [3,4] ，没有满足条件的“右侧”区间。
对于 [2,3] ，区间[3,4]具有最小的“右”起点;
对于 [1,2] ，区间[2,3]具有最小的“右”起点。


示例 3：


输入：intervals = [[1,4],[2,3],[3,4]]
输出：[-1,2,-1]
解释：对于区间 [1,4] 和 [3,4] ，没有满足条件的“右侧”区间。
对于 [2,3] ，区间 [3,4] 有最小的“右”起点。


&nbsp;

提示：


	1 &lt;=&nbsp;intervals.length &lt;= 2 * 104
	intervals[i].length == 2
	-106 &lt;= starti &lt;= endi &lt;= 106
	每个间隔的起点都 不相同

Approach: TODO
Time: O(?), Space: O(?)
"""
class Solution(object):
    def findRightInterval(self, intervals):
        """
        :type intervals: List[List[int]]
        :rtype: List[int]
        """
        