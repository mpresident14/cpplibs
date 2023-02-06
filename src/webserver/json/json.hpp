#ifndef PREZ_WEBSERVER_JSON_JSON_HPP
#define PREZ_WEBSERVER_JSON_JSON_HPP

#include <iostream>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace prez::webserver::json {

class JsonElement {
public:
  virtual ~JsonElement() {}

  friend std::ostream& operator<<(std::ostream& out, const JsonElement& elem);

protected:
  virtual void to_stream(std::ostream& out) const = 0;
};

inline std::ostream& operator<<(std::ostream& out, const JsonElement& elem) {
  elem.to_stream(out);
  return out;
}

class JsonBool final : public JsonElement {
public:
  JsonBool(bool value) : value_(value) {}

  bool value_;

protected:
  void to_stream(std::ostream& out) const override {
    out << std::boolalpha << value_ << std::noboolalpha;
  }
};

class JsonString final : public JsonElement {
public:
  JsonString(std::string_view value) : value_(value) {}

  std::string value_;

protected:
  void to_stream(std::ostream& out) const override { out << '"' << value_ << '"'; }
};

class JsonLong final : public JsonElement {
public:
  JsonLong(long value) : value_(value) {}

  long value_;

protected:
  void to_stream(std::ostream& out) const override { out << value_; }
};

class JsonArray final : public JsonElement {
public:
  JsonArray(std::vector<std::unique_ptr<JsonElement>>&& elems) : elems_(std::move(elems)) {}

  std::vector<std::unique_ptr<JsonElement>> elems_;

protected:
  void to_stream(std::ostream& out) const override {
    if (elems_.empty()) {
      out << "[]";
      return;
    }

    auto iter = elems_.cbegin();
    out << '[' << **iter;
    ++iter;
    for (; iter != elems_.cend(); ++iter) {
      out << ", " << **iter;
    }
    out << ']';
  }
};

class JsonObject final : public JsonElement {
public:
  JsonObject(std::vector<std::pair<std::string, std::unique_ptr<JsonElement>>>&& entries)
      : entries_() {
    entries_.reserve(entries.size());
    for (auto&& entry : entries) {
      entries_.insert(std::move(entry));
    }
  }

  std::unordered_map<std::string, std::unique_ptr<JsonElement>> entries_;

protected:
  void to_stream(std::ostream& out) const override {
    if (entries_.empty()) {
      out << "{}";
      return;
    }

    auto iter = entries_.cbegin();
    out << "{\n" << iter->first << ": " << *(iter->second);
    ++iter;
    for (; iter != entries_.cend(); ++iter) {
      out << ", \n" << iter->first << ": " << *(iter->second);
    }
    out << "\n}";
  }
};

} // namespace prez::webserver::json

#endif // PREZ_WEBSERVER_JSON_JSON_HPP
