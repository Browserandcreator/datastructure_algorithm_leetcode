"""
LeetCode 19. Remove Nth Node From End of List [Medium]
Link: https://leetcode.cn/problems/remove-nth-node-from-end-of-list (source: leetcode.cn)
Tags: Linked List, Two Pointers

Problem:
给你一个链表，删除链表的倒数第&nbsp;n&nbsp;个结点，并且返回链表的头结点。

&nbsp;

示例 1：


输入：head = [1,2,3,4,5], n = 2
输出：[1,2,3,5]


示例 2：


输入：head = [1], n = 1
输出：[]


示例 3：


输入：head = [1,2], n = 1
输出：[1]


&nbsp;

提示：


	链表中结点的数目为 sz
	1 &lt;= sz &lt;= 30
	0 &lt;= Node.val &lt;= 100
	1 &lt;= n &lt;= sz


&nbsp;

进阶：你能尝试使用一趟扫描实现吗？

Approach: TODO
Time: O(?), Space: O(?)
"""
# Definition for singly-linked list.
# class ListNode(object):
#     def __init__(self, val=0, next=None):
#         self.val = val
#         self.next = next
class Solution(object):
    def removeNthFromEnd(self, head, n):
        """
        :type head: Optional[ListNode]
        :type n: int
        :rtype: Optional[ListNode]
        """
        