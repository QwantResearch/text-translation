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

## Query example

### Translation route

Ask for a translation:
```
curl -X POST \
  http://localhost:8888/translate \
  -H 'Cache-Control: no-cache' \
  -H 'Content-Type: application/json' \
  -d '{"text":"This is a test to be sure what I do.","source":"en","target":"fr","count":10}'
```
which response:
```
{
    "target": "fr",
    "text": "This is a test to be sure what I do.",
    "count": 10,
    "source": "en",
    "domain": "all",
    "result": [
        {
            "generic_model_en-fr": [
                "C' est un test pour être sûr de ce que je fais .\n"
            ]
        },
        {
            "full_model_en-fr": [
                "C' est un test pour être sûr de ce que je fais .\n"
            ]
        }
    ]
}
```

### language route
Ask which translation models are available:
```
curl -X GET \
  http://localhost:8888/languages \
  -H 'Authorization: customized-token' \
  -H 'Cache-Control: no-cache' \
  -H 'Content-Type: application/json'
```
Which responds:
```
{
    "language_pairs": [
        "en-fr-generic",
        "en-fr-full",
        "fr-en-generic",
    ],
    "languages": {
        "de": [
            "Allemand",
            "de"
        ],
        "pt": [
            "Portugais",
            "pt"
        ],
        "en": [
            "Anglais",
            "gb"
        ],
        "it": [
            "Italien",
            "it"
        ],
        "fr": [
            "Français",
            "fr"
        ],
        "es": [
            "Espagnol",
            "es"
        ],
        "cs": [
            "Tchèque",
            "cz"
        ],
        "nl": [
            "Néerlandais",
            "nl"
        ]
    }
}
```
