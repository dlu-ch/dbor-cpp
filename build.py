# SPDX-License-Identifier: LGPL-3.0-or-later
# dbor-c++ - C++ implementation of DBOR encoder and decoder
# Copyright (C) 2020 Daniel Lutz <dlu-ch@users.noreply.github.com>

# https://github.com/dlu-ch/dlb
# Run this script by one of the following shell commands:
#
#    dlb build-all                  # from anywhere in the working tree (with directory of 'dlb' in $PATH)
#    python3 -m build-all           # in the directory of this file
#    python3 "$PWD"/build-all.py'   # in the directory of this file

import sys
import os

import dlb.di
import dlb.fs
import dlb.ex
import dlb_contrib.gcc
import dlb_contrib.iso6429

if sys.stderr.isatty():
    # assume terminal compliant with ISO/IEC 6429 ("VT-100 compatible")
    dlb.di.set_output_file(dlb_contrib.iso6429.MessageColorator(sys.stderr))


class Path(dlb.fs.PosixPath, dlb.fs.WindowsPath, dlb.fs.NoSpacePath):
    pass


class Compiler(dlb_contrib.gcc.CplusplusCompilerGcc):
    DIALECT = 'c++11'


class Linker(dlb_contrib.gcc.CplusplusLinkerGcc):
    pass


# TODO move to dlb_contrib
class GenerateIncludeStub(dlb.ex.Tool):
    included_files = dlb.ex.input.RegularFile[1:]()  # TODO make sure it contains no " and only 7 bit ASCII
    output_file = dlb.ex.output.RegularFile()

    PATH_COMPONENTS_TO_STRIP = 0  # number of leading path component to strip for #include

    async def redo(self, result, context):
        with context.temporary() as output_file:
            with open(output_file.native, 'w') as f:
                for p in self.included_files:
                    p = p[self.PATH_COMPONENTS_TO_STRIP:]
                    f.write('#include "{}"\n'.format(p.as_string()))
            context.replace_output(result.output_file, output_file)


with dlb.ex.Context():
    source_directory = Path('src/')
    test_directory = Path('test/')
    output_directory = Path('build/out/')
    generated_source_directory = output_directory / 'Dbor/Generated/'
    generated_test_directory = output_directory / 'test/generated/'

    with dlb.di.Cluster('compile'), dlb.ex.Context():
        compile_results = [
            Compiler(
                source_files=[p],
                object_files=[output_directory / p.with_appended_suffix('.o')],
                include_search_directories=[source_directory]
            ).start()
            for p in source_directory.iterdir(name_filter=r'.+\.cpp',
                                              recurse_name_filter='', is_dir=False)
        ] + [
            Compiler(
                source_files=[p],
                object_files=[output_directory / p.with_appended_suffix('.o')],
                include_search_directories=[source_directory]
            ).start()
            for p in test_directory.iterdir(name_filter=r'.+\.cpp',
                                            recurse_name_filter='', is_dir=False)
        ]

    with dlb.di.Cluster('check all .hpp'), dlb.ex.Context():
        class GenerateIncludeStub(GenerateIncludeStub):
            PATH_COMPONENTS_TO_STRIP = len(source_directory.components) - 1

        for p in source_directory.iterdir_r(name_filter=r'.+\.hpp',
                                            recurse_name_filter='', is_dir=False):
            include_stub_file = GenerateIncludeStub(
                included_files=[source_directory / p],
                output_file=(generated_test_directory / p).with_appended_suffix('.cpp')
            ).start().output_file
            Compiler(
                source_files=[include_stub_file],
                object_files=[include_stub_file.with_appended_suffix('.o')],
                include_search_directories=[source_directory]
            ).start()

    with dlb.di.Cluster('link'), dlb.ex.Context():
        application_file = Linker(
            object_and_archive_files=[r.object_files[0] for r in compile_results],
            linked_file=output_directory / 'tester').start().linked_file

dlb.di.inform(f'test application size: {application_file.native.raw.stat().st_size} B')
