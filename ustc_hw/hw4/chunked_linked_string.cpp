#include <iostream>
#include <cstring>
using namespace std;

const int CHUNKSIZE = 8;   // 例子：每块 8 个字符，可根据题意修改

struct Chunk {
    char  ch[CHUNKSIZE];
    int   len;             // 1..CHUNKSIZE
    Chunk *next;
    Chunk(): len(0), next(nullptr) {}
};

struct LString {
    Chunk *head;
    LString(): head(nullptr) {}
};

// 创建一个仅含字符串 s 的链串（辅助构造/测试）
LString makeFromCStr(const char* s) {
    LString T;
    int n = (int)strlen(s);
    if (n == 0) return T;
    Chunk *tail = nullptr;
    for (int i = 0; i < n; ) {
        Chunk *node = new Chunk();
        int take = min(CHUNKSIZE, n - i);
        memcpy(node->ch, s + i, take);
        node->len = take;
        if (!T.head) T.head = node;
        else tail->next = node;
        tail = node;
        i += take;
    }
    return T;
}

Chunk* tail(Chunk* h) {
    if (!h) return nullptr;
    while (h->next) h = h->next;
    return h;
}

// 把串 s 插入到串 t 中字符 x 的第一次出现之后；若不存在 x，则拼接到末尾。
// 插入以“拼接节点”为主；只有插入点在块中部时，才把“后半段”复制到新块。
void insertAfterChar(LString &t, LString &s, char x) {
    if (!s.head) return;               // 空 s，直接返回
    if (!t.head) {                     // 空 t => 直接接到末尾（其实就是置为 s）
        t.head = s.head;
        s.head = nullptr;
        return;
    }

    Chunk *p = t.head;
    while (p) {
        // 在当前块 p 内查找 x
        int pos = -1;
        for (int i = 0; i < p->len; ++i) {
            if (p->ch[i] == x) { pos = i; break; }
        }
        if (pos != -1) {
            Chunk *tailS = tail(s.head);
            if (pos == p->len - 1) {
                // 插入点恰在块尾：只改指针
                tailS->next = p->next;
                p->next     = s.head;
            } else {
                // 插入点在块中部：把后半段切出到新块 q
                Chunk *q = new Chunk();
                int rightLen = p->len - (pos + 1);
                memcpy(q->ch, p->ch + pos + 1, rightLen);
                q->len  = rightLen;
                q->next = p->next;

                p->len  = pos + 1;         // p 保留到 x 的部分
                p->next = s.head;
                tailS->next = q;
            }
            s.head = nullptr;              // s 并入 t
            return;
        }
        p = p->next;
    }

    // 整个 t 未含 x：尾接
    Chunk *tailT = tail(t.head);
    tailT->next = s.head;
    s.head = nullptr;
}

// 打印串（调试用）
void printLString(const LString &T) {
    for (Chunk *p = T.head; p; p = p->next) {
        cout.write(p->ch, p->len);
    }
    cout << "\n";
}

// 简单演示
int main() {
    // t: "aaaa|bbbb|cccc|dddd" 每 | 表示可能跨块
    LString t = makeFromCStr("aaaabbbbccccdddd");
    LString s = makeFromCStr("XYZ");

    cout << "T = "; printLString(t);
    cout << "S = "; printLString(s);

    insertAfterChar(t, s, 'b');   // 在 t 中第一个 'b' 之后插入 S
    cout << "After insert('b'): ";
    printLString(t);

    // 若想再次使用 s，请重新构造，因为 s 已被并入 t
    return 0;
}
