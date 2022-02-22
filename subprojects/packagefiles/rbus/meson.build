project('rbus', 'c',
        version: '0.0.1',
        meson_version: '>=0.54.0',
        license: ['Apache-2.0'],
        default_options: ['c_std=c18'])

subdir('include')
subdir('include-workaround-meson/rbus')

librbus_dep = declare_dependency(include_directories: ['include-workaround-meson',
                                                       'include-workaround-meson/rbus'])
