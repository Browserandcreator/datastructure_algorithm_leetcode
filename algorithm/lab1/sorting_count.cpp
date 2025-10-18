// 计算输入为n时，各种排序算法的最坏情况的比较次数，返回最大值
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
using namespace std;

long long worstMergeComparisonsClosed(long long n) {
    if (n <= 1) return 0;
    long long k = ceil(log2((double)n));
    return n * k - (1LL << k) + 1;
}


int main() {
    long long n;
    cin >> n;

    // 冒泡排序的最坏情况比较次数
    long long bubble_sort_comparisons = n * (n - 1) / 2;

    // 选择排序的最坏情况比较次数
    long long selection_sort_comparisons = n * (n - 1) / 2;

    // 插入排序的最坏情况比较次数
    long long insertion_sort_comparisons = n * (n - 1) / 2;

    // 归并排序的最坏情况比较次数
    long long merge_sort_comparisons = worstMergeComparisonsClosed(n);

    long long min_comparisons = min({bubble_sort_comparisons, selection_sort_comparisons, insertion_sort_comparisons, merge_sort_comparisons});

    cout  << min_comparisons << endl;

    return 0;
}