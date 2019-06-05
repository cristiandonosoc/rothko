# Copyright 2019, Cristián Donoso.
# This code has a BSD license. See LICENSE.

# Example of how to set extra flags common to only this checkout of the repo.
# Name this file "ycm_extra_conf_local.py" in the checkout root directory.
# It is ignored by git.

local_flags = [
  # Freakishly important for when using modern headers!!!!
  "-nostdinc++",

  "-isystem",
  "/home/cdc/Source/fuchsia/buildtools/linux-x64/clang/lib/clang/9.0.0/include/c++/v1",
  "-isystem", "/mnt/c/Code/include",
  "-isystem", "/mnt/c/Code/include/fake-windows-headers-for-ycm",
]

def GetYCMLocalFlags():
  return local_flags

