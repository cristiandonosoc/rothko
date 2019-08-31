# Copyright 2019, Cristián Donoso.
# This code has a BSD license. See LICENSE.

# Simple utility to count the lines of code within Rothko.
# NOTE: This file is python3 only.

import os
import pathlib
import pprint
import sys

pp = pprint.PrettyPrinter(indent=2)

dir_blacklist = [
    ".git",
    ".vscode",
    "out",
    "third_party",
]

extension_groups = [
    {"id": "c_headers", "extensions": ["h", "hpp"],             "title": "C/C++ Headers"},
    {"id": "c_code",    "extensions": ["c", "cc", "cpp"],       "title": "C/C++ Code"},
    # {"id": "cs_code",   "extensions": ["cs"],                   "title": "C# Code"},
    {"id": "python",    "extensions": ["py"],                   "title": "Python"},
    {"id": "gn",        "extensions": ["gn"],                   "title": "GN"},
    {"id": "extras",    "extensions": ["license", "txt", "md"], "title": "Other"},
]

class Colors:
    PURPLE = '\033[95m'
    CYAN = '\033[96m'
    DARKCYAN = '\033[36m'
    BLUE = '\033[94m'
    GREEN = '\033[92m'
    YELLOW = '\033[93m'
    RED = '\033[91m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
    END = '\033[0m'

def LookForCode(root_path, show_files):
    sub_dirs = {}

    result = SearchDir(root_path, "*", show_files)
    for res in result:
        relpath = os.path.relpath(res[0], root_path)
        ext_res = res[1]
        sub_dirs[relpath] = ext_res

    # Search each sub-directory recursively.
    for entry in os.listdir(root_path):
        sub_path = os.path.join(root_path, entry)
        if not os.path.isdir(sub_path):
            continue

        if entry in dir_blacklist:
            continue

        result = SearchDir(sub_path, "**/*", show_files)
        for res in result:
            relpath = os.path.relpath(res[0], root_path)
            ext_res = res[1]
            sub_dirs[relpath] = ext_res

    return sub_dirs

def CountLines(filepath):
    non_empty = 0
    total = 0
    try:
        with open(filepath) as f:
            for line in f:
                total += 1
                if not line.isspace():
                    non_empty += 1

    except:
        print("{} could not be read as lines.".format(filepath))
    return (non_empty, total)

def SearchDir(path, glob_pattern, show_files):
    result = []

    path_result = {}
    for extension_group in extension_groups:
        non_empty = 0
        count = 0

        for extension in extension_group["extensions"]:
            for f in pathlib.Path(path).glob("{}.{}".format(glob_pattern, extension)):
                file_result = {}
                line_count = CountLines(f)
                non_empty += line_count[0]
                count += line_count[1]

                # Create a dummy entry for a file (will have only one real category filled).
                if show_files:
                    for grp in extension_groups:
                        file_result[grp["id"]] = (0, 0)

                    # Actually fill the real category.
                    file_result[extension_group["id"]] = (line_count[0], line_count[1])
                    result.append((f, file_result))

        path_result[extension_group["id"]] = (non_empty, count)
    result.append((path, path_result))

    for res in result:
        total = 0
        non_empty_total = 0
        for k,v in res[1].items():
            non_empty_total += v[0]
            total += v[1]
        res[1]["total"] = (non_empty_total, total)
    return result

def EntryToRow(entry):
    rjust = 7
    result = []
    for i, e in enumerate(entry):
        # The first index is the directory name.
        if i == 0:
            result.append(e)
            continue
        result.append("{}/{}".format(str(e[0]), str(e[1])))
    return result

def GetIndentedRow(data, first_row_length, color):
    rjust = 18
    rest_of_line = '|'.join(str(x).rjust(rjust) for x in data[1:len(data)])
    line = "{}|{}".format(str(data[0]).rjust(first_row_length), rest_of_line)
    return line

def PrintIndentedRow(data, first_row_length, color):
    line = GetIndentedRow(data, first_row_length, color)
    print(color + line + Colors.END)
    return len(line)

def CreateTotalEntry(root_path, entries):
    # We sum over all entries.
    totals = {}
    for extension_group in extension_groups:
        group_non_empty_totals = 0
        group_totals = 0
        group_id = extension_group["id"]
        for dir_name, dir_result in entries.items():
            # We only count the total using directories.
            if not os.path.isdir(os.path.join(root_path, dir_name)):
                continue
            group_non_empty_totals += dir_result[group_id][0]
            group_totals += dir_result[group_id][1]
        totals[group_id] = (group_non_empty_totals, group_totals)

    result = ["totals"]
    non_empty_final_total = 0
    final_total = 0
    for extension_group in extension_groups:
        group_id = extension_group["id"]
        group_total = totals[group_id]
        result.append(group_total)
        non_empty_final_total += group_total[0]
        final_total += group_total[1]
    result.append((non_empty_final_total, final_total))

    return result

def PrintHeaders(headers, longest_dir_len):
    line = GetIndentedRow(headers, longest_dir_len, Colors.GREEN)
    print("-" * len(line))
    print(Colors.GREEN + line + Colors.END)
    print("-" * len(line))

def PrintResult(root_path, result_dirs):
    print("Rothko Line Total Utility.")
    print("Each entry is <non-empty lines>/<total lines>")
    print("Blue lines are directories. Red lines are files over 1000 lines long.")
    print("")

    # Entry format: [<DIR_NAME>, <EACH |extension_groups| TOTAL FOR THAT DIR>, <TOTAL>]
    entries = []
    longest_dir_len = 0
    for dir_name, dir_result in result_dirs.items():
        if len(dir_name) > longest_dir_len:
            longest_dir_len = len(dir_name)

        dir_entry = [dir_name]

        for extension_group in extension_groups:
            dir_entry.append(dir_result[extension_group["id"]])
        dir_entry.append(dir_result["total"])
        entries.append(dir_entry)

    headers = ["directory"]
    for extension_group in extension_groups:
        headers.append(extension_group["title"])
    headers.append("total")

    printed_once = False
    total_entries = len(entries)
    for i, entry in enumerate(entries):
        # We print the header every certain amount of lines.
        # We're careful to not print a header too close to the end, except for the initial.
        if i % 60 == 0:
            if not printed_once or (total_entries - i) > 15:
                printed_once = True
                PrintHeaders(headers, longest_dir_len)

        row_entry = EntryToRow(entry)
        color = Colors.END
        if (os.path.isdir(os.path.join(root_path, entry[0]))):
            color = Colors.BLUE
        elif (entry[-1][1] > 1000):
            color = Colors.RED
        PrintIndentedRow(EntryToRow(entry), longest_dir_len, color)

    total_line = GetIndentedRow(EntryToRow(CreateTotalEntry(root_path, result_dirs)),
                                longest_dir_len,
                                Colors.GREEN)
    print("=" * len(total_line))
    print(Colors.GREEN + total_line + Colors.END)

def PrintUsage():
    print("Usage: python line_count.py [--show-files] [DIR]")
    print("")
    print("Will use CWD if non is given.")

if __name__  == "__main__":
    if len(sys.argv) > 3:
        PrintUsage()
        sys.exit(1)

    show_files = False
    path = os.getcwd()
    if len(sys.argv) == 3:
        if sys.argv[1] != "--show-files":
            PrintUsage()
            sys.exit(1)
        show_files = True
        path = os.path.abspath(sys.argv[2])
    elif len(sys.argv) == 2:
        path = os.path.abspath(sys.argv[1])

    result = LookForCode(path, show_files)
    PrintResult(path, result)
