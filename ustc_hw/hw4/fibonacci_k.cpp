#include <iostream>
using namespace std;

#define MAXSIZE 100

struct CirQueue {
    double data[MAXSIZE];
    int front, rear;
    int size;  // 队列容量 k
};

void InitQueue(CirQueue &Q, int k) {
    Q.front = Q.rear = 0;
    Q.size = k;
}

bool EnQueue(CirQueue &Q, double x) {
    // 由于队列只存 k 个元素，这里要留一个空位，所以判满条件为：
    if ((Q.rear + 1) % (Q.size + 1) == Q.front) return false;
    Q.data[Q.rear] = x;
    Q.rear = (Q.rear + 1) % (Q.size + 1);
    return true;
}

bool DeQueue(CirQueue &Q, double &x) {
    if (Q.front == Q.rear) return false; // 空
    x = Q.data[Q.front];
    Q.front = (Q.front + 1) % (Q.size + 1);
    return true;
}

double GetElem(const CirQueue &Q, int i) {
    return Q.data[(Q.front + i) % (Q.size + 1)];
}

int main() {
    int k, n;
    double maxv;
    cout << "输入 k, n, max: ";
    cin >> k >> n >> maxv;

    CirQueue Q;
    InitQueue(Q, k);

    cout << "输入前 " << k << " 项: ";
    for (int i = 0; i < k; i++) {
        double x;
        cin >> x;
        EnQueue(Q, x);
    }

    for (int r = k; r <= n; r++) {
        double fr = 2 * GetElem(Q, k - 1) - GetElem(Q, 0);

        // ---- 修改1：增加上下界反射 ----
        if (fr > maxv)
            fr = 2 * maxv - fr;  // 超上界反射
        else if (fr < 0)
            fr = -fr;            // 低于0时反射回来（可选，确保无负数）

        double tmp;
        DeQueue(Q, tmp);  // 出队一个旧元素
        EnQueue(Q, fr);   // 入队新元素
    }

    cout << "最后 k 项为: ";
    for (int i = 0; i < k; i++)
        cout << GetElem(Q, i) << " ";
    cout << endl;
    return 0;
}
