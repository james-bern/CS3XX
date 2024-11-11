from io import TextIOWrapper
import re

def is_jim_style_include(line):
    if line.strip().startswith("#include"):
        query = re.search(r'"(.+?\.cpp)"(.*)', line)
        if query:
            return query.group(1), query.group(2)
    return None, None

already_pasted_files = set()
def paste_file(output: TextIOWrapper, filename, remainder=""):
    if filename in already_pasted_files:
        output.write(f'#include "{filename}"{remainder}\n')
        return
    already_pasted_files.add(filename)

    with open(filename) as file:
        output.write(f"// <!> Begin {filename} <!> {remainder}\n")
        for line in file.readlines():
            filename_or_none, remainder = is_jim_style_include(line)
            if filename_or_none:
                print(f"{filename}: {filename_or_none} {remainder}")
                paste_file(combined, filename_or_none, remainder)
            else:
                combined.write(line)
        output.write(f"// <!> End {filename} <!>\n")

with open("combined.cpp", "w") as combined:
    with open("main.cpp") as file:
        for line in file.readlines():
            filename_or_none, remainder = is_jim_style_include(line)
            if filename_or_none:
                print(f"main.cpp: {filename_or_none} {remainder}")
                paste_file(combined, filename_or_none, remainder)
            else:
                combined.write(line)

