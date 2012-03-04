include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
)

set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

file(GLOB CommonHeaders ./*.h)
file(GLOB CommonSources ./*.cpp ./*.h)

#add_executable(DesuraCommon ${CommonSources})
