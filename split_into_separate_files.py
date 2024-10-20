from io import TextIOWrapper
import re

file_stack = []
with open("main.cpp", "w") as main:
    file = main
    with open("combined.cpp") as combined:
        for line in combined.readlines():
            if line.startswith("// <!> Begin"):
                file_stack.append(file)
                query = re.search(r'Begin (.+?\.cpp) <!> (.*)', line)
                filename = query.group(1)
                remainder = query.group(2)
                file.write(f'#include "{filename}"{remainder}\n')
                file = open(filename, "w")
            elif line.startswith("// <!> End"):
                file.close()
                file = file_stack.pop()
            else:
                file.write(line)
