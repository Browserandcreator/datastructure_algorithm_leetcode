"""
public functions
"""

# !/usr/bin/env python
# coding=utf-8


def cmp_str(element1, element2):
    """
    compare number in str format correctley
    """
    try:
        val1, val2 = int(element1), int(element2)
        if val1 < val2:
            return -1
        elif val1 > val2:
            return 1
        else:
            return 0
    except ValueError:
        if element1 < element2:
            return -1
        elif element1 > element2:
            return 1
        else:
            return 0


def qid_to_key(value_list, sep=';'):
    """convert qid list to str key
    value (splited by sep). This fuction is value safe, which means
    value_list will not be changed.
    return str list.
    """
    return sep.join(value_list)


def list_to_str(value_list, cmpfun=None, sep=';'):
    """covert sorted str list (sorted by cmpfun) to str
    value (splited by sep). This fuction is value safe, which means
    value_list will not be changed.
    return str list.
    """
    temp = value_list[:]
    if cmpfun is not None:
        from functools import cmp_to_key
        temp.sort(key=cmp_to_key(cmpfun))
    else:
        temp.sort()
    return sep.join(temp)


def get_num_list_from_str(stemp):
    """
    if float(stemp) works, return [stemp]
    else return, stemp.split(',')

    """
    try:
        float(stemp)
        return [stemp]
    except ValueError:
        return stemp.split(',')
