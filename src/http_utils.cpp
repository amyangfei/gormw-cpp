#include "http_utils.h"
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>

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

// HTTP response have status code in same position as `path` for requests
auto HttpUtils::http_status(std::string payload) -> std::string {
  return http_path(payload);
}

auto HttpUtils::set_http_status(std::string payload, std::string new_status)
    -> std::string {
  return set_http_path(payload, new_status);
}

// Parse the payload and return http headers in a map
// :param payload: the http payload to inspect
// :return: a map mapping from key to value of each http header item
auto HttpUtils::http_headers(std::string payload)
    -> std::unordered_map<std::string, std::string> {
  size_t start_index = payload.find("\r\n");
  size_t end_index = payload.find("\r\n\r\n");
  std::unordered_map<std::string, std::string> headers;
  std::vector<std::string> items;
  std::string item;
  start_index += 2;
  size_t pos;
  while (true) {
    pos = payload.find("\r\n", start_index);
    item = payload.substr(start_index, pos - start_index);
    start_index = pos + 2;

    size_t sep_index = item.find(":");
    std::string key = item.substr(0, sep_index);
    std::string value = trim(item.substr(sep_index + 1));
    headers[key] = value;

    if (pos == end_index || pos == std::string::npos) {
      break;
    }
  }
  return headers;
}

auto HttpUtils::http_header(std::string payload, std::string name, bool *found)
    -> std::tuple<int, int, int, std::string> {
  int current_line = 0;
  int idx = 0;
  int header_start = -1, header_end = -1, header_value_start = -1;
  std::string header_name = name, header_value = "";
  name = name;
  while (idx < payload.length()) {
    char c = payload[idx];
    if (c == '\n') {
      current_line += 1;
      idx += 1;
      header_end = idx;

      if (current_line > 0 && header_start > 0 && header_value_start > 0) {
        if (payload.substr(header_start,
                           header_value_start - header_start - 1) == name) {
          header_value =
              trim(payload.substr(header_value_start, idx - header_value_start)
                       .substr(0, idx - header_value_start - 1));
          *found = true;
          return std::make_tuple(header_start, header_end, header_value_start,
                                 header_value);
        }
      }
      header_start = -1;
      header_value_start = -1;
      continue;
    } else if (c == '\r') {
      idx += 1;
      continue;
    } else if (c == ':') {
      if (header_value_start == -1) {
        idx += 1;
        header_value_start = idx;
        continue;
      }
    }
    if (header_start == -1) {
      header_start = (idx);
    }
    idx += 1;
  }
  *found = false;
  return std::make_tuple(-1, -1, -1, "");
}

auto HttpUtils::set_http_header(std::string payload, std::string name,
                                std::string value) -> std::string {
  int value_start = -1, end = -1;
  bool found;
  std::tie(std::ignore, end, value_start, std::ignore) =
      http_header(payload, name, &found);
  if (found) {
    return payload.substr(0, value_start) + " " + value + "\r\n" +
           payload.substr(end);
  } else {
    int header_start = payload.find("\n") + 1;
    return payload.substr(0, header_start) + name + ": " + value + "\r\n" +
           payload.substr(header_start);
  }
}

auto HttpUtils::delete_http_header(std::string payload, std::string name)
    -> std::string {
  int start, end;
  bool found;
  std::tie(start, end, std::ignore, std::ignore) =
      http_header(payload, name, &found);
  if (!found) {
    return payload;
  } else {
    return payload.substr(0, start) + payload.substr(end);
  }
}

auto HttpUtils::http_body(std::string payload) -> std::string {
  size_t start_index = payload.find("\r\n\r\n");
  if (start_index == std::string::npos) {
    return "";
  }
  return payload.substr(start_index + 4);
}

auto HttpUtils::set_http_body(std::string payload, std::string new_body)
    -> std::string {
  std::string new_payload = set_http_header(payload, "Content-Length",
                                            std::to_string(new_body.length()));
  size_t start_index = new_payload.find("\r\n\r\n");
  if (start_index == std::string::npos) {
    return new_payload + "\r\n\r\n" + new_body;
  } else {
    return new_payload.substr(0, start_index + 4) + new_body;
  }
}

auto HttpUtils::decompress_gzip_body(std::string payload) -> std::string {
  throw std::logic_error("unimplemented");
}

auto HttpUtils::trim(const std::string &source) -> std::string {
  std::string s(source);
  s.erase(0, s.find_first_not_of(" \n\r\t"));
  s.erase(s.find_last_not_of(" \n\r\t") + 1);
  return s;
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