from io import TextIOWrapper
import re

def paste_file(output: TextIOWrapper, filename):
    with open(filename) as file:
        output.write(f"// <!> Begin {filename} <!>\n")
        for line in file.readlines():
            output.write(line)
        output.write(f"// <!> End {filename} <!>\n")

with open("combined.cpp", "w") as combined:
    with open("main.cpp") as main:
        for line in main.readlines():
            if line.strip().startswith("#include"):
                query = re.search(r'"(.+?\.cpp)"', line)
                if query:
                    filename = query.group(1)
                    paste_file(combined, filename)
                else:
                    combined.write(line)
            else:
                combined.write(line)

