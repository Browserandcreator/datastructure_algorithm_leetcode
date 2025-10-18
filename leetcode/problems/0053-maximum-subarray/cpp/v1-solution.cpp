/*
 * LeetCode 53. Maximum Subarray [Medium]
 * Link: https://leetcode.cn/problems/maximum-subarray (source: leetcode.cn)
 * Tags: Array, Divide and Conquer, Dynamic Programming
 *
 * Problem:
 * 给你一个整数数组 nums ，请你找出一个具有最大和的连续子数组（子数组最少包含一个元素），返回其最大和。
 *
 * 子数组 是数组中的一个连续部分。
 *
 * &nbsp;
 *
 * 示例 1：
 *
 *
 * 输入：nums = [-2,1,-3,4,-1,2,1,-5,4]
 * 输出：6
 * 解释：连续子数组&nbsp;[4,-1,2,1] 的和最大，为&nbsp;6 。
 *
 *
 * 示例 2：
 *
 *
 * 输入：nums = [1]
 * 输出：1
 *
 *
 * 示例 3：
 *
 *
 * 输入：nums = [5,4,-1,7,8]
 * 输出：23
 *
 *
 * &nbsp;
 *
 * 提示：
 *
 *
 * 	1 &lt;= nums.length &lt;= 105
 * 	-104 &lt;= nums[i] &lt;= 104
 *
 *
 * &nbsp;
 *
 * 进阶：如果你已经实现复杂度为 O(n) 的解法，尝试使用更为精妙的 分治法 求解。
 *
 * Approach: TODO
 * Time: O(?), Space: O(?)
 */
#include <vector>
using namespace std;
class Solution {
public:
    int maxSubArray(vector<int>& nums) {
        int maxSum = nums[0];
        int currentSum = nums[0];
        for (int i = 1; i < nums.size(); i++) {
            currentSum = max(nums[i], currentSum + nums[i]);
            maxSum = max(maxSum, currentSum);
        }
        return maxSum;
    }
};

int main() {
    // Example test case
    Solution sol;
    vector<int> nums = {-2,1,-3,4,-1,2,1,-5,4};
    int result = sol.maxSubArray(nums);
    // Expected output: 6
    return 0;
}