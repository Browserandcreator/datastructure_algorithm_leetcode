#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <climits>
using namespace std;

// 数组项
struct Term {
    long long c;
    int e;
};

// 链表节点
struct Node {
    long long c;  
    int e;  
    Node* next;
    Node(long long c_, int e_, Node* nx=nullptr): c(c_), e(e_), next(nx) {}
};

class Poly {
public:
    // 创建空多项式
    Poly() { head = new Node(0, INT_MAX); }
    // 拷贝构造
    Poly(const Poly& other) { head = new Node(0, INT_MAX); copyFrom(other); }
    // 拷贝赋值
    Poly& operator=(const Poly& other){ if(this!=&other){clear(); copyFrom(other);} return *this; }
    // 移动构造
    Poly(Poly&& other) noexcept { head = other.head; other.head = new Node(0, INT_MAX); }
    // 移动赋值
    Poly& operator=(Poly&& other) noexcept {
        if (this != &other) { destroy(); head = other.head; other.head = new Node(0, INT_MAX); }
        return *this;
    }
    // 析构
    ~Poly(){ destroy(); }

    // 按降幂排列插入项
    void insertTerm(long long c, int e) {
        if (c == 0) return;
        Node* prev = head; Node* cur = head->next;
        while (cur && cur->e > e) { prev = cur; cur = cur->next; }
        if (cur && cur->e == e) {
            cur->c += c;
            if (cur->c == 0) { prev->next = cur->next; delete cur; }
        } else {
            prev->next = new Node(c, e, cur);
        }
    }
    
    // 用手写数组构建链表
    void buildFromTerms(const Term* terms, int n){
        for (int i = 0; i < n; ++i) insertTerm(terms[i].c, terms[i].e);
    }

    // 多项式运算
    Poly add(const Poly& B) const {
        Poly R; Node *p=head->next, *q=B.head->next, *r=R.head;
        while (p||q){
            if (q==nullptr || (p&&p->e>q->e)) { r->next=new Node(p->c,p->e); r=r->next; p=p->next; }
            else if (p==nullptr || (q&&q->e>p->e)) { r->next=new Node(q->c,q->e); r=r->next; q=q->next; }
            else { long long c=p->c+q->c; if(c) { r->next=new Node(c,p->e); r=r->next; } p=p->next; q=q->next; }
        }
        return R;
    }
    Poly sub(const Poly& B) const {
        Poly R; Node *p=head->next, *q=B.head->next, *r=R.head;
        while (p||q){
            if (q==nullptr || (p&&p->e>q->e)) { r->next=new Node(p->c,p->e); r=r->next; p=p->next; }
            else if (p==nullptr || (q&&q->e>p->e)) { r->next=new Node(-q->c,q->e); r=r->next; q=q->next; }
            else { long long c=p->c-q->c; if(c){ r->next=new Node(c,p->e); r=r->next; } p=p->next; q=q->next; }
        }
        return R;
    }
    Poly multiply(const Poly& B) const {
        Poly R;
        for (Node* p = head->next; p; p = p->next) {
            for (Node* q = B.head->next; q; q = q->next) {
                R.insertTerm(p->c * q->c, p->e + q->e);
            }
        }
        return R;
    }

    // 求导
    Poly derivative() const {
        Poly R;
        for (Node* p = head->next; p; p = p->next) {
            if (p->e != 0) {
                R.insertTerm(p->c * p->e, p->e - 1);
            }
        }
        return R;
    }

    // 求值
    double eval(double x) const {
        double s = 0.0;
        for (Node* p=head->next; p; p=p->next) {
            s += static_cast<double>(p->c) * std::pow(x, p->e);
        }
        return s;
    }

    // 代数式输出
    string toAlgebra() const {
        Node* p = head->next;
        if (!p) return "0";
        ostringstream ss;
        bool first = true;
        while (p){
            long long c = p->c;
            int e = p->e;
            if (c == 0) { p=p->next; continue; }
            if (first) {
                if (c < 0) ss << "-";
            } else {
                ss << (c >= 0 ? "+" : "-");
            }
            long long absc = llabs(c);
            if (e == 0) {
                ss << absc;
            } else if (e == 1) {
                // 系数为1不输出
                if (absc != 1) ss << absc;
                ss << "x";
            } else {
                if (absc != 1) ss << absc;
                ss << "x^" << e;
            }
            first = false;
            p = p->next;
        }
        return ss.str();
    }

    // 系数 指数对输出
    void printPairs(ostream& os=cout) const {
        Node* p=head->next; if(!p){ os<<"0 0\n"; return; }
        bool first=true;
        while(p){ if(!first) os<<' '; os<<p->c<<' '<<p->e; first=false; p=p->next; }
        os<<'\n';
    }
    void printAlgebra(ostream& os=cout) const { os << toAlgebra() << '\n'; }

    void clear(){ Node* p=head->next; while(p){auto t=p->next; delete p; p=t;} head->next=nullptr; }

private:
    Node* head;
    void copyFrom(const Poly& other){ head->next=nullptr; Node* p=other.head->next; while(p){ insertTerm(p->c,p->e); p=p->next; } }
    void destroy(){ clear(); delete head; head=nullptr; }
};

// ------- 演示 -------
void printMenu() {
    cout << "请选择操作:\n";
    cout << "1. 输出多项式的系数-指数序列\n";
    cout << "2. 输出多项式的代数形式\n";
    cout << "3. 计算多项式在x处的值\n";
    cout << "4. 求多项式的导函数\n";
    cout << "5. 多项式加法\n";
    cout << "6. 多项式减法\n";
    cout << "7. 多项式乘法\n";
    cout << "0. 退出\n";
}

Poly inputPoly() {
    int n;
    cout << "请输入项数: ";
    cin >> n;
    Term* terms = new Term[n];
    cout << "请输入每一项的系数和指数(如 2 3 表示2x^3):\n";
    for (int i = 0; i < n; ++i) cin >> terms[i].c >> terms[i].e;
    Poly P; P.buildFromTerms(terms, n);
    delete[] terms;
    return P;
}

int main() {
    // ios::sync_with_stdio(false);
    // cin.tie(nullptr);

    cout << "请输入第一个多项式:\n";
    Poly A = inputPoly();

    double x = 0.0;
    cout << "请输入x的值: ";
    cin >> x;

    int choice;
    do {
        printMenu();
        cout << "请输入操作编号: ";
        cin >> choice;
        if (choice == 1) {
            A.printPairs();
        } else if (choice == 2) {
            A.printAlgebra();
        } else if (choice == 3) {
            cout.setf(ios::fixed);
            cout << setprecision(6) << A.eval(x) << "\n";
        } else if (choice == 4) {
            Poly dA = A.derivative();
            cout << "导函数的系数-指数序列:\n";
            dA.printPairs();
            cout << "导函数的代数形式:\n";
            dA.printAlgebra();
        } else if (choice == 5) {
            cout << "请输入另一个多项式:\n";
            Poly B = inputPoly();
            Poly C = A.add(B);
            cout << "加法结果的系数-指数序列:\n";
            C.printPairs();
            cout << "加法结果的代数形式:\n";
            C.printAlgebra();
        } else if (choice == 6) {
            cout << "请输入另一个多项式:\n";
            Poly B = inputPoly();
            Poly C = A.sub(B);
            cout << "减法结果的系数-指数序列:\n";
            C.printPairs();
            cout << "减法结果的代数形式:\n";
            C.printAlgebra();
        } else if (choice == 7) {
            cout << "请输入另一个多项式:\n";
            Poly B = inputPoly();
            Poly C = A.multiply(B);
            cout << "乘法结果的系数-指数序列:\n";
            C.printPairs();
            cout << "乘法结果的代数形式:\n";
            C.printAlgebra();
        } else if (choice == 0) {
            cout << "退出程序。\n";
        } else {
            cout << "无效的选择，请重新输入。\n";
        }
    } while (choice != 0);

    return 0;
}
