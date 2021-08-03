# Meson build file
#
# SPDX-FileCopyrightText: 2021 Comcast Cable Communications Management, LLC
# SPDX-License-Identifier: Apache-2.0

project('wrp-c', 'c',
        version: '2.0.0',
        license: ['Apache-2.0'],
        default_options: ['c_std=c99',
                          'b_coverage=true'])

if not meson.is_subproject()
  # We want strict flags
  warning_level = 3   # -Wall
  werror = true
endif

################################################################################
# Common variable definitions
################################################################################

inc_base = 'include/'+meson.project_name()

################################################################################
# Generate the version header file
################################################################################

subdir('include/'+meson.project_name())

################################################################################
# Define the dependencies
################################################################################
ludocode_mpack_dep = dependency('mpack', version: '>=1.0',
                                #default_options: [ 'MPACK_STDLIB=1' ],
                                fallback: ['ludocode-mpack', 'ludocode_mpack_dep'],
                                )
cutils_dep = dependency('cutils', version: '>=1.0.0')

all_deps = [ludocode_mpack_dep, cutils_dep]

################################################################################
# Define the libraries
################################################################################

inc = include_directories([inc_base])

install_headers([inc_base+'/wrp-c.h', ver_h], subdir: meson.project_name())

sources = [ 'src/constants.c',
            'src/decode.c',
            'src/encode.c',
            'src/internal.c',
            'src/locator.c',
            'src/string.c']

libwrpc = library(meson.project_name(),
                  sources,
                  include_directories: inc,
                  dependencies: all_deps,
                  install: true)

################################################################################
# Define the tests
################################################################################

if not meson.is_subproject()
  test_args = ['-fprofile-arcs', '-g', '-ftest-coverage', '-O0']

  cunit_dep = dependency('cunit')

  tests = ['test_00',
           'test_01',
           'test_02',
           'test_03',
           'test_04',
           'test_05',
           'test_06',
           'test_07',
           'test_08',
           'test_09',
           'test_10',
           'test_11',
           'test_12',
           'test_13',
           'test_14',
           'test_15',

           'test_err_00',
           'test_err_01',
           'test_err_02',
           'test_err_03',
           ]

  foreach test : tests
    test(test,
         executable(test, ['tests/'+test+'.c',
                              'tests/test_common.c'],
                    include_directories: inc,
                    dependencies: [cunit_dep, cutils_dep, ludocode_mpack_dep],
                    install: false,
                    link_args: test_args,
                    link_with: libwrpc))
  endforeach

  others = [ 'test_locator', 'test_misc' ]
  foreach other : others
    test(other,
         executable(other, ['tests/'+other+'.c'],
                    include_directories: inc,
                    dependencies: [cunit_dep, cutils_dep, ludocode_mpack_dep],
                    install: false,
                    link_args: test_args,
                    link_with: libwrpc))
  endforeach

  add_test_setup('valgrind',
                 is_default: true,
                 exe_wrapper: [ 'valgrind',
                                '--leak-check=full',
                                '--error-exitcode=1' ],
                 timeout_multiplier: 2)
endif

################################################################################
# Add the license check target
################################################################################

reuse_bin = find_program('reuse', required: false)
if reuse_bin.found()
  custom_target('license',
                command: [reuse_bin, 'lint'],
                build_always_stale: true,
                output: ['license'])
endif

################################################################################
# Provide a nice way to be included
################################################################################

libwrpc_dep = declare_dependency(include_directories: ['include'],
                                          link_with: libwrpc)

if meson.version().version_compare('>=0.54.0')
  meson.override_dependency(meson.project_name(), libwrpc_dep)
endif
