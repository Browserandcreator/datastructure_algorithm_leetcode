# include <bits/stdc++.h>
using namespace std;

long long maxSubArray(vector<long long> nums) {
    // cur表示以当前位置i为结尾的子数组的最大和，当i增加后，新的cur要么是原先cur+nums[i]，要么只是nums[i]
    // best表示目前为止的最大子数组和
    long long cur = nums[0];
    long long best = nums[0];
    for (int i = 0; i < nums.size(); i++){
        cur = max(cur+nums[i], nums[i]);
        best = max(best, cur);
    }
    return best;
}

int main() {
    int n;
    cin >> n;
    vector<long long> flowers(n);
    for(int i = 0; i < n; i++) {
        cin >> flowers[i];
    }
    cout << maxSubArray(flowers);
}