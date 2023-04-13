#ifndef INCLUDE_GOR_H_
#define INCLUDE_GOR_H_

#include <chrono>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace gor {

class GorMessage;
class Gor;
class Callback;

using callback_fn = std::shared_ptr<GorMessage> (*)(
    Gor *g, std::shared_ptr<GorMessage> msg, std::string id,
    std::shared_ptr<GorMessage> request, std::shared_ptr<GorMessage> response,
    void *extra);

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

class Callback {
private:
  std::chrono::system_clock::time_point start;
  std::string id;
  std::shared_ptr<GorMessage> request;
  std::shared_ptr<GorMessage> response;
  void *extra;
  callback_fn callback;

public:
  Callback(std::string _id, std::shared_ptr<GorMessage> _request,
           std::shared_ptr<GorMessage> _response, void *_extra,
           callback_fn _callback)
      : id(_id), request(_request), response(_response), extra(_extra),
        callback(_callback) {
    start = std::chrono::system_clock::now();
  }
  ~Callback() = default;
  auto do_callback(Gor *g, std::shared_ptr<GorMessage>)
      -> std::shared_ptr<GorMessage>;
};

class Gor {
private:
  std::unordered_map<std::string, std::vector<Callback *>> callbacks;

public:
  virtual ~Gor() = default;
  auto parse_message(std::string line) -> std::unique_ptr<GorMessage>;
  virtual void process_message(std::unique_ptr<GorMessage> msg) = 0;
  void on(std::string channel, callback_fn callback, void *extra = nullptr,
          std::string id = "", std::shared_ptr<GorMessage> request = nullptr,
          std::shared_ptr<GorMessage> response = nullptr);
  void emit(std::shared_ptr<GorMessage> msg);
};

class SimpleGor : public Gor {
public:
  void run();
  void process_message(std::unique_ptr<GorMessage> msg) override;
};

} // namespace gor

#endif // INCLUDE_GOR_H_