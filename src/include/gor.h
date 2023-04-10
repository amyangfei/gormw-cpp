#ifndef INCLUDE_GOR_H_
#define INCLUDE_GOR_H_

#include <string>
#include <vector>
#include <iostream>

using namespace std;

namespace gor {

class GorMessage {

public:
  explicit GorMessage(string _id, string _type, vector<string> metas, string raw_meta, string http) 
      : id(_id), type(_type), metas(metas), raw_meta(raw_meta), http(http) {}

  auto hexlify() -> string;

  string id;
  string type;
  vector<string> metas;
  string raw_meta;
  string http;
};

class Utils {
public:
  static auto decode_chunked(string &chunked_data) -> string;
};

} // namespace gor

#endif  // INCLUDE_GOR_H_