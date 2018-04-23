echo "**********************************"
echo "******* Compilation **************"
echo "**********************************"


rm -f qtranslate.so ../qtranslate.so
# c++ -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` translate.cc -o qtranslate`python3-config --extension-suffix` -I/usr/local/include/eigen3 -I/usr/local/include/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/ g++ -g -O3 -Wall -shared -std=c++11 -fPIC `python3 -m pybind11 --includes` -I/usr/local/include/eigen3 -I/usr/local/include/ -L/usr/local/lib/ -I./include -I/usr/include/python3.4m/  src/qtranslate.cpp src/BatchReader.cpp src/BatchWriter.cpp -o qtranslate.so -L/usr/local/lib/ -lfasttext -lqnlp -lonmt
echo "Releasing..."
rsync -a --progress qtranslate* ../ 
