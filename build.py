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
from typing import Iterable, Union

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

    def get_compile_arguments(self) -> Iterable[Union[str, dlb.fs.Path, dlb.fs.Path.Native]]:
        return ['-g']


class Linker(dlb_contrib.gcc.CplusplusLinkerGcc):
    pass


class Application(dlb.ex.Tool):
    EXECUTABLE = '@application'

    async def redo(self, result, context):
        await context.execute_helper(self.EXECUTABLE)


with dlb.ex.Context():
    source_directory = Path('src/')
    test_directory = Path('test/')
    output_directory = Path('build/out/')
    generated_source_directory = output_directory / 'Dbor/Generated/'
    generated_test_directory = output_directory / 'test/generated/'

    with dlb.di.Cluster('compile each .hpp'), dlb.ex.Context():
        for p in source_directory.iterdir(name_filter=r'.+\.hpp',
                                          recurse_name_filter='', is_dir=False):
            Compiler(
                source_files=[p],
                object_files=[output_directory / p.with_appended_suffix('.ot')],
                include_search_directories=[source_directory]
            ).start()

    with dlb.di.Cluster('compile'), dlb.ex.Context():
        source_files = source_directory.list(
            name_filter=r'.+\.cpp', recurse_name_filter='', is_dir=False)
        source_files += test_directory.list(
            name_filter=r'.+\.cpp', recurse_name_filter='', is_dir=False)
        compile_results = [
            Compiler(
                source_files=[p],
                object_files=[output_directory / p.with_appended_suffix('.o')],
                include_search_directories=[source_directory]
            ).start()
            for p in source_files
        ]

    with dlb.di.Cluster('link'), dlb.ex.Context():
        application_file = Linker(
            object_and_archive_files=[r.object_files[0] for r in compile_results],
            linked_file=output_directory / 'tester').start().linked_file

    with dlb.di.Cluster('test'), dlb.ex.Context():
        dlb.ex.Context.active.helper[Application.EXECUTABLE] = application_file
        Application().start(force_redo=True)


dlb.di.inform(f'test application size: {application_file.native.raw.stat().st_size} B')
