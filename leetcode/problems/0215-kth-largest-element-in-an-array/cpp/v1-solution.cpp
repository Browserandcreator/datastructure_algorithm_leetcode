/*
 * LeetCode 215. Kth Largest Element in an Array [Medium]
 * Link: https://leetcode.cn/problems/kth-largest-element-in-an-array (source: leetcode.cn)
 * Tags: Array, Divide and Conquer, Quickselect, Sorting, Heap (Priority Queue)
 *
 * Problem:
 * 给定整数数组 nums 和整数 k，请返回数组中第 k 个最大的元素。
 *
 * 请注意，你需要找的是数组排序后的第 k 个最大的元素，而不是第 k 个不同的元素。
 *
 * 你必须设计并实现时间复杂度为 O(n) 的算法解决此问题。
 *
 * &nbsp;
 *
 * 示例 1:
 *
 *
 * 输入: [3,2,1,5,6,4], k = 2
 * 输出: 5
 *
 *
 * 示例&nbsp;2:
 *
 *
 * 输入: [3,2,3,1,2,4,5,5,6], k = 4
 * 输出: 4
 *
 * &nbsp;
 *
 * 提示： 
 *
 *
 * 	1 &lt;= k &lt;= nums.length &lt;= 105
 * 	-104&nbsp;&lt;= nums[i] &lt;= 104
 *
 * Approach: TODO
 * Time: O(?), Space: O(?)
 */
#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;
class Solution {
public:
    int findKthLargest(vector<int>& nums, int k) {
        // 使用nth_element
        nth_element(nums.begin(), nums.end() - k, nums.end());
        return nums[nums.size() - k];
    }
};

int main() {
    // 测试写好的代码
    Solution sol;
    vector<int> nums = {3,2,1,5,6,4};
    int k = 2;
    cout << sol.findKthLargest(nums, k) << endl; // 输出 5
    return 0;
}