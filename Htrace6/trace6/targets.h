/* Copyright (C) 2019 Neo Huang - All Rights Reserved */
#pragma once

#include <string>

#include "absl/strings/string_view.h"

#include "trace6/blacklist.h"
#include "trace6/dcb_manager.h"
#include "trace6/bogon_filter.h"

namespace trace6 {

class Targets {
 public:
  Targets(const uint8_t defaultSplitTtl, const uint32_t seed,
          Blacklist* blacklist, BogonFilter* bogerFilter);

  // Load targets from file.
  DcbManager* loadTargetsFromFile(absl::string_view filePath,
                                  const uint8_t granularity,
                                  const bool LookupByPrefixSupport) const;

  // Generate targets from a range.
  DcbManager* generateTargetsFromNetwork(absl::string_view targetNetwork,
                                         const uint8_t granularity,
                                         const bool LookupByPrefixSupport) const;

 private:
  Blacklist* blacklist_;
  BogonFilter* bogerFilter_;
  uint8_t defaultSplitTtl_;
  uint32_t seed_;
  uint32_t granularity_;

};

}  // namespace trace6
