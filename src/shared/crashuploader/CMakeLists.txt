include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Headers code/*.h)
file(GLOB Sources code/*.cpp
)

if(WIN32)
  file(GLOB PlattformSources)
else()
  file(GLOB PlattformSources)
endif()

add_library(crashuploader SHARED ${Sources} ${PlattformSources})
target_link_libraries(crashuploader
    ${CARES_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${TINYXML_LIBRARIES}
    ${STATIC_LIBS}
)

install(TARGETS crashuploader
        DESTINATION "${LIB_INSTALL_DIR}")
