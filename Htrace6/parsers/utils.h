#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#include "trace6/address.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

using trace6::IpAddress;
using trace6::Ipv4Address;
using trace6::Ipv6Address;
using trace6::IpAddressHash;
using trace6::IpAddressEquality;

struct RouteNodev4 {
  uint32_t address;

  /* <Destination IP, Sucessor> */
  std::unordered_map<uint32_t, std::shared_ptr<RouteNodev4>> next;
  /* <Destination IP, Predecessor> */
  std::unordered_map<uint32_t, std::shared_ptr<RouteNodev4>> previous;
  /* <Destination IP, Distance> */
  std::unordered_map<uint32_t, uint8_t> distances;
};

struct RouteNodev6 {
  absl::uint128 address;

  /* <Destination IP, Sucessor> */
  absl::flat_hash_map<absl::uint128, std::shared_ptr<RouteNodev6>> next;
  /* <Destination IP, Predecessor> */
  absl::flat_hash_map<absl::uint128, std::shared_ptr<RouteNodev6>> previous;
  /* <Destination IP, Distance> */
  absl::flat_hash_map<absl::uint128, uint8_t> distances;
};

enum RouteType {
  Acyclic,
  Regular
};

struct RouteConstructNodev4 {
  uint32_t address;
  uint32_t destination;
  uint8_t distance;
};

struct RouteConstructNodev6 {
  absl::uint128 address;
  absl::uint128 destination;
  uint8_t distance;
};

struct Routev4 {
  std::vector<RouteConstructNodev4> route;
  RouteType routeType;
  uint8_t convergencePoint; 
};

struct Routev6 {
  std::vector<RouteConstructNodev6> route;
  RouteType routeType;
  uint8_t convergencePoint; 
};

// <address, corrsponding route node>
using RouteFullMap = std::unordered_map<uint32_t, std::shared_ptr<RouteNodev4>>;
using RouteFullMap6 = absl::flat_hash_map<absl::uint128, std::shared_ptr<RouteNodev6>>;

struct DataElement {
  uint32_t destination[4];
  uint32_t responder[4];
  uint32_t rtt;
  uint8_t distance;
  uint8_t fromDestination;
  uint8_t ipv4;
} __attribute__((packed));;

struct DataElement6 {
  absl::uint128 destination;
  absl::uint128 responder;
  uint32_t rtt;
  uint8_t distance;
  uint8_t fromDestination;
  uint8_t ipv4;
}__attribute__((packed));;

using RouteMap =
    std::unordered_map<IpAddress * /* Destiantion */,
                       std::shared_ptr<std::unordered_map<
                           int8_t, IpAddress *>> /* <Distance, Responder> */,
                       IpAddressHash, IpAddressEquality>;

// Ipv4 Generic edge and interface set.
using GenericEdgeSet = std::unordered_set<uint64_t>;
using GenericInterfaceSet = std::unordered_set<uint32_t>;

using InterfaceSet = std::unordered_set<IpAddress * /* Interface */,
                                        IpAddressHash, IpAddressEquality>;

std::string getLogFileName(const std::string &directory,
                           const std::string &prefix);

std::string getStartingTime(const std::string &logFile);

// Read dataset to a graph map
void readDataset(
    std::string file, RouteFullMap &addressMap,
    std::unordered_map<uint32_t, std::shared_ptr<RouteNodev4>> &routeMap);

void readDataset(
    std::string file, RouteFullMap6 &addressMap,
    absl::flat_hash_map<absl::uint128, std::shared_ptr<RouteNodev6>> &routeMap);


// Find route from a given point backward to the vantage point
bool findRouteBack(uint32_t address, uint32_t dest,
                   std::vector<RouteConstructNodev4> &route,
                   std::vector<Routev4> &routes,
                   std::unordered_set<uint32_t> &visited,
                   RouteFullMap &addressMap, uint8_t convergencePoint);

bool findRouteBack(absl::uint128 address, absl::uint128 dest,
                   std::vector<RouteConstructNodev6> &route,
                   std::vector<Routev6> &routes,
                   absl::flat_hash_set<absl::uint128> &visited,
                   RouteFullMap6 &addressMap, uint8_t convergencePoint);

std::string parseIpv6FromIntToString(absl::uint128 addr);

void readDataset(std::string file, RouteMap &edgeMap,
                 InterfaceSet &interfaceSet);

GenericEdgeSet edgeMapToGenericEdgeSet(RouteMap &edgeMap);

GenericInterfaceSet
interfaceSetToGenericInterfaceSet(InterfaceSet &interfaceSet);

void cleanInterfaceSet(InterfaceSet &interfaceSet);

void cleanEdgeMap(RouteMap &edgeMap);