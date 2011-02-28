{
  'variables': {
    'library': 'static_library',
  },
  'conditions': [
    ['OS=="linux"', {
      'target_defaults': {
        'cflags': ['-fPIC', '-g', '-O2',],
        'defines': ['OS_LINUX'],
      },
    },],
    ['OS=="win"', {
      'target_defaults': {
        # 'cflags': ['-fPIC', '-g', '-O2',],
        'defines': ['WIN32', 'OS_WIN', 'NOMINMAX', 'UNICODE', '_UNICODE', 'WIN32_LEAN_AND_MEAN', '_WIN32_WINNT=0x0501'],
        'msvs_settings': {
          'VCLinkerTool': {'GenerateDebugInformation': 'true',},
          'VCCLCompilerTool': {'DebugInformationFormat': '3',},
        },
      },
    },],
  ],
  'targets': [
    {
      'target_name': 'cwf_unittest',
      'type': 'executable',
      'msvs_guid': '11384247-5F84-4DAE-8AB2-655600A90963',
      'dependencies': [
        'cwf',
        'libfcgi',
        'base.gyp:base',
        'google-ctemplate.gyp:ctemplate',
        '../testing/gtest.gyp:gtestmain',
      ],
      'conditions':[
        ['OS=="linux"', {'libraries': ['-lboost_system', '-lboost_thread', '-lpthread'] }],
        ['OS=="win"', {'libraries': ['ws2_32.lib'] }],
      ],
      'sources': [
        '../cwf/stream_test.cc',
      ],
      'include_dirs': [
        '..',
        '../testing/gtest/include',
      ],
    },
    {
      'target_name': 'cwf',
      'type': 'static_library',
      'msvs_guid': 'B0FA2852-A0D3-44B8-BDE0-E8B89D372D05',
      'include_dirs': ['..'],
      'dependencies': ['base.gyp:base'],
#      'dependencies': ['google-ctemplate.gyp:ctemplate'],
#      'export_dependent_settings': ['google-ctemplate.gyp:ctemplate'],
      'sources': [
'../cwf/404.tpl',
'../cwf/action.h',
'../cwf/action.cc',
# '../cwf/config_parse.h',
'../cwf/connect.cc',
'../cwf/connect.h',
'../cwf/cookie.cc',
'../cwf/cookie.h',
'../cwf/cwf.h',
# '../cwf/doc/fcgi-spec.html',
# '../cwf/doc/resource.txt',
# '../cwf/dynalib.cc',
# '../cwf/dynalib.h',
'../cwf/foo.tpl',
'../cwf/frame.cc',
'../cwf/frame.h',
'../cwf/http.cc',
'../cwf/http.h',
'../cwf/pattern.h',
'../cwf/setting',
'../cwf/site.h',
'../cwf/stream.cc',
'../cwf/stream.h',
# '../cwf/task.h',
# '../cwf/tplaction.cc',
# '../cwf/tplaction.h',
      ],
    },
    {
      'target_name': 'tplaction',
      'type': 'static_library',
      'msvs_guid': '2DEFD2EF-88E3-41CD-AF2C-2249BE811651',
      'include_dirs': ['..'],
      'dependencies': ['google-ctemplate.gyp:ctemplate'],
      'export_dependent_settings': ['google-ctemplate.gyp:ctemplate'],
      'sources': [
'../cwf/404.tpl',
'../cwf/tplaction.cc',
'../cwf/tplaction.h',
      ],
    },
    {
      'target_name': 'cwfmain',
      'type': 'static_library',
      'msvs_guid': '0829F556-C016-4F0A-8C1D-A094AD174534',
      'include_dirs': ['..'],
      'dependencies': ['cwf', 'libfcgi'],
      'export_dependent_settings': ['cwf'],
      'sources': [
'../cwf/cwfmain.cc',
      ],
    },
    {
      'target_name': 'libfcgi',
      'type': 'static_library',
      'msvs_guid': '7D8110AD-6AA4-4A68-AACA-3DC84541C6A0',
      'include_dirs': ['..'],
      'sources': [
'../cwf/libfcgi/fastcgi.h',
'../cwf/libfcgi/fcgiapp.c',
'../cwf/libfcgi/fcgiapp.h',
'../cwf/libfcgi/fcgi_config.h',
'../cwf/libfcgi/fcgi_config_x86.h',
'../cwf/libfcgi/fcgimisc.h',
'../cwf/libfcgi/fcgio.cpp',
'../cwf/libfcgi/fcgio.h',
'../cwf/libfcgi/fcgios.h',
'../cwf/libfcgi/fcgi_stdio.c',
'../cwf/libfcgi/fcgi_stdio.h',
'../cwf/libfcgi/os_unix.c',
'../cwf/libfcgi/os_win32.c',
      ],
      'conditions': [
        ['OS == "linux"', {
          'sources!': [
'../cwf/libfcgi/os_win32.c',
            ],}],
        ['OS == "win"', {
          'sources!': [
'../cwf/libfcgi/os_unix.c',
            ],}]
      ],
    },
  ],
}
