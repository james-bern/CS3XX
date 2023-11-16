#!/usr/bin/env python3

class Foo:
    def __init__(self, num):
        self.num = num

lst = []
for i in range(300):
    for j in range(300):
        for k in range(300):
            lst.append(Foo(i + j + k))


print(lst[26999999].num)
