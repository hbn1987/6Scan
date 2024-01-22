/* Copyright (C) 2019 Neo Huang - All Rights Reserved */
#pragma once

#include <string>

#include "trace6/traceroute.h"

namespace trace6 {

// Hitlist
// Load Hitlist and, for each target block, use Hitlist to select the IP
// addresses that are likly to be active.

class Hitlist {
 public:
  static void loadHitlist(const std::string& filePath,
                          Tracerouter* tracerouter);
};

}  // namespace trace6
