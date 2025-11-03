#include <bits/stdc++.h>
using namespace std;

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    TreeNode(int v): val(v), left(nullptr), right(nullptr) {}
};

TreeNode* build(int L, int R,
                const vector<int>& level,
                const vector<int>& inorder,
                const unordered_map<int,int>& pos) {
    if (L > R || level.empty()) return nullptr;

    int root_val = level[0];
    int k = pos.at(root_val);
    TreeNode* root = new TreeNode(root_val);

    vector<int> left_level, right_level;
    left_level.reserve(level.size());
    right_level.reserve(level.size());

    for (size_t i = 1; i < level.size(); ++i) {
        int x = level[i];
        int idx = pos.at(x);
        if (L <= idx && idx <= k - 1) left_level.push_back(x);
        else if (k + 1 <= idx && idx <= R) right_level.push_back(x);
    }

    root->left  = build(L, k - 1, left_level, inorder, pos);
    root->right = build(k + 1, R, right_level, inorder, pos);
    return root;
}

TreeNode* buildTreeFromInAndLevel(const vector<int>& inorder,
                                  const vector<int>& levelorder) {
    if (inorder.size() != levelorder.size() || inorder.empty())
        return nullptr;
    unordered_set<int> s1(inorder.begin(), inorder.end());
    unordered_set<int> s2(levelorder.begin(), levelorder.end());
    if (s1 != s2) return nullptr; // 值集合不一致，非法

    unordered_map<int,int> pos;
    pos.reserve(inorder.size() * 2);
    for (int i = 0; i < (int)inorder.size(); ++i) pos[inorder[i]] = i;

    return build(0, (int)inorder.size() - 1, levelorder, inorder, pos);
}

// 辅助：前序打印
void preorder(TreeNode* r) {
    if (!r) return;
    cout << r->val << " ";
    preorder(r->left);
    preorder(r->right);
}

int main() {
    vector<int> inorder    = {4,2,5,1,6,3,7};
    vector<int> levelorder = {1,2,3,4,5,6,7};
    TreeNode* root = buildTreeFromInAndLevel(inorder, levelorder);
    preorder(root); // 期望：1 2 4 5 3 6 7
    cout << "\n";
    return 0;
}
