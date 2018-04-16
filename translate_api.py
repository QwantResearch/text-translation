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


inet='172.30.1.5'
iport=8888

class BPEEngine(object):
    
    cors = globalcors
    def __init__(self):
        # _filename=filename
        # codes = codecs.open(bpe_filename, encoding='utf-8')
        self._bpe={}
        # self._bpe[str(src_language)+"-"+str(tgt_language)]=apply_bpe.BPE(codes)
        
    def apply_bpe_function(self, text,src_language,tgt_language):
        # return self._bpe.segment(text.strip())
        sys.stderr.write("PROCESS BPE: "+str(src_language)+"-"+str(tgt_language)+"\n BPE: "+text+"\n")
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
        # _filename=model_filename
        self._transmodel={}
        # sys.stderr.write("SET MODEL: "+str(src_language)+"-"+str(tgt_language)+"\n Model: "+model_filename+"\n BPE: "+bpe_filename+"\n")
        # print ("CREATION MODEL: "+str(src_language)+"-"+str(tgt_language))
        # self._transmodel[str(src_language)+"-"+str(tgt_language)] = qtranslate.qtranslate(model_filename,bpe_filename,src_language,tgt_language,2)
        #self._bpe=apply_bpe.BPE(codes)
        
    def translate_function(self, text,src_language,tgt_language):
        # print ("TRANSLATION: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text)
        sys.stderr.write("ASK TRANSLATION: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
        if src_language+"-"+tgt_language in self._transmodel:
            l_translated = self._transmodel[str(src_language)+"-"+str(tgt_language)].process_translation(text.strip())
            sys.stderr.write("TRANSLATION: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+" ||| "+l_translated+"\n")
            return l_translated
        else:
            sys.stderr.write("TRANSLATION ERROR: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
            # print ("TRANSLATION ERROR: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+")
            return "None"

    def set_translation_model(self,model_filename,bpe_filename,src_language,tgt_language):
        sys.stderr.write("SET MODEL: "+str(src_language)+"-"+str(tgt_language)+"\n Model: "+model_filename+"\n BPE: "+bpe_filename+"\n")
        self._transmodel[str(src_language)+"-"+str(tgt_language)] = qtranslate.qtranslate(model_filename,bpe_filename,src_language,tgt_language,2)

    def apply_bpe_function_new(self, text,src_language,tgt_language):
        sys.stderr.write("BPE: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
        if src_language+"-"+tgt_language in self._transmodel:
            return self._transmodel[src_language+"-"+tgt_language].apply_bpe(text.strip())
        else:
            return "None"

    def tokenize(self, text,src_language,tgt_language):
        # print ("TOKENIZE: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text)
        if (len(text.split(" ")) > 70):
            text=text.replace(". ",".\n")
        text=text.replace("’","'")

        sys.stderr.write("TOKENIZE: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
        if src_language+"-"+tgt_language in self._transmodel:
            l_tokenized=self._transmodel[src_language+"-"+tgt_language].tokenize_str(text.strip())
            sys.stderr.write(l_tokenized+"\n")
            return l_tokenized
        else:
            sys.stderr.write("TOKENIZE ERROR: "+str(src_language)+"-"+str(tgt_language)+" ||| "+text+"\n")
            return "None"

    def get_language_pairs(self):
        l_lp=[]
        for l_keys in self._transmodel.keys():
            l_lp.append(l_keys)
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
    # NOTE: Starting with Falcon 1.3, you can simply
    # use req.media and resp.media for this instead.

    cors = globalcors
    def process_request(self, req, resp):
        # req.stream corresponds to the WSGI wsgi.input environ variable,
        # and allows you to read bytes from the request body.
        #
        # See also: PEP 3333
        if req.content_length in (None, 0):
            # Nothing to do
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
    # def on_get(self, req, resp):
        # # try:
            # # doc = req.context['doc']
        # # except KeyError:
            # # raise falcon.HTTPBadRequest(
                # # 'Missing thing',
                # # 'A thing must be submitted in the request body.')
        # resp.set_header('Powered-By', 'Qwant Research')
        # resp.status = falcon.HTTP_200
        # doc={}
        # doc["language_pairs"]=[]
        # doc["language_pairs"]=self.transmodel.get_language_pairs()
        # resp.context['result'] = doc
        
    def on_post(self, req, resp):
        try:
            doc = req.context['doc']
        except KeyError:
            raise falcon.HTTPBadRequest(
                'Missing thing',
                'A thing must be submitted in the request body.')
        if len(doc["text"]) > 0:
            doc["tokenized"]=self.transmodel.tokenize(doc["text"],doc["source"],doc["target"])
            doc["BPE"]=self.bpe.apply_bpe_function(doc["tokenized"],doc["source"],doc["target"])
            #doc["hypothesis"]=[[1,"result1","score1"],[2,"result2","score2"],[3,"result3","score3"],[4,"result4","score4"]]
            doc["result"]=[]
            doc["result"].append({"generic_model_"+doc["source"]+"-"+doc["target"]:[self.transmodel.translate_function(doc["tokenized"],doc["source"],doc["target"])]})
        else:
            doc["tokenized"]=""
            doc["BPE"]=""
            #doc["hypothesis"]=[[1,"result1","score1"],[2,"result2","score2"],[3,"result3","score3"],[4,"result4","score4"]]
            doc["result"]=[]
        # print (doc)
        resp.context['result'] = doc
        resp.set_header('Powered-By', 'Qwant Research')
        # resp.set_header('Access-Control-Allow-Methods', 'POST,OPTIONS,GET')
        # resp.set_header('Access-Control-Allow-Headers','Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With') 
        resp.status = falcon.HTTP_202

        #resp.status = falcon.HTTP_201
        #resp.location = '/%s/things/%s' % (user_id, proper_thing['id'])

    #def on_post(self, req, resp):
        #text = req.get_param('text') or 'test_txt'
        #src = req.get_param('src') or 'test_src'
        #tgt = req.get_param('tgt') or 'test_src'
        #nbest = req.get_param_as_int('nbest') or 1
        #print (req.content_length)
        #data = req.stream.read(req.content_length)
        #doc = req.get_param_as_json(req.bounded_stream)
        #if req.content_length:
            #print("test: ")
            #print(req.query_string)
        #print (data)
        ##doc = req.get_param_as_json(req.bounded_stream)
        #print (text,src,tgt,nbest,doc)
        #try:
            #result = [text,src]
            ##result = self.db.get_things(marker, limit)
        #except Exception as ex:
            #self.logger.error(ex)

            #description = ('Aliens have attacked our base! We will '
                           #'be back as soon as we fight them off. '
                           #'We appreciate your patience.')

            #raise falcon.HTTPServiceUnavailable(
                #'Service Outage',
                #description,
                #30)

        ## An alternative way of doing DRY serialization would be to
        ## create a custom class that inherits from falcon.Request. This
        ## class could, for example, have an additional 'doc' property
        ## that would serialize to JSON under the covers.
        ##
        ## NOTE: Starting with Falcon 1.3, you can simply
        ## use resp.media for this instead.
        #resp.context['input'] = result

        #resp.set_header('Powered-By', 'Falcon')
        #resp.status = falcon.HTTP_200

class LanguageResource(object):
    cors = globalcors   
    def __init__(self, transmodel):
        # self.bpe = bpe
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
        # try:
            # doc = req.context['doc']
        # except KeyError:
            # raise falcon.HTTPBadRequest(
                # 'Missing thing',
                # 'A thing must be submitted in the request body.')
        resp.set_header('Powered-By', 'Qwant Research')
        resp.status = falcon.HTTP_200
        doc={}
        doc["language_pairs"]=[]
        doc["language_pairs"]=self.transmodel.get_language_pairs()
        doc["languages"]={}
        doc["languages"]=self.get_language_data()
        resp.context['result'] = doc
        resp.set_header('Powered-By', 'Qwant Research')
        # resp.set_header('Access-Control-Allow-Methods', 'GET')
        # resp.set_header('Access-Control-Allow-Headers','Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With')

    


# Configure your WSGI server to load "things.app" (app is a WSGI callable)
# app = falcon.API(middleware=[
    # AuthMiddleware(),
    # RequireJSON(),
    # JSONTranslator(),
# ])
app = falcon.API(middleware=[globalcors.middleware,RequireJSON(),JSONTranslator(),])
# middleware=[
    # globalcors.middleware,RequireJSON(),JSONTranslator(),
# ])

#db = StorageEngine()
#things = ThingsResource(db)

fconfig = open("models_config.txt","r")
bpe = BPEEngine()
transEngine = TRANSEngine()
for lines in fconfig:
    l_data=lines.split()
    transEngine.set_translation_model(l_data[3],l_data[2],l_data[0],l_data[1])
    bpe.set_bpe_model(l_data[2],l_data[0],l_data[1])

# bpe.set_bpe_model("resources/fr-en/fr.bpe","fr","en")
# bpe.set_bpe_model("resources/de-en/de.bpe","de","en")
# bpe.set_bpe_model("resources/nl-en/nl.bpe","nl","en")
# bpe.set_bpe_model("resources/pt-en/pt.bpe","pt","en")
# bpe.set_bpe_model("resources/en-fr/en.bpe","en","fr")
# bpe.set_bpe_model("resources/en-de/en.bpe","en","de")
# bpe.set_bpe_model("resources/en-nl/en.bpe","en","nl")
# bpe.set_bpe_model("resources/en-pt/en.bpe","en","pt")

# transEngine.set_translation_model("resources/fr-en/model.fr-en.cpu","resources/fr-en/fr.bpe","fr","en")
# transEngine.set_translation_model("resources/de-en/model.de-en.cpu","resources/de-en/de.bpe","de","en")
# transEngine.set_translation_model("resources/nl-en/model.nl-en.cpu","resources/nl-en/nl.bpe","nl","en")
# transEngine.set_translation_model("resources/pt-en/model.pt-en.cpu","resources/pt-en/pt.bpe","pt","en")
# transEngine.set_translation_model("resources/en-fr/model.en-fr.cpu","resources/en-fr/en.bpe","en","fr")
# transEngine.set_translation_model("resources/en-de/model.en-de.cpu","resources/en-de/en.bpe","en","de")
# transEngine.set_translation_model("resources/en-nl/model.en-nl.cpu","resources/en-nl/en.bpe","en","nl")
# transEngine.set_translation_model("resources/en-pt/model.en-pt.cpu","resources/en-pt/en.bpe","en","pt")
translate = TranslationResource(bpe,transEngine)
languages= LanguageResource(transEngine)
#app.add_route('/{user_id}/things', things)
app.add_route('/translate',translate)
app.add_route('/languages',languages)

# If a responder ever raised an instance of StorageError, pass control to
# the given handler.
#app.add_error_handler(StorageError, StorageError.handle)

# Proxy some things to another service; this example shows how you might
# send parts of an API off to a legacy system that hasn't been upgraded
# yet, or perhaps is a single cluster that all data centers have to share.
#sink = SinkAdapter()
#app.add_sink(sink, r'/search/(?P<engine>ddg|y)\Z')

# Useful for debugging problems in your API; works with pdb.set_trace(). You
# can also use Gunicorn to host your app. Gunicorn can be configured to
# auto-restart workers when it detects a code change, and it also works
# with pdb.
if __name__ == '__main__':
    httpd = simple_server.make_server(inet, iport, app)
    sys.stderr.write("Translation server online on "+str(inet)+":"+str(iport)+"\n")
    httpd.serve_forever()

