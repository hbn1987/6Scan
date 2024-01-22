/* Copyright (C) 2019 Neo Huang - All Rights Reserved */
#pragma once

#include <unordered_set>
#include <vector>

#include "trace6/address.h"
#include "trace6/trie.h"

namespace trace6 {

class BogonFilter {
 public:
  explicit BogonFilter(const std::string& filePath);

  bool isBogonAddress(const IpAddress& ip);

 private:
  std::unique_ptr<TrieManager> trie_;
  bool initialized_;
};

}  // namespace trace6
