# Copyright (c) 2009 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.
{
  'variables': {
    'chromium_code': 1,
  },
  'targets': [
    {
      'target_name': 'patcher',
      'type': 'none',
      'msvs_guid': 'A6D0953E-899E-4C60-AB6B-CAE75A44B8E6',
      'conditions': [
        ['OS=="win"', {
          'actions': [{
            'action_name': 'patch_source',
            'msvs_cygwin_shell': 0,
            'inputs': [
              'tools/patch_source.bat',
            ],
            'outputs': [
              'tools/patch_source.bat.output',
            ],
            'action': ['', '<@(_inputs)'],
          }],
        }, { # OS!="win"
          'actions': [{
            'action_name': 'patch_source',
            'inputs': [
              'tools/patch_source.sh',
            ],
            'outputs': [
              'tools/patch_source.sh.output',
            ],
            'action': ['<@(_inputs)'],
          }],
        }],
      ],
    },
    {
      'target_name': 'courgette_desura',
      'type': 'shared_library',
      'msvs_guid': 'C13650D5-CF1A-4259-BE45-B1EBA6280E47',
      'dependencies': [
        '../courgette/courgette.gyp:courgette_lib',
      ],
      'defines': [
      ],
      'include_dirs': [
        './include',
        './code',
		'..',
      ],
      # Avoid "RC1102: internal error : too many arguments to RCPP" error by
      # explicitly specifying a short list of resource include directories.
      'resource_include_dirs' :  [
        '.',
        '..',
      ],
      'sources': [
        'include/CourgetteI.h',
		'code/Courgette.cpp',
      ],
      'conditions': [
        ['OS=="win"', {
          'sources': [
            'code/courgette_desura.rc',
          ],	  
        }]
      ],
    },
]}
