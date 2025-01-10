#!/usr/bin/env python3

import glob

source_dir = f'/Users/lnan/Documents/Projects/Easy3D/easy3d'
all_include_filename = f'/Users/lnan/Documents/Projects/Easy3D/easy3d/all_includes.hpp'

def make_all_includes():
    all_includes = []
    for filename in (glob.glob(f'{source_dir}/**/*.hpp', recursive=True) +
                     glob.glob(f'{source_dir}/**/*.cpp', recursive=True) +
                     glob.glob(f'{source_dir}/**/*.h', recursive=True) +
                     glob.glob(f'{source_dir}/**/*.cc', recursive=True) +
                     glob.glob(f'{source_dir}/**/*.c', recursive=True)):
        with open(filename, 'r') as fh:
            for line in fh:
                if line.startswith('#include'):
                    all_includes.append(line.strip())
    all_includes = list(set(all_includes))
    # This is to ensure that the list is always the same and doesn't
    # depend on the filesystem state.  Not technically necessary, but
    # will cause inconsistent errors without it.
    all_includes.sort()
    with open(all_include_filename, 'w') as fh:
        for include in all_includes:
            fh.write(f'{include}\n')
    return all_include_filename



def main():
    all_includes_fn = make_all_includes()
    print(all_includes_fn)

if __name__ == '__main__':
    main()
