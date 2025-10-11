/*
 * LeetCode 45. Jump Game II [Medium]
 * Link: https://leetcode.cn/problems/jump-game-ii (source: leetcode.cn)
 * Tags: Greedy, Array, Dynamic Programming
 *
 * Problem:
 * 给定一个长度为 n 的 0 索引整数数组 nums。初始位置在下标 0。
 *
 * 每个元素 nums[i] 表示从索引 i 向后跳转的最大长度。换句话说，如果你在索引&nbsp;i&nbsp;处，你可以跳转到任意 (i + j) 处：
 *
 *
 * 	0 &lt;= j &lt;= nums[i]&nbsp;且
 * 	i + j &lt; n
 *
 *
 * 返回到达&nbsp;n - 1&nbsp;的最小跳跃次数。测试用例保证可以到达 n - 1。
 *
 * &nbsp;
 *
 * 示例 1:
 *
 *
 * 输入: nums = [2,3,1,1,4]
 * 输出: 2
 * 解释: 跳到最后一个位置的最小跳跃数是 2。
 * &nbsp;    从下标为 0 跳到下标为 1 的位置，跳&nbsp;1&nbsp;步，然后跳&nbsp;3&nbsp;步到达数组的最后一个位置。
 *
 *
 * 示例 2:
 *
 *
 * 输入: nums = [2,3,0,1,4]
 * 输出: 2
 *
 *
 * &nbsp;
 *
 * 提示:
 *
 *
 * 	1 &lt;= nums.length &lt;= 104
 * 	0 &lt;= nums[i] &lt;= 1000
 * 	题目保证可以到达&nbsp;n - 1
 *
 * Approach: TODO
 * Time: O(?), Space: O(?)
 */
#include <vector>
using namespace std;
class Solution {
public:
    int jump(vector<int>& nums) {
        int n = nums.size();
        int jumps = 0, current_end = 0, farthest = 0;
        for (int i = 0; i < n - 1; ++i) {
            farthest = max(farthest, i + nums[i]);
            if (i == current_end) {
                jumps++;
                current_end = farthest;
            }
        }
        return jumps;
    }
};

int main() {
    Solution solution;
    vector<int> nums = {2, 3, 1, 1, 4};
    int result = solution.jump(nums);
    return 0;
}