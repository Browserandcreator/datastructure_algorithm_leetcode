#include <iostream>
using namespace std;

const int MAXLEN = 10000;   // 允许的最大串长
const int MAXK   = MAXLEN;  // 最坏情况下每个字符都不同

struct SeqStr {
    char data[MAXLEN];
    int  len;
};

// 初始化为空串
void StrInit(SeqStr &s) { s.len = 0; }

// 读入一整行（含空格、标点），不含结尾'\n'
void StrReadLine(SeqStr &s) {
    StrInit(s);
    char ch;
    // 跳过行首可能的 '\n'
    while (cin.get(ch)) {
        if (ch == '\n') continue;  // 若题目保证一行输入，可去掉这行
        s.data[s.len++] = ch;
        break;
    }
    // 继续读到行末
    while (cin.get(ch) && ch != '\n') {
        if (s.len < MAXLEN) s.data[s.len++] = ch;
    }
}

// 求不同字符及其个数（按首次出现顺序）
void CountKinds(const SeqStr &s, char kinds[], int cnt[], int &diff) {
    diff = 0;
    for (int i = 0; i < s.len; ++i) {
        char c = s.data[i];
        // 在线性表 kinds[0..diff-1] 中查找
        int j = 0;
        while (j < diff && kinds[j] != c) ++j;
        if (j == diff) {       // 新字符
            kinds[diff] = c;
            cnt[diff]   = 1;
            ++diff;
        } else {               // 已出现过
            ++cnt[j];
        }
    }
}

int main() {
    SeqStr s;
    cout << "请输入一行串 s：";
    StrReadLine(s);

    char kinds[MAXK];
    int  cnt[MAXK];
    int  diff = 0;

    CountKinds(s, kinds, cnt, diff);

    cout << "不同字符总数 = " << diff << "\n";
    cout << "各字符计数：\n";
    for (int i = 0; i < diff; ++i) {
        // 为了可见性，空格等不可见字符可特殊显示
        if (kinds[i] == ' ') cout << "' '";
        else if (kinds[i] == '\t') cout << "'\\t'";
        else cout << "'" << kinds[i] << "'";
        cout << " : " << cnt[i] << "\n";
    }
    return 0;
}
