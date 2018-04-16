#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>

using namespace std;
namespace webee {

vector<string> str2vec(string &sentence,char & c);
class Tokenizer {
    public:
        Tokenizer (void);
        Tokenizer (istream* in);
        Tokenizer (string &text);
        Tokenizer (istream* in, int& syntax, string& flag);
        Tokenizer (istream* in, bool lowercased, string& lang);
        Tokenizer (istream* in, bool lowercased, bool underscore, bool dash, bool aggressive, string& lang);
        Tokenizer (string &text, bool lowercased, bool underscore, bool dash, bool aggressive, string& lang);
        Tokenizer (bool lowercased, bool underscore, bool dash, bool aggressive, string& lang);

        bool read (string &token, bool newdoc);

        // Syntax enumerator
        enum {PLAIN, XHTML};

        struct xmlDom {
            string tag;
            string content;
            int status;
        };
        void setParam(bool lowercased, bool underscore, bool dash, bool aggressive, string& lang);
        void setLowercased(bool lowercased);
        void setUnderscore(bool underscore);
        void setDash(bool dash);
        void setAggressive(bool aggressive);
        void setLang(string& lang);

        void setSyntax (int syntax);
        void setSyntaxFlag (string& flag);
//         string tokenize_sentence(string &text);
        vector<string> tokenize_sentence(string &text);

    private:
        istream* _in;
        streambuf* _sb;

        string _flag;
        int _syntax;

        bool _lowercased; // set it to true if you want your output to be lowercased
        bool _dash; // set it to true if you want to split words with dash
        bool _underscore; // set it to true if you want to split words with underscores
        bool _aggressive; // set it to true if you want to split words with everyseparators
        string _lang; // set language in ISO (like "fr" for french or "de" for german)
        bool _read;

        bool spe_char_processing(string& token, char& c);
        bool french_processing(string& token, char& c);
        bool english_processing(string& token, char& c);
        bool default_processing(string& token, char& c);

        bool spe_char_processing_empty_token(string& token, char& c);
        bool french_processing_empty_token(string& token, char& c);
        bool english_processing_empty_token(string& token, char& c);
        bool default_processing_empty_token(string& token, char& c);

        bool streamReader (string& token, int flag_type, const string& flag, int32_t& flag_id);

        bool separatorRules (char& c);
        void addspaceRules (char& c, string& lang, string& token);
        bool separatorRulesInline (char& c);

        int parserXHTML (char& c, xmlDom& dom);
        bool stopChecker (string& ref, string& leq);
        bool test_abrv(string& token);
        bool test_nbr(string& token);
        bool dot_processing(string& token, char& c);
        bool comma_processing(string& token, char& c);
        
        const map<string,string> html_codex = { {"<",">"}, {"<!--","-->"}, {"<!", ">"},
            {"<?", "?>"}, {"</", ">"} };
        const map<string,int> _lang_mapping = { {"global",0}, {"fr",1}, {"en",2}, {"de",3}, {"it",4}, {"es",5} };
        const vector<string> _abrvs = {"Mme.","Mmes.","Mlle.","Mlles.","MM.","M.","Mr.","Vve.","Dr.","Drs.","Pr.","Prs.","Me.","Mes.","Mgr.","Art.","Fig.","art.","fig.","etc.","e.g.","i.e.","cf.","av.","bc.", "J.C.", "J.-C."};
};

}
#endif // TOKENIZER_H
