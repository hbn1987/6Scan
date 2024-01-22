/* Copyright (C) 2019 Neo Huang - All Rights Reserved */
#pragma once

#include <memory>
#include <string>

#include <boost/asio.hpp>
#include <boost/process.hpp>
#include "absl/strings/string_view.h"

#include "trace6/address.h"

namespace trace6 {

class CommandExecutor {
 public:
  CommandExecutor();
  void run(const std::string& command);
  void stop();

 private:
  std::unique_ptr<boost::process::child> child_process_;
};

// Translate string network to IpNetowrk. (Currently only support Ipv4)
IpNetwork* parseNetworkFromStringToNetworkAddress(
    absl::string_view stringNetwork);

// Translate string IP to integer IP.
uint32_t parseIpFromStringToInt(const std::string& stringIp);

// Translate string IP to IpAddress. (Currently only support Ipv4)
IpAddress* parseIpFromStringToIpAddress(const std::string& stringIp);

// Convert decimal IP to string.
std::string parseIpv4FromIntToString(const uint32_t ip);

// Convert decimal IP to string.
std::string parseIpFromIpAddressToString(const IpAddress& ip);

// Get IP address by interface name. Return empty string, if interface does not
// exist.
std::string getAddressByInterface(const std::string& interface, bool ipv4);

// Get first address of a IP block.
IpAddress* getFirstAddressOfBlock(const IpAddress& address,
                                 const int32_t prefixLength);

// Get last address of a IP block.
IpAddress* getLastAddressOfBlock(const IpAddress& address,
                                const int32_t prefixLength);

// Similar to htonl but designed for absl::uint128 ipv6 address.
absl::uint128 htonll(absl::uint128 in);

// Similar to ntohl but designed for absl::uint128 ipv6 address.
absl::uint128 ntohll(absl::uint128 in);

bool isNetwork(const std::string& input);

bool isValidDestiantion(const std::string& input);

std::string getDefaultInterface();

}  // namespace trace6
