#ifndef INJECTOR_HPP
#define INJECTOR_HPP

#define INJECT(ctorDecl)                                                    \
  int injector_dummy_var = injector::addCtor<ctorDecl>(__LINE__, __FILE__); \
  ctorDecl

#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <prez/print_stuff.hpp>

namespace {

using namespace std;

/* Templates to extract the class type and parameter types from a constructor. */
template <typename R, typename... Args>
struct CtorTypes;

template <typename R, typename... Args>
struct CtorTypes<R(Args...)> {
  string classType = typeid(R).name();
  vector<string> argTypes = { typeid(Args).name()... };
};

/* Info to be stored in bindings map for some class. */
struct CtorInfo {
  std::vector<std::string> argTypes;
  size_t line;
  const char* filename;
};

unordered_map<string, CtorInfo> constructors;
// unordered_map<string, InstanceInfo> constructors;

}  // namespace


namespace injector {

/* Add constructor type info to the constructors map. */
template <typename F>
int addCtor(size_t line, const char* filename) {
  CtorTypes<F> ctorTypes;
  auto p = constructors.emplace(
      move(ctorTypes.classType), CtorInfo{ move(ctorTypes.argTypes), line, filename });
  if (!p.second) {
    throw runtime_error(
        "INTERNAL ERROR: Should not reach here because injector_dummy_var should have been "
        "redefined within class "
        + ctorTypes.classType);
  }

  cout << p.first->first << endl;
  cout << p.first->second.argTypes << endl;
  cout << p.first->second.line << endl;
  cout << p.first->second.filename << endl;

  return 0;
}

}  // namespace injector


#endif  // INJECTOR_HPP
