#include "tokenizer.h"

using namespace webee;

vector< string > str2vec(string& sentence, char & c)
{
    int l_inc;
    vector<string> to_return;
    to_return.clear();
    string token;
    token.clear();
    for (l_inc=0; l_inc < (int) sentence.size(); l_inc++)
    {
        if (sentence[l_inc] == c)
        {
            if ((int)token.size() > 0)
            {
                to_return.push_back(token);
                token.clear();
            }
        }
        else
        {
            token.push_back(c);
        }
    }
    if ((int)token.size() > 0)
    {
        to_return.push_back(token);
    }
    return to_return;
}



Tokenizer::Tokenizer(void)
{
    _read = false;
    _in = NULL;
    _sb = NULL;

    _lowercased=true;
    _lang="fr";
    _dash=true;
    _underscore=true;
    _aggressive=true;
    _syntax = PLAIN;
}

Tokenizer::Tokenizer (string &text) {
    _read = false;
    istringstream iss(text);
    _sb = iss.rdbuf();

    _lowercased=true;
    _lang="fr";
    _dash=true;
    _underscore=true;
    _aggressive=true;
    _syntax = PLAIN;
}

Tokenizer::Tokenizer (istream* in) {
    _read = false;
    _in = in;
    _sb = _in->rdbuf();

    _lowercased=true;
    _lang="fr";
    _dash=true;
    _underscore=true;
    _aggressive=true;
    _syntax = PLAIN;
}


Tokenizer::Tokenizer (istream* in, int& syntax, string& flag) {
    _read = false;
    _in = in;
    _sb = _in->rdbuf();

    _lowercased=true;
    _lang="fr";
    _dash=true;
    _underscore=true;
    _aggressive=true;
    _syntax = syntax;
    _flag = _flag;
}

Tokenizer::Tokenizer (istream* in, bool lowercased, string& lang) {
    _read = false;
    _in = in;
    _sb = _in->rdbuf();
    _lowercased=lowercased;
    _lang=lang;

    _dash=true;
    _underscore=true;
    _aggressive=true;
    _syntax = PLAIN; 
}
Tokenizer::Tokenizer(istream* in, bool lowercased, bool underscore, bool dash, bool aggressive, string& lang) {
    _read = false;
    _in = in;
    _sb = _in->rdbuf();
    _lowercased=lowercased;
    _underscore=underscore;
    _dash=dash;
    _aggressive=aggressive;
    _lang=lang;
    _syntax = PLAIN;
}

Tokenizer::Tokenizer(string &text, bool lowercased, bool underscore, bool dash, bool aggressive, string& lang) {
    _read = false;
    istringstream iss(text);
    _sb = iss.rdbuf();
    _lowercased=lowercased;
    _underscore=underscore;
    _dash=dash;
    _aggressive=aggressive;
    _lang=lang;
    _syntax = PLAIN;
}

Tokenizer::Tokenizer(bool lowercased, bool underscore, bool dash, bool aggressive, string& lang) {
    _read = false;
    istringstream iss("");
    _sb = iss.rdbuf();
    _lowercased=lowercased;
    _underscore=underscore;
    _dash=dash;
    _aggressive=aggressive;
    _lang=lang;
    _syntax = PLAIN;
}

vector<string> Tokenizer::tokenize_sentence(string& text)
{
    istringstream iss(text);
    vector<string> to_return;
    _sb = iss.rdbuf();
    string l_token;
    cerr << "Mon test BEFORE : " << text << endl;
    while(read (l_token,false)) 
    {
        if(!l_token.empty()) 
        {
                to_return.push_back(l_token);
                cerr << "Mon test : " << l_token << endl;
        }
//         l_token_pred = l_token;
    }
    if(!l_token.empty()) 
    {
            to_return.push_back(l_token);
            cerr << "Mon test : " << l_token << endl;
    }
    return to_return;
}

bool Tokenizer::read (string& token, bool newdoc) {
    char c;
    token.clear();
    newdoc = false;

    xmlDom dom;

    while ((c = _sb->sbumpc()) != EOF) 
    {
//         if (c == '\xC2') 
//         {
//             if ((c = _sb->sbumpc()) != EOF)
//             {
//                 if (c == '\xAD') c = '-';
//                 else c=_sb->sungetc();
//             }
//         }
        if (_syntax == XHTML)
        {
            if (parserXHTML(c, dom) == 1)
            {
                if (dom.tag == _flag) 
                    _read = dom.status;

                newdoc = _read;
            } // parserXHTML(c, dom) == 1
        } // _syntax == XHTML

        if (_read || _syntax == PLAIN)
        {
//             spe_char_processing(token,c);
            if (separatorRules(c))
            {
                if (!token.empty())
                {
                    // **** Processing of not empty token ****
                    switch(_lang_mapping.find(_lang)->second)
                    {
                        case 1: // **** Begining of the French processing **** 
                            if (french_processing(token,c)) return true;
                            break;

                        case 2:  // **** Begining of the English processing ****
                            if (english_processing(token,c)) return true;
                            break;
                    
                        default: // **** Begining of the Default Lang processing ****
                            if (default_processing(token,c)) return true;
                            break;
                    } // switch
                }
                else // token.empty() == true
                {
//                     if (spe_char_processing_empty_token(token,c)) return true;
                    // **** Processing of empty token ****
                    switch(_lang_mapping.find(_lang)->second)
                    {
                        case 1: // **** Begining of the French processing **** 
                            if (french_processing_empty_token(token,c)) return true;
                            break; 

                        case 2: // **** Begining of the English processing ****
                            if (english_processing_empty_token(token,c)) return true;
                            break;
                      
                        default: // **** Begining of the Default Lang processing ****
                            if (default_processing_empty_token(token,c)) return true;
                            break;
                    } // switch
                } // !token.empty()
            }
            else // separatorRules(c) == true
            {
                if (_lowercased)
                    c = tolower(c);

                token.push_back(c);
            } // separatorRules (c)
        } // _read || _syntax == PLAIN
    } // while

    return false;
}


/*********************
 * 
 * Function which manage the specific processing of special characters when the token is not empty.
 * 
***********************/
bool Tokenizer::spe_char_processing(string& token, char& c)
{
    if (c == '\xC2') 
    {
        if ((c = _sb->sbumpc()) != EOF)
        {
//             if (c == '\xAD') { c = '-'; if (!_dash) { token.push_back(c); return false; } else { _sb->sputbackc(c); return true; }}
            if (c == '\xAD') { c = '-'; return true; }
            else 
            if (c == '\xBB' ) { c = '"'; _sb->sungetc(); return true; }
            else 
            if (c == '\xAB' ) { c = '"'; _sb->sungetc(); return true; }
            else 
            if (c == '\xA0' ) { c = '!'; _sb->sungetc(); return true; }
            else 
            {
//                 c=_sb->sungetc();
//                 c = '\xC2';
//                 _sb->sputbackc(c);
//                 c=_sb->sungetc();
                return false;
            }
        }
        else 
        {
//             c=_sb->sungetc();
//                 c = '\xC2';
//                 _sb->sputbackc(c);
//                 c=_sb->sungetc();
            return false;
        }
    }
    return false;
}

/*********************
 * 
 * Function which manage the specific processing of special characters when the token is empty.
 * 
***********************/
bool Tokenizer::spe_char_processing_empty_token(string& token, char& c)
{
    if (c == '\xC2') 
    {
        if ((c = _sb->sbumpc()) != EOF)
        {
            if (c == '\xAD') { c = '-' ; token.push_back(c); return true; }
            else 
            if (c == '\xBB' ) {c = '"'; token.push_back(c); return true; }
            else 
            if (c == '\xAB' ) {c = '"'; token.push_back(c); return true; }
            else 
            {
                c=_sb->sungetc();
                c=_sb->sungetc();
                return false;
            }
        }
        else 
        {
            c=_sb->sungetc();
            c=_sb->sungetc();
            return false;
        }
    }
    return false;
}

/*********************
 * 
 * Function which manage the french specific processing when the token is not empty.
 * 
***********************/
bool Tokenizer::french_processing(string& token, char& c)
{
    if (!_aggressive)
    {
        if (test_nbr(token)) 
        {
            if ((int)token.size() >= 2 && token[(int)token.size()-2] <= '\x039' && token[(int)token.size()-2] >= '\x030' && token[(int)token.size()-1] == '.')
            {
                char l_char=token.substr((int)token.size()-1,1)[0];
                token=token.substr(0,(int)token.size()-1);
                c=_sb->sungetc();
                c=_sb->sungetc();
                _sb->sputbackc(l_char);
                return true;
            }
        }
        switch(c)
        {
            case '\'':
                token.push_back(c);
                if ((int)token.find("jourd") > -1) return false;
                if ((int)token.find("ock") > -1) return false;
                if ((int)token.find("-d") > -1) return false;
                if ((int)token.find("-l") > -1) return false;
                if ((int)token.find("squ") > -1) return true;
                if (token.size() >= 2 && token[0]=='q' && token[1]=='u') return true;
                if (token.size() == 2) return true;
                if (token.size() >= 3) {token=token.substr(0,((int)token.size())-1);_sb->sungetc();return true;}
                break;
            case '.':
                if (dot_processing(token,c)) return true;
                else return false;
            case ',':
                if (comma_processing(token,c)) return true;
                else return false;
                break;
            default:
                if (c < 0)
                {
                    if (c == -30 && (int)token.size() == 1)
                    {
                        token.push_back(c);
                        if ((c = _sb->sbumpc()) != EOF)
                        {
                            if (!separatorRulesInline(c))
                            {
                                token.push_back(c);
                            }
                            else
                            {
                                token=token.substr(0,((int)token.size())-1);
                                _sb->sungetc();
                                _sb->sungetc();
                                _sb->sputbackc(c);
                                return true;                                                      
                            }
                        }
                        if ((c = _sb->sbumpc()) != EOF)
                        {
                            if (!separatorRulesInline(c))
                            {
                                token.push_back(c);
                            }
                            else
                            {
                                token=token.substr(0,((int)token.size())-1);
                                _sb->sungetc();
                                _sb->sungetc();
                                _sb->sputbackc(c);
                                return true;                                                      
                            }
                        }
                        return true;
                    }
                    _sb->sungetc();
                    return true;
                }
                _sb->sungetc();
                return true;
                break;
        }
    }
    else
    {
        _sb->sungetc();
        return true;
    }
    return false;
}


/*********************
 * 
 * Function which manage the french specific processing when the token is empty.
 * 
***********************/
bool Tokenizer::french_processing_empty_token(string& token, char& c)
{
    switch(c)
    {
        case '.':
            token.push_back(c);
            if ((c = _sb->sbumpc()) != EOF)
            {
                if (c == '.')
                {
                    token.push_back(c);
                    return false;
                }
                else
                {
                    _sb->sungetc();
                    return true;
                }
            }
            else
            {
                return true;
            }
            break;
        default:
            if (c < 0)
            {
                token.push_back(c);
                if ((c = _sb->sbumpc()) != EOF)
                {
                    if (c < 0)
                    {
                        token.push_back(c);
                        if (token[0] == -62 && token[1] == -85 ) return true;
                        if (token[0] == -62 && token[1] == -96)
                        {
                            token="";
                            return false;
                        }
                    }
                    else
                    {
                        c=_sb->sungetc();
                        return true;
                    }
                      
                      
                }
                if ((c = _sb->sbumpc()) != EOF)
                {
                    if (c < 0)
                    {
                        token.push_back(c);
                    }
                    else
                    {
                        c=_sb->sungetc();
                        return true;
                    }
                }
                return true;
            }
            if (!separatorRulesInline(c) || c=='\n') 
            {
                token.push_back(c);
                return true;
            }
            break;
    }
    return false;
}


/*********************
 * 
 * Function which manage the english specific processing when the token is not empty.
 * 
***********************/
bool Tokenizer::english_processing(string& token, char& c)
{
    if (!_aggressive)
    {
        if (token.at(0) == '\'' && token.size() == 1)
        {
            if (c == 't')
            {
                token.push_back(c);
                return false;
            }
            else
            {
                token=token.substr(0,1);
                _sb->sungetc();
                _sb->sungetc();
                return true;
            }
        }
        switch(c)
        {
            case '\'':
                token.push_back(c);
                if ((c = _sb->sbumpc()) != EOF)
                {
                    if ((int)token.size() > 2 && c == 't' && token.at(((int)token.size())-2) == 'n')
                    {
                        _sb->sungetc();
                        _sb->sputbackc('\'');
                        _sb->sputbackc('n');
                        if (token.at(((int)token.size())-3) == 'a')
                        {
                                token=token.substr(0,((int)token.size())-1);
                        }
                        else
                        {
                                token=token.substr(0,((int)token.size())-2);
                        }
                        return true;
                    }
                    else
                    {
                        if ((int)token.size() == 2 && token[0]!='n')
                        {
                            _sb->sungetc();
                            return true;
                        }
                        token.push_back(c);
                        // _sb->sungetc();
                        return true;
                    }
                }
                // if (token.at(0) == '\'' 
                _sb->sungetc();
                return true;
            case '.':
                if (dot_processing(token,c)) return true;
                else return false;
                break;
            case ',':
                if (comma_processing(token,c)) return true;
                else return false;
                break;
            default:
                _sb->sungetc();
                return true;
                break;
        }
    }
    else
    {
        _sb->sungetc();
        return true;
    }
}


/*********************
 * 
 * Function which manage the english specific processing when the token is empty.
 * 
***********************/
bool Tokenizer::english_processing_empty_token(string& token, char& c)
{
    switch(c)
    {
        case '\'':
            token.push_back(c);
            if ((c = _sb->sbumpc()) != EOF)
            {
                if (c == 't' || c == 's')
                {
                    token.push_back(c);
                    return false;
                }
                else
                {
                    _sb->sungetc();
                    return true;
                }
            }
            else
            {
                return true;
            }
            break;
        case '.':
            token.push_back(c);
            if ((c = _sb->sbumpc()) != EOF)
            {
                if (c == '.')
                {
                    token.push_back(c);
                    return false;
                }
                else
                {
                    _sb->sungetc();
                    return true;
                }
            }
            else
            {
                return true;
            }
            break;
        default:
            if (c < 0)
            {
                token.push_back(c);
                if ((c = _sb->sbumpc()) != EOF)
                {
                    if (c < 0)
                    {
                        token.push_back(c);
//                                                     return false;
                    }
                    else
                    {
                        c=_sb->sungetc();
//                                         token=token.substr(0,((int)token.size())-1);
                        return true;
                    }
                      
                      
                }
                if ((c = _sb->sbumpc()) != EOF)
                {
                    if (c < 0)
                    {
                        token.push_back(c);
//                                                     return false;
                    }
                    else
                    {
                        c=_sb->sungetc();
//                                         token=token.substr(0,((int)token.size())-1);
                        return true;
                    }
                }
                return true;
            }
            if (!separatorRulesInline(c) || c=='\n') 
            {
                token.push_back(c);
                return true;
            }
            break;
    }    
    return false;
}


/*********************
 * 
>>>>>>> bad9c0e09898c00971f7481272c9ff77e390b710
 * Function which manage the default language specific processing when the token is not empty.
 * 
***********************/
bool Tokenizer::default_processing(string& token, char& c)
{
    if (!_aggressive)
    {
        switch(c)
        {
            case '.':
                if (dot_processing(token,c)) return true;
                else return false;
                break;
            case ',':
                if (comma_processing(token,c)) return true;
                else return false;
                break;
            default:
                _sb->sungetc();
                break;
        }
        _sb->sungetc();
        return true;
    }
    else
    {
        _sb->sungetc();
        return true;
    }
}


/*********************
 * 
 * Function which manage the default language specific processing when the token is empty.
 * 
***********************/
bool Tokenizer::default_processing_empty_token(string& token, char& c)
{
    switch(c)
    {
        case '.':
            token.push_back(c);
            if ((c = _sb->sbumpc()) != EOF)
            {
                if (c == '.')
                {
                    token.push_back(c);
                    return false;
                }
                else
                {
                    _sb->sungetc();
                    return true;
                }
            }
            else
            {
                return true;
            }
            break;
        default:
            if (c < 0)
            {
                token.push_back(c);
                if ((c = _sb->sbumpc()) != EOF)
                {
                    if (c < 0)
                    {
                        token.push_back(c);
//                                                     return false;
                    }
                    else
                    {
                        c=_sb->sungetc();
//                                         token=token.substr(0,((int)token.size())-1);
                        return true;
                    }
                      
                      
                }
                if ((c = _sb->sbumpc()) != EOF)
                {
                    if (c < 0)
                    {
                        token.push_back(c);
//                                                     return false;
                    }
                    else
                    {
                        c=_sb->sungetc();
//                                         token=token.substr(0,((int)token.size())-1);
                        return true;
                    }
                }
                return true;
            }
            if (!separatorRulesInline(c) || c=='\n') 
            {
                token.push_back(c);
                return true;
            }
            break;
    }
    return false;
}


bool Tokenizer::dot_processing(string& token, char& c)
{
    token.push_back(c);
    if (test_nbr(token)) 
    {
        if ((int)token.size() > 2 && token[(int)token.size()-3] <= '\x039' && token[(int)token.size()-3] >= '\x030' && token[(int)token.size()-2] == '.' && separatorRulesInline(token[(int)token.size()-1]))
        {
            
            c=_sb->sungetc();
            c=_sb->sungetc();
            _sb->sputbackc(token[(int)token.size()-1]);
            token=token.substr(0,(int)token.size()-2);
            return true;
        }
        return false;
    }
    else if (test_abrv(token)) 
    {
        if ((token.size() >= 2 && token[token.size()-2]<'\x05B' && token[token.size()-2]>'\x040') || (token.size() == 2 && (token[0]=='e' || token[0]=='i'  || token[0]=='c')))
        {
            if ((c = _sb->sbumpc()) != EOF)
            {
                if (!separatorRules(c))
                {
                    
                    token.push_back(c);
                    return false;
                }
                else
                {
                    _sb->sungetc();
                    return true;
                }
            }
            else
            {
                return true;
            }
        }
        return true;
    }
    else
    {
        c=_sb->sungetc();
        token=token.substr(0,((int)token.size())-1);
        return true;
    }
    return true;
}

bool Tokenizer::comma_processing(string& token, char& c)
{
    token.push_back(c);
    if (test_nbr(token)) 
    {
        if ((c = _sb->sbumpc()) != EOF)
        {
            if (!separatorRulesInline(c))
            {
                token.push_back(c);
                return false;
            }
            else
            {
                token=token.substr(0,((int)token.size())-1);
                _sb->sungetc();
                _sb->sputbackc(',');
                return true;
            }
        }
        return true;
    }
    else
    {
        c=_sb->sungetc();
        token=token.substr(0,((int)token.size())-1);
        return true;
    }
    return true;
}



bool Tokenizer::test_abrv(string& token)
{
    if (token.find(".") != (token.size()-1)) return true;
    int l_i =0;
    while (l_i < (int)_abrvs.size())
    {
        if ((int)token.find(_abrvs.at(l_i)) > -1 ) {return true;}
        l_i++;
    }
    if (token.size() >= 2 && token[token.size()-2]== '.' && token[token.size()-1]=='.') return false;
    if ((int)token.size() > 2 && token.find(".") == (token.size()-1)) return false;
    if ((token.size() >= 2 && token[token.size()-2]<'\x05B' && token[token.size()-2]>'\x040') || (token.size() == 2 && (token[0]=='e' || token[0]=='i' || token[0]=='c'))) return true;
    return false;
}

bool Tokenizer::test_nbr(string& token)
{
    int l_i =0;
    while (l_i < (int)token.size())
    {
        if (l_i == 0 && token[0] == '-') { l_i++; continue;} 
        if ((token[l_i] <= '\x039' && token[l_i] >= '\x030') || token[l_i] == '.'  || token[l_i] == ',')
        {
            l_i++;
        }
        else
        {
            return false;
        }
        l_i++;
    }
    return true;
}


inline bool Tokenizer::separatorRulesInline (char& c) {
//     if (c == -30 ) return true;
    if (c == -62 ) return true;
    if (c == -30 ) return true;
    if (c <= '\x020' && c > 0) {
        return true;
    } else {
        if (c == '-' && !_dash ) return false;
        if (c == '_' && !_underscore) return false;
        return (c == '\x07F');
    }
}


inline bool Tokenizer::separatorRules (char& c) {
    if (c == -62 ) return true;
    if (c == -30 ) return true;
    if (c == '-' && !_dash ) return false;
    if (c == '_' && !_underscore) return false;
    if (c <= '\x02f' && c > 0) {
        return true;
    } else {
//         return (c >= '\x03a' && c <= '\x040') || (c >= '\x05b' && c <= '\x060') || (c >= '\x07b' && c <= '\x07e');
        return (c >= '\x03a' && c <= '\x040') || (c >= '\x05b' && c <= '\x060') || (c >= '\x07b' && c <= '\x07e');
    }
}

inline int Tokenizer::parserXHTML(char& c, xmlDom& dom) {
    if (c == '<') {
        string starter;
        starter.push_back(c);

        // Catch starter
        while ((c = _sb->sbumpc()) != EOF) {
            if (!(starter.size() > 0 && (c == '-' || c == '/' || c == '?' || c == '!'))) {
                break;
            }

            if (c == ' ' && starter.empty()) continue; // syntax errors
            starter.push_back(c);
        }

        string stopper;
        map<string,string>::const_iterator it = html_codex.find(starter);

        if (it != html_codex.end()) {
            stopper = it->second;
        } else {
            return 0;
        }

        if (dom.tag == "script" && starter != "</") return 0;

        bool ignore_content = false;
        if (starter == "<?" || starter == "<!" || starter == "<!--")
            ignore_content = true;

        _sb->sungetc();

        string tag;
        string content;
        string tmp;
        bool get_tag = true;

        while ((c = _sb->sbumpc()) != EOF) {
            if (c == '>' || c == '?' || c == '!' || c == '-') {
                tmp.push_back(c);
            } else {
                tmp.clear();
            }

            if (c == ' ') {
                if (tag.empty()) continue;
                get_tag = false;
            }

            if (stopChecker(stopper, tmp)) {
                dom.tag = tag;
                dom.content = content;
                dom.status = !(starter == "</");
                return 1;
            }

            if (!ignore_content) {
                if (get_tag) {
                    tag.push_back(c);
                } else {
                    content.push_back(c);
                }
            }
        }
    }

    return 0;
}

inline bool Tokenizer::stopChecker (string& ref, string& leq) {
    size_t s_ref = ref.size();
    size_t s_leq = leq.size();
    size_t s_delta = s_leq - s_ref;

    for (int i = s_ref; i >= 0; i--) {
        if (ref[i] != leq[i+s_delta])
            return false;
    }

    return true;
}

void Tokenizer::setParam(bool lowercased, bool underscore, bool dash, bool aggressive, string& lang)
{
    _lowercased=lowercased;
    _underscore=underscore;
    _dash=dash;
    _aggressive=aggressive;
    _lang=lang;
}

void Tokenizer::addspaceRules(char& c, string& lang, string& token)
{
    return;
}

void Tokenizer::setAggressive(bool aggressive)
{
    _aggressive=aggressive;
}

void Tokenizer::setDash(bool dash)
{
    _dash=dash;
}

void Tokenizer::setLang(string& lang)
{
    _lang=lang;
}

void Tokenizer::setLowercased(bool lowercased)
{
    _lowercased=lowercased;
}

void Tokenizer::setUnderscore(bool underscore)
{
    _underscore=underscore;
}

void Tokenizer::setSyntax(int syntax)
{
    _syntax = syntax;
}
void Tokenizer::setSyntaxFlag(string& flag)
{
    _flag = flag;
}
bool Tokenizer::streamReader(string& token, int flag_type, const string& flag, int32_t& flag_id)
{
    return false;
}
