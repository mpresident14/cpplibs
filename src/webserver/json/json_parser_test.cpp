#include "src/testing/unit_test.hpp"
#include "src/webserver/json/json.hpp"
#include "src/webserver/json/json_parser.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

using namespace prez::unit_test;
namespace json = prez::webserver::json;

std::string& remove_whitespace(std::string& str) {
  std::remove_if(str.begin(), str.end(), [](char c) { return std::isspace(c); });
  return str;
}

TEST(test_parse_0) {
  std::ifstream fstrm("src/webserver/json/testdata/test0.json");
  if (!fstrm.is_open()) {
    throw std::invalid_argument("not open");
  }
  std::string json_str =
      std::string(std::istreambuf_iterator<char>{fstrm}, std::istreambuf_iterator<char>{});

  auto json_root = json_parser::parseString(json_str);

  assertEquals(json_root->get_bool("my_bool"), false);

  const json::JsonArray& json_arr = json_root->get_array("my_array");
  json_arr.get_object(0);
  assertEquals(json_arr.get_bool(1), true);
  assertEquals(json_arr.get_long(2), 0);
  assertEquals(json_arr.get_array(3).get_string(0), "hey");

  const json::JsonObject& json_obj = json_root->get_object("my_obj");
  assertEquals(json_obj.get_long("my_num"), 234);
}

int main() { return runTests(); }
