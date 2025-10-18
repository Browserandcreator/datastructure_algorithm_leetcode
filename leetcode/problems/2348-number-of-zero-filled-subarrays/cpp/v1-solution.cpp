/*
 * LeetCode 2348. Number of Zero-Filled Subarrays [Medium]
 * Link: https://leetcode.cn/problems/number-of-zero-filled-subarrays (source: leetcode.cn)
 * Tags: Array, Math
 *
 * Problem:
 * 给你一个整数数组&nbsp;nums&nbsp;，返回全部为&nbsp;0&nbsp;的&nbsp;子数组&nbsp;数目。
 *
 * 子数组&nbsp;是一个数组中一段连续非空元素组成的序列。
 *
 * &nbsp;
 *
 * 示例 1：
 *
 * 输入：nums = [1,3,0,0,2,0,0,4]
 * 输出：6
 * 解释：
 * 子数组 [0] 出现了 4 次。
 * 子数组 [0,0] 出现了 2 次。
 * 不存在长度大于 2 的全 0 子数组，所以我们返回 6 。
 *
 * 示例 2：
 *
 * 输入：nums = [0,0,0,2,0,0]
 * 输出：9
 * 解释：
 * 子数组 [0] 出现了 5 次。
 * 子数组 [0,0] 出现了 3 次。
 * 子数组 [0,0,0] 出现了 1 次。
 * 不存在长度大于 3 的全 0 子数组，所以我们返回 9 。
 *
 *
 * 示例 3：
 *
 * 输入：nums = [2,10,2019]
 * 输出：0
 * 解释：没有全 0 子数组，所以我们返回 0 。
 *
 *
 * &nbsp;
 *
 * 提示：
 *
 *
 * 	1 &lt;= nums.length &lt;= 105
 * 	-109 &lt;= nums[i] &lt;= 109
 *
 * Approach: TODO
 * Time: O(?), Space: O(?)
 */
#include <vector>
using namespace std;
class Solution {
public:
    long long zeroFilledSubarray(vector<int>& nums) {
        // 每遇到连续n个0，其贡献的子数组数量为1 + 2 + ... + n = n * (n + 1) / 2
        long long count = 0;
        long long total = 0;
        for (int i = 0; i < nums.size(); i++) {
            if (nums[i] == 0) {
                count++;
                total += count;
            } else {
                count = 0;
            }
        }
        return total;
    }
};