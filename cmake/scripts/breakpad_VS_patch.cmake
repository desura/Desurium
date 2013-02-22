# - Correct breakpad VS solution
# A simple cmake script designed to patch breakpad so that it will build
# with /MD instead of /MT.
#
#  VCXPROJ_PATH - Path to the breakpad Visual Studio solution

file(READ ${VCXPROJ_PATH} VCXPROJ)
string(REGEX REPLACE "<RuntimeLibrary>MultiThreaded</RuntimeLibrary>"
  "<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>" VCXPROJ "${VCXPROJ}")
string(REGEX REPLACE "<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>"
  "<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>" VCXPROJ "${VCXPROJ}")
file(WRITE ${VCXPROJ_PATH} "${VCXPROJ}")