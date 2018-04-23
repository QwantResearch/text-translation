mkdir -p build 
pushd build 
cmake .. && make
cp libqtranslate.so ../../qtranslate.so -a
popd

