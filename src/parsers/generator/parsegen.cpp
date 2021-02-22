#include "src/logger/logger.hpp"
#include "src/parsers/generator/generate.hpp"
#include "src/parsers/generator/pgen_parse.hpp"

#include <iostream>
#include <string>
#include <string_view>

#include <unistd.h>


int main(int argc, char** argv) {
  const char* errMsg = "Usage: parsegen -g <grammar_file> -d <output_dir> -i <include_path> "
                       "-n name [-l <log_file>]";

  std::string pgenFile;
  std::string outDir;
  std::string includePath;
  std::string name;
  std::string logFile;
  char c;
  while ((c = getopt(argc, argv, "g:d:i:n:l:")) != -1) {
    switch (c) {
    case 'g':
      pgenFile = optarg;
      break;
    case 'd':
      outDir = optarg;
      break;
    case 'i':
      includePath = optarg;
      break;
    case 'n':
      name = optarg;
      break;
    case 'l':
      logFile = optarg;
      break;
    default:
      std::cerr << errMsg << std::endl;
      return 1;
    }
  }

  if (pgenFile.empty() || outDir.empty() || name.empty()) {
    std::cerr << errMsg << std::endl;
    return 1;
  }

  if (!pgenFile.ends_with(".pgen")) {
    std::cerr << "Grammar file must have a .pgen extension" << std::endl;
    return 1;
  }

  try {
    std::ostringstream warnings;
    GenerateInfo generateInfo = parseConfig(pgenFile, warnings);
    generateParserCode(generateInfo, {outDir, includePath, name, logFile}, warnings);
    const std::string& warningsStr = warnings.str();
    if (!warningsStr.empty()) {
      std::cerr << warnings.str() << std::endl;
    }
    return 0;
  } catch (Logger::Exception& e) {
    std::cerr << pgenFile << ":\n" << e.what() << std::endl;
    return 1;
  }
}
