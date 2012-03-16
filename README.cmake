How to compile Desurium using cmake:

Run ./build_cmake.sh

In order to make use of multiple CPU cores while compiling, run
./build_cmake.sh -j8
which will invoke "make -j8" so the build is speeded up extremely
only 7 mins on my machine (intel core i7 2635QM)
"8" will be the number of compile-thread to run at a time

Required dependencies (can be installed by package manager or install-deps.sh):
boost
c-ares
gtk2
OpenSSL
tinyxml
v8

Optional dependencies (will be built by cmake if not existing):
bzip2
sqlite

cef is needed from official distribution see #5
Just copy required libs to install/lib

Finally run Desurium using ./run_desurium.sh
