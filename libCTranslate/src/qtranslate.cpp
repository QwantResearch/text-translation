#include <iostream>

// #include <boost/program_options.hpp>
// #include <chrono>
// #include <algorithm>
#include "qnlp/tokenizer.h"
#include "qnlp/en_tokenizer.h"
#include "qnlp/fr_tokenizer.h"
#include <onmt/onmt.h>
#include "qnlp/bpe.h"
#include "BatchReader.h"
#include "BatchWriter.h"
#include "utils.h"

// namespace po = boost::program_options;

#include <pybind11/pybind11.h>
// #include "../../../../../../../usr/local/include/qnlp/fr_tokenizer.h"

using namespace pybind11;
using namespace std;
using namespace qnlp;

class qtranslate
{
    public:
        std::unique_ptr<onmt::ITranslator> _translator;
        BPE * _BPE;
        qnlp::Tokenizer * _tokenizer;
        qnlp::Tokenizer_fr * _fr_tokenizer;
        qnlp::Tokenizer_en * _en_tokenizer;
        string _src_lang;
        string _tgt_lang;
        qtranslate(string &translation_model_filename, string &BPE_model_filename, string &src_lang, string &tgt_lang, size_t threads)
        {
            _src_lang = src_lang;
            _tgt_lang = tgt_lang;
            onmt::Threads::set(threads);
            _translator = onmt::TranslatorFactory::build(translation_model_filename,
                                                        "", // phrase table
                                                        false, // replace unk
                                                        250, // max length
                                                        5, // beam size
                                                        false, // use cuda
                                                        false); // output per module computation time
            _BPE = new qnlp::BPE(BPE_model_filename);
            if (src_lang == "fr")
            {
                _fr_tokenizer = new qnlp::Tokenizer_fr(qnlp::Tokenizer::PLAIN,false,false,false,false);
            }
            else if (src_lang == "en")
            {
                _en_tokenizer = new qnlp::Tokenizer_en(qnlp::Tokenizer::PLAIN,false,false,false,false);
            }
            else
            {
                _tokenizer = new qnlp::Tokenizer(qnlp::Tokenizer::PLAIN,false,false,false,false);
            }
        }
        void load_translation_model(string &filename, size_t threads)
        {
            onmt::Threads::set(threads);
            _translator = onmt::TranslatorFactory::build(filename,
                                                        "", // phrase table
                                                        false, // replace unk
                                                        250, // max length
                                                        5, // beam size
                                                        false, // use cuda
                                                        false); // output per module computation time
        }
        void load_BPE_model(string &BPE_model_filename)
        {
            _BPE = new qnlp::BPE(BPE_model_filename);
        }
        void set_tokenizer(string &lang)
        {
            if (_src_lang == "fr")
            {
                _fr_tokenizer = new qnlp::Tokenizer_fr(qnlp::Tokenizer::PLAIN,false,false,false,false);
            }
            else if (_src_lang == "en")
            {
                _en_tokenizer = new qnlp::Tokenizer_en(qnlp::Tokenizer::PLAIN,false,false,false,false);
            }
            else
            {
                _tokenizer = new qnlp::Tokenizer(qnlp::Tokenizer::PLAIN,false,false,false,false);
            }
        }
        vector<string> tokenize(string &input)
        {
            vector<string> to_return;
            if (count_spaces(input) > 70)
            {
                cut_sentence(input);
            }
            if (_src_lang == "fr")
            {
                to_return=_fr_tokenizer->tokenize_sentence(input);
            }
            else if (_src_lang == "en")
            {
                to_return=_en_tokenizer->tokenize_sentence(input);
            }
            else
            {
                to_return=_tokenizer->tokenize_sentence(input);
            }
            return to_return;
        }
        string tokenize_str(string &input)
        {
            if (_src_lang == "fr")
            {
                return _fr_tokenizer->tokenize_sentence_to_string(input);
            }
            else if (_src_lang == "en")
            {
                return _en_tokenizer->tokenize_sentence_to_string(input);
            }
            else
            {
                return _tokenizer->tokenize_sentence_to_string(input);
            }
        }
        string apply_bpe(string &input)
        {
            string to_return=_BPE->apply_bpe_to_string(input);
            if (count_spaces(to_return) > 70)
            {
                cut_sentence(to_return);
            }
            return to_return;
        }
        vector<string> apply_bpe_vec(string &input)
        {
            return _BPE->apply_bpe(input);
        }
        string apply_bpe_vec_to_str(vector<string> &input)
        {
            return _BPE->apply_bpe_to_string(input);
        }
        string detokenize(string &input)
        {
            vector<int> pos;
            string to_return = input;
            for (int i = 0 ; i < (int)to_return.size() - 3 ; i++)
            {
                if (to_return[i] == '@' && to_return[i+1] == '@' && to_return[i+2] == ' ')
                {
                    pos.push_back(i);
                }
            }
            for (int i = (int)pos.size() - 1 ; i > -1 ; i--)
            {
                to_return.erase(pos[i],3);
            }
            return to_return;
        }
        string process_translation(string &input)
        {
            istringstream iss(input);
            ostringstream oss("");
            std::unique_ptr<BatchReader> reader;
            reader.reset(new BatchReader(iss, 16));
            std::unique_ptr<BatchWriter> writer;
            writer.reset(new BatchWriter(oss));


              for (vector<string> batch = reader->read_next(); !batch.empty(); batch = reader->read_next())
              {
                vector<string> trans = _translator->translate_batch(batch);
                writer->write(trans);
              }
            string trans = oss.str();
            trans = detokenize(trans);  
            return trans;
        }
        string process_translation_with_BPE(string &input)
        {
            istringstream iss(apply_bpe(input));
            ostringstream oss("");
            std::unique_ptr<BatchReader> reader;
            reader.reset(new BatchReader(iss, 16));
            std::unique_ptr<BatchWriter> writer;
            writer.reset(new BatchWriter(oss));


              for (vector<string> batch = reader->read_next(); !batch.empty(); batch = reader->read_next())
              {
                vector<string> trans = _translator->translate_batch(batch);
                writer->write(trans);
              }
            string trans = oss.str();
            trans = detokenize(trans);  
            return trans;
        }
        string process_translation_with_all_preprocess(string &input)
        {
            vector<string> tokenized= tokenize(input);
            string BPEed=apply_bpe_vec_to_str(tokenized);
//             if (count_spaces(BPEed) > 70)
//             {
                cut_sentence(BPEed);
//             }
//             protect_paragraph(BPEed);
            istringstream iss(BPEed);
            ostringstream oss("");
            std::unique_ptr<BatchReader> reader;
            reader.reset(new BatchReader(iss, 16));
            std::unique_ptr<BatchWriter> writer;
            writer.reset(new BatchWriter(oss));
            for (vector<string> batch = reader->read_next(); !batch.empty(); batch = reader->read_next())
            {
                vector<int> ids = pre_process_batch(batch);
                vector<string> trans = _translator->translate_batch(batch);
                post_process_batch(trans,ids);
                writer->write(trans);
            }
            string trans = oss.str();
            trans = detokenize(trans);  
            return trans;
        }
        int count_spaces(string &text) 
        {
            int count = 0;
            for (int i = 0; i < (int)text.size(); i++) 
            {
                if (text[i] == ' ') 
                {      
                    count++;
                }
            }
            return count;
        }
        void cut_sentence_txt(string &text) 
        {
            for (int i = 0; i < (int)text.size()-3; i++) 
            {
                if (text[i] >= 'a' && text[i] <= 'z' && text[i+1] == '.' && text[i+2] == ' ') 
                {      
                    text[i+2] = '\n';
                }
            }
        }
        void cut_sentence(string &text) 
        {
            cut_sentence_counts(text, 0);
        }
        void cut_sentence_counts(string &text, int count_limit) 
        {
            int count = 0;
            for (int i = 0; i < (int)text.size()-3; i++) 
            {
                if (text[i] == ' ') 
                {      
                    count++;
                }
                if (text[i] == ' ' && text[i+1] == '.' && text[i+2] == ' ' && count > count_limit) 
                {      
                    text[i+2] = '\n';
                    count = 0;
                }
            }
        }
        vector<int> pre_process_batch(vector<string> &batch)
        {
            vector<int> l_list;
            for (int i=0; i<(int)batch.size(); i++)
            {
                if (batch.at(i).length() < 1) 
                {      
                    l_list.push_back(i);
                }                
            }
            for (int i = (int)l_list.size()-1; i > -1; i--) 
            {
                batch.erase(batch.begin()+l_list[i]);
            }
            return l_list;
        }
        void detect_anormalities(vector<string> &batch, vector<string> &trans)
        {
            if (batch.size() != trans.size())
            {
                cerr << "Error input and ouput size differs" <<endl;
                return;
            }
            vector<string> l_sent_input;  
            vector<string> l_sent_output;  
            for (int i=0; i<(int)batch.size(); i++)
            {
                l_sent_input.clear();
                l_sent_output.clear();
                Split(batch.at(i),l_sent_input," ");
                Split(trans.at(i),l_sent_output," ");
                if (((int)l_sent_output.size() > 0) && (((int)l_sent_input.size() / (int)l_sent_output.size()) > 3 || ((int)l_sent_output.size() / (int)l_sent_input.size()) > 3))
                {
                    int osize=(int)l_sent_output.size();
                    for (int j=osize-1; j > 1 ; j++)
                    {
                        if (l_sent_output[j] == l_sent_output[j-1])
                        {
                            l_sent_output.erase (l_sent_output.begin()+j);
                        }
                    }
                    trans.at(i)=Join(l_sent_output," ");
                }
            }
        }
        void post_process_batch(vector<string> &batch, vector<int> l_list)
        {
//             vector<string> l_sent=batch
            for (int i=0; i<(int)l_list.size(); i++)
            {
                batch.insert(batch.begin()+l_list[i],"");
            }
        }
//         void protect_paragraph(string &text) 
//         {
//             int count = 0;
//             vector<int> l_list;
//             for (int i = 0; i < (int)text.size()-2; i++) 
//             {
//                 if (text[i] == '\n' && text[i+1] == '\n') 
//                 {      
//                     l_list.push_back(i);
//                 }
//             }
//             for (int i = (int)l_list.size()-1; i > -1; i--) 
//             {
//                 text.insert(l_list[i]+1,"@@ ");
//             }
//         }
};


PYBIND11_MODULE(qtranslate, t) 
{
//     m.def("process_translation",&process_translation, "process translation.");
    pybind11::class_<qtranslate>(t, "qtranslate")
        .def(pybind11::init<std::string&, std::string&, std::string&, std::string&, size_t >())
        .def("detokenize", &qtranslate::detokenize)
        .def("tokenize", &qtranslate::tokenize)
        .def("tokenize_str", &qtranslate::tokenize_str)
        .def("apply_bpe", &qtranslate::apply_bpe)
        .def("apply_bpe_vec", &qtranslate::apply_bpe_vec)
        .def("process_translation_with_BPE", &qtranslate::process_translation_with_BPE)
        .def("process_translation_with_all_preprocess", &qtranslate::process_translation_with_all_preprocess)
        .def("process_translation", &qtranslate::process_translation);
}

