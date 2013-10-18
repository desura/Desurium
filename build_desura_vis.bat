IF NOT EXIST build_vis (
  mkdir build_vis
)

cd build_vis
cmake .. -DCMAKE_INSTALL_PREFIX=../install_vis -G "Visual Studio 12" -DBUILD_CEF=OFF -DDEBUG=ON -DBUILD_TESTS=OFF -DBUILD_TESTING=OFF -DDEBUG_V8=ON
start Desura.sln
cd ..