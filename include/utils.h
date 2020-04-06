// Copyright 2019 Qwant Research. Licensed under the terms of the Apache 2.0
// license. See LICENSE in the project root.

#pragma once
#include <string>
#include <vector>
// #include <boost/algorithm/string.hpp>

// void Trim(std::string& s);

void Split(const std::string& line, std::vector<std::string>& pieces, const std::string del=" ");

std::string Join(const std::vector<std::string>& words, const std::string del=" ");

bool EndsWith(std::string const &fullString, std::string const suffix);

std::string EscapeRegex(std::string text);

std::string FindAndReplace(std::string text, const std::string to_replace_from, const std::string to_replace_with) ;

const std::string currentDateTime();
