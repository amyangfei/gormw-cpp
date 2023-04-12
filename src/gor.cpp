#include "gor.h"
#include "utils.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace gor {

auto GorMessage::hexlify() -> std::string {
  std::stringstream ss;
  for (char &c : this->raw_meta) {
    ss << std::setfill('0') << std::setw(2) << std::hex
       << (int)(unsigned char)c;
  }
  ss << "0a"; // hex of \n
  for (char &c : this->http) {
    ss << std::setfill('0') << std::setw(2) << std::hex
       << (int)(unsigned char)c;
  }
  ss << std::endl;
  return ss.str();
}

auto Gor::parse_message(std::string line) -> std::unique_ptr<GorMessage> {
  std::string payload = Utils::hex_to_str(Utils::trim(line));
  int meta_pos = payload.find("\n");
  if (meta_pos == std::string::npos) {
    throw std::invalid_argument("raw meta separator not found" + payload);
  }
  std::string meta = payload.substr(0, meta_pos);
  std::vector<std::string> metas;
  metas.reserve(3);
  Utils::str_split(meta, ' ', metas);
  if (metas.size() != 3) {
    throw std::invalid_argument("meta must contain three fields" + meta);
  }
  return std::make_unique<GorMessage>(metas[0], metas[1], metas, meta,
                                      payload.substr(meta_pos + 1));
}
} // namespace gor