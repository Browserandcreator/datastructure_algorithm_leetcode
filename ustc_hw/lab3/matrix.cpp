#include <bits/stdc++.h>
using namespace std;

/*** 十字链表结点 ***/
struct OLNode {
    int r, c;          // 行、列（1-based）
    long long v;       // 值
    OLNode *right;     // 行指针（同一行按列递增）
    OLNode *down;      // 列指针（同一列按行递增）
    OLNode(int r_, int c_, long long v_)
        : r(r_), c(c_), v(v_), right(nullptr), down(nullptr) {}
};

/*** 稀疏矩阵：十字链表表示 ***/
class CrossSparseMatrix {
public:
    int rows, cols;
    vector<OLNode*> rhead;  // 各行头指针
    vector<OLNode*> chead;  // 各列头指针
    size_t nnz;

    CrossSparseMatrix(int R=0, int C=0)
        : rows(R), cols(C), rhead(R+1,nullptr), chead(C+1,nullptr), nnz(0) {}

    ~CrossSparseMatrix(){ clear(); }

    void clear(){
        // 逐行释放即可
        for (int i=1;i<=rows;i++){
            OLNode* p = rhead[i];
            while(p){
                OLNode* nxt = p->right;
                delete p;
                p = nxt;
            }
            rhead[i]=nullptr;
        }
        // 列头只保存指针，不要重复释放
        fill(chead.begin(), chead.end(), nullptr);
        nnz=0;
    }

    // 插入/更新：在 (r,c) 累加 val；若结果为 0 则删除该结点
    void insert_add(int r, int c, long long val){
        if (val==0) return;
        // 1) 找到行内插入位置（按列有序）
        OLNode *pr = nullptr, *p = rhead[r];
        while(p && p->c < c){ pr = p; p = p->right; }

        if (p && p->c == c){
            // 已存在，累加
            p->v += val;
            if (p->v == 0){
                // 行链删除 p
                if (pr) pr->right = p->right;
                else    rhead[r] = p->right;
                // 同时在列链删除 p
                OLNode *pc=nullptr, *q = chead[c];
                while(q && q!=p){ pc=q; q=q->down; }
                if (q){ // q==p
                    if (pc) pc->down = q->down;
                    else    chead[c] = q->down;
                }
                delete p; nnz--;
            }
            return;
        }

        // 新结点
        OLNode* node = new OLNode(r,c,val);

        // 2) 挂到行链
        if (pr){ node->right = pr->right; pr->right = node; }
        else   { node->right = rhead[r];  rhead[r]  = node; }

        // 3) 挂到列链（按行有序）
        OLNode *pc = nullptr, *q = chead[c];
        while(q && q->r < r){ pc = q; q = q->down; }
        if (pc){ node->down = pc->down; pc->down = node; }
        else   { node->down = chead[c]; chead[c] = node; }

        nnz++;
    }

    // 由三元组构造： triplets = { {r,c,val}, ... } ；1-based
    static CrossSparseMatrix FromTriplets(
        int R, int C, const vector<tuple<int,int,long long>>& triplets)
    {
        CrossSparseMatrix A(R,C);
        for (auto [r,c,v] : triplets) A.insert_add(r,c,v);
        return A;
    }

    // A + B
    CrossSparseMatrix add(const CrossSparseMatrix& B) const {
        assert(rows==B.rows && cols==B.cols);
        CrossSparseMatrix C(rows, cols);
        // 把 A 的所有结点加入 C
        for (int i=1;i<=rows;i++){
            for (OLNode* p=rhead[i]; p; p=p->right){
                C.insert_add(p->r, p->c, p->v);
            }
        }
        // 把 B 的所有结点加入 C
        for (int i=1;i<=B.rows;i++){
            for (OLNode* p=B.rhead[i]; p; p=p->right){
                C.insert_add(p->r, p->c, p->v);
            }
        }
        return C;
    }

    // A - B
    CrossSparseMatrix sub(const CrossSparseMatrix& B) const {
        assert(rows==B.rows && cols==B.cols);
        CrossSparseMatrix C(rows, cols);
        for (int i=1;i<=rows;i++){
            for (OLNode* p=rhead[i]; p; p=p->right){
                C.insert_add(p->r, p->c, p->v);
            }
        }
        for (int i=1;i<=B.rows;i++){
            for (OLNode* p=B.rhead[i]; p; p=p->right){
                C.insert_add(p->r, p->c, -p->v);
            }
        }
        return C;
    }

    // 按普通矩阵格式打印
    void printDense(ostream& os=cout) const {
        for (int i=1;i<=rows;i++){
            OLNode* p = rhead[i];
            for (int j=1;j<=cols;j++){
                long long x = 0;
                if (p && p->c==j){ x=p->v; p=p->right; }
                os << setw(4) << x;
            }
            os << '\n';
        }
    }
};

/*** 示例主程序
输入格式（可自行改动）：
R C
nnzA
r c v   (共 nnzA 行，1-based)
nnzB
r c v   (共 nnzB 行)
输出：
A+B
A-B
***/
int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int R, C;
    if(!(cin >> R >> C)) return 0;

    int nzA; cin >> nzA;
    vector<tuple<int,int,long long>> TA;
    TA.reserve(nzA);
    for (int i=0;i<nzA;i++){
        int r,c; long long v; cin >> r >> c >> v;
        TA.emplace_back(r,c,v);
    }
    int nzB; cin >> nzB;
    vector<tuple<int,int,long long>> TB;
    TB.reserve(nzB);
    for (int i=0;i<nzB;i++){
        int r,c; long long v; cin >> r >> c >> v;
        TB.emplace_back(r,c,v);
    }

    CrossSparseMatrix A = CrossSparseMatrix::FromTriplets(R,C,TA);
    CrossSparseMatrix B = CrossSparseMatrix::FromTriplets(R,C,TB);

    auto S = A.add(B);
    auto D = A.sub(B);

    cout << "A + B:\n";
    S.printDense();
    cout << "A - B:\n";
    D.printDense();
    return 0;
}
