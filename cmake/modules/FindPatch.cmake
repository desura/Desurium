# - Find patch executable
# Find the (BSD/GNU) patch executable for Windows systems
#
#  PATCH_EXECUTABLE - where to find patch.exe
#  PATCH_FOUND      - True if patch is found.

find_program(PATCH_EXECUTABLE
  NAME patch
  PATHS "$ENV{ProgramFiles}/Git/bin"
        "$ENV{ProgramFiles(x86)}/Git/bin"
)

# handle the QUIETLY and REQUIRED arguments and set PATCH_FOUND to TRUE if 
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Patch DEFAULT_MSG PATCH_EXECUTABLE)