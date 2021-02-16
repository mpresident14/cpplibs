#include "src/parsers/generator/regex.hpp"

#include <algorithm>
#include <functional>
#include <numeric>
#include <stdexcept>

using namespace std;

ostream& operator<<(ostream& out, RgxType type) {
  switch (type) {
    case RgxType::EMPTYSET:
      return out << "EMPTYSET";
    case RgxType::EPSILON:
      return out << "EPSILON";
    case RgxType::CHARACTER:
      return out << "CHARACTER";
    case RgxType::ALT:
      return out << "ALT";
    case RgxType::CONCAT:
      return out << "CONCAT";
    case RgxType::STAR:
      return out << "STAR";
    case RgxType::RANGE:
      return out << "RANGE";
    case RgxType::NOT:
      return out << "NOT";
    default:
      throw invalid_argument("Out of options");
  }
}

/***************************
 * Derivative Optimization *
 ***************************/

RgxPtr makeConcat(RgxPtr r1, RgxPtr r2) {
  RgxType r1Type = r1->getType();
  RgxType r2Type = r2->getType();
  // ∅ r2 = ∅
  // r1 ∅ = ∅
  if (r1Type == RgxType::EMPTYSET || r2Type == RgxType::EMPTYSET) {
    return make_shared<EmptySet>();
  }
  // ε r2 = r2
  if (r1Type == RgxType::EPSILON) {
    return r2;
  }
  // r1 ε = r1
  if (r2Type == RgxType::EPSILON) {
    return r1;
  }

  if (r1Type == RgxType::CONCAT) {
    vector<RgxPtr>& r1Vec = static_cast<Concat*>(r1.get())->rVec_;
    vector<RgxPtr> newVec(r1Vec.cbegin(), r1Vec.cend());
    // [r1s] [r2s] = [r1s + r2s]
    if (r2Type == RgxType::CONCAT) {
      vector<RgxPtr>& r2Vec = static_cast<Concat*>(r2.get())->rVec_;
      copy(r2Vec.cbegin(), r2Vec.cend(), back_inserter(newVec));
      return make_shared<Concat>(move(newVec));
    }
    // [r1s] r2 = [r1s + r2]
    newVec.push_back(r2);
    return make_shared<Concat>(move(newVec));
  }
  // r1 [r2s] = [r1 + r2s]
  if (r2Type == RgxType::CONCAT) {
    vector<RgxPtr> newVec = { r1 };
    vector<RgxPtr>& r2Vec = static_cast<Concat*>(r2.get())->rVec_;
    copy(r2Vec.cbegin(), r2Vec.cend(), back_inserter(newVec));
    return make_shared<Concat>(move(newVec));
  }

  return make_shared<Concat>(vector<RgxPtr>{ r1, r2 });
}

RgxPtr makeAlt(RgxPtr r1, RgxPtr r2) {
  RgxType r1Type = r1->getType();
  RgxType r2Type = r2->getType();
  // ^∅ | r2 = ^∅
  // r1 | ^∅ = ^∅
  if ((r1Type == RgxType::NOT && static_cast<Not*>(r1.get())->rgx_->getType() == RgxType::EMPTYSET)
      || (r2Type == RgxType::NOT
          && static_cast<Not*>(r2.get())->rgx_->getType() == RgxType::EMPTYSET)) {
    return make_shared<Not>(new EmptySet);
  }
  // ∅ | r2 = r2
  if (r1Type == RgxType::EMPTYSET) {
    return r2;
  }
  // r1 | ∅ = r1
  if (r2Type == RgxType::EMPTYSET) {
    return r1;
  }

  if (r1Type == RgxType::ALT) {
    unordered_set<RgxPtr, Regex::PtrHash>& r1Set = static_cast<Alt*>(r1.get())->rSet_;
    unordered_set<RgxPtr, Regex::PtrHash> newSet(r1Set.cbegin(), r1Set.cend());
    // Alt [r1s] | Alt [r2s] = Alt [r1s + r2s]
    if (r2Type == RgxType::ALT) {
      unordered_set<RgxPtr, Regex::PtrHash>& r2Set = static_cast<Alt*>(r2.get())->rSet_;
      copy(r2Set.cbegin(), r2Set.cend(), inserter(newSet, newSet.end()));
      return make_shared<Alt>(move(newSet));
    }
    // Alt [r1s] | r2 = Alt [r1s + r2]
    newSet.insert(r2);
    return make_shared<Alt>(move(newSet));
  }
  // r1 | Alt [r2s] = Alt [r1 + r2s]
  if (r2Type == RgxType::ALT) {
    unordered_set<RgxPtr, Regex::PtrHash>& r2Set = static_cast<Alt*>(r2.get())->rSet_;
    unordered_set<RgxPtr, Regex::PtrHash> newSet(r2Set.cbegin(), r2Set.cend());
    newSet.insert(r1);
    return make_shared<Alt>(move(newSet));
  }

  // NOTE: r1 = r2 is covered by the set itself
  return make_shared<Alt>(unordered_set<RgxPtr, Regex::PtrHash>{ r1, r2 });
}

RgxPtr makeConcats(vector<RgxPtr>&& rs) {
  if (rs.empty()) {
    make_shared<Epsilon>();
  }
  return accumulate(rs.cbegin() + 1, rs.cend(), rs[0], makeConcat);
}

RgxPtr makeAlts(unordered_set<RgxPtr, Regex::PtrHash>&& rs) {
  if (rs.empty()) {
    make_shared<EmptySet>();
  }
  auto iter = rs.cbegin();
  const RgxPtr& rgx = *iter;
  return accumulate(++iter, rs.cend(), rgx, makeAlt);
}

RgxPtr makeStar(RgxPtr r) {
  switch (r->getType()) {
    case RgxType::STAR:
      return r;
    case RgxType::EPSILON:
      return r;
    case RgxType::EMPTYSET:
      return make_shared<Epsilon>();
    default:
      return make_shared<Star>(r);
  }
}

/************
 * EmptySet *
 ************/
bool EmptySet::isNullable() const { return false; }
RgxPtr EmptySet::getDeriv(char) const { return make_shared<EmptySet>(); }
bool EmptySet::operator==(const Regex& other) const { return other.getType() == RgxType::EMPTYSET; }
RgxType EmptySet::getType() const { return RgxType::EMPTYSET; }
size_t EmptySet::hashFn() const noexcept { return 0; }
void EmptySet::toStream(ostream& out) const { out << "EMPTYSET"; }

/***********
 * Epsilon *
 ***********/
bool Epsilon::isNullable() const { return true; }
RgxPtr Epsilon::getDeriv(char) const { return make_shared<EmptySet>(); }
bool Epsilon::operator==(const Regex& other) const { return other.getType() == RgxType::EPSILON; }
RgxType Epsilon::getType() const { return RgxType::EPSILON; }
size_t Epsilon::hashFn() const noexcept { return 0; }
void Epsilon::toStream(ostream& out) const { out << "EPSILON"; }

/*******
 * Dot *
 *******/
bool Dot::isNullable() const { return false; }
RgxPtr Dot::getDeriv(char) const { return make_shared<Epsilon>(); }
bool Dot::operator==(const Regex& other) const { return other.getType() == RgxType::DOT; }
RgxType Dot::getType() const { return RgxType::DOT; }
size_t Dot::hashFn() const noexcept { return 0; }
void Dot::toStream(ostream& out) const { out << "DOT"; }

/*************
 * Character *
 *************/
Character::Character(char c) : c_{ c } {}

bool Character::isNullable() const { return false; }

RgxPtr Character::getDeriv(char c) const {
  if (c == c_) {
    return make_shared<Epsilon>();
  }
  return make_shared<EmptySet>();
}

RgxType Character::getType() const { return RgxType::CHARACTER; }

bool Character::operator==(const Regex& other) const {
  return other.getType() == RgxType::CHARACTER && static_cast<const Character&>(other).c_ == c_;
}

size_t Character::hashFn() const noexcept { return hash<char>()(c_); }

void Character::toStream(ostream& out) const { out << "CHAR " << c_; }

/*******
 * Alt *
 *******/
Alt::Alt(Regex* r1, Regex* r2) : rSet_{ RgxPtr(r1), RgxPtr(r2) } {}
Alt::Alt(std::unordered_set<RgxPtr, Regex::PtrHash>&& rSet, Regex* r) : rSet_(move(rSet)) {
  rSet_.emplace(r);
}
Alt::Alt(std::unordered_set<RgxPtr, Regex::PtrHash>&& rSet) : rSet_(move(rSet)) {}

Alt::Alt(const string& charVec) {
  for (char c : charVec) {
    rSet_.insert(make_shared<Character>(c));
  }
}

bool Alt::isNullable() const {
  return any_of(rSet_.cbegin(), rSet_.cend(), [](const RgxPtr rPtr) { return rPtr->isNullable(); });
}

RgxPtr Alt::getDeriv(char c) const {
  unordered_set<RgxPtr, Regex::PtrHash> derivs;
  transform(rSet_.cbegin(), rSet_.cend(), inserter(derivs, derivs.end()), [c](const RgxPtr rPtr) {
    return rPtr->getDeriv(c);
  });
  return makeAlts(move(derivs));
}

RgxType Alt::getType() const { return RgxType::ALT; }

bool Alt::operator==(const Regex& other) const {
  return other.getType() == RgxType::ALT && rSet_ == static_cast<const Alt&>(other).rSet_;
}

size_t Alt::hashFn() const noexcept {
  Regex::PtrHash hasher;
  size_t hashSum = 0;
  for (const RgxPtr& rgx : rSet_) {
    hashSum ^= hasher(rgx) << 1;
  }
  return hashSum ^ static_cast<int>(RgxType::ALT);
}

void Alt::toStream(ostream& out) const { out << "ALT " << rSet_; }

/**********
 * Concat *
 **********/

Concat::Concat(Regex* r1, Regex* r2) : rVec_{ RgxPtr(r1), RgxPtr(r2) } {}
Concat::Concat(std::vector<RgxPtr>&& rVec, Regex* r) : rVec_(move(rVec)) {
  rVec_.push_back(RgxPtr(r));
}
Concat::Concat(std::vector<RgxPtr>&& rVec) : rVec_(move(rVec)) {}

bool Concat::isNullable() const {
  return all_of(rVec_.cbegin(), rVec_.cend(), [](const RgxPtr rPtr) { return rPtr->isNullable(); });
}

RgxPtr Concat::getDeriv(char c) const {
  if (rVec_.empty()) {
    return make_shared<EmptySet>();
  }

  vector<RgxPtr> derivAndRest = { rVec_[0]->getDeriv(c) };
  copy(rVec_.cbegin() + 1, rVec_.cend(), back_inserter(derivAndRest));

  if (rVec_[0]->isNullable()) {
    vector<RgxPtr> rest(rVec_.cbegin() + 1, rVec_.cend());
    return makeAlt(makeConcats(move(rest))->getDeriv(c), makeConcats(move(derivAndRest)));
  }

  return makeConcats(move(derivAndRest));
}

RgxType Concat::getType() const { return RgxType::CONCAT; }

bool Concat::operator==(const Regex& other) const {
  return other.getType() == RgxType::CONCAT && static_cast<const Concat&>(other).rVec_ == rVec_;
}

size_t Concat::hashFn() const noexcept {
  Regex::PtrHash hasher;
  size_t hashSum = 0;
  for (const RgxPtr& rgx : rVec_) {
    hashSum ^= hasher(rgx) << 1;
  }
  return hashSum ^ static_cast<int>(RgxType::ALT);
}

void Concat::toStream(ostream& out) const { out << "CONCAT " << rVec_; }

/********
 * Star *
 ********/
Star::Star(Regex* rgx) : rgx_(RgxPtr(rgx)) {}
Star::Star(RgxPtr rgx) : rgx_(rgx) {}

bool Star::isNullable() const { return true; }
RgxPtr Star::getDeriv(char c) const { return makeConcat(rgx_->getDeriv(c), makeStar(rgx_)); }

RgxType Star::getType() const { return RgxType::STAR; }

bool Star::operator==(const Regex& other) const {
  return other.getType() == RgxType::STAR && *static_cast<const Star&>(other).rgx_ == *rgx_;
}

size_t Star::hashFn() const noexcept { return rgx_->hashFn(); }

void Star::toStream(ostream& out) const { out << "STAR (" << rgx_ << ')'; }

/*********
 * Range *
 *********/
Range::Range(char start, char end) : start_(start), end_(end) {}
Range::Range(pair<char, char> range) : start_(get<0>(range)), end_(get<1>(range)) {}
bool Range::isNullable() const { return false; }
RgxPtr Range::getDeriv(char c) const {
  if ((start_ <= c && c <= end_)) {
    return make_shared<Epsilon>();
  }
  return make_shared<EmptySet>();
}
RgxType Range::getType() const { return RgxType::RANGE; }

bool Range::operator==(const Regex& other) const {
  return other.getType() == RgxType::RANGE && static_cast<const Range&>(other).start_ == start_
         && static_cast<const Range&>(other).end_ == end_;
}

size_t Range::hashFn() const noexcept {
  hash<char> hasher;
  return hasher(start_) ^ (hasher(end_) << 1);
}

void Range::toStream(std::ostream& out) const { out << "RANGE (" << start_ << "-" << end_ << ')'; }

/*******
 * Not *
 *******/
Not::Not(Regex* rgx) : rgx_(RgxPtr(rgx)) {}
Not::Not(RgxPtr rgx) : rgx_(rgx) {}
bool Not::isNullable() const { return false; }
RgxPtr Not::getDeriv(char c) const {
  switch (rgx_->getDeriv(c)->getType()) {
    case RgxType::EPSILON:
      return make_shared<EmptySet>();
    case RgxType::EMPTYSET:
      return make_shared<Epsilon>();
    default:
      throw invalid_argument("Invalid NOT derivative type");
  }
}

RgxType Not::getType() const { return RgxType::NOT; }

bool Not::operator==(const Regex& other) const {
  return other.getType() == RgxType::NOT && *static_cast<const Not&>(other).rgx_ == *rgx_;
}

size_t Not::hashFn() const noexcept { return rgx_->hashFn(); }

void Not::toStream(ostream& out) const { out << "NOT (" << rgx_ << ')'; }
