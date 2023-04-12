#ifndef INCLUDE_GOR_H_
#define INCLUDE_GOR_H_

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace gor {

class GorMessage {

public:
  explicit GorMessage(std::string _type, std::string _id,
                      std::vector<std::string> metas, std::string raw_meta,
                      std::string http)
      : type(_type), id(_id), metas(metas), raw_meta(raw_meta), http(http) {}

  auto hexlify() -> std::string;

  std::string type;
  std::string id;
  std::vector<std::string> metas;
  std::string raw_meta;
  std::string http;
};

class Gor {
public:
  virtual ~Gor() = default;
  auto parse_message(std::string line) -> std::unique_ptr<GorMessage>;
};

} // namespace gor

#endif // INCLUDE_GOR_H_