#ifndef DFA_HPP
#define DFA_HPP

#include "src/parsers/generator/utils.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <prez/print_stuff.hpp>

template <
    typename V,
    typename T,
    typename HashV = std::hash<V>,
    typename EqV = std::equal_to<V>,
    typename HashT = std::hash<T>,
    typename EqT = std::equal_to<T>>
class DFA {
public:
  template <
      typename V2,
      typename T2,
      typename HashV2,
      typename EqV2,
      typename HashT2,
      typename EqT2>
  friend class DFA;

  struct Node {
  public:
    friend class DFA;

    Node(const V& value) : value_(value) {}
    Node(V&& value) : value_(std::move(value)) {}

    bool operator==(const Node& other) const noexcept { return value_ == other.value_; }

    friend std::ostream& operator<<(std::ostream& out, const Node& node) noexcept {
      return out << node.value_;
    }

    const V& getValue() const noexcept { return value_; }

    const std::unordered_map<T, Node*, HashT, EqT>& getTransitions() const noexcept {
      return transitions_;
    }

    size_t getId() const noexcept { return id_; }

  private:
    V value_;
    std::unordered_map<T, Node*, HashT, EqT> transitions_;
    size_t id_;
  };

  friend struct Node;

  DFA(V value) : root_(new Node(value)), valueToNode_{{&root_->value_, root_}}, size_(1) {}

  ~DFA() {
    // If invalidated via move construction/assignment
    if (!root_) {
      return;
    }

    // Delete via BFS
    std::queue<const Node*> q;
    q.push(root_);
    std::unordered_set<const Node*> visited = {root_};
    while (!q.empty()) {
      const Node* node = q.front();
      q.pop();
      for (auto& keyVal : node->transitions_) {
        const Node* successor = keyVal.second;
        if (!visited.contains(successor)) {
          q.push(successor);
          visited.insert(successor);
        }
      }
      delete node;
    }
  };

  DFA(const DFA& other) = delete;
  DFA(DFA&& other)
      : root_(other.root_), valueToNode_(move(other.valueToNode_)), size_(other.size_) {
    other.root_ = nullptr;
  }
  DFA& operator=(const DFA& other) = delete;
  DFA& operator=(DFA&& other) {
    root_ = other.root_;
    size_ = other.size_;
    valueToNode_ = move(other.valueToNode_);
    other.root_ = nullptr;
  }

  Node* getRoot() const noexcept { return root_; }

  Node* run(const std::vector<T>& input) const {
    Node* currentNode = root_;
    for (const T& inputToken : input) {
      currentNode = step(currentNode, inputToken);
      if (currentNode == nullptr) {
        return nullptr;
      }
    }
    return currentNode;
  }

  /* Step from node with transition inputToken */
  static Node* step(Node* node, const T& inputToken) {
    auto iter = node->transitions_.find(inputToken);
    if (iter == node->transitions_.end()) {
      return nullptr;
    }
    return iter->second;
  }

  /* Add a transition to an existing or new node */
  template <typename T2 = T, typename V2 = V>
  Node* addTransition(Node* node, T2&& transition, V2&& newNodeValue) {
    // No duplicate or updated transitions
    auto transIterBool = node->transitions_.try_emplace(std::forward<T2>(transition), nullptr);
    if (!transIterBool.second) {
      // No duplicate or updated transitions
      return nullptr;
    }

    // If a node with this value already exists, just add a transition to the
    // existing node
    auto nodeIter = valueToNode_.find(&newNodeValue);
    if (nodeIter != valueToNode_.end()) {
      transIterBool.first->second = nodeIter->second;
      return nullptr;
    }

    // Otherwise, create a new node
    Node* newNode = new Node(std::forward<V2>(newNodeValue));
    transIterBool.first->second = newNode;
    valueToNode_.emplace(&newNode->value_, newNode);
    newNode->id_ = size_++;
    return newNode;
  }


  template <typename F1, typename F2>
  void streamAsCode(
      std::ostream& out,
      const std::string& newValueType,
      const std::string& newTranType,
      const F1& valueToStr,
      const F2& tranToStr) const {
    std::ostringstream init;
    std::ostringstream nodeDecls;
    std::ostringstream tranStmts;

    init << "struct "
         << "Node {\n"
         << "Node(" << newValueType << "&& v) : v_(std::move(v)) {}\n";

    init << R"(
      Node* step(const )"
         << newTranType << R"(& t) const {
        auto iter = ts_.find(t);
        if (iter == ts_.end()) {
          return nullptr;
        }
        return iter->second;
      }
    )";

    init << newValueType << " v_;\n"
         << "std::unordered_map<" << newTranType << ", Node*> ts_;};\n";

    tranStmts << "auto makeDFA(){\n";

    std::unordered_set<const Node*> visited = {root_};
    std::queue<const Node*> q;
    q.push(root_);

    while (!q.empty()) {
      const Node* currentNode = q.front();
      q.pop();

      // Add node declaration
      nodeDecls << "auto n" << currentNode << "=std::make_unique<Node>("
                << valueToStr(currentNode->value_) << ");\n";

      // Add the transitions
      tranStmts << 'n' << currentNode << "->ts_={\n";
      for (const auto& tranAndNode : currentNode->transitions_) {
        const Node* successor = tranAndNode.second;
        tranStmts << '{' << tranToStr(tranAndNode.first) << ',' << 'n' << successor << ".get()},\n";
        if (!visited.contains(successor)) {
          visited.insert(successor);
          q.push(successor);
        }
      }
      tranStmts << "};\n";
    }

    tranStmts << "return std::move(n" << root_ << ");\n}\nauto root=makeDFA();";

    // Write declarations and statements to the file
    // TODO: Change to .view() when implemented
    out << init.str() << '\n' << nodeDecls.str() << tranStmts.str();
  }

  size_t size() const noexcept { return size_; }

  friend std::ostream& operator<<(std::ostream& out, const DFA& dfa) {
    std::queue<const Node*> q;
    q.push(dfa.root_);
    std::unordered_set<const Node*> visited = {dfa.root_};
    while (!q.empty()) {
      const Node* node = q.front();
      out << node->value_ << '\n';
      q.pop();
      for (auto& keyVal : node->getTransitions()) {
        const Node* successor = keyVal.second;
        out << "\t[" << keyVal.first << "] -> " << node->getValue() << '\n';
        if (!visited.contains(successor)) {
          q.push(successor);
          visited.insert(successor);
        }
      }
    }
    return out;
  }

  // These structs allow us to store V*s as hashmap keys, which prevent
  // expensive copies, since the values themselves are already stored in
  // the corresponding Node.
  struct VPtrHash {
    size_t operator()(const V* vptr) const noexcept { return HashV()(*vptr); }
  };

  struct VPtrEquals {
    bool operator()(const V* vptr1, const V* vptr2) const noexcept { return EqV()(*vptr1, *vptr2); }
  };

private:
  void addDirectTransition(Node* fromNode, T transition, Node* toNode) {
    fromNode->transitions_.emplace(std::move(transition), toNode);
  }

  Node* root_;
  // Allows us to check whether a node with some value exists in the
  // DFA and grab a pointer to it.
  std::unordered_map<const V*, Node*, VPtrHash, VPtrEquals> valueToNode_;
  // We don't use valueToNode_.size() because of the comment in convert.
  size_t size_;
};

#endif
