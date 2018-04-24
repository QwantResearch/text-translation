rm -rf build
mkdir -p build 
pushd build 
cmake .. && make -j 40
rm -rfv ../../qtranslate.so
rsync -a --progress libqtranslate.so ../../qtranslate.so 
popd

