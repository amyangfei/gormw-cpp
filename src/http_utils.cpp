#include "http_utils.h"
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>

using namespace gor;

auto HttpUtils::parse_query_string(std::string query)
    -> std::unordered_map<std::string, std::vector<std::string>> {
  std::unordered_map<std::string, std::vector<std::string>> query_dict;
  std::string key, value;
  int idx = 0, sep1 = 0, sep2 = 0;
  while (true) {
    sep1 = query.find("=", idx);
    key = query.substr(idx, sep1 - idx);
    sep2 = query.find("&", sep1 + 1);
    if (sep2 == std::string::npos) {
      value = query.substr(sep1 + 1);
    } else {
      value = query.substr(sep1 + 1, sep2 - sep1 - 1);
    }
    query_dict[key].push_back(value);
    if (sep2 == std::string::npos) {
      break;
    }
    idx = sep2 + 1;
  }
  return query_dict;
}

auto HttpUtils::quote_plus(const std::string &value) -> std::string {
  std::ostringstream escaped;
  escaped.fill('0');
  escaped << std::hex;

  for (auto it = value.begin(); it != value.end(); ++it) {
    std::string::value_type c = (*it);

    // Keep alphanumeric and other accepted characters intact
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      escaped << c;
      continue;
    }

    // Any other characters are percent-encoded
    escaped << std::uppercase;
    escaped << '%' << std::setw(2) << int((unsigned char)c);
    escaped << std::nouppercase;
  }

  return escaped.str();
}

auto HttpUtils::http_method(std::string payload) -> std::string {
  int sep = payload.find(" ");
  return payload.substr(0, sep);
}

auto HttpUtils::http_path(std::string payload) -> std::string {
  int start = payload.find(" ") + 1;
  int end = payload.find(" ", start);
  return payload.substr(start, end - start);
}

auto HttpUtils::set_http_path(std::string payload, std::string new_path)
    -> std::string {
  int start = payload.find(" ") + 1;
  int end = payload.find(" ", start);
  return payload.substr(0, start) + new_path + payload.substr(end);
}

auto HttpUtils::http_path_param(std::string payload, std::string name,
                                bool *found) -> std::vector<std::string> {
  std::string path = http_path(payload);
  int query_string_index = path.find("?");
  auto query_dict = parse_query_string(path.substr(query_string_index + 1));
  *found = query_dict.count(name) > 0 ? true : false;
  auto res = std::move(query_dict[name]);
  std::unordered_map<std::string, std::vector<std::string>>().swap(query_dict);
  return res;
}

auto HttpUtils::set_http_path_param(std::string payload, std::string name,
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

auto HttpUtils::decode_chunked(std::string &chunked_data) -> std::string {
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