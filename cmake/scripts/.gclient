solutions = [
  { "name"        : "src",
    "url"         : "https://src.chromium.org/svn/trunk/src",
    "custom_deps" : {
      "src/chrome/tools/test/reference_build/chrome": None,
      "src/chrome/tools/test/reference_build/chrome_linux": None,
      "src/chrome/tools/test/reference_build/chrome_mac": None,
      "src/chrome_frame/tools/test/reference_build/chrome": None,
      "src/third_party/gles2_book": None,
      "src/third_party/hunspell_dictionaries": None,
      "src/third_party/WebKit/LayoutTests": None,
    },
    "deps_file"   : "DEPS",
    "managed"     : True,
    "safesync_url": "",
  },
]

