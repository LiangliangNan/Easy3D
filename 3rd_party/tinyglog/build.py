#!/usr/bin/env python

#
# Copyright (c) 2016 Tzutalin
# Create by TzuTaLin <tzu.ta.lin@gmail.com>
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import argparse
import sys
import os
import subprocess
from subprocess import PIPE
import shutil
from subprocess import call

build_version_string = "v1.0"


class bcolors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'


def cmd(cmd):
    rc = call(cmd)
    if rc is not 0:
        print bcolors.FAIL + 'cmd:' + str(cmd) + bcolors.ENDC
        os.sys.exit(1);


class MakeItem:
    def __init__(self, path, name):
        self.name = name
        self.path = path

    def make(self, build_folder='build', ninja=False, install=False, android=False):
        curr = os.getcwd()
        os.chdir(self.path)

        if os.path.exists(build_folder):
            shutil.rmtree(build_folder)

        os.mkdir(build_folder)
        os.chdir(build_folder)
        cmake_cmd = None
        build_cmd = None
        install_cmd = None

        if ninja is True:
            cmake_cmd = ['cmake', '-G', 'Ninja']
            build_cmd = ['ninja']
            install_cmd = ['ninja', 'install']
        else:
            cmake_cmd = ['cmake']
            build_cmd = ['make']
            install_cmd = ['make', 'install']

        if android is True:
            global NDK_PATH
            global ABI
            if os.path.exists(NDK_PATH):
                cmake_cmd = cmake_cmd + ['-DCMAKE_SYSTEM_NAME=Android', '-DCMAKE_SYSTEM_VERSION=21',
                                         '-DCMAKE_ANDROID_ARCH_ABI=' + ABI, '-DCMAKE_ANDROID_STL_TYPE=gnustl_static',
                                         '-DCMAKE_ANDROID_NDK=' + NDK_PATH]
            else:
                print bcolors.FAIL + 'No NDK' + bcolors.ENDC
                os.sys.exit(1);

        if install is True:
            cmake_cmd = cmake_cmd + ['-D', 'CMAKE_INSTALL_PREFIX=.']

        cmake_cmd = cmake_cmd + ['..']
        print 'command: ' + str(cmake_cmd)

        cmd(cmake_cmd)
        cmd(build_cmd)
        if install is True:
            cmd(install_cmd)

        os.chdir(curr)


# BuildError exception class.
class BuildError(Exception):
    def __init__(self, value):
        self.value = value

    def __str__(self):
        return repr(self.value)


# Print version function.
def version():
    print build_version_string


# Program main:
def main(argv):
    parser = argparse.ArgumentParser(
        description='build.py [options]')
    parser.add_argument('--ninja', action='store_true',
                        help='Use ninja.')
    parser.add_argument('-n', '--ndk', default=None, type=str,
                        help='NDK path')
    parser.add_argument('--abi', default='arm64-v8a', type=str,
                        help='android abi')

    args = parser.parse_args()
    isUseNinja = args.ninja
    global NDK_PATH
    NDK_PATH = args.ndk
    global ABI
    ABI = args.abi

    # Build for PC
    library = MakeItem(".", "miniglog")
    library.make(ninja=isUseNinja, install=True)
    binary = MakeItem("example/test", "test")
    binary.make(ninja=isUseNinja)
    print bcolors.BOLD + 'Run example/test/build/test' + bcolors.ENDC
    cmd(['example/test/build/test'])

    # Build for mobile
    if NDK_PATH is not None:
        print bcolors.BOLD + 'Build so for Android arm64' + bcolors.ENDC
        android_library = MakeItem(".", "miniglog")
        android_library.make(build_folder='build-android-' + ABI, ninja=isUseNinja, install=True, android=True)


# Entry point.
if __name__ == "__main__":
    main(sys.argv[1:])
