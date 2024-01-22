#include "utils.h"

#include <map>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>

#include "glog/logging.h"
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

std::string getLogFileName(const std::string &directory,
                           const std::string &prefix) {
  for (const auto &entry : boost::make_iterator_range(
           boost::filesystem::directory_iterator(directory), {})) {
    std::string file = entry.path().string();
    if (prefix != file && file.find(prefix + "_log") != std::string::npos) {
      return file;
    }
  }
  return "";
}

std::string getStartingTime(const std::string &logFile) {

  std::ifstream inFile;
  inFile.open(logFile, std::ios::in);
  std::string line;
  std::getline(inFile, line);
  inFile.clear();
  inFile.seekg(0);
  inFile.close();
  return line.substr(21);
}

static void connectRouteNodev4(uint32_t dest, std::shared_ptr<RouteNodev4> x,
                               std::shared_ptr<RouteNodev4> y) {

  x->next.insert({dest, y});
  y->previous.insert({dest, x});
}

static void connectRouteNodev6(absl::uint128 dest, std::shared_ptr<RouteNodev6> x,
                               std::shared_ptr<RouteNodev6> y) {

  x->next.insert({dest, y});
  y->previous.insert({dest, x});
}

void readDataset(
    std::string file, RouteFullMap &addressMap,
    std::unordered_map<uint32_t, std::shared_ptr<RouteNodev4>>& routeMap) {

  std::ifstream inFile;
  inFile.open(file, std::ios::in | std::ios::binary);
  DataElement buffer;

  uint32_t records = 0;

  std::unordered_map<uint32_t,
                     std::shared_ptr<std::map<uint8_t, uint32_t>>>
      routeRawMap;
  // <destination, <distance, address>>

  while (inFile.peek() != EOF) {
    inFile.read(reinterpret_cast<char *>(&buffer), 39);
    records++;
    if (buffer.ipv4 == 1) {
      // IPv4 address handling.
      auto addr = buffer.responder[0];
      auto dest = buffer.destination[0];
      auto distance = buffer.distance;
      auto destFound = routeRawMap.find(dest);
      if (destFound == routeRawMap.end()) {
        auto tmp = std::make_shared<std::map<uint8_t, uint32_t>>();
        destFound = routeRawMap.insert({dest, tmp}).first;
      }
      if (destFound->second->find(distance) == destFound->second->end())
        destFound->second->insert({distance, addr});
    } else {
      // IPv6 address handling
      // TODO: we need to add the code logic handle IPv6 Address.
    }
  }
  inFile.close();

  LOG(INFO) << "Preprocessing finished.";
  uint32_t count = 0;
  std::shared_ptr<RouteNodev4> lastNode = nullptr;
  for(const auto& elem : routeRawMap) {
    auto dest = elem.first;
    auto& route = elem.second;
    lastNode = nullptr;

    std::shared_ptr<RouteNodev4> previousNode = nullptr;
    for (const auto& node : *route) {
      auto distance = node.first;
      auto addr = node.second;
      auto nodeFound = addressMap.find(addr);
      if (nodeFound == addressMap.end()) {
        auto tmp = std::make_shared<RouteNodev4>();
        tmp->address = addr;
        nodeFound = addressMap.insert({addr, std::move(tmp)}).first;
      }
      auto currentNode = nodeFound->second;
      if (currentNode->distances.find(dest) == currentNode->distances.end())
        currentNode->distances.insert({dest, distance});

      if (previousNode) {
        connectRouteNodev4(dest, previousNode, currentNode);
      }
      previousNode = currentNode;
      lastNode = currentNode;
    }
    assert(lastNode);
    routeMap.insert({dest, lastNode});

    LOG_EVERY_N(INFO, 100000) << static_cast<double>(count) / routeRawMap.size() * 100 << "% finished.";
    count++;
  }
  LOG(INFO) << "Processing finished.";
}

void readDataset(
    std::string file, RouteFullMap6 &addressMap,
    absl::flat_hash_map<absl::uint128, std::shared_ptr<RouteNodev6>>& routeMap) {
 
  std::ifstream inFile;
  inFile.open(file, std::ios::in | std::ios::binary);
  if (!inFile)
    LOG(FATAL) << "File not exist, make sure to enter the absolute path" << std::endl;
  DataElement6 buffer;

  uint32_t records = 0;

  absl::flat_hash_map<absl::uint128, std::shared_ptr<std::map<uint8_t, absl::uint128>>>
      routeRawMap;
  // <destination, <distance, address>>
  while (inFile.peek() != EOF) {
    inFile.read(reinterpret_cast<char *>(&buffer), 39);
    records++;
    if (buffer.ipv4 != 1) {
      // IPv6 address handling.
      auto addr = buffer.responder;
      auto dest = buffer.destination;
      auto distance = buffer.distance;
      std::cout << parseIpv6FromIntToString(dest) << " " << parseIpv6FromIntToString(addr) << " " << \
      static_cast<uint32_t>(distance) << std::endl;
      // auto destFound = routeRawMap.find(dest);
      // if (destFound == routeRawMap.end()) {
      //   auto tmp = std::make_shared<std::map<uint8_t, absl::uint128>>();
      //   destFound = routeRawMap.insert({dest, tmp}).first;
      // }
      // if (destFound->second->find(distance) == destFound->second->end())
      //   destFound->second->insert({distance, addr});
    }
  }
  inFile.close();
  LOG(INFO) << "Processing finished.";
  exit(-1);

  LOG(INFO) << "Preprocessing finished.";
  uint32_t count = 0;
  std::shared_ptr<RouteNodev6> lastNode = nullptr;
  for(const auto& elem : routeRawMap) {
    auto dest = elem.first;
    auto& route = elem.second;
    lastNode = nullptr;

    std::shared_ptr<RouteNodev6> previousNode = nullptr;
    for (const auto& node : *route) {
      auto distance = node.first;
      auto addr = node.second;
      auto nodeFound = addressMap.find(addr);
      if (nodeFound == addressMap.end()) {
        auto tmp = std::make_shared<RouteNodev6>();
        tmp->address = addr;
        nodeFound = addressMap.insert({addr, std::move(tmp)}).first;
      }
      auto currentNode = nodeFound->second;
      if (currentNode->distances.find(dest) == currentNode->distances.end())
        currentNode->distances.insert({dest, distance});

      if (previousNode) {
        connectRouteNodev6(dest, previousNode, currentNode);
      }
      previousNode = currentNode;
      lastNode = currentNode;
    }
    assert(lastNode);
    routeMap.insert({dest, lastNode});

    LOG_EVERY_N(INFO, 100000) << static_cast<double>(count) / routeRawMap.size() * 100 << "% finished.";
    count++;
  }
  LOG(INFO) << "Processing finished.";
}

bool findRouteBack(uint32_t address, uint32_t dest,
                   std::vector<RouteConstructNodev4> &route,
                   std::vector<Routev4> &routes,
                   std::unordered_set<uint32_t> &visited,
                   RouteFullMap &addressMap, uint8_t convergencePoint) {
  static int depth = 0;
  if (visited.find(address) != visited.end()) {
    // Cycle detected.
    return false;
  }
  auto node = addressMap.find(address)->second;
  auto distance = node->distances[dest];
  route.push_back({address, dest, distance});
  visited.insert(address);

  if ((node->distances.find(dest) != node->distances.end() &&
       node->distances[dest] <= 2) ||
      (node->previous.size() == 0)) {
    routes.push_back({route, RouteType::Regular, convergencePoint});
    route.pop_back();
    visited.erase(address);
    return true;
  }
  // If we can find the same destionation predecessor.
  bool success = false;
  if (node->previous.find(dest) != node->previous.end()) {
    depth++;
    success =
        findRouteBack(node->previous.find(dest)->second->address, dest, route,
                      routes, visited, addressMap, convergencePoint);
    depth--;
  } else {
    std::unordered_set<uint32_t> visitedPredecessor;
    for (auto &tmp : node->previous) {
      auto predecessorAddress = tmp.second->address;
      auto predecessorDestination = tmp.first;

      if (visitedPredecessor.find(predecessorAddress) !=
          visitedPredecessor.end())
        continue;
      else
        visitedPredecessor.insert(predecessorAddress);

      depth++;
      success =
          findRouteBack(predecessorAddress, predecessorDestination, route,
                        routes, visited, addressMap, convergencePoint + 1) ||
          success;
      depth--;
    }
  }
  if (!success) {
    // If all attemps fail, this is the end.
    routes.push_back({route, RouteType::Acyclic, convergencePoint});
  }
  route.pop_back();
  visited.erase(address);
  return true;
}

bool findRouteBack(absl::uint128 address, absl::uint128 dest,
                   std::vector<RouteConstructNodev6> &route,
                   std::vector<Routev6> &routes,
                   absl::flat_hash_set<absl::uint128> &visited,
                   RouteFullMap6 &addressMap, uint8_t convergencePoint) {
  static int depth = 0;
  if (visited.find(address) != visited.end()) {
    // Cycle detected.
    return false;
  }
  auto node = addressMap.find(address)->second;
  auto distance = node->distances[dest];
  route.push_back({address, dest, distance});
  visited.insert(address);

  if ((node->distances.find(dest) != node->distances.end() &&
       node->distances[dest] <= 2) ||
      (node->previous.size() == 0)) {
    routes.push_back({route, RouteType::Regular, convergencePoint});
    route.pop_back();
    visited.erase(address);
    return true;
  }
  // If we can find the same destionation predecessor.
  bool success = false;
  if (node->previous.find(dest) != node->previous.end()) {
    depth++;
    success =
        findRouteBack(node->previous.find(dest)->second->address, dest, route,
                      routes, visited, addressMap, convergencePoint);
    depth--;
  } else {
    absl::flat_hash_set<absl::uint128> visitedPredecessor;
    for (auto &tmp : node->previous) {
      auto predecessorAddress = tmp.second->address;
      auto predecessorDestination = tmp.first;

      if (visitedPredecessor.find(predecessorAddress) !=
          visitedPredecessor.end())
        continue;
      else
        visitedPredecessor.insert(predecessorAddress);

      depth++;
      success =
          findRouteBack(predecessorAddress, predecessorDestination, route,
                        routes, visited, addressMap, convergencePoint + 1) ||
          success;
      depth--;
    }
  }
  if (!success) {
    // If all attemps fail, this is the end.
    routes.push_back({route, RouteType::Acyclic, convergencePoint});
  }
  route.pop_back();
  visited.erase(address);
  return true;
}

std::string parseIpv6FromIntToString(absl::uint128 addr) { 
  in6_addr p;
  for (auto i = 15; i>=0; i--){
    auto j = 15 - i;
    p.s6_addr[i] = static_cast<uint8_t>((addr >> j*8) & 0xFF);
  }
    
  char interfaceTmp[256];
  inet_ntop(AF_INET6, &p, interfaceTmp,
            sizeof(interfaceTmp));
  std::string ip = interfaceTmp;
  return ip;
}

void readDataset(std::string file, RouteMap &edgeMap,
                 InterfaceSet &interfaceSet) {

  std::ifstream inFile;
  inFile.open(file, std::ios::in | std::ios::binary);
  DataElement buffer;

  uint32_t records = 0;

  while (inFile.peek() != EOF) {
    inFile.read(reinterpret_cast<char *>(&buffer), 39);
    records++;
    if (buffer.ipv4 == 1) {
      // IPv4 address handling.
      auto addr = new Ipv4Address(buffer.responder[0]);
      if (buffer.fromDestination == true) {
        // Do nothing
      } else if (interfaceSet.find(addr) == interfaceSet.end()) {
        if (buffer.fromDestination == 0) {
          interfaceSet.insert(addr);
        }
      } else {
        delete addr;
      }
      auto dest = new Ipv4Address(buffer.destination[0]);
      if (edgeMap.find(dest) == edgeMap.end()) {
        auto tmp =
            std::make_shared<std::unordered_map<int8_t, IpAddress *>>();
        edgeMap.insert({dest, tmp});
      }
      auto tmp = edgeMap.find(dest)->second;
      if (tmp->find(buffer.distance) == tmp->end()) {
        tmp->insert({buffer.distance, new Ipv4Address(buffer.responder[0])});
      }
    } else {
      // IPv6 address handling
      // TODO: we need to add the code logic handle IPv6 Address.
    }
  }
  inFile.close();
}

GenericInterfaceSet
interfaceSetToGenericInterfaceSet(InterfaceSet &interfaceSet) {
  GenericInterfaceSet genericItSet;
  for (const auto &element : interfaceSet) {
    genericItSet.insert(element->getIpv4Address());
  }
  return genericItSet;
}

GenericEdgeSet edgeMapToGenericEdgeSet(RouteMap &edgeMap) {
  GenericEdgeSet edgeSet;
  for (const auto &key : edgeMap) {
    auto route = key.second;
    uint64_t edge = 0;
    for (const auto &node : *route) {
      if (route->find(node.first - 1) != route->end()) {
        uint64_t current = node.second->getIpv4Address();
        uint64_t previous =
            route->find(node.first - 1)->second->getIpv4Address();
        edge = previous | current >> 32;
        edgeSet.insert(edge);
      }
    }
  }
  return edgeSet;
}

void cleanInterfaceSet(InterfaceSet &interfaceSet) {
  while (!interfaceSet.empty()) {
    auto element = interfaceSet.begin();
    interfaceSet.erase(element);
  }
}

void cleanEdgeMap(RouteMap &edgeMap) {
  while (!edgeMap.empty()) {
    auto element = edgeMap.begin();
    auto keyAddress = element->first;
    auto routeMap = element->second;
    while (!routeMap->empty()) {
      auto pair = routeMap->begin();
      delete pair->second;
      routeMap->erase(pair->first);
    }
    edgeMap.erase(keyAddress);
    delete keyAddress;
  }
}