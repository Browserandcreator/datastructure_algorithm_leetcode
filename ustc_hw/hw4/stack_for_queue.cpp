#include <iostream>
using namespace std;

template <typename T>
struct Node {
    T data;
    Node* next;
    Node(const T& d = T(), Node* n = nullptr) : data(d), next(n) {}
};

template <typename T>
class CLinkQueue {
public:
    CLinkQueue() { init(); }
    ~CLinkQueue() { clear(); }

    // 初始化：构造空队列
    void init() {
        rear = new Node<T>();  // 头结点
        rear->next = rear;     // 自环
    }

    bool empty() const {
        Node<T>* head = rear->next;
        return head->next == head;   // 或 rear == head
    }

    // 入队：尾插
    void push(const T& x) {
        Node<T>* head = rear->next;
        Node<T>* p = new Node<T>(x, head); // p->next = head
        rear->next = p;                    // 链入尾部
        rear = p;                          // 更新队尾
    }

    // 出队：从队头删除，成功返回 true 并用 x 带回元素
    bool pop(T& x) {
        Node<T>* head = rear->next;
        if (head->next == head) return false;   // 空

        Node<T>* s = head->next;                // 队头
        x = s->data;
        head->next = s->next;                   // 头删

        if (s == rear)                          // 删的是最后一个元素
            rear = head;                        // 恢复空队形态

        delete s;
        return true;
    }

    // 取队头元素（不删除），空队列抛出异常或自行处理
    const T& front() const {
        Node<T>* head = rear->next;
        if (head->next == head) throw runtime_error("queue is empty");
        return head->next->data;
    }

private:
    Node<T>* rear;  // 唯一指针：指向队尾元素（空时指向头结点）

    void clear() {
        Node<T>* head = rear->next;
        while (head->next != head) {
            Node<T>* s = head->next;
            head->next = s->next;
            delete s;
        }
        delete head; // head == rear
        rear = nullptr;
    }
};

// 示例
int main() {
    CLinkQueue<int> q;
    for (int i = 1; i <= 5; ++i) q.push(i);

    cout << "front = " << q.front() << "\n"; // 1

    int x;
    while (q.pop(x)) cout << x << " ";       // 1 2 3 4 5
    cout << "\n" << (q.empty() ? "empty" : "not empty") << "\n";
    return 0;
}
