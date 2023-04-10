#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include "gor.h"

using namespace gor;

auto GorMessage::hexlify() -> string{
  std::stringstream ss;
  for (char &c : this->raw_meta) {
    ss << std::setfill('0') << std::setw(2) << std::hex << (int)(unsigned char)c;
  }
  ss << "0a"; // hex of \n
  for (char &c : this->http) {
    ss << std::setfill('0') << std::setw(2) << std::hex << (int)(unsigned char)c;
  }
  ss << std::endl;
  return ss.str();
}

auto Utils::decode_chunked(string &chunked_data) -> string {
  string result = "";
  while (chunked_data != "") {
    size_t chunked_size_sep = chunked_data.find("\r\n");
    int offset = stoi(chunked_data.substr(0, chunked_size_sep), nullptr, 16);
    if (offset == 0) {
      break;
    }
    chunked_data = chunked_data.substr(chunked_size_sep + 2);
    result += chunked_data.substr(0, offset);
    chunked_data = chunked_data.substr(offset+2);
  }
  return result;
}