@echo OFF

cd ..

copy curl_deps\curl_MakefileBuild.vc curl\winbuild\MakefileBuild.vc

cd curl
cmd /C buildconf.bat

cd winbuild

echo ------------------------------------------------------
echo Building curl debug
echo ------------------------------------------------------
nmake /f Makefile.vc MODE=static WITH_SSL=static WITH_ZLIB=static DEBUG=yes GEN_PDB=yes WITH_DEVEL="../../curl_deps" USE_IDN=no

echo ------------------------------------------------------
echo Building curl release
echo ------------------------------------------------------
nmake /f Makefile.vc MODE=static WITH_SSL=static WITH_ZLIB=static DEBUG=no GEN_PDB=yes WITH_DEVEL="../../curl_deps" USE_IDN=no

echo ------------------------------------------------------
echo Building curl release static
echo ------------------------------------------------------
nmake /f Makefile.vc MODE=static WITH_SSL=no DEBUG=no GEN_PDB=no RTLIBCFG=static USE_SSSPI=no USE_IPV6=no USE_IDN=no


echo ------------------------------------------------------
echo done
echo ------------------------------------------------------

cd ..\..\build_win
