#!/usr/bin/env bash

echo "Checking and compiling dependencies."
echo ""

mkdir -p sdl2-lib/
mkdir -p sdl2-bin/
mkdir -p sdl2-include/
mkdir -p sdl2-src/build

if hash gmake 2>/dev/null; then
    MAKE="gmake"
else
    MAKE="make"
fi

if [ ! -f sdl2-lib/libSDL2.a ]; then
    echo "Compiling SDL2 ..."
    echo "------------------"
    
    cd sdl2-src
    ./fix-timestamp.sh
    mkdir -p build
    cd build
    ../configure --enable-input-tslib=no --disable-shared --disable-example
    $MAKE
    
    cp build/.libs/libSDL2.a ../../sdl2-lib/
    
    cp sdl2-config ../../sdl2-bin/
    cp include/* ../../sdl2-include/SDL2/
    cd ../
    mkdir -p ../sdl2-include/SDL2
    cp include/* ../sdl2-include/SDL2/
    cd ../
    
    if [ ! -f sdl2-lib/libSDL2.a ]; then
        echo "Failed to compile sdl2. Exiting."
        exit 1
    fi

    echo "----"
    echo "Done"
    echo ""
fi

mkdir -p lua-lib/
mkdir -p lua-bin/

if [ ! -f lua-lib/liblua.a ]; then
    echo "Compiling Lua ..."
    echo "-----------------"
    
    cd lua-src
    mkdir -p src
    uname=$(uname)
    if [ "$OSTYPE" == "msys" ]; then luaplatform="mingw"
    else luaplatform="${uname,,}"
    fi
    echo "Compiling for $luaplatform"
    
    cd src
    mkdir -p build
    cd ../
    mv src/build/*.o src/
    $MAKE $luaplatform
    
    cd src
    mv *.o build;   mv *.a build
    if [ "$OSTYPE" == "msys" ]; then mv *.exe build/; mv *.dll build/
    else mv luac build/; mv lua build/; mv *.so build/
    fi
    cd build
    cp liblua.a ../../../lua-lib/
    
    cd ../../../
    
    if [ ! -f lua-lib/liblua.a ]; then
        echo "Failed to compile lua. If this is a platform mis-detection issue (the current logic is just basically lowercase uname), please submit a pull request. Exiting."
        exit 1
    fi
    
    echo "----"
    echo "Done"
    echo ""
fi
