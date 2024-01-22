/* Copyright (C) 2019 Neo Huang - All Rights Reserved */
#include "trace6/utils.h"

#include "glog/logging.h"

#include <vector>
#include <memory>
#include <string>
#include <arpa/inet.h>
#include <net/if.h>
#include <ifaddrs.h>

#include <boost/process.hpp>
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"

#include "trace6/address.h"
namespace trace6 {

const uint32_t kThreadPoolSize = 1;

using namespace boost::process;

CommandExecutor::CommandExecutor() {}

void CommandExecutor::run(const std::string& command) {
  child_process_ = std::make_unique<boost::process::child>(command);
  LOG(INFO) << "Child process is running.";
}

void CommandExecutor::stop() {
  if (child_process_.get() != nullptr) {
    child_process_->terminate();
  }
  LOG(INFO) << "Child process teminated.";
}

IpNetwork* parseNetworkFromStringToNetworkAddress(
    absl::string_view stringNetwork) {
  std::vector<absl::string_view> parts = absl::StrSplit(stringNetwork, "/");
  IpAddress* baseAddr = parseIpFromStringToIpAddress(std::string(parts[0]));
  uint32_t subnetPrefixLength = 0;
  if (parts.size() == 2) {
    if (!absl::SimpleAtoi(parts[1], &subnetPrefixLength)) {
      return NULL;
    }
  } else if (parts.size() == 1) {
    if (baseAddr->isIpv4())
      subnetPrefixLength = 32;
    else
      subnetPrefixLength = 128;
  } else {
    return NULL;
  }

  IpNetwork* ret = new IpNetwork(*baseAddr, subnetPrefixLength);
  free(baseAddr);
  return ret;
}

uint32_t parseIpFromStringToInt(const std::string& stringIp) {
  return ntohl(inet_addr(stringIp.c_str()));
}

IpAddress* parseIpFromStringToIpAddress(const std::string& stringIp) {
  if (absl::StrContains(absl::string_view(stringIp), ":")) {
    // IPv6
    struct in6_addr result;
    if (inet_pton(AF_INET6, stringIp.c_str(), &result) == 1) {
      // successfully parsed string into "result"
      absl::uint128 addr_ = 0;
      memcpy(&addr_, &result, sizeof(result));
      return new Ipv6Address(addr_);
    } else {
      VLOG(2) << "Failed to parse: " << stringIp;
      return NULL;
    }

  } else {
    // IPv4
    return new Ipv4Address(ntohl(inet_addr(stringIp.c_str())));
  }
}

std::string parseIpv4FromIntToString(const uint32_t ip) {
  uint32_t section[4];
  section[0] = ip & 0xFF;
  section[1] = (ip >> 8) & 0xFF;
  section[2] = (ip >> 16) & 0xFF;
  section[3] = (ip >> 24) & 0xFF;
  std::string result = "";
  absl::StrAppend(&result, section[3], ".", section[2], ".", section[1], ".",
                  section[0]);
  return result;
}

std::string parseIpFromIpAddressToString(const IpAddress& ip) {
  if (ip.isIpv4()) {
    return parseIpv4FromIntToString(ip.getIpv4Address());
  } else {
    char interfaceTmp[256];
    absl::uint128 tempAddr = ip.getIpv6Address();
    inet_ntop(AF_INET6, &tempAddr, interfaceTmp,
              sizeof(interfaceTmp));
    std::string ip = interfaceTmp;
    return ip;
  }
}

std::string getAddressByInterface(const std::string& interface, bool ipv4) {
  struct ifaddrs *addrs, *iap;
  char interfaceTmp[256];
  std::string ip;
  getifaddrs(&addrs);
  for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
    if (iap->ifa_addr && (iap->ifa_flags & IFF_UP)) {
      if (ipv4 && iap->ifa_addr->sa_family == AF_INET) {
        struct sockaddr_in* sa = (struct sockaddr_in*)(iap->ifa_addr);
        inet_ntop(iap->ifa_addr->sa_family, (void*)&(sa->sin_addr),
                  interfaceTmp, sizeof(interfaceTmp));
        std::string tmp = iap->ifa_name;
        if (!tmp.compare(interface)) {
          ip = interfaceTmp;
          VLOG(2) << "Interface: " << interface << " IPv4 address: " << ip;
          break;
        }
      } else if (!ipv4 && iap->ifa_addr->sa_family == AF_INET6){
        struct sockaddr_in6* in6 = (struct sockaddr_in6*)iap->ifa_addr;
        inet_ntop(AF_INET6, &in6->sin6_addr, interfaceTmp,
                  sizeof(interfaceTmp));
        std::string tmp = iap->ifa_name;
        if (!tmp.compare(interface)) {
          ip = interfaceTmp;
          VLOG(2) << "Interface: " << interface << " IPv6 address: " << ip;
          break;
        }
      }
    }
  }
  freeifaddrs(addrs);
  return ip;
}

IpAddress* getFirstAddressOfBlock(const IpAddress& address,
                                 const int32_t prefixLength) {
  if (address.isIpv4()) {
    if (prefixLength > 32 || prefixLength < 0) {
      LOG(FATAL) << "network prefix length is incorrect!!!";
    }
    int64_t result = address.getIpv4Address();
    for (int32_t i = 0; i < 32 - prefixLength; i++) {
      result = result & (~static_cast<uint32_t>(1UL << i));
    }
    return new Ipv4Address(static_cast<uint32_t>(result));
  } else {
    if (prefixLength > 128 || prefixLength < 0) {
      LOG(FATAL) << "network prefix length is incorrect!!!";
    }
    absl::uint128 result = ntohll(address.getIpv6Address());
    for (int32_t i = 0; i < 128 - prefixLength; i++) {
      result = result & (~(static_cast<absl::uint128>(1) << i));
    }
    return new Ipv6Address(htonll(result));
  }
}

IpAddress* getLastAddressOfBlock(const IpAddress& address,
                                const int32_t prefixLength) {
  if (address.isIpv4()) {
    if (prefixLength > 32 || prefixLength < 0) {
      LOG(FATAL) << "network prefix length is incorrect!!!";
    }
    int64_t result = address.getIpv4Address();
    for (int32_t i = 0; i < 32 - prefixLength; i++) {
      result = result | (1UL << i);
    }
    return new Ipv4Address(static_cast<uint32_t>(result));
  } else {
    if (prefixLength > 128 || prefixLength < 0) {
      LOG(FATAL) << "network prefix length is incorrect!!!";
    }
    absl::uint128 result = ntohll(address.getIpv6Address());
    for (int32_t i = 0; i < 128 - prefixLength; i++) {
      result = result | (static_cast<absl::uint128>(1) << i);
    }
    return new Ipv6Address(htonll(result));
  }
}

absl::uint128 htonll(absl::uint128 in) {
  absl::uint128 result;
  uint8_t* srcPtr = reinterpret_cast<uint8_t*>(&in);
  uint8_t* dstPtr = reinterpret_cast<uint8_t*>(&result);
  for (uint32_t i = 0; i < sizeof(absl::uint128); i++) {
    dstPtr[sizeof(absl::uint128) - i - 1] = srcPtr[i];
  }
  return result;
}

absl::uint128 ntohll(absl::uint128 in) {
  absl::uint128 result;
  uint8_t* srcPtr = reinterpret_cast<uint8_t*>(&in);
  uint8_t* dstPtr = reinterpret_cast<uint8_t*>(&result);
  for (uint32_t i = 0; i < sizeof(absl::uint128); i++) {
    dstPtr[sizeof(absl::uint128) - i - 1] = srcPtr[i];
  }
  return result;
}

bool isNetwork(const std::string& input) {
  std::vector<absl::string_view> parts = absl::StrSplit(input, "/");
  if (parts.size() == 2) {
    return true;
  } else {
    return false;
  }
}

bool isValidDestiantion(const std::string& input) {
  std::vector<absl::string_view> parts = absl::StrSplit(input, ".");
  if (parts.size() != 4) {
    return false;
  } else {
    return true;
  }
}

std::string getDefaultInterface() {
  struct ifaddrs *addrs, *iap;
  struct sockaddr_in *sa;
  char interfaceTmp[256];
  getifaddrs(&addrs);
  std::string tmp;
  for (iap = addrs; iap != NULL; iap = iap->ifa_next) {
    if (iap->ifa_addr && (iap->ifa_flags & IFF_UP) &&
        iap->ifa_addr->sa_family == AF_INET) {
      sa = (struct sockaddr_in *)(iap->ifa_addr);
      inet_ntop(iap->ifa_addr->sa_family, (void *)&(sa->sin_addr), interfaceTmp,
                sizeof(interfaceTmp));
      tmp = iap->ifa_name;
    }
  }
  freeifaddrs(addrs);
  return tmp;
}

}  // namespace trace6
