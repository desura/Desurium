include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${COMMON_INCLUDE_DIRS}/service_pipe
  ${COMMON_INCLUDE_DIRS}/tool_pipe
  ${Boost_INCLUDE_DIR}
  ${CEF_INCLUDE_DIRS}
  ${IPC_PIPE_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
)

if(UNIX AND 32BIT_SUPPORT)
  add_definitions(-DUSE_BITTEST)
endif()

add_definitions(-DDESURA_CLIENT)
if(WIN32)
  add_definitions(-DSVENHACK -DDESURA_DIABLO2_HACK)
  set(WINDOWS_LIBRARIES Comctl32 crypt32)
#  add_linker_flags(/NOD:LIBCMT)
endif()

file(GLOB Sources code/BaseItemServiceTask.cpp
                  code/BaseItemTask.cpp
                  code/BDManager.cpp
                  code/BranchInfo.cpp
                  code/BranchInstallInfo.cpp
                  code/CDKeyManager.cpp
                  code/CIPManager.cpp
                  code/ComplexLaunchServiceTask.cpp
                  code/CreateMCFThread.cpp
                  code/DownloadTask.cpp
                  code/DownloadToolItemTask.cpp
                  code/DownloadToolTask.cpp
                  code/DownloadUpdateTask.cpp
                  code/GatherInfoTask.cpp
                  code/GatherInfoThread.cpp
                  code/GetItemListThread.cpp
                  code/InstallCheckTask.cpp
                  code/InstalledWizardThread.cpp
                  code/InstallInfo.cpp
                  code/InstallServiceTask.cpp
                  code/InstallToolTask.cpp
                  code/ItemHandle.cpp
                  code/ItemHandleEvents.cpp
                  code/ItemInfo.cpp
                  code/ItemManager.cpp
                  code/ItemTaskGroup.cpp
                  code/ItemThread.cpp
                  code/Log.cpp
                  code/McfManager.cpp
                  code/ToolInfo.cpp
                  code/ToolInstallThread.cpp
                  code/ToolManager.cpp
                  code/ToolTransaction.cpp
                  code/UIBaseServiceTask.cpp
                  code/UIBranchServiceTask.cpp
                  code/UIComplexModServiceTask.cpp
                  code/UIPatchServiceTask.cpp
                  code/UIServiceTask.cpp
                  code/UIUpdateServiceTask.cpp
                  code/UpdateThread.cpp
                  code/UpdateThread_Old.cpp
                  code/UploadInfoThread.cpp
                  code/UploadManager.cpp
                  code/UploadPrepThread.cpp
                  code/UploadResumeThread.cpp
                  code/UploadThread.cpp
                  code/UserCoreMain.cpp
                  code/User.cpp
                  code/UserIPCPipeClient.cpp
                  code/User_Login.cpp
                  code/UserTask.cpp
                  code/UserTasks.cpp
                  code/UserThreadManager.cpp
                  code/ValidateTask.cpp
                  code/VerifyServiceTask.cpp
                  code/VSBaseTask.cpp
                  code/VSCheckInstall.cpp
                  code/VSCheckMcf.cpp
                  code/VSCheckMcfDownload.cpp
                  code/VSDownloadMissing.cpp
                  code/VSInstallMissing.cpp
)

if(WIN32)
  file(GLOB PlattformSources
                  code/GameExplorerInfo_win.cpp
                  code/GameExplorerManager_win.cpp
                  code/ItemHandle_win.cpp
                  code/ToolInstallThread_win.cpp
                  code/ToolIPCPipeClient.cpp
                  code/ToolManager_Script.cpp
                  code/ToolManager_win.cpp
                  code/User_win.cpp
)
else()
  file(GLOB PlattformSources
                  code/ItemHandle_nix.cpp
                  code/ToolInstallThread_nix.cpp
                  code/ToolManager_lin.cpp
                  code/User_nix.cpp
)
endif()
# TODO: code/InstallHandler.cpp

set(AllSources ${Sources} ${PlattformSources} ${SERVICE_PIPE_SPOURCES} ${TOOL_PIPE_SPOURCES})
setup_precompiled_header(Common.h ${COMMON_INCLUDE_DIRS}/Common.cpp AllSources)
add_library(usercore SHARED ${AllSources})
target_link_libraries(usercore
  gcJSBase
  ipc_pipe
  managers
  managers_window
  mcfcore
  threads
  util
  util_fs
  util_web
  webcore
  ${Boost_DATE_TIME_LIBRARY}
  ${CURL_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${SQLITE3X_LIBRARIES}
  ${TINYXML_LIBRARIES}
  ${WINDOWS_LIBRARIES}
)

if(WIN32)
  SetSharedRuntime(usercore)
endif()

LinkWithGTest(usercore)
install_library(usercore)
