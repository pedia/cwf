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
      'target_name': 'rof_unittest',
      'type': 'executable',
      'msvs_guid': '11384348-7F84-5DAE-8AB2-655600A90963',
      'dependencies': [
        'gtest.gyp:gtest_main',
        'rof',
      ],
      'conditions':[
        ['OS=="linux"', {'libraries': ['-lboost_system', '-lboost_thread', '-lpthread'] }],
        ['OS=="win"', {'libraries': ['ws2_32.lib'] }],
      ],
      'sources': [
'../src/resizeonthefly/pichelper_test.cc',
      ],
      'include_dirs': [
        '../src',
      ],
    },
    {
      'target_name': 'rof',
      'type': 'static_library',
      'msvs_guid': 'B0FA2953-A1D3-55C8-BDE0-E8B89D372D16',
      'include_dirs': ['../src'],
      'dependencies': [
        'base3.gyp:base3',
        'graphmagick.gyp:magick',
        'graphmagick.gyp:coders',
      ],
      'sources': [
'../src/resizeonthefly/pichelper.h',
'../src/resizeonthefly/pichelper.cc',
      ],
      'export_dependent_settings': [
        'base3.gyp:base3',
        'graphmagick.gyp:magick',
      ],
    },
    {
      'target_name': 'urdl',
      'type': 'static_library',
      'msvs_guid': '2DEFD400-AA14-41CD-A03C-3349BE811651',
      'include_dirs': ['../src/3rdparty/urdl', '../src'],
      'dependencies': [],
      'conditions':[
        ['OS=="linux"', {
          'direct_dependent_settings': {
            'libraries': ['-lssl', '-lboost_system', '-lboost_thread', '-lpthread'], 
          },
        },],
        ['OS=="win"', {
          'source!': ['../src/3rdparty/urdl/urdl.cpp',],
        },],
      ],
      'direct_dependent_settings': {
        'include_dirs': ['../src/3rdparty/urdl'],
      },
      'sources': [
'../src/resizeonthefly/httpdownload.h',
'../src/resizeonthefly/httpdownload.cc',
'../src/3rdparty/urdl/urdl/read_stream.hpp',
'../src/3rdparty/urdl/urdl/http.hpp',
'../src/3rdparty/urdl/urdl/url.hpp',
'../src/3rdparty/urdl/urdl.cpp',
      ],
    },
    {
      'target_name': 'urdl_unittest',
      'type': 'executable',
      'msvs_guid': '22384448-7F84-5DBE-8AB2-655600A90974',
      'dependencies': [
        'gtest.gyp:gtest_main',
        #'gtest.gyp:gtest',
        'urdl',
      ],
      'conditions':[
        ['OS=="linux"', {'libraries': ['-lboost_system', '-lboost_thread', '-lpthread'] }],
        ['OS=="win"', {'libraries': ['ws2_32.lib'] }],
      ],
      'sources': [
        '../src/resizeonthefly/urdl_test.cc',
      ],
      'include_dirs': [
        '../src',
      ],
    },
    {
      'target_name': 'rofd',
      'type': 'executable',
      'msvs_guid': '2DEFD4FF-9903-41CD-A03C-3349BE811651',
      'include_dirs': ['../src', '../src/3rdparty/urdl'],
      'dependencies': [
        'base3.gyp:base3',
        'cwf.gyp:cwfmain',
        'rof',
        'urdl',
      ],
      'conditions':[
        ['OS=="linux"', {'libraries': ['-lssl', '-lboost_system', '-lboost_thread', '-lpthread'] }],
        ['OS=="win"', {'libraries': ['ws2_32.lib'] }],
      ],
      'sources': [
'../src/resizeonthefly/rofaction.cc',
# '../src/resizeonthefly/fileaction.cc',
'../src/resizeonthefly/httpdownload.h',
'../src/resizeonthefly/httpdownload.cc',
'../src/3rdparty/urdl/urdl/read_stream.hpp',
'../src/3rdparty/urdl/urdl/http.hpp',
'../src/3rdparty/urdl/urdl/url.hpp',
      ],
    }
  ],
}
