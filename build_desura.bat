IF NOT EXIST build (
  mkdir build
)
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install -G "NMake Makefiles" -DBUILD_CEF=OFF
nmake install
cd ..