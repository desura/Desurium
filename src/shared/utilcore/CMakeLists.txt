include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIR}
  ${BZIP2_INCLUDE_DIR}
  ${CEF_INCLUDE_DIRS}
  ${IPC_PIPE_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
)

file(GLOB Headers code/*.h)

file(GLOB Sources code/BZip2.cpp
                  code/CDOverView.cpp
                  code/CDProcess.cpp
                  code/ChangeDirForm.cpp
                  code/ChangeDirThread.cpp
                  code/ControlerForm.cpp
                  code/CrashDumpForm.cpp
                  code/CrashDumpThread.cpp
                  code/CrashDumpUploadThread.cpp
                  code/FlashDebug.cpp
                  code/Log.cpp
                  code/McfViewerForm.cpp
                  code/MiniDumpGenerator_Extern.cpp
                  code/ToolInstallForm.cpp
                  code/ToolInstallThread.cpp
                  code/UDFProgressPanel.cpp
                  code/UDFSettingsPanel.cpp
                  code/UninstallAllThread.cpp
                  code/UninstallDesuraForm.cpp
                  code/UtilCoreEntry.cpp
                  code/UtilCoreMain.cpp)

if(WIN32)
  use_unicode_here()
  add_definitions(-DWXUSINGDLL)
  set(WINDOWS_LIBRARIES msi Wintrust Psapi)
  add_linker_flags(/FORCE)
else()
  set(PLATTFORM_SOURCES code/Browser.cpp
                        code/BrowserTest.cpp)
endif()

add_library(utilcore SHARED ${Sources} ${PLATTFORM_SOURCES})
target_link_libraries(utilcore
	managers
	managers_window
	threads
	usercore
    util
	util_fs
	wx_controls
	${BZIP2_LIBRARIES}
    ${TINYXML_LIBRARIES}
	${wxWidgets_LIBRARIES}
	${WINDOWS_LIBRARIES}
)

install(TARGETS utilcore
        DESTINATION "${LIB_INSTALL_DIR}")
