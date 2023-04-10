#ifndef INCLUDE_GOR_H_
#define INCLUDE_GOR_H_

#include <iostream>
#include <string>
#include <unordered_map>
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

class Gor {
private:
  auto parse_query_string(std::string payload)
      -> std::unordered_map<std::string, std::vector<std::string>>;
  auto quote_plus(const std::string &value) -> std::string;

public:
  auto http_method(std::string payload) -> std::string;
  auto http_path(std::string payload) -> std::string;
  auto set_http_path(std::string payload, std::string new_path) -> std::string;
  auto http_path_param(std::string payload, std::string name, bool *found)
      -> std::vector<std::string>;
  auto set_http_path_param(std::string payload, std::string name,
                           std::string value) -> std::string;
};

class Utils {
public:
  static auto decode_chunked(std::string &chunked_data) -> std::string;
};

} // namespace gor

#endif // INCLUDE_GOR_H_