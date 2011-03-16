# gyp --depth . -Dlibrary=static_library htmlsax.gyp -fmsvs -G msvs_version=2008e
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
        'defines': ['OS_WIN', 'ARCH_CPU_X86_FAMILY', 'NOMINMAX', 'UNICODE', '_UNICODE', 'WIN32_LEAN_AND_MEAN', '_WIN32_WINNT=0x0501', 'BASE_DISABLE_POOL'],
      },
    },],
  ],
  'targets': [
    {
      'target_name': 'htmlsax_unittest',
      'type': 'executable',
      'dependencies': [
        'htmlsax',
        '../src/testing/gtest.gyp:gtest_main',
      ],
      'conditions':[
        ['OS=="linux"', {'libraries': ['-lpthread'] }],
      ],
      'sources': [
'../src/htmlsax/parser_test.cc',
      ],
      'include_dirs': [
        '../src',
        '../src/testing/gtest/include',
      ],
    },
    {
      'target_name': 'htmlsax',
      'type': 'none',
      'msvs_guid': 'A301A56A-6DEB-4D11-9332-B1E30AEACB8E',
      'include_dirs': ['../src'],
      'dependencies': [
      ],
      'sources': [
'../src/htmlsax/parser.h',
'../src/htmlsax/handler.h',
'../src/htmlsax/tidy_handler.h',
      ],
      'conditions': [
        ['OS!="mac"', {'sources/': [['exclude', '_mac\\.(cc|mm?)$']]}],
        ['OS!="win"', {'sources/': [['exclude', '_win\\.cc$']]
          }, {  # else: OS=="win"
            'sources/': [['exclude', '_posix\\.cc$']]
        }],
      ],
    },
  ],
}
