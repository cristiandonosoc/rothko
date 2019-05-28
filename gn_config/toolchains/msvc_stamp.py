# Copyright 2019, Cristián Donoso.
# This code has a BSD license. See LICENSE.

# Python script used by GN to update the timestamp on a file.
# This is used in the Windows build, as Linux has the touch command.

import sys

def main(path):
    open(path, "w").close()

if __name__ == "__main__":
    sys.exit(main(*sys.argv[1:]))
