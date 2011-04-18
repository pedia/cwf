{
  'variables' : { 'component': 'static_library'},
  'conditions': [
    ['OS=="linux"', {
      'target_defaults': {
        'cflags': ['-fPIC', '-g', '-O3',],
        #'defines': ['OS_LINUX'],
      },
    },],
    ['OS=="win"', {
      'target_defaults': {
        #'defines': ['OS_WIN', 'WIN32', 'NOMINMAX', 'UNICODE', '_UNICODE', 'WIN32_LEAN_AND_MEAN', '_WIN32_WINNT=0x0501'],
        'msvs_settings': {
          'VCLinkerTool': {'GenerateDebugInformation': 'true',},
          'VCCLCompilerTool': {'DebugInformationFormat': '3',},
        },
      },
    },],
  ],
  'targets': [
  {
    'target_name': 'libpng',
    'type': 'static_library',
    'dependencies': [
      'zlib.gyp:zlib',
    ],
    'include_dirs': [
      '../src/3rdparty/libpng-stable',
      '../src/3rdparty/zlib-stable',
    ],
    'defines': [
      'CHROME_PNG_WRITE_SUPPORT', # used in pngusr.h
      'PNG_USER_CONFIG',
    ],
    'msvs_guid': 'C564F145-9172-42C3-BFCB-6014CA97DBCD',
    'sources': [
      '../src/3rdparty/libpng-stable/png.c',
      '../src/3rdparty/libpng-stable/png.h',
      '../src/3rdparty/libpng-stable/pngconf.h',
      '../src/3rdparty/libpng-stable/pngerror.c',
      # '../src/3rdparty/libpng-stable/pnggccrd.c',
      '../src/3rdparty/libpng-stable/pngget.c',
      '../src/3rdparty/libpng-stable/pngmem.c',
      '../src/3rdparty/libpng-stable/pngpread.c',
      '../src/3rdparty/libpng-stable/pngread.c',
      '../src/3rdparty/libpng-stable/pngrio.c',
      '../src/3rdparty/libpng-stable/pngrtran.c',
      '../src/3rdparty/libpng-stable/pngrutil.c',
      '../src/3rdparty/libpng-stable/pngset.c',
      '../src/3rdparty/libpng-stable/pngtrans.c',
      '../src/3rdparty/libpng-stable/pngusr.h',
      # '../src/3rdparty/libpng-stable/pngvcrd.c',
      '../src/3rdparty/libpng-stable/pngwio.c',
      '../src/3rdparty/libpng-stable/pngwrite.c',
      '../src/3rdparty/libpng-stable/pngwtran.c',
      '../src/3rdparty/libpng-stable/pngwutil.c',
    ],
    'direct_dependent_settings': {
      'include_dirs': ['../src/3rdparty/libpng-stable'],
      'defines': [
        'PNG_USER_CONFIG',
      ],
    },
    'export_dependent_settings': [
      'zlib.gyp:zlib',
    ],
    'conditions': [
      ['OS!="win"', {'product_name': 'png'}],
      ['OS=="win" and component=="shared_library"', {
        'defines': [
          'PNG_BUILD_DLL',
          'PNG_NO_MODULEDEF',
        ],
        'direct_dependent_settings': {
          'defines': [
            'PNG_USE_DLL',
          ],
        },          
      }],
    ],
  },
  ]
}