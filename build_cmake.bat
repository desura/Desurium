IF NOT EXIST build (
  mkdir build
)
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
