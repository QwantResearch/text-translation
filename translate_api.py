import json
import logging
import sys
import uuid
from wsgiref import simple_server

import falcon
import requests
import apply_bpe
import codecs
import qtranslate
from falcon_cors import CORS

globalcors=CORS(allow_all_origins=True,allow_all_headers=True,allow_all_methods=True)

# IP and port ask for the API
inet='172.30.1.5'
iport=8888

class BPEEngine(object):
    
    cors = globalcors
    def __init__(self):
    # initializing BPE models
        self._bpe={}
        
    def apply_bpe_function(self, text,src_language,tgt_language):
        sys.stderr.write("PROCESS BPE: "+str(src_language)+"-"+str(tgt_language)+"\n BPE MODEL: "+text+"\n")
        if src_language+"-"+tgt_language in self._bpe:
            return self._bpe[str(src_language)+"-"+str(tgt_language)].segment(text.strip())
        else:
            return "None"

    def set_bpe_model(self,bpe_filename,src_language,tgt_language):
        sys.stderr.write("SET BPE MODEL: "+str(src_language)+"-"+str(tgt_language)+"\n BPE: "+bpe_filename+"\n")
        codes = codecs.open(bpe_filename, encoding='utf-8')
        self._bpe[str(src_language)+"-"+str(tgt_language)] = apply_bpe.BPE(codes)

class TRANSEngine(object):
    
    cors = globalcors
    def __init__(self):
    # initializing translation models
        self._transmodel={}
        
    def translate_function(self, text,src_language,tgt_language,l_domain):
        sys.stderr.write("ASK TRANSLATION: "+str(src_language)+"-"+str(tgt_language)+" ||| "+l_domain+" ||| "+text+"\n")
        if src_language+"-"+tgt_language in self._transmodel:
            l_translated = self._transmodel[str(src_language)+"-"+str(tgt_language)][l_domain].process_translation(text.strip())
            sys.stderr.write("TRANSLATION: "+str(src_language)+"-"+str(tgt_language)+" ||| "+l_domain+" ||| "+text+" ||| "+l_translated+"\n")
            return l_translated
        else:
            sys.stderr.write("TRANSLATION ERROR: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
            return "None"

    def set_translation_model(self,model_filename,bpe_filename,src_language,tgt_language,l_domain):
        sys.stderr.write("SET MODEL: "+str(src_language)+"-"+str(tgt_language)+"\n Model: "+model_filename+"\n DOMAIN: "+l_domain+"\n")
        if str(src_language)+"-"+str(tgt_language) not in self._transmodel:
            self._transmodel[str(src_language)+"-"+str(tgt_language)]={}
        self._transmodel[str(src_language)+"-"+str(tgt_language)][l_domain] = qtranslate.qtranslate(model_filename,bpe_filename,src_language,tgt_language,2)

    def apply_bpe_function_new(self, text,src_language,tgt_language):
        sys.stderr.write("BPE: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
        if src_language+"-"+tgt_language in self._transmodel:
            return self._transmodel[src_language+"-"+tgt_language].apply_bpe(text.strip())
        else:
            return "None"

    def tokenize(self, text,src_language,tgt_language):
        if (len(text.split(" ")) > 70):
            text=text.replace(". ",".\n")
        text=text.replace("’","'")

        sys.stderr.write("TOKENIZE: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
        if src_language+"-"+tgt_language in self._transmodel:
            l_tokenized=self._transmodel[src_language+"-"+tgt_language]["generic"].tokenize_str(text.strip())
            sys.stderr.write(l_tokenized+"\n")
            return l_tokenized
        else:
            sys.stderr.write("TOKENIZE ERROR: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
            return "None"

    def get_language_pairs(self):
        l_lp=[]
        for l_keys in self._transmodel.keys():
            for l_domain in self._transmodel[l_keys].keys():
                l_lp.append(l_keys+"-"+l_domain)
        return l_lp
    def get_domains(self,src_language,tgt_language):
        l_lp=[]
        for l_domain in self._transmodel[src_language+"-"+tgt_language].keys():
            l_lp.append(l_domain)
        return l_lp

class AuthMiddleware(object):
    cors = globalcors
    def process_request(self, req, resp):
        token = req.get_header('Authorization')
        account_id = req.get_header('Account-ID')

        challenges = ['Token type="Fernet"']

        if token is None:
            description = ('Please provide an auth token '
                           'as part of the request.')
            print (description)
            raise falcon.HTTPUnauthorized('Auth token required',
                                          description,
                                          challenges,
                                          href='http://docs.example.com/auth')

        if not self._token_is_valid(token, account_id):
            description = ('The provided auth token is not valid. '
                           'Please request a new token and try again.')

            raise falcon.HTTPUnauthorized('Authentication required',
                                          description,
                                          challenges,
                                          href='http://docs.example.com/auth')

    def _token_is_valid(self, token, account_id):
        return True  # Suuuuuure it's valid...


class RequireJSON(object):

    cors = globalcors
    def process_request(self, req, resp):
        if not req.client_accepts_json:
            raise falcon.HTTPNotAcceptable(
                'This API only supports responses encoded as JSON.',
                href='http://docs.examples.com/api/json')

        if req.method in ('POST', 'PUT'):
            if 'application/json' not in req.content_type:
                raise falcon.HTTPUnsupportedMediaType(
                    'This API only supports requests encoded as JSON.',
                    href='http://docs.examples.com/api/json')


class JSONTranslator(object):
    cors = globalcors
    def process_request(self, req, resp):
        if req.content_length in (None, 0):
            return

        body = req.stream.read()
        if not body:
            raise falcon.HTTPBadRequest('Empty request body',
                                        'A valid JSON document is required.')

        try:
            req.context['doc'] = json.loads(body.decode('utf-8'))

        except (ValueError, UnicodeDecodeError):
            raise falcon.HTTPError(falcon.HTTP_753,
                                   'Malformed JSON',
                                   'Could not decode the request body. The '
                                   'JSON was incorrect or not encoded as '
                                   'UTF-8.')

    def process_response(self, req, resp, resource):
        if 'result' not in resp.context:
            return

        resp.body = json.dumps(resp.context['result'])


def max_body(limit):

    def hook(req, resp, resource, params):
        length = req.content_length
        if length is not None and length > limit:
            msg = ('The size of the request is too large. The body must not '
                   'exceed ' + str(limit) + ' bytes in length.')

            raise falcon.HTTPRequestEntityTooLarge(
                'Request body is too large', msg)

    return hook


class TranslationResource(object):
    cors = globalcors
    def __init__(self, bpe, transmodel):
        self.bpe = bpe
        self.logger = logging.getLogger('qtranslate_api.' + __name__)
        self.transmodel = transmodel 


    @falcon.before(max_body(64 * 1024))
    def on_post(self, req, resp):
# Here is the heart of the API: the translation part.
        try:
            doc = req.context['doc']
        except KeyError:
            raise falcon.HTTPBadRequest(
                'Missing thing',
                'A thing must be submitted in the request body.')
        if len(doc["text"]) > 0:
            doc["tokenized"]=self.transmodel.tokenize(doc["text"],doc["source"],doc["target"])
            doc["BPE"]=self.bpe.apply_bpe_function(doc["tokenized"],doc["source"],doc["target"])
            doc["result"]=[]
            if "domain" in doc:
                if doc["domain"] == "":
                    doc["domain"] = "all"
            else:
                doc["domain"] = "all"
            if doc["domain"] == "all":
                for l_domain in self.transmodel.get_domains(doc["source"],doc["target"]):
                    doc["result"].append({l_domain+"_model_"+doc["source"]+"-"+doc["target"]:[self.transmodel.translate_function(doc["BPE"],doc["source"],doc["target"],l_domain)]})
            else:
                doc["result"].append({doc["domain"]+"_model_"+doc["source"]+"-"+doc["target"]:[self.transmodel.translate_function(doc["BPE"],doc["source"],doc["target"],doc["domain"])]})
        else:
            doc["tokenized"]=""
            doc["BPE"]=""
            doc["result"]=[]
        resp.context['result'] = doc
        resp.set_header('Powered-By', 'Qwant Research')
        resp.status = falcon.HTTP_202


class LanguageResource(object):
    cors = globalcors   
    def __init__(self, transmodel):
        self.logger = logging.getLogger('qtranslate_api.' + __name__)
        self.transmodel = transmodel 
    
    def get_language_data(self):
        return {
        "de": ['Allemand', 'de'],
        "en": ['Anglais', 'gb'],
        "es": ['Espagnol', 'es'],
        "fr": ['Français', 'fr'],
        "nl": ['Néerlandais', 'nl'],
        "pt": ['Portugais', 'pt'],
        "cs": ['Tchèque', 'cz'],
        "it": ['Italien', 'it']
        }


    @falcon.before(max_body(64 * 1024))
    def on_get(self, req, resp):
        resp.set_header('Powered-By', 'Qwant Research')
        resp.status = falcon.HTTP_200
        doc={}
        doc["language_pairs"]=[]
        doc["language_pairs"]=self.transmodel.get_language_pairs()
        # doc["language_domains"]=self.transmodel.get_language_pairs()
        doc["languages"]={}
        doc["languages"]=self.get_language_data()
        resp.context['result'] = doc
        resp.set_header('Powered-By', 'Qwant Research')

    


app = falcon.API(middleware=[globalcors.middleware,RequireJSON(),JSONTranslator(),])

#loading all models described in models_config.txt
fconfig = open("models_config.txt","r")
bpe = BPEEngine()
transEngine = TRANSEngine()
for lines in fconfig:
    l_data=lines.split()
    transEngine.set_translation_model(l_data[3],l_data[2],l_data[0],l_data[1],l_data[4])
    bpe.set_bpe_model(l_data[2],l_data[0],l_data[1])

translate = TranslationResource(bpe,transEngine)
languages= LanguageResource(transEngine)
app.add_route('/translate',translate)
app.add_route('/languages',languages)

if __name__ == '__main__':
# launching the server with the port and IP described in global variables.
    httpd = simple_server.make_server(inet, iport, app)
    sys.stderr.write("Translation server online on "+str(inet)+":"+str(iport)+"\n")
    httpd.serve_forever()

