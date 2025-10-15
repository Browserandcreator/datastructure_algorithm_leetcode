#include <iostream>
#include <stack>
using namespace std;

int main() {
    stack<char> S;
    string str;
    char ch;

    cout << "请输入字符串（以'@'结束）: ";
    while (cin.get(ch) && ch != '@') {
        str.push_back(ch);
    }

    int n = str.length();
    for (int i = 0; i < n / 2; ++i)
        S.push(str[i]);

    int start = (n % 2 == 0) ? n / 2 : n / 2 + 1;
    bool isPalindrome = true;

    for (int i = start; i < n; ++i) {
        if (S.empty() || S.top() != str[i]) {
            isPalindrome = false;
            break;
        }
        S.pop();
    }

    if (isPalindrome && S.empty())
        cout << "是回文" << endl;
    else
        cout << "不是回文" << endl;

    return 0;
}
