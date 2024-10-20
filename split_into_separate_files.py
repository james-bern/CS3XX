from io import TextIOWrapper
import re

with open("main.cpp", "w") as main:
    file = main
    with open("combined.cpp") as combined:
        for line in combined.readlines():
            # print(line)
            if line.startswith("// <!> Begin"):
                filename = re.search(r'Begin (.+?\.cpp)', line).group(1)
                main.write(f'#include "{filename}"\n')
                file = open(filename, "w")
            elif line.startswith("// <!> End"):
                file.close()
                file = main
            else:
                file.write(line)
