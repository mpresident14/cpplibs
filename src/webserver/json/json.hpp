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

class JsonObject;
class JsonArray final : public JsonElement {
public:
  JsonArray(std::vector<std::unique_ptr<JsonElement>>&& elems) : elems_(std::move(elems)) {}

  bool get_bool(size_t index) const { return get_element<JsonBool>(index, "bool")->value_; }

  const std::string& get_string(size_t index) const {
    return get_element<JsonString>(index, "string")->value_;
  }

  long get_long(size_t index) const { return get_element<JsonLong>(index, "long")->value_; }

  const JsonArray& get_array(size_t index) const { return *get_element<JsonArray>(index, "array"); }

  const JsonObject& get_object(size_t index) const {
    return *get_element<JsonObject>(index, "array");
  }

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

private:
  template <typename T>
  T* get_element(size_t index, const char* type) const {
    T* element = dynamic_cast<T*>(elems_.at(index).get());
    if (element == nullptr) {
      throw std::invalid_argument("JsonElement at " + std::to_string(index) + " is not a " + type);
    }
    return element;
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

  bool get_bool(const std::string& key) { return get_element<JsonBool>(key, "bool")->value_; }

  const std::string& get_string(const std::string& key) const {
    return get_element<JsonString>(key, "string")->value_;
  }

  long get_long(const std::string& key) const { return get_element<JsonLong>(key, "long")->value_; }

  const JsonArray& get_array(const std::string& key) const {
    return *get_element<JsonArray>(key, "array");
  }

  const JsonObject& get_object(const std::string& key) const {
    return *get_element<JsonObject>(key, "array");
  }


  std::unordered_map<std::string, std::unique_ptr<JsonElement>> entries_;

private:
  template <typename T>
  T* get_element(const std::string& key, const char* type) const {
    T* element = dynamic_cast<T*>(entries_.at(key).get());
    if (element == nullptr) {
      throw std::invalid_argument("JsonElement at " + key + " is not a " + type);
    }
    return element;
  }


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
