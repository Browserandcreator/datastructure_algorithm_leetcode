/*
 * LeetCode 239. Sliding Window Maximum [Hard]
 * Link: https://leetcode.cn/problems/sliding-window-maximum (source: leetcode.cn)
 * Tags: Queue, Array, Sliding Window, Monotonic Queue, Heap (Priority Queue)
 *
 * Problem:
 * 给你一个整数数组 nums，有一个大小为&nbsp;k&nbsp;的滑动窗口从数组的最左侧移动到数组的最右侧。你只可以看到在滑动窗口内的 k&nbsp;个数字。滑动窗口每次只向右移动一位。
 *
 * 返回 滑动窗口中的最大值 。
 *
 * &nbsp;
 *
 * 示例 1：
 *
 *
 * 输入：nums = [1,3,-1,-3,5,3,6,7], k = 3
 * 输出：[3,3,5,5,6,7]
 * 解释：
 * 滑动窗口的位置                最大值
 * ---------------               -----
 * [1  3  -1] -3  5  3  6  7       3
 *  1 [3  -1  -3] 5  3  6  7       3
 *  1  3 [-1  -3  5] 3  6  7       5
 *  1  3  -1 [-3  5  3] 6  7       5
 *  1  3  -1  -3 [5  3  6] 7       6
 *  1  3  -1  -3  5 [3  6  7]      7
 *
 *
 * 示例 2：
 *
 *
 * 输入：nums = [1], k = 1
 * 输出：[1]
 *
 *
 * &nbsp;
 *
 * 提示：
 *
 *
 * 	1 &lt;= nums.length &lt;= 105
 * 	-104&nbsp;&lt;= nums[i] &lt;= 104
 * 	1 &lt;= k &lt;= nums.length
 *
 * Approach: TODO
 * Time: O(?), Space: O(?)
 */
#include <vector>
#include <iostream>
#include <deque>
using namespace std;
class Solution {
public:
    vector<int> maxSlidingWindow(vector<int>& nums, int k) {
        // 用单调队列完成这道题
        deque<int> dq;
        vector<int> result;

        for (int i = 0; i < nums.size(); i++) {
            // 移除队列中小于当前元素的所有元素
            while (!dq.empty() && dq.back() < nums[i]) {
                dq.pop_back();
            }
            dq.push_back(nums[i]);

            // 移除超出窗口的元素
            if (i >= k && dq.front() == nums[i - k]) {
                dq.pop_front();
            }

            // 记录当前窗口的最大值
            if (i >= k - 1) {
                result.push_back(dq.front());
            }
        }
        return result;
    }
};

int main() {
    Solution sol;
    vector<int> nums = {1,3,-1,-3,5,3,6,7};
    int k = 3;
    vector<int> result = sol.maxSlidingWindow(nums, k);
    // Expected output: [3,3,5,5,6,7]
    for (int num : result) {
        printf("%d ", num);
    }
    printf("\n");
    return 0;
}