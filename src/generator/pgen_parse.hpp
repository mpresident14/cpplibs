#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

#include "src/logger/logger.hpp"
#include "src/generator/utils.hpp"
#include "src/generator/parse_info.hpp"

#include <ostream>
#include <string>
#include <vector>

GenerateInfo parseConfig(const std::string &filename, std::ostream &warnings);

#endif
