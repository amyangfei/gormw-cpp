#include "gor.h"
#include "queue.h"
#include "utils.h"
#include <atomic>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

namespace gor {

auto Callback::do_callback(Gor *g, std::shared_ptr<GorMessage> msg)
    -> std::shared_ptr<GorMessage> {
  return this->callback(g, msg, this->id, this->request, this->response,
                        this->extra);
}

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

auto Gor::parse_message(std::string line) -> std::unique_ptr<GorMessage> {
  std::string payload = Utils::hex_to_str(Utils::trim(line));
  int meta_pos = payload.find("\n");
  if (meta_pos == std::string::npos) {
    throw std::invalid_argument("raw meta separator not found" + payload);
  }
  std::string meta = payload.substr(0, meta_pos);
  std::vector<std::string> metas;
  metas.reserve(3);
  Utils::str_split(meta, ' ', metas);
  if (metas.size() != 3) {
    throw std::invalid_argument("meta must contain three fields" + meta);
  }
  return std::make_unique<GorMessage>(metas[0], metas[1], metas, meta,
                                      payload.substr(meta_pos + 1));
}

void Gor::on(std::string channel, callback_fn callback, void *extra,
             std::string id, std::shared_ptr<GorMessage> request,
             std::shared_ptr<GorMessage> response) {
  if (!id.empty()) {
    channel += "#" + id;
  }
  if (this->callbacks.find(channel) == this->callbacks.end()) {
    this->callbacks[channel] = std::vector<Callback *>();
  }
  auto cb = new Callback(id, request, response, extra, callback);
  this->callbacks[channel].emplace_back(cb);
}

void Gor::emit(std::shared_ptr<GorMessage> msg) {
  std::string chan_prefix;
  if (msg->type == "1") {
    chan_prefix = "request";
  } else if (msg->type == "2") {
    chan_prefix = "response";
  } else if (msg->type == "3") {
    chan_prefix = "replay";
  }
  std::shared_ptr<GorMessage> resp;
  auto iter = [&](std::string channel, bool gc = false) {
    if (this->callbacks.find(channel) != this->callbacks.end()) {
      for (auto &cb : this->callbacks[channel]) {
        auto r = cb->do_callback(this, msg);
        if (r != nullptr) {
          resp = r;
        }
      }
      if (gc) {
        this->callbacks.erase(channel);
      }
    }
  };
  iter("message");
  iter(chan_prefix);
  iter(chan_prefix + "#" + msg->id, true);
  if (resp != nullptr) {
    std::cout << resp->hexlify() << std::flush;
  }
}

void SimpleGor::run() {
  bounded_queue<std::unique_ptr<GorMessage>> q{16};
  auto read_from_stdin = [&]() {
    std::string line;
    while (std::getline(std::cin, line)) {
      try {
        auto msg = this->parse_message(line);
        q.push(std::move(msg));
      } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        break;
      }
    }
    this->mark_done();
  };

  // returns true on error, false on success
  auto emit_one_msg = [this](std::unique_ptr<GorMessage> msg) -> bool {
    try {
      this->emit(std::move(msg));
    } catch (std::exception &e) {
      std::cerr << e.what() << std::endl;
      return true;
    }
    return false;
  };

  auto work = [&]() {
    std::unique_ptr<gor::GorMessage> msg;
    while (!this->done_) {
      if (!q.pop(msg) || emit_one_msg(std::move(msg))) {
        break;
      }
    }
    if (this->done_) {
      while (q.size() > 0) {
        if (!q.pop(msg) || emit_one_msg(std::move(msg))) {
          break;
        }
      }
    }
    this->mark_done();
  };
  std::thread reader(std::bind(read_from_stdin));
  std::thread worker(std::bind(work));

  reader.join();
  worker.join();
}
} // namespace gor