/*
 * LeetCode 2462. Total Cost to Hire K Workers [Medium]
 * Link: https://leetcode.cn/problems/total-cost-to-hire-k-workers (source: leetcode.cn)
 * Tags: Array, Two Pointers, Simulation, Heap (Priority Queue)
 *
 * Problem:
 * 给你一个下标从 0&nbsp;开始的整数数组&nbsp;costs&nbsp;，其中&nbsp;costs[i]&nbsp;是雇佣第 i&nbsp;位工人的代价。
 *
 * 同时给你两个整数&nbsp;k 和&nbsp;candidates&nbsp;。我们想根据以下规则恰好雇佣&nbsp;k&nbsp;位工人：
 *
 *
 * 	总共进行&nbsp;k&nbsp;轮雇佣，且每一轮恰好雇佣一位工人。
 * 	在每一轮雇佣中，从最前面 candidates&nbsp;和最后面 candidates&nbsp;人中选出代价最小的一位工人，如果有多位代价相同且最小的工人，选择下标更小的一位工人。
 *
 * 		比方说，costs = [3,2,7,7,1,2] 且&nbsp;candidates = 2&nbsp;，第一轮雇佣中，我们选择第&nbsp;4&nbsp;位工人，因为他的代价最小&nbsp;[3,2,7,7,1,2]&nbsp;。
 * 		第二轮雇佣，我们选择第&nbsp;1&nbsp;位工人，因为他们的代价与第&nbsp;4&nbsp;位工人一样都是最小代价，而且下标更小，[3,2,7,7,2]&nbsp;。注意每一轮雇佣后，剩余工人的下标可能会发生变化。
 *
 *
 * 	如果剩余员工数目不足 candidates&nbsp;人，那么下一轮雇佣他们中代价最小的一人，如果有多位代价相同且最小的工人，选择下标更小的一位工人。
 * 	一位工人只能被选择一次。
 *
 *
 * 返回雇佣恰好&nbsp;k&nbsp;位工人的总代价。
 *
 * &nbsp;
 *
 * 示例 1：
 *
 * 输入：costs = [17,12,10,2,7,2,11,20,8], k = 3, candidates = 4
 * 输出：11
 * 解释：我们总共雇佣 3 位工人。总代价一开始为 0 。
 * - 第一轮雇佣，我们从 [17,12,10,2,7,2,11,20,8] 中选择。最小代价是 2 ，有两位工人，我们选择下标更小的一位工人，即第 3 位工人。总代价是 0 + 2 = 2 。
 * - 第二轮雇佣，我们从 [17,12,10,7,2,11,20,8] 中选择。最小代价是 2 ，下标为 4 ，总代价是 2 + 2 = 4 。
 * - 第三轮雇佣，我们从 [17,12,10,7,11,20,8] 中选择，最小代价是 7 ，下标为 3 ，总代价是 4 + 7 = 11 。注意下标为 3 的工人同时在最前面和最后面 4 位工人中。
 * 总雇佣代价是 11 。
 *
 *
 * 示例 2：
 *
 * 输入：costs = [1,2,4,1], k = 3, candidates = 3
 * 输出：4
 * 解释：我们总共雇佣 3 位工人。总代价一开始为 0 。
 * - 第一轮雇佣，我们从 [1,2,4,1] 中选择。最小代价为 1 ，有两位工人，我们选择下标更小的一位工人，即第 0 位工人，总代价是 0 + 1 = 1 。注意，下标为 1 和 2 的工人同时在最前面和最后面 3 位工人中。
 * - 第二轮雇佣，我们从 [2,4,1] 中选择。最小代价为 1 ，下标为 2 ，总代价是 1 + 1 = 2 。
 * - 第三轮雇佣，少于 3 位工人，我们从剩余工人 [2,4] 中选择。最小代价是 2 ，下标为 0 。总代价为 2 + 2 = 4 。
 * 总雇佣代价是 4 。
 *
 *
 * &nbsp;
 *
 * 提示：
 *
 *
 * 	1 &lt;= costs.length &lt;= 105 
 * 	1 &lt;= costs[i] &lt;= 105
 * 	1 &lt;= k, candidates &lt;= costs.length
 *
 * Approach: TODO
 * Time: O(?), Space: O(?)
 */
# include <bits/stdc++.h>
using namespace std;

class Solution {
public:
    long long totalCost(vector<int>& costs, int k, int candidates) {
        // 维护两个小根堆，内部元素类型为<cost, index>
        int n = (int) costs.size();
        using P = pair<int, int>;
        // 左右各一个小根堆，两者不会重叠
        priority_queue<P, vector<P>, greater<P>> L, R;

        // i为下一个放入左堆的候选人，j为右堆候选人
        // i，j当前指向的元素都没放进堆里，i==j时随便哪个放一下都行
        int i = 0, j = n-1;
        // 维护堆长度<= candidates
        for(int t = 0; t < candidates && i <= j; ++t) {
            L.emplace(costs[i], i);
            i++;
        }
        // 当i == j时，恰好还可以放入一个元素

        for(int t = 0; t < candidates && i <= j; ++t) {
            R.emplace(costs[j], j);
            j--;
        }

        long long ans = 0;
        for (int hire = 0; hire < k; hire++) {
            // 决定从某一侧选取
            bool takeleft;
            if (L.empty() && R.empty()) break;
            else if (L.empty()) takeleft = false;
            else if (R.empty()) takeleft = true;
            else {
                // 比较两堆堆顶
                auto [cl, il] = L.top();
                auto [cr, ir] = R.top();
                if (cl < cr) takeleft = true;
                else if (cl > cr) takeleft = false;
                // cost相同时取index较小项，原则上index不会重复
                else takeleft = (il <= ir);
            }

            // 从左边开始
            if (takeleft) {
                auto [c, idx] = L.top(); 
                L.pop();
                ans += c;

                // 更新堆，注意i在
                if (i <= j) {
                    L.emplace(costs[i], i);
                    ++i;
                }
            }

            else {
                auto [c, idx] = R.top(); 
                R.pop();
                ans += c;

                // 更新堆，注意i在
                if (i <= j) {
                    R.emplace(costs[j], j);
                    --j;
                }
            }

        }
        return ans;
    }
};

int main() {
    Solution sol;
    vector<int> costs = {17,12,10,2,7,2,11,20,8};
    int k = 3;
    int candidates = 4;
    long long result = sol.totalCost(costs, k, candidates);
    cout << result << endl; // Expected output: 11
    return 0;
}