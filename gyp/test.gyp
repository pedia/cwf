{
  'targets': [
    {
      'target_name': 'test',
      'type': 'executable',
      'msvs_guid': '11384247-5F84-4DAE-8AB2-655600A90964',
      'dependencies': [],
      'variables' : {'target' : 'out/Default'},
      'conditions':[
        ['"OS"=="linux"', {
          'libraries': ['-lboost_system', '-lboost_thread', '-lpthread'],
          'defines': ['<(DEPTH)/'],
          }],
          
        ['"OS"=="win"', {'libraries': ['ws2_32.lib'] }],
      ],
      'sources': [
        'test.cc',
      ],
      'include_dirs': [],
    },
  ],
}
