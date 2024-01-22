/* Copyright (C) 2019 Neo Huang - All Rights Reserved */

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>

#include "absl/numeric/int128.h"

namespace trace6 {

class IpAddress {
 public:
  virtual IpAddress* clone() const = 0;

  // return ipv4 decimal address if there is one.
  virtual uint32_t getIpv4Address() const = 0;

  // return the first 64 bits of ipv6 decimal address if there is one.
  virtual absl::uint128 getIpv6Address() const = 0;

  // return the prefix of the address.
  virtual absl::uint128 getPrefix(uint8_t length) const = 0;

  // randomize the suffix of address but keep prefix unchanged. Length is the
  // length of the unchanged prefix.
  virtual void randomizeAddress(uint8_t length) = 0;

  // return a boolean value telling whether this is a ipv4 address (only for
  // IpAddress).
  virtual bool isIpv4() const = 0;

  // return a size_t hash value of address.
  virtual size_t hash() const = 0;

  // overload assignment operator.
  virtual IpAddress& operator=(const IpAddress& rhs) {
    return set_to(rhs);
  }

  // overload comparison operator.
  virtual bool operator==(const IpAddress& rhs) const {
    return equal_to(rhs);
  }

  virtual bool operator!=(const IpAddress& rhs) const {
    return !equal_to(rhs);
  }

  virtual bool operator>(const IpAddress& rhs) const {
    return compare_to(rhs);
  }

  virtual bool operator>=(const IpAddress& rhs) const {
    return compare_to(rhs) || equal_to(rhs);
  }

  virtual bool operator<(const IpAddress& rhs) const {
    return !compare_to(rhs);
  }

  virtual bool operator<=(const IpAddress& rhs) const {
    return !compare_to(rhs) || equal_to(rhs);
  }

 protected:
  // compare with anohter IpAddress object return true if content is the same.
  virtual bool equal_to(const IpAddress& rhs) const = 0;
  virtual bool compare_to(const IpAddress& rhs) const = 0;

  // set the current address based on the content of another IpAddress object.
  virtual IpAddress& set_to(const IpAddress& rhs) = 0;
};

class Ipv4Address : public IpAddress {
 public:
  Ipv4Address();
  explicit Ipv4Address(uint32_t ipv4);
  Ipv4Address(const Ipv4Address& copy);
  // vitrual constructor (copy)
  virtual Ipv4Address* clone() const;

  uint32_t getIpv4Address() const override;
  absl::uint128 getIpv6Address() const override;
  absl::uint128 getPrefix(uint8_t length) const override;
  void randomizeAddress(uint8_t length) override;
  bool isIpv4() const override;

  size_t hash() const override;

  Ipv4Address& operator=(const Ipv4Address& rhs) {
    this->address_ = rhs.getIpv4Address();
    return *this;
  }

 protected:
  bool equal_to(const IpAddress& rhs) const override;
  bool compare_to(const IpAddress& rhs) const override;

  IpAddress& set_to(const IpAddress& rhs) override;

 private:
  uint32_t address_;
};

class Ipv6Address : public IpAddress {
 public:
  Ipv6Address();
  explicit Ipv6Address(absl::uint128 address);
  Ipv6Address(const Ipv6Address& copy);
  // vitrual constructor (copy)
  virtual Ipv6Address* clone() const;

  uint32_t getIpv4Address() const override;
  absl::uint128 getIpv6Address() const override;
  absl::uint128 getPrefix(uint8_t length) const override;
  void randomizeAddress(uint8_t length) override;
  bool isIpv4() const override;

  size_t hash() const override;

  Ipv6Address& operator=(const Ipv6Address& rhs) {
    this->address_ = rhs.getIpv6Address();
    return *this;
  }

 protected:
  bool equal_to(const IpAddress& rhs) const override;
  bool compare_to(const IpAddress& rhs) const override;

  IpAddress& set_to(const IpAddress& rhs) override;

 private:
  absl::uint128 address_;
};

// customize hash and equality function for IpAddress object
struct IpAddressHash {
  std::size_t operator()(const IpAddress* tmp) const { return tmp->hash(); }
};

struct IpAddressEquality {
  std::size_t operator()(const IpAddress* lhs, const IpAddress* rhs) const {
    return *lhs == *rhs;
  }
};

class IpNetwork {
 public:
  IpNetwork(const IpAddress& addr, const uint32_t prefix);

  // copy constructor
  IpNetwork(const IpNetwork& copy);

  bool contains(const IpAddress& addr) const;
  IpNetwork* clone() const;
  absl::uint128 getPrefix() const;
 private:
  std::unique_ptr<IpAddress> addr_;
  uint32_t prefix_;
};

// customize hash and equality function for IpAddress object
struct IpNetworkHash {
  std::size_t operator()(const IpNetwork* tmp) const {
    return static_cast<std::size_t>(tmp->getPrefix() % __SIZE_MAX__);
  }
};

struct IpNetworkEquality {
  std::size_t operator()(const IpNetwork* lhs, const IpNetwork* rhs) const {
    return lhs->getPrefix() == lhs->getPrefix();
  }
};

}  // namespace trace6
