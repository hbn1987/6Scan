#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/numeric/int128.h"
#include "absl/strings/str_cat.h"
#include "glog/logging.h"

#include "trace6/address.h"
#include "trace6/dump_result.h"
#include "trace6/utils.h"
#include "parsers/utils.h"

ABSL_FLAG(std::string, file, "", "Path to the directory of output files");

int main(int argc, char *argv[]) {
  LOG(INFO) << "Program starts.";
  FLAGS_alsologtostderr = 1;
  absl::SetProgramUsageMessage("Organizing output files.");
  absl::ParseCommandLine(argc, argv);

  google::InitGoogleLogging(argv[0]); 

  std::ifstream inFile;
  std::string file = absl::GetFlag(FLAGS_file);
  inFile.open(file, std::ios::in | std::ios::binary);
  if (!inFile)
    LOG(FATAL) << absl::GetFlag(FLAGS_file) << " not exist, make sure to enter the absolute path" << std::endl;
  DataElement6 buffer;

  while (inFile.peek() != EOF) {
    inFile.read(reinterpret_cast<char *>(&buffer), 39);
    if (buffer.ipv4 != 1) {
      // IPv6 address handling.
      auto addr = buffer.responder;
      auto dest = buffer.destination;
      auto distance = buffer.distance;
      std::cout << parseIpv6FromIntToString(dest) << ", " << parseIpv6FromIntToString(addr) << ", " << \
      static_cast<uint32_t>(distance) << std::endl;
    }
  }
  inFile.close();
  LOG(INFO) << "Processing finished.";
}