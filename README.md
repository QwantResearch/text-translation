# Qtranslate

A new Python API for Neural Machine Translation (NMT) at Qwant Research.
The API is based on OpenNMT a lua/Torch7 toolkit for MT.

Contact: c.servan@qwantresearch.com

## Installation

```  git clone https://github.com/QwantResearch/qtranslate.git
  git clone https://github.com/QwantResearch/CTranslate.git 
  git clone https://github.com/QwantResearch/qnlp-toolkit.git 
  git clone https://github.com/eigenteam/eigen-git-mirror.git 
  
  sudo -H python3 -m pip --upgrade pytest pybind11 falcon requests json wsgiref falcon falcon_cors 
  
  pushd eigen-git-mirror && mkdir build && cd build && cmake .. && make -j4 && sudo make install && popd 
  pushd CTranslate && git submodule update --init && mkdir build && cd build && cmake .. && make -j4 && sudo make install && popd
  pushd qnlp-toolkit && mkdir build && cd build && cmake .. && make -j4 && sudo make install && popd 
  cd qtranslate/libCTranslate/ && bash cbuild.sh 
  
``` 

## Launch the API

Set up the models in the file `models_config.txt` and set the IP and desired listened port in the file `translate_api.py`.
Then:

```  
  cd qtranslate
  python3 translate_api.py
``` 

