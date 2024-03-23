cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE="../../3rdparty/build/conan_toolchain.cmake" -DCMAKE_POLICY_DEFAULT_CMP0091="NEW"
