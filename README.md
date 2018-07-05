# Qnlu

A new Python API for Machine Translation at Qwant Research.
The API is based on tensorflow library.

Contact: christophe[dot]servan[at]qwantresearch[dot]com

## Installation

### Pre-requieres
* install qnlp-toolkit (https://github.com/QwantResearch/qnlp-toolkit.git)
* install pistache (https://github.com/oktal/pistache.git)
* install nlohmann json (https://github.com/nlohmann/json.git)
* compile tensorflow from source (see next session)

### Install Tensorflow

```
git clone https://github.com/tensorflow/tensorflow.git ~/tensorflow
cd ~/tensorflow
git checkout r1.6
```

#### Add a custom compilation target at the end of ```tensorflow/BUILD```:
```

tf_cc_shared_object(
    name = "libtensorflow_qnlp.so",
    linkopts = select({
        "//tensorflow:darwin": [
            "-Wl,-exported_symbols_list",  # This line must be directly followed by the exported_symbols.lds file
            "//tensorflow:tf_exported_symbols.lds",
        ],
        "//tensorflow:windows": [],
        "//tensorflow:windows_msvc": [],
        "//conditions:default": [
            "-z defs",
            "-s",
            "-Wl,--version-script",  #  This line must be directly followed by the version_script.lds file
            "//tensorflow:tf_version_script.lds",
        ],
    }),
    deps = [
        "//tensorflow:tf_exported_symbols.lds",
        "//tensorflow:tf_version_script.lds",
        "//tensorflow/c:c_api",
        "//tensorflow/c/eager:c_api",
        "//tensorflow/cc:cc_ops",
        "//tensorflow/cc:client_session",
        "//tensorflow/cc:scope",
        "//tensorflow/core:tensorflow",
        "//tensorflow/contrib/seq2seq:beam_search_ops_kernels",
        "//tensorflow/contrib/seq2seq:beam_search_ops_op_lib",
    ],
)
```
#### Configure & build

```
./configure
bazel build --config=opt //tensorflow:libtensorflow_qnlp.so
```

### Installation


```
  git clone https://github.com/QwantResearch/qtranslate.git 
  git checkout cpp
  mkdir -pv build && cd build && cmake .. && make 
``` 


## Launch the API


```
  ./api_nmt [#port] [#threads] [config filename]
``` 

