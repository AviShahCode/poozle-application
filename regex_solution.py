import re

t = int(input())

answer = 0
do_add, do_mul = True, True

for _ in range(t):
    s = input()
    s = re.findall("(do_mul\(\)|do_add\(\)|don't_mul\(\)|don't_add\(\)|add\(\d+,\d+\)|mul\(\d+,\d+\))", s)
    # print(s)
    for item in s:
        if item == "do_mul()":
            do_mul = True
        elif item == "do_add()":
            do_add = True
        elif item == "don't_mul()":
            do_mul = False
        elif item == "don't_add()":
            do_add = False
        elif item.startswith("add") and do_add:
            a, b = map(int, item[4:-1].split(","))
            answer += a + b
        elif item.startswith("mul") and do_mul:
            a, b = map(int, item[4:-1].split(","))
            answer += a * b
print(answer)