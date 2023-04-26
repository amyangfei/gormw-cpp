#include "gor.h"
#include "utils.h"
#include <iostream>

using sptr_gor_msg = std::shared_ptr<gor::GorMessage>;

auto on_replay(gor::Gor *g, sptr_gor_msg msg, std::string id,
               sptr_gor_msg request, sptr_gor_msg response, void *extra)
    -> sptr_gor_msg {
  auto replay_status = gor::HttpUtils::http_status(msg->http);
  auto resp_status = gor::HttpUtils::http_status(response->http);
  if (replay_status != resp_status) {
    std::cerr << "replay status:" << replay_status
              << " diffs from response status:" << resp_status << std::endl;
  } else {
    std::cerr << "replay status is the same as response status" << std::endl;
  }
  std::cerr << std::flush;
  return nullptr;
}

auto on_response(gor::Gor *g, sptr_gor_msg msg, std::string id,
                 sptr_gor_msg request, sptr_gor_msg response, void *extra)
    -> sptr_gor_msg {
  g->on("replay", on_replay, nullptr, request->id, request, msg);
  return nullptr;
}

auto on_request(gor::Gor *g, sptr_gor_msg msg, std::string id,
                sptr_gor_msg request, sptr_gor_msg response, void *extra)
    -> sptr_gor_msg {
  g->on("response", on_response, nullptr, msg->id, msg);
  return nullptr;
}

int main(int argc, char **argv) {
  gor::Gor *g = new gor::SimpleGor();
  g->on("request", on_request);
  g->run();
}