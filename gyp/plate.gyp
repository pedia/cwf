{
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
      'target_name': 'plate_unittest',
      'type': 'executable',
      'msvs_guid': '11384248-6F84-5DAE-8AB2-655600A90963',
      'dependencies': [
        'gtest.gyp:gtest_main',
        'plate',
      ],
      'conditions':[
        ['OS=="linux"', {'libraries': ['-lboost_system', '-lboost_thread', '-lpthread'] }],
        ['OS=="win"', {'libraries': ['ws2_32.lib'] }],
      ],
      'sources': [
'../src/plate/fslock_test.cc',
'../src/plate/metaimpl_test.cc',
'../src/plate/sixty_test.cc',
      ],
      'include_dirs': [
        '../src',
      ],
    },
    {
      'target_name': 'plate',
      'type': 'static_library',
      'msvs_guid': 'B0FA2853-A0D3-44B8-BDE0-E8B89D372D16',
      'include_dirs': ['../src'],
      'dependencies': [
        'base3.gyp:base3',
      ],
      'sources': [
'../src/plate/fslock.h',
'../src/plate/itf.h',
'../src/plate/itf.cc',
'../src/plate/metaimpl.h',
'../src/plate/metaimpl.cc',
'../src/plate/tenandsixty.h',
      ],
      'export_dependent_settings': ['base3.gyp:base3'],
    },
    {
      'target_name': 'plated',
      'type': 'executable',
      'msvs_guid': '2DEFD3FF-9903-41CD-AF2C-2249BE811651',
      'include_dirs': ['../src'],
      'dependencies': [
        'base3.gyp:base3',
        # 'cwf.gyp:cwf',
        'cwf.gyp:cwfmain',
        'plate',
      ],
      'conditions':[
        ['OS=="linux"', {'libraries': ['-lboost_system', '-lboost_thread', '-lpthread'] }],
        ['OS=="win"', {'libraries': ['ws2_32.lib'] }],
      ],
      'sources': [
'../src/plate/plateaction.cc',
'../src/plate/readaction.cc',
      ],
    }
  ],
}
