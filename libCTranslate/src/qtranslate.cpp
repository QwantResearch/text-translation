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

// namespace po = boost::program_options;

#include <pybind11/pybind11.h>
#include "../../../../../../../usr/local/include/qnlp/fr_tokenizer.h"

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
            return _BPE->apply_bpe_to_string(input);
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


              for (auto batch = reader->read_next(); !batch.empty(); batch = reader->read_next())
              {
                auto trans = _translator->translate_batch(batch);
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


              for (auto batch = reader->read_next(); !batch.empty(); batch = reader->read_next())
              {
                auto trans = _translator->translate_batch(batch);
                writer->write(trans);
              }
            string trans = oss.str();
            trans = detokenize(trans);  
            return trans;
        }
        string process_translation_with_all_preprocess(string &input)
        {
            vector<string> tokenized= tokenize(input);
            istringstream iss(apply_bpe_vec_to_str(tokenized));
            ostringstream oss("");
            std::unique_ptr<BatchReader> reader;
            reader.reset(new BatchReader(iss, 16));
            std::unique_ptr<BatchWriter> writer;
            writer.reset(new BatchWriter(oss));


              for (auto batch = reader->read_next(); !batch.empty(); batch = reader->read_next())
              {
                auto trans = _translator->translate_batch(batch);
                writer->write(trans);
              }
            string trans = oss.str();
            trans = detokenize(trans);  
            return trans;
        }
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

        // .def("load_translation_model", &qtranslate::load_translation_model)

/*
int main(int argc, char* argv[])
{
  po::options_description desc("OpenNMT Translator");
  desc.add_options()
    ("help", "display available options")
    ("model", po::value<std::string>(), "path to the OpenNMT model")
    ("src", po::value<std::string>(), "path to the file to translate (read from the standard input if not set)")
    ("tgt", po::value<std::string>(), "path to the output file (write to the standard output if not set")
    ("phrase_table", po::value<std::string>()->default_value(""), "path to the phrase table")
    ("replace_unk", po::bool_switch()->default_value(false), "replace unknown tokens by source tokens with the highest attention")
    ("batch_size", po::value<size_t>()->default_value(30), "batch size")
    ("beam_size", po::value<size_t>()->default_value(5), "beam size")
    ("max_sent_length", po::value<size_t>()->default_value(250), "maximum sentence length to produce")
    ("time", po::bool_switch()->default_value(false), "output average translation time")
    ("profiler", po::bool_switch()->default_value(false), "output per module computation time")
    ("threads", po::value<size_t>()->default_value(0), "number of threads to use (set to 0 to use the number defined by OpenMP)")
    ("cuda", po::bool_switch()->default_value(false), "use cuda when available")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
  {
    std::cerr << desc << std::endl;
    return 1;
  }

  if (!vm.count("model"))
  {
    std::cerr << "missing model" << std::endl;
    return 1;
  }

  if (vm["threads"].as<size_t>() > 0)
    onmt::Threads::set(vm["threads"].as<size_t>());

  auto translator = onmt::TranslatorFactory::build(vm["model"].as<std::string>(),
                                                   vm["phrase_table"].as<std::string>(),
                                                   vm["replace_unk"].as<bool>(),
                                                   vm["max_sent_length"].as<size_t>(),
                                                   vm["beam_size"].as<size_t>(),
                                                   vm["cuda"].as<bool>(),
                                                   vm["profiler"].as<bool>());

  std::unique_ptr<BatchReader> reader;
  if (vm.count("src"))
    reader.reset(new BatchReader(vm["src"].as<std::string>(), vm["batch_size"].as<size_t>()));
  else
    reader.reset(new BatchReader(std::cin, vm["batch_size"].as<size_t>()));

  std::unique_ptr<BatchWriter> writer;
  if (vm.count("tgt"))
    writer.reset(new BatchWriter(vm["tgt"].as<std::string>()));
  else
    writer.reset(new BatchWriter(std::cout));

  std::chrono::high_resolution_clock::time_point t1, t2;

  double total_time_s = 0;
  size_t num_sents = 0;

  for (auto batch = reader->read_next(); !batch.empty(); batch = reader->read_next())
  {
    if (vm["time"].as<bool>())
      t1 = std::chrono::high_resolution_clock::now();

    auto trans = translator->translate_batch(batch);

    if (vm["time"].as<bool>())
    {
      t2 = std::chrono::high_resolution_clock::now();
      std::chrono::duration<float> sec = t2 - t1;
      total_time_s += sec.count();
      num_sents += batch.size();
    }

    writer->write(trans);
  }

  if (vm["time"].as<bool>())
    std::cerr << "avg real\t" << total_time_s / num_sents << std::endl;

  return 0;
}
*/