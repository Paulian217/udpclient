if [ -e build ]; then
    rm -rf build
fi
cmake -Bbuild && make -C build/ -j16