#include <bits/stdc++.h>
using namespace std;
using int64 = long long;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    // n个居民，k个行业
    int n, k;
    cin >> n >> k;
    vector<int> a(n);              // a[i]：第 i 人所属行业（1..k）
    for (int i = 0; i < n; ++i) cin >> a[i];
    vector<long long> b(n);        // b[i]：第 i 人转行业代价
    for (int i = 0; i < n; ++i) cin >> b[i];

    // 1) 分组：按行业收集该行业内所有人的 b
    // groups[id]：属于这个行业的所有人的 代价 列表
    vector<vector<long long>> groups(k + 1);
    for (int i = 0; i < n; ++i) {
        groups[a[i]].push_back(b[i]);
    }

    int have = 0;                  // 已经“有人”的行业数量
    vector<long long> pool;        // 可调配池：每个行业除“最大 b 的那一个”以外的其余人

    // 2) 对每个行业：保留 开支b 最大者，其余进 pool
    // 遍历id = 1..k
    for (int id = 1; id <= k; ++id) {
        auto &v = groups[id];
        // 这个行业没人，留待后面填
        if (v.empty()) continue;   
        // 这个行业已经有人了（我们会保留其中 b 最大的那位）
        ++have;          
        // 找到最大值，保留其余进入 pool       
        nth_element(v.begin(), v.end() - 1, v.end());
        long long keep = v.back(); 
        // 去掉最大值，其余入 pool
        v.pop_back();      
        for (long long x : v) pool.push_back(x);
        // （注意：keep 并不需要记录，它只是被“固定”留在本行业，不进入 pool）
    }

    int missing = k - have;        // 缺失行业数：需要转入这么多人
    if (missing <= 0) {            // 已覆盖所有行业
        cout << 0 << '\n';
        return 0;
    }

    // pool 里的人都“可转行”，取其中代价最小的 missing 个
    // 方法 A：nth_element 取前 missing 个，无需完整排序
    if ((int)pool.size() < missing) {
        // 理论上不会发生，因为 n >= k，且每个已有行业至少留 1，其余总人数 >= 缺失行业数
        // 这里写成防御式：若真的数据异常，尽最大能力做（题面保证足够）
        sort(pool.begin(), pool.end());
    } else {
        // 将前 missing 个最小值排到最前面
        nth_element(pool.begin(), pool.begin() + missing, pool.end());
    }

    long long ans = 0;
    for (int i = 0; i < missing && i < (int)pool.size(); ++i) {
        ans += pool[i];
    }
    cout << ans << '\n';
    return 0;
}
