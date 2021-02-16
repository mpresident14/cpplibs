#ifndef REGEX_HPP
#define REGEX_HPP

#include <memory>
#include <ostream>
#include <unordered_set>
#include <utility>
#include <vector>

enum class RgxType { EMPTYSET, EPSILON, DOT, CHARACTER, ALT, CONCAT, STAR, RANGE, NOT };

std::ostream& operator<<(std::ostream& out, RgxType type);

class Regex;
using RgxPtr = std::shared_ptr<Regex>;

class Regex {
public:
  virtual ~Regex(){};
  virtual void toStream(std::ostream& out) const = 0;
  virtual RgxPtr getDeriv(char) const = 0;
  virtual bool isNullable() const = 0;
  virtual RgxType getType() const = 0;
  virtual bool operator==(const Regex& other) const = 0;
  virtual size_t hashFn() const noexcept = 0;

  friend std::ostream& operator<<(std::ostream& out, const Regex& rgx) {
    rgx.toStream(out);
    return out;
  }

  friend std::ostream& operator<<(std::ostream& out, Regex* rgx) { return out << *rgx; }

  friend std::ostream& operator<<(std::ostream& out, RgxPtr rgx) { return out << *rgx; }

  friend bool operator==(const RgxPtr& r1, const RgxPtr& r2) noexcept { return *r1 == *r2; }

  // NOTE: This could be dangerous because it is easy to forget to use
  // Regex::PtrHash and end up with std::hash<shared_ptr<Regex>>, but
  // I didn't want to define things in namespace std.
  struct PtrHash {
    size_t operator()(const RgxPtr& rgx) const noexcept {
      return static_cast<int>(rgx->getType()) ^ (rgx->hashFn() << 1);
    }
  };
};

class EmptySet : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const noexcept override;
};

class Dot : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const noexcept override;
};

class Epsilon : public Regex {
public:
  bool isNullable() const override;
  RgxPtr getDeriv(char) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const noexcept override;
};

class Character : public Regex {
public:
  Character(char c);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  void toStream(std::ostream& out) const override;
  size_t hashFn() const noexcept override;

private:
  char c_;
};

class Alt : public Regex {
public:
  Alt(Regex* r1, Regex* r2);
  Alt(std::unordered_set<RgxPtr, Regex::PtrHash>&& rSet, Regex* r);
  Alt(std::unordered_set<RgxPtr, Regex::PtrHash>&& rSet);
  Alt(const std::string& charVec);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const noexcept override;
  void toStream(std::ostream& out) const override;

  friend RgxPtr makeAlt(RgxPtr r1, RgxPtr r2);

private:
  std::unordered_set<RgxPtr, Regex::PtrHash> rSet_;
};

class Concat : public Regex {
public:
  Concat(Regex* r1, Regex* r2);
  Concat(std::vector<RgxPtr>&& rVec, Regex* r);
  Concat(std::vector<RgxPtr>&& rVec);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const noexcept override;
  void toStream(std::ostream& out) const override;

  friend RgxPtr makeConcat(RgxPtr r1, RgxPtr r2);

private:
  std::vector<RgxPtr> rVec_;
};

class Star : public Regex {
public:
  Star(Regex* rgx);
  Star(RgxPtr rgx);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const noexcept override;
  void toStream(std::ostream& out) const override;

private:
  RgxPtr rgx_;
};

class Range : public Regex {
public:
  Range(char start, char end);
  Range(std::pair<char, char> range);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const noexcept override;
  void toStream(std::ostream& out) const override;

private:
  char start_;
  char end_;
};

/* *
 * Match a single character NOT present in bracketed list
 * (e.g. [^ab...z] or [^a-z])
 * */
class Not : public Regex {
public:
  Not(Regex* rgx);
  Not(RgxPtr rgx);
  bool isNullable() const override;
  RgxPtr getDeriv(char c) const override;
  RgxType getType() const override;
  bool operator==(const Regex& other) const override;
  size_t hashFn() const noexcept override;
  void toStream(std::ostream& out) const override;

  friend RgxPtr makeAlt(RgxPtr r1, RgxPtr r2);

private:
  /* This must be a Character, Range, or Concat (enforced by lexer and parser)
   */
  RgxPtr rgx_;
};

#endif
