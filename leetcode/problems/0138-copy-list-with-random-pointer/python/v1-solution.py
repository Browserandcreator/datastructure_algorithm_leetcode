"""
LeetCode 138. Copy List with Random Pointer [Medium]
Link: https://leetcode.cn/problems/copy-list-with-random-pointer (source: leetcode.cn)
Tags: Hash Table, Linked List

Problem:
给你一个长度为 n 的链表，每个节点包含一个额外增加的随机指针 random ，该指针可以指向链表中的任何节点或空节点。

构造这个链表的&nbsp;深拷贝。&nbsp;深拷贝应该正好由 n 个 全新 节点组成，其中每个新节点的值都设为其对应的原节点的值。新节点的 next 指针和 random 指针也都应指向复制链表中的新节点，并使原链表和复制链表中的这些指针能够表示相同的链表状态。复制链表中的指针都不应指向原链表中的节点 。

例如，如果原链表中有 X 和 Y 两个节点，其中 X.random --&gt; Y 。那么在复制链表中对应的两个节点 x 和 y ，同样有 x.random --&gt; y 。

返回复制链表的头节点。

用一个由&nbsp;n&nbsp;个节点组成的链表来表示输入/输出中的链表。每个节点用一个&nbsp;[val, random_index]&nbsp;表示：


	val：一个表示&nbsp;Node.val&nbsp;的整数。
	random_index：随机指针指向的节点索引（范围从&nbsp;0&nbsp;到&nbsp;n-1）；如果不指向任何节点，则为&nbsp;&nbsp;null&nbsp;。


你的代码 只 接受原链表的头节点 head 作为传入参数。

&nbsp;

示例 1：




输入：head = [[7,null],[13,0],[11,4],[10,2],[1,0]]
输出：[[7,null],[13,0],[11,4],[10,2],[1,0]]


示例 2：




输入：head = [[1,1],[2,1]]
输出：[[1,1],[2,1]]


示例 3：




输入：head = [[3,null],[3,0],[3,null]]
输出：[[3,null],[3,0],[3,null]]


&nbsp;

提示：


	0 &lt;= n &lt;= 1000
	-104&nbsp;&lt;= Node.val &lt;= 104
	Node.random&nbsp;为&nbsp;null 或指向链表中的节点。


&nbsp;

Approach: TODO
Time: O(?), Space: O(?)
"""
"""
# Definition for a Node.
class Node:
    def __init__(self, x, next=None, random=None):
        self.val = int(x)
        self.next = next
        self.random = random
"""

class Solution(object):
    def copyRandomList(self, head):
        """
        :type head: Node
        :rtype: Node
        """
        