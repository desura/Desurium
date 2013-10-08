include_directories(  
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${Boost_INCLUDE_DIR}
)

file(GLOB Sources code/util/gcBuff_test.cpp
                  code/util/MD5_test.cpp
                  code/util_fs/util_fs_copyFile.cpp
                  code/util_fs/util_fs_copyFolder.cpp
                  code/util_fs/util_fs_getAllFiles.cpp
                  code/util_fs/util_fs_getAllFolders.cpp
                  code/util_string/util_string_sanitizeFilePath.cpp
)


#code/util/UtilLinux_test.cpp


add_library(unittest SHARED ${Sources})

target_link_libraries(unittest
  util
  util_fs
)

link_with_gtest(unittest)
desurium_install_library(unittest "Shared")
