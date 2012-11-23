if(NOT WIN32)
  add_desura_test(UtilLinux_test util "util")
  add_desura_test(gcBuff_test util "util")
  add_desura_test(MD5_test util "util")

  add_desura_test(util_fs_copyFile util_fs "util;util_fs")
  add_desura_test(util_fs_copyFolder util_fs "util;util_fs")
  add_desura_test(util_fs_getAllFiles util_fs "util;util_fs")
  add_desura_test(util_fs_getAllFolders util_fs "util;util_fs")

  add_desura_test(util_string_sanitizeFilePath util_string "util")
endif()

