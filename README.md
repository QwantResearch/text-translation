# text-translation

A new C++ API for Machine Translation at Qwant Research.
The API is based on marian toolkit available at https://github.com/marian-nmt/marian/

Contact: christophe[dot]servan[at]qwantresearch[dot]com

## Installation

``` bash install.sh```
or

```docker build -t texttranslation.```


## Launch the API

Launch either the API, jointly with TF serving (api_nmt_remote) or locally loaded models (api_nmt_local)
```
  ./text-translation -c [config filename]
``` 
Or 
``` 
  docker run -d -p9009:9009 -c [config filename]

``` 

