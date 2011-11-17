# Copyright (c) 2009 The Chromium Embedded Framework Authors. All rights
# reserved. Use of this source code is governed by a BSD-style license that
# can be found in the LICENSE file.

{
  'variables': {
    # Directory for CEF source files.
    'conditions': [
      [ 'OS=="win"', {
        'des_courgette_directory' : '<!(echo %DES_COURGETTE_DIRECTORY%)',
      }, { # OS!="win"
        'des_courgette_directory' : '<!(echo $DES_COURGETTE_DIRECTORY)',
      }],
    ]
  },
}
