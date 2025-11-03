// 找到节点值之和最大的子树
#include <iostream>
using namespace std;

struct TreeNode {
    int val;
    TreeNode *left;
    TreeNode *right;
    TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
};
int maxSum = INT_MIN;

int findMaxSumSubtree(TreeNode* root) {
    if (!root) return 0;
    int leftSum = findMaxSumSubtree(root->left);
    int rightSum = findMaxSumSubtree(root->right);
    int currentSum = root->val + leftSum + rightSum;
    maxSum = max(maxSum, currentSum);
    return currentSum;
}

int getMaxSumSubtree(TreeNode* root) {
    findMaxSumSubtree(root);
    return maxSum;
}

int main() {
    // 示例用法
    TreeNode* root = new TreeNode(1);
    root->left = new TreeNode(-2);
    root->right = new TreeNode(3);
    root->left->left = new TreeNode(4);
    root->left->right = new TreeNode(5);
    root->right->right = new TreeNode(-6);

    int result = getMaxSumSubtree(root);
    // 输出结果
    cout << "The maximum sum of any subtree is: " << result << endl;
    return 0;
}