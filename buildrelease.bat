mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A arm64 -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake -DBUILD_CRASHREPORTER=OFF -DBUILD_CRASHHANDLER=ON -DImageMagick_convert_EXECUTABLE="C:/Program Files/ImageMagick-7.1.1-Q16-HDRI/magick.exe" -DDATA_FILES=../data -DCMAKE_CXX_FLAGS="/DUSE_SOFT_INTRINSICS /DPOPCNT64=__popcnt64" -DENABLE_SSE=OFF -DGLM_FORCE_PURE=ON -DWITH_OPENGL=epoxy
cmake --build . --config Release
pause