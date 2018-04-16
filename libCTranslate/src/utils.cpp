#include "utils.h"
#include <iostream>
#include <sstream>
#include <regex>

void Trim(std::string& s) {
  boost::trim_if(s, boost::is_any_of(" \t\n"));
}

void Split(const std::string& line, std::vector<std::string>& pieces, const std::string del) {
  size_t begin = 0;
  size_t pos = 0;
  std::string token;
  while ((pos = line.find(del, begin)) != std::string::npos) {
    if (pos > begin) {
      token = line.substr(begin, pos - begin);
      if(token.size() > 0)
        pieces.push_back(token);
    }
    begin = pos + del.size();
  }
  if (pos > begin) {
    token = line.substr(begin, pos - begin);
  }
  if(token.size() > 0)
    pieces.push_back(token);
}

std::string Join(const std::vector<std::string>& words, const std::string del) {
  std::stringstream ss;
  if (words.empty()) {
    return "";
  }
  ss << words[0];
  for (size_t i = 1; i < words.size(); ++i) {
    ss << del << words[i];
  }
  return ss.str();
}

bool EndsWith(std::string const &fullString, std::string const suffix) {
  if (fullString.length() >= suffix.length()) {
    return (0 == fullString.compare(fullString.length() - suffix.length(), suffix.length(), suffix));
  } else {
    return false;
  }
}

std::string EscapeRegex(std::string text) {
  std::stringstream result;
  for(char& c: text) {
    if(c == '.' || c == '^' || c == '$' || c == '|' || c == '(' || c == ')' || c == '[' || c == ']' || c == '{' || c == '}' || c == '*' || c == '+' || c == '?' || c == '\\') {
	result << '\\';
    }
    result << c;
  }
  return result.str();
}

