# include <bits/stdc++.h>
using namespace std;

struct TreeNode {
     int val;
     TreeNode *left;
     TreeNode *right;
     TreeNode() : val(0), left(nullptr), right(nullptr) {}
     TreeNode(int x) : val(x), left(nullptr), right(nullptr) {}
     TreeNode(int x, TreeNode *left, TreeNode *right) : val(x), left(left), right(right) {}
};

void buildPost(const vector<int>& preorder, int pl, int pr,
               const vector<int>& inorder, int il, int ir,
               unordered_map<int,int>& pos,
               vector<int>& postorder) {
    if (pl >= pr || il >= ir) return;

    int rootVal = preorder[pl];
    int k = pos[rootVal];       // 根在中序中的位置
    int leftLen = k - il;       // 左子树大小

    // 递归左、右子树
    buildPost(preorder, pl + 1, pl + 1 + leftLen, inorder, il, k, pos, postorder);
    buildPost(preorder, pl + 1 + leftLen, pr, inorder, k + 1, ir, pos, postorder);

    postorder.push_back(rootVal);  // 最后加入根
}

vector<int> getPostorder(vector<int>& preorder, vector<int>& inorder) {
    int n = (int)preorder.size();
    unordered_map<int,int> pos;
    pos.reserve(n * 2);
    for (int i = 0; i < n; ++i) pos[inorder[i]] = i;
    vector<int> postorder;
    postorder.reserve(n);
    buildPost(preorder, 0, n, inorder, 0, n, pos, postorder);
    return postorder;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    string inorder_s, preorder_s;
    if (!(cin >> inorder_s)) return 0;
    if (!(cin >> preorder_s)) return 0;

    // 基本合法性检查（题目保证合法，可选）
    if (inorder_s.size() != preorder_s.size()) return 0;

    // 将字符装入 int 向量（直接用字符的 ASCII）
    vector<int> inorder, preorder;
    inorder.reserve(inorder_s.size());
    preorder.reserve(preorder_s.size());
    for (char c : inorder_s) inorder.push_back((unsigned char)c);
    for (char c : preorder_s) preorder.push_back((unsigned char)c);

    vector<int> post = getPostorder(preorder, inorder);

    // 按字符输出
    string out;
    out.reserve(post.size());
    for (int x : post) out.push_back((char)x);
    cout << out << '\n';
    return 0;
}
