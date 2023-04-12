#ifndef INCLUDE_HTTP_UTILS_H_
#define INCLUDE_HTTP_UTILS_H_

#include <iostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

namespace gor {

class HttpUtils {
private:
  static auto parse_query_string(std::string payload)
      -> std::unordered_map<std::string, std::vector<std::string>>;
  static auto quote_plus(const std::string &value) -> std::string;

public:
  static auto http_method(std::string payload) -> std::string;
  static auto http_path(std::string payload) -> std::string;
  static auto set_http_path(std::string payload, std::string new_path)
      -> std::string;
  static auto http_path_param(std::string payload, std::string name,
                              bool *found) -> std::vector<std::string>;
  static auto set_http_path_param(std::string payload, std::string name,
                                  std::string value) -> std::string;
  static auto http_status(std::string payload) -> std::string;
  static auto set_http_status(std::string payload, std::string new_status)
      -> std::string;
  static auto http_headers(std::string payload)
      -> std::unordered_map<std::string, std::string>;
  static auto http_header(std::string payload, std::string name, bool *found)
      -> std::tuple<int, int, int, std::string>;
  static auto set_http_header(std::string payload, std::string name,
                              std::string value) -> std::string;
  static auto delete_http_header(std::string payload, std::string name)
      -> std::string;
  static auto http_body(std::string payload) -> std::string;
  static auto set_http_body(std::string payload, std::string new_body)
      -> std::string;
  static auto decompress_gzip_body(std::string payload) -> std::string;

  static auto decode_chunked(std::string &chunked_data) -> std::string;
};

class Utils {
public:
  static auto trim(const std::string &source) -> std::string;
  static auto str_to_hex(const std::string &str) -> std::string;
  static auto hex_to_str(const std::string &hex_str) -> std::string;
  static void str_split(const std::string &str, const char split,
                        std::vector<std::string> &res);
};

} // namespace gor

#endif // INCLUDE_HTTP_UTILS_H_