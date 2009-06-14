#!/bin/bash
grep ".. cfunction::" *.rst -h |
python -c "import sys
print \"opencv_function_names = [\"
for line in sys.stdin.readlines():
    print \"'%s',\" % line.split()[3].strip(' (')
print \"]\"" > function_names.py
