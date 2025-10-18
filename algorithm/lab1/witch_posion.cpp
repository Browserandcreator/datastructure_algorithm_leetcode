# include <vector>
# include <iostream>
# include <algorithm>
using namespace std;

int main() {
    int n;
    cin >> n;

    int total = 0;
    vector<int> nums(n);
    for(int i = 0; i < n; i++) {
        cin >> nums[i];
        total += nums[i];
    }
    // 对nums排序后得到中位数右边的位置+1即可
    sort(nums.begin(), nums.end());

    // 一半以上的元素包括最大元素，不可能有解
    if (n <= 2) {
        cout << -1;
        return 0;
    }
    // 取n/2向下取整的位置作为下标就行
    // 注意可能本来就满足要求，那就输出0即可
    // nums[n/2] < average/2 = (total + x)/2n 即可
    int x = max(0, 2 * n * nums[n/2] - total + 1);
    cout << x;
}