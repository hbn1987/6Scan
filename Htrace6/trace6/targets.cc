/* Copyright (C) 2019 Neo Huang - All Rights Reserved */
/* Copyright (C) 2023 Bingnan Hou - All Rights Reserved */

#include "trace6/targets.h"

#include <fstream>
#include <memory>
#include <unordered_set>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "trace6/blacklist.h"
#include "trace6/dcb_manager.h"
#include "trace6/utils.h"
#include "glog/logging.h"
#include <boost/format.hpp>
#include <boost/random.hpp>

namespace trace6 {

Targets::Targets(const uint8_t defaultSplitTtl, const uint32_t seed,
                 Blacklist* blacklist, BogonFilter* bogerFilter)
    : blacklist_(blacklist),
      bogerFilter_(bogerFilter),
      defaultSplitTtl_(defaultSplitTtl),
      seed_(seed) {}

DcbManager* Targets::loadTargetsFromFile(
    absl::string_view filePath, const uint8_t granularity,
    const bool LookupByPrefixSupport) const {
  DcbManager* dcbManager =
      new DcbManager(1000, granularity, seed_, LookupByPrefixSupport);
  if (filePath.empty()) {
    LOG(INFO) << "Targets disabled.";
    return dcbManager;
  }

  LOG(INFO) << "Load targets from file: " << filePath;
  auto filePathStr = std::string(filePath);
  std::ifstream in(filePathStr);
  int64_t count = 0;
  for (std::string line; std::getline(in, line);) {
    if (!line.empty()) {
      std::vector<absl::string_view> parts = absl::StrSplit(line, "/");
      if (parts.size() != 2)
        LOG(FATAL) << "Target network format is incorrect!!! " << line;
      uint32_t subnetPrefixLength = 0;
      if (!absl::SimpleAtoi(parts[1], &subnetPrefixLength)) {
        LOG(FATAL) << "Failed to parse the target network.";
      }

      IpAddress* targetBaseAddress =
          parseIpFromStringToIpAddress(std::string(parts[0]));

      IpAddress* targetNetworkFirstAddress_ =
          getFirstAddressOfBlock(*targetBaseAddress, subnetPrefixLength);
      IpAddress* targetNetworkLastAddress_ =
          getLastAddressOfBlock(*targetBaseAddress, subnetPrefixLength);

      if (*targetNetworkFirstAddress_ >= *targetNetworkLastAddress_) {
        LOG(FATAL) << "Ip address range is incorrect.";
      }

      // LOG(INFO) << boost::format("The target network is from %1% to %2%.") %
      //               parseIpFromIpAddressToString(*targetNetworkFirstAddress_) %
      //               parseIpFromIpAddressToString(*targetNetworkLastAddress_);

      // absl::uint128 targetNetworkSize =
      //   ntohll(targetNetworkLastAddress_->getIpv6Address()) -
      //   ntohll(targetNetworkFirstAddress_->getIpv6Address()) + 1;

      absl::uint128 randFactor_ =
          static_cast<absl::uint128>(std::pow(2, 128 - subnetPrefixLength - 4));

      // absl::uint128 dcbCount =
      //     static_cast<absl::uint128>(targetNetworkSize / blockFactor_);
      // set random seed.
      // std::srand(seed_);
      boost::random::mt19937 gen;

      // 定义要生成的随机数的范围（例如，128位整数的范围）
      boost::random::uniform_int_distribution<absl::uint128> dist(
          std::numeric_limits<absl::uint128>::min(),
          std::numeric_limits<absl::uint128>::max()
      );

      for (absl::uint128 i = 0; i < 16; i++) {
        // 生成128位随机数
        absl::uint128 random_number = dist(gen);
        Ipv6Address tmp(htonll(
            ntohll(targetNetworkFirstAddress_->getIpv6Address()) +
            ((i) << (128 - subnetPrefixLength - 4)) + (random_number % (randFactor_ - 3)) + 2));
        if (blacklist_ != nullptr && !blacklist_->contains(tmp)) {
          dcbManager->addDcb(tmp, defaultSplitTtl_, subnetPrefixLength);
          // LOG(INFO) << i << ": " << parseIpFromIpAddressToString(tmp);
        }
      }

      for (auto i = subnetPrefixLength + 4; i <= granularity; i=i+4) {
        // 生成128位随机数
        absl::uint128 random_number = dist(gen);
        absl::uint128 randFactor_ =
          static_cast<absl::uint128>(std::pow(2, 128 - i));
          Ipv6Address tmp(htonll(
            ntohll(targetNetworkFirstAddress_->getIpv6Address()) + (random_number % (randFactor_ - 3)) + 2));
        if (blacklist_ != nullptr && !blacklist_->contains(tmp)) {
          dcbManager->addDcb(tmp, defaultSplitTtl_, subnetPrefixLength);
          // LOG(INFO) << i << ": " << parseIpFromIpAddressToString(tmp);
        }

      }
    count++;
    }
  }
  in.close();
  LOG(INFO) << "Load " << count << " prefixes from file.";
  return dcbManager;
}

DcbManager* Targets::generateTargetsFromNetwork(
    absl::string_view targetNetwork, const uint8_t granularity,
    const bool LookupByPrefixSupport) const {
  DcbManager* dcbManager =
      new DcbManager(1000, granularity, seed_, LookupByPrefixSupport);

  std::vector<absl::string_view> parts = absl::StrSplit(targetNetwork, "/");
  if (parts.size() != 2) {
    LOG(FATAL) << "Target network format is incorrect!!! " << targetNetwork;
  }

  uint32_t subnetPrefixLength = 0;

  if (!absl::SimpleAtoi(parts[1], &subnetPrefixLength)) {
    LOG(FATAL) << "Failed to parse the target network.";
  }

  IpAddress* targetBaseAddress =
      parseIpFromStringToIpAddress(std::string(parts[0]));

  IpAddress* targetNetworkFirstAddress_ =
      getFirstAddressOfBlock(*targetBaseAddress, subnetPrefixLength);
  IpAddress* targetNetworkLastAddress_ =
      getLastAddressOfBlock(*targetBaseAddress, subnetPrefixLength);

  if (*targetNetworkFirstAddress_ >= *targetNetworkLastAddress_) {
    LOG(FATAL) << "Ip address range is incorrect.";
  }

  LOG(INFO) << boost::format("The target network is from %1% to %2%.") %
                   parseIpFromIpAddressToString(*targetNetworkFirstAddress_) %
                   parseIpFromIpAddressToString(*targetNetworkLastAddress_);

  if (targetBaseAddress->isIpv4()) {
    uint64_t targetNetworkSize =
        static_cast<int64_t>(targetNetworkLastAddress_->getIpv4Address()) -
        static_cast<int64_t>(targetNetworkFirstAddress_->getIpv4Address()) + 1;

    uint64_t blockFactor_ =
        static_cast<uint64_t>(std::pow(2, 32 - granularity));
    uint64_t dcbCount = static_cast<uint64_t>(targetNetworkSize / blockFactor_);

    // set random seed.
    std::srand(seed_);
    uint32_t actualCount = 0;
    uint32_t bogonCount = 0;
    for (uint64_t i = 0; i < dcbCount; i++) {
      // randomly generate IP addresse avoid the first and last ip address
      // in the block.
      Ipv4Address tmp(targetNetworkFirstAddress_->getIpv4Address() +
                      ((i) << (32 - granularity)) +
                      (rand() % (blockFactor_ - 3)) + 2);

      if ((blacklist_ == nullptr || !blacklist_->contains(tmp)) &&
          (bogerFilter_ == nullptr || !bogerFilter_->isBogonAddress(tmp))) {
        dcbManager->addDcb(tmp, defaultSplitTtl_, subnetPrefixLength);
        actualCount++;
      } else if (bogerFilter_ != nullptr && bogerFilter_->isBogonAddress(tmp)) {
        bogonCount ++;
      }
    }
    VLOG(2) << boost::format("Created %1% entries (1 reserved dcb).") %
                   actualCount;
    VLOG(2) << "Bogon filter removes addresses " << bogonCount;
  } else {
    absl::uint128 targetNetworkSize =
        ntohll(targetNetworkLastAddress_->getIpv6Address()) -
        ntohll(targetNetworkFirstAddress_->getIpv6Address()) + 1;

    absl::uint128 blockFactor_ =
        static_cast<absl::uint128>(std::pow(2, 128 - granularity));
    absl::uint128 dcbCount =
        static_cast<absl::uint128>(targetNetworkSize / blockFactor_);

    // set random seed.
    absl::uint128 actualCount = 0;
    std::srand(seed_);
    for (absl::uint128 i = 0; i < dcbCount; i++) {
      // randomly generate IP addresse avoid the first and last ip address
      // in the block.
      Ipv6Address tmp(htonll(
          ntohll(targetNetworkFirstAddress_->getIpv6Address()) +
          ((i) << (128 - granularity)) + (rand() % (blockFactor_ - 3)) + 2));
      if (blacklist_ != nullptr && !blacklist_->contains(tmp)) {
        dcbManager->addDcb(tmp, defaultSplitTtl_, subnetPrefixLength);
        actualCount++;
      }
    }
    LOG(INFO) << "Created " << actualCount << " entries (1 reserved dcb).";
  }

  return dcbManager;
}

}  // namespace trace6
