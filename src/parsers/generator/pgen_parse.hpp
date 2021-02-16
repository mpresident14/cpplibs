#ifndef CONFIG_PARSE_HPP
#define CONFIG_PARSE_HPP

#include "src/logger/logger.hpp"
#include "src/parsers/generator/parse_info.hpp"
#include "src/parsers/generator/utils.hpp"

#include <ostream>
#include <string>
#include <vector>

ParseInfo parseConfig(const std::string& filename, std::ostream& warnings);

#endif
