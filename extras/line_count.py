# Copyright 2019, Cristián Donoso.
# This code has a BSD license. See LICENSE.

# Simple utility to count the lines of code within Rothko.
# NOTE: This file is python3 only.

import os
import pathlib
import sys

dir_whitelist = [
    "examples",
    "extras",
    "rothko",
    "tests",
]

dir_blacklist = [
    ".git",
    ".vscode",
    "out",
    "third_party",
]

def LookForCode(root_path):
    sub_dirs = {}

    sub_dirs["."] = SearchDir(root_path)
    for entry in os.listdir(root_path):
        sub_path = os.path.join(root_path, entry)
        if not os.path.isdir(sub_path):
            continue

        # if entry not in dir_whitelist:
        #     continue
        if entry in dir_blacklist:
            continue

        result = SearchSubDir(sub_path)
        sub_dirs[entry] = result
    return sub_dirs

def SearchDir(path):
    h_count = 0
    for h in pathlib.Path(path).glob("*.h"):
        h_count += CountLines(h)

    cc_count = 0
    for cc in pathlib.Path(path).glob("*.cc"):
        cc_count += CountLines(cc)

    gn_count = 0
    for gn in pathlib.Path(path).glob("*.gn"):
        gn_count += CountLines(gn)

    py_count = 0
    for py in pathlib.Path(path).glob("*.py"):
        py_count += CountLines(py)

    return {
        "h": h_count,
        "cc": cc_count,
        "gn": gn_count,
        "py": py_count,
        "total": h_count + cc_count + gn_count + py_count
    }

def SearchSubDir(path):
    h_count = 0
    for h in pathlib.Path(path).glob("**/*.h"):
        h_count += CountLines(h)

    cc_count = 0
    for cc in pathlib.Path(path).glob("**/*.cc"):
        cc_count += CountLines(cc)

    gn_count = 0
    for gn in pathlib.Path(path).glob("**/*.gn"):
        gn_count += CountLines(gn)

    py_count = 0
    for py in pathlib.Path(path).glob("**/*.py"):
        py_count += CountLines(py)

    return {
        "h": h_count,
        "cc": cc_count,
        "gn": gn_count,
        "py": py_count,
        "total": h_count + cc_count + gn_count + py_count
    }

def CountLines(filepath):
    total = 0
    try:
        with open(filepath) as f:
            for line in f:
                total += 1
    except:
        print("{} could not be read as lines.".format(filepath))
    return total

def PrintResult(results):
    print("Rothko line totals.")
    print("")
    totals = {"h": 0, "cc": 0, "gn": 0, "py": 0, "total": 0}
    entries = []
    for k in sorted(results):
        v = results[k]
        entries.append((k, v["h"], v["cc"], v["gn"], v["py"], v["total"]))
        totals["h"] += v["h"]
        totals["cc"] += v["cc"]
        totals["gn"] += v["gn"]
        totals["py"] += v["py"]
        totals["total"] += v["total"]

    titles = ["directory", ".h", ".cc", ".gn", ".py", "total"]
    data = [titles] + entries

    rjust = 15

    # Print a nice table.
    for i, d in enumerate(data):
        line = '|'.join(str(x).rjust(rjust) for x in d)
        print(line)
        if i == 0:
            print('-' * len(line))

    # Print the total line.
    totals = ("total", totals["h"], totals["cc"], totals["gn"],
                       totals["py"], totals["total"])
    line = '|'.join(str(x).rjust(rjust) for x in totals)
    print("=" * len(line))
    print(line)

if __name__  == "__main__":

    # Get the root path.
    if len(sys.argv) == 1:
        this_path = os.path.dirname(os.path.abspath(__file__))
        parent_dir = os.path.abspath(os.path.join(this_path, os.pardir))
    else:
        parent_dir = os.path.abspath(sys.argv[1])

    result = LookForCode(parent_dir)
    PrintResult(result)
