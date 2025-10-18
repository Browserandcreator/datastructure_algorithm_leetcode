// 依旧贪心算法
#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

int main() {
    int n, k;
    if (!(cin >> n >> k)) return 0;
    vector<int> a(n);
    for (int i = 0; i < n; ++i) cin >> a[i];
    vector<long long> b(n);
    for (int i = 0; i < n; ++i) cin >> b[i];

    // 行业编号 1..k
    vector<vector<long long>> groups(k + 1);
    for (int i = 0; i < n; ++i) groups[a[i]].push_back(b[i]);

    int t = 0; // 已有从业者的行业数
    vector<long long> pool; // 可调配池（候选要被说服的人）
    for (int id = 1; id <= k; ++id) {
        auto &v = groups[id];
        if (v.empty()) continue;
        ++t;
        // 保留 b 最大者，其他进入候选池
        nth_element(v.begin(), v.end()-1, v.end()); // O(sz)
        long long keep = v.back();                  // 最大的保留
        v.pop_back();
        // 其余全部进入可调配池
        for (long long x : v) pool.push_back(x);
    }

    int m = k - t; // 缺失行业数
    if (m <= 0) { cout << 0 << '\n'; return 0; }

    if ((int)pool.size() < m) {
        // 理论上不该发生（因为总人数 >= k），这里防御一下
        // 但若题目保证 1 ≤ k ≤ n，就一定能凑够。
        cout << 0 << '\n';
        return 0;
    }

    // 取 pool 中代价最小的 m 个
    nth_element(pool.begin(), pool.begin() + m, pool.end());
    long long ans = 0;
    for (int i = 0; i < m; ++i) ans += pool[i];

    cout << ans << '\n';
    return 0;
}
