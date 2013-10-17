# - Find nmake executable
#
#  NMAKE_EXECUTABLE - where to find nmake/jom
#  NMAKE_FOUND      - True if nmake is found.

find_program(NMAKE_EXECUTABLE
  NAMES nmake.exe
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(nmake REQUIRED_VARS NMAKE_EXECUTABLE)

mark_as_advanced(NMAKE_EXECUTABLE)
