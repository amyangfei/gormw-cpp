#include "gor.h"
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

using namespace gor;

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

auto Gor::parse_query_string(std::string query)
    -> std::unordered_map<std::string, std::vector<std::string>> {
  std::unordered_map<std::string, std::vector<std::string>> query_dict;
  std::string key, value;
  int idx = 0, sep1 = 0, sep2 = 0;
  std::cout << "parse_query_string, query: " << query << std::endl;
  while (true) {
    sep1 = query.find("=", idx);
    key = query.substr(idx, sep1 - idx);
    sep2 = query.find("&", sep1 + 1);
    if (sep2 == std::string::npos) {
      value = query.substr(sep1 + 1);
    } else {
      value = query.substr(sep1 + 1, sep2 - sep1 - 1);
    }
    std::cout << "key:" << key << " value:" << value << std::endl;
    query_dict[key].push_back(value);
    if (sep2 == std::string::npos) {
      break;
    }
    idx = sep2 + 1;
  }
  return query_dict;
}

auto Gor::quote_plus(const std::string &value) -> std::string {
  std::string quoted_str = "";
  for (auto &c : value) {
    if (isalnum(c)) {
      quoted_str += c;
    } else {
      char hex[3];
      snprintf(hex, sizeof(hex), "%02X", (unsigned char)c);
      quoted_str += '%';
      quoted_str += hex;
    }
  }
  std::cout << "quote_plus "
            << "value: " << value << " quoted: " << quoted_str << std::endl;
  return quoted_str;
}

auto Gor::http_method(std::string payload) -> std::string {
  int sep = payload.find(" ");
  return payload.substr(0, sep);
}

auto Gor::http_path(std::string payload) -> std::string {
  int start = payload.find(" ") + 1;
  int end = payload.find(" ", start);
  return payload.substr(start, end - start);
}

auto Gor::set_http_path(std::string payload, std::string new_path)
    -> std::string {
  int start = payload.find(" ") + 1;
  int end = payload.find(" ", start);
  return payload.substr(0, start) + new_path + payload.substr(end);
}

auto Gor::http_path_param(std::string payload, std::string name, bool *found)
    -> std::vector<std::string> {
  std::string path = http_path(payload);
  int query_string_index = path.find("?");
  auto query_dict = parse_query_string(path.substr(query_string_index + 1));
  *found = query_dict.count(name) > 0 ? true : false;
  auto res = std::move(query_dict[name]);
  std::unordered_map<std::string, std::vector<std::string>>().swap(query_dict);
  return res;
}

auto Gor::set_http_path_param(std::string payload, std::string name,
                              std::string value) -> std::string {
  std::string path_qs = http_path(payload);
  std::string new_path = std::regex_replace(
      path_qs, std::regex(name + "=([^&$]+)"), name + "=" + quote_plus(value));
  if (new_path == path_qs) {
    if (new_path.find('?') == std::string::npos) {
      new_path += '?';
    } else {
      new_path += '&';
    }
    new_path += name + '=' + quote_plus(value);
  }
  return set_http_path(payload, new_path);
}

auto Utils::decode_chunked(std::string &chunked_data) -> std::string {
  std::string result = "";
  while (chunked_data != "") {
    size_t chunked_size_sep = chunked_data.find("\r\n");
    int offset =
        std::stoi(chunked_data.substr(0, chunked_size_sep), nullptr, 16);
    if (offset == 0) {
      break;
    }
    chunked_data = chunked_data.substr(chunked_size_sep + 2);
    result += chunked_data.substr(0, offset);
    chunked_data = chunked_data.substr(offset + 2);
  }
  return result;
}