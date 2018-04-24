#pragma once

#include <fstream>
#include <string>
#include <vector>

class BatchWriter
{
public:
  BatchWriter(const std::string& file);
  BatchWriter(std::ostream& out);

  void write(const std::vector<std::string>& batch);
  void write(const std::vector<std::string>& batch, std::vector<int>& ids);

private:
  std::ofstream _file;
  std::ostream& _out;
};
