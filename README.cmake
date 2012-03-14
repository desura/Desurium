easy way to build with cmake:

run ./build_cmake.sh

of course you can build it with multicore support
./build_cmake.sh -j8
will invoke "make -j8" so the build is speeded up extremely
only 7 mins on my machine (intel core i7 2635QM)

required dependencies (all can be installed by package manager):
boost
c-ares
gtk2
OpenSSL
tinyxml
v8

optional dependencies (will be built by cmake if not exist):
bzip2
sqlite

cef is needed from official distribution see #5
just copy required libs to install/lib

finally run desurium with ./run_desurium.sh
