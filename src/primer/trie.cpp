#include "primer/trie.h"
#include <sys/types.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include "common/exception.h"

namespace bustub {

template <class T>
auto Trie::Get(std::string_view key) const -> const T * {

  auto node = root_;
  for(auto c: key){
    if(!node || node->children_.find(c) == node->children_.end()){
      return nullptr;
    }
    node = node->children_.at(c);
  }
  if(!node || !node->is_value_node_){
    return nullptr;
  }
  auto value_node = dynamic_cast<const TrieNodeWithValue<T>*>(node.get());
  if(value_node == nullptr){
    return nullptr;
  }
  return value_node->value_.get();
  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.

  std::shared_ptr<T> shared_value = std::make_shared<T>(std::move(value));
  std::vector<std::shared_ptr<const TrieNode>> node_stack;  // store the same node
  std::shared_ptr<const TrieNode> cur = root_;

  std::size_t key_size = key.size();
  decltype(key_size) idx = 0;

  // 1. store old path
  while (idx < key_size && cur){
    char ch = key[idx];
    idx++;
    node_stack.push_back(cur);
    cur = cur->children_.find(ch) == cur->children_.end() ? nullptr : cur->children_.at(ch);
  }

  // 2. create diff node, cur is currently at one node before where value should be inserted

  // 2.1 create leaf node
  std::shared_ptr<const TrieNodeWithValue<T>> leaf_node =
      cur ? std::make_shared<const TrieNodeWithValue<T>>(cur->children_, shared_value)
      : std::make_shared<const TrieNodeWithValue<T>>(shared_value);

  // 2.2 create diff inner nodes
  std::shared_ptr<const TrieNode> child_node = leaf_node;
  while(idx < key_size){
    key_size--;
    char ch = key[key_size];
    std::map<char, std::shared_ptr<const TrieNode>> children{{ch, child_node}};
    cur = std::make_shared<const TrieNode>(children);
    child_node = cur;
  }

  // 3. copy same node
  cur = child_node;
  for(size_t i = node_stack.size() - 1; i < node_stack.size(); i--){
    cur = std::shared_ptr<const TrieNode>(node_stack[i]->Clone());
    const_cast<TrieNode *>(cur.get())->children_[key[i]] = child_node;
    child_node = cur;
  }

  return Trie(cur);
}

auto Trie::Remove(std::string_view key) const -> Trie {
  
  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.
  
  std::vector<std::shared_ptr<const TrieNode>> node_stack;  // store the same node
  std::shared_ptr<const TrieNode> cur = root_;
  std::size_t key_size = key.size();
  decltype(key_size) idx = 0;

  // 1. store old path
  while(idx < key_size && cur){
    char ch = key[idx];
    idx++;
    node_stack.push_back(cur);
    cur = cur->children_.find(ch) == cur->children_.end() ? nullptr : cur->children_.at(ch);
  }
  if(idx != key_size || !cur || !cur->is_value_node_){
    return *this;
  }

  // 2. create end node, cur is currently at the node where value should be deleted
  auto end_node = cur->children_.empty() ? nullptr : std::make_shared<const TrieNode>(cur->children_);

  // 3. copy along path
  std::shared_ptr<const TrieNode> child_node = end_node;
  cur = end_node;
  for(size_t i = node_stack.size() - 1; i < node_stack.size(); i--){
    cur = std::shared_ptr<const TrieNode>(node_stack[i]->Clone());
    const_cast<TrieNode *>(cur.get())->children_[key[i]] = child_node;
    child_node = cur;
  }

  return Trie(cur);
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
