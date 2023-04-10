#ifndef INCLUDE_GOR_H_
#define INCLUDE_GOR_H_

#include <iostream>
#include <string>
#include <vector>

namespace gor {

class GorMessage {

public:
  explicit GorMessage(std::string _id, std::string _type,
                      std::vector<std::string> metas, std::string raw_meta,
                      std::string http)
      : id(_id), type(_type), metas(metas), raw_meta(raw_meta), http(http) {}

  auto hexlify() -> std::string;

  std::string id;
  std::string type;
  std::vector<std::string> metas;
  std::string raw_meta;
  std::string http;
};

class Utils {
public:
  static auto decode_chunked(std::string &chunked_data) -> std::string;
};

} // namespace gor

#endif // INCLUDE_GOR_H_