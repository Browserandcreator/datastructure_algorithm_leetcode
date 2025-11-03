# include <vector>
# include <iostream>
using namespace std;

vector<int> Counting_sort(vector<int>nums, int k) {
    // 非原地排序，空间开支O(n+k)
    int n = nums.size();
    vector<int> res(n);
    vector<int> cnt(k+1, 0);

    for (int x: nums) cnt[x] ++ ;
    for (int i = 1; i <= k; i++) {
        cnt[i] += cnt[i-1];
    }
    
    // 注意边界, 数值最大为k，计数桶大小为k+1
    for (int j = n-1; j >= 0; j--) {
        res[--cnt[nums[j]]] = nums[j];    // <=nums[j]的最后一位一定可以是nums[j]，但是此处下标需要是<=nums[j]的元素个数-1
    }
    return res;
}

void printArray(vector<int>nums) {
    for (int x : nums) cout << x << " ";
}

int main() {
    // 生成测试数据
    vector<int> nums = {4, 2, 2, 8, 3, 3, 1};
    vector<int> res = Counting_sort(nums, 8); 
    printArray(res);
}