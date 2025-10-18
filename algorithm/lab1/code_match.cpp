#include <vector>
#include <iostream>
#include <unordered_map>
#include <algorithm>
using namespace std;

int main() {
    long long n;
    cin >> n;
    unordered_map<long long, long long> counts;
    for (long long i = 0; i < n; i++) {
        long long temp; cin >> temp;
        counts[temp] ++;
    }
    long long E = 0;
    long long S = 0;
    for (auto &kv : counts) {
        long long c = kv.second; //c为频数
        if (c >= 3) E += (c - 2);
        else if (c == 1) S += 1;
    }
    // 细节是需要向上取整，2 3 4 5 6至少需要改3个
    cout << max(E, (S+1)/2);
    return 0;
}