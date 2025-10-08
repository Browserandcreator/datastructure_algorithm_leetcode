/*
 * LeetCode 1338. Reduce Array Size to The Half [Medium]
 * Link: https://leetcode.cn/problems/reduce-array-size-to-the-half (source: leetcode.cn)
 * Tags: Greedy, Array, Hash Table, Sorting, Heap (Priority Queue)
 *
 * Problem:
 * 给你一个整数数组&nbsp;arr。你可以从中选出一个整数集合，并删除这些整数在数组中的每次出现。
 *
 * 返回&nbsp;至少&nbsp;能删除数组中的一半整数的整数集合的最小大小。
 *
 * &nbsp;
 *
 * 示例 1：
 *
 *
 * 输入：arr = [3,3,3,3,5,5,5,2,2,7]
 * 输出：2
 * 解释：选择 {3,7} 使得结果数组为 [5,5,5,2,2]、长度为 5（原数组长度的一半）。
 * 大小为 2 的可行集合有 {3,5},{3,2},{5,2}。
 * 选择 {2,7} 是不可行的，它的结果数组为 [3,3,3,3,5,5,5]，新数组长度大于原数组的二分之一。
 *
 *
 * 示例 2：
 *
 *
 * 输入：arr = [7,7,7,7,7,7]
 * 输出：1
 * 解释：我们只能选择集合 {7}，结果数组为空。
 *
 *
 * &nbsp;
 *
 * 提示：
 *
 *
 * 	1 &lt;= arr.length &lt;= 105
 * 	arr.length&nbsp;为偶数
 * 	1 &lt;= arr[i] &lt;= 105
 *
 * Approach: TODO
 * Time: O(?), Space: O(?)
 */

// 使用桶计数实现
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <iostream>
using namespace std;

class Solution {
public:
    int minSetSize(vector<int>& arr) {
        unordered_map<int, int> count;
        for (int num : arr) {
            count[num]++;
        }
        vector<int> freq;
        for (auto& [num, cnt] : count) {
            freq.push_back(cnt);
        }
        sort(freq.rbegin(), freq.rend());
        int removed = 0, sets = 0;
        for (int i = 0; i < freq.size(); i++) {
            removed += freq[i];
            sets++;
            if (removed >= arr.size() / 2) {
                break;
            }
        }
        return sets;
    }
};

int main() {
    Solution sol;
    vector<int> arr = {3,3,3,3,5,5,5,2,2,7};
    int result = sol.minSetSize(arr);
    cout << result << endl; // Expected output: 2
    return 0;
}