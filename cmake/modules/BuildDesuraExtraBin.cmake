ExternalProject_Add(
  desura_extra_bin
  URL "${DESURA_EXTRA_BIN_URL}"
  URL_MD5 ${DESURA_EXTRA_BIN_MD5}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(
  desura_extra_bin
  source_dir
)

install_external_library(desura_extra_bin
  "${source_dir}/Desura_Uninstaller.exe"
  "${source_dir}/Blank_GDF.dll"
  "${source_dir}/courgette_desura.dll"
  "${source_dir}/dbghelp-desura.dll"
  "${source_dir}/Desura_GDF.dll"
  "${source_dir}/FreeImage.dll"
  "${source_dir}/GameuxInstallHelper.dll")
