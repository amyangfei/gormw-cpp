#include "gor.h"
#include "utils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>

using namespace gor;

TEST(GorTest, Hexlify) {
  GorMessage msg("id", "type", {"meta1", "meta2"}, "raw_meta", "http");
  EXPECT_EQ(msg.hexlify(), "7261775f6d6574610a68747470\n");
}

TEST(GorTest, ParseMessage) {
  std::string payload = Utils::str_to_hex("1 2 3\nGET / HTTP/1.1\r\n\r\n");
  auto g = std::make_unique<SimpleGor>();
  auto msg = g->parse_message(payload);
  EXPECT_EQ(msg->type, "1");
  EXPECT_EQ(msg->id, "2");
  ASSERT_THAT(msg->metas, ::testing::ElementsAre("1", "2", "3"));
  EXPECT_EQ(msg->http, "GET / HTTP/1.1\r\n\r\n");
}

TEST(GorTest, SimpleGorInit) {
  auto g = std::make_unique<SimpleGor>();
  int counter = 0;
  callback_fn inc = [](Gor *g, std::shared_ptr<GorMessage> msg, std::string id,
                       std::shared_ptr<GorMessage> request,
                       std::shared_ptr<GorMessage> response,
                       void *extra) -> std::shared_ptr<GorMessage> {
    int *p_counter = static_cast<int *>(extra);
    *p_counter += 1;
    return nullptr;
  };
  g->on("message", inc, &counter);
  g->on("request", inc, &counter);
  g->on("response", inc, &counter, "2");

  auto req =
      g->parse_message(Utils::str_to_hex("1 2 3\nGET / HTTP/1.1\r\n\r\n"));
  auto resp =
      g->parse_message(Utils::str_to_hex("2 2 3\nHTTP/1.1 200 OK\r\n\r\n"));
  auto resp2 =
      g->parse_message(Utils::str_to_hex("2 3 3\nHTTP/1.1 200 OK\r\n\r\n"));

  g->emit(std::move(req));
  g->emit(std::move(resp));
  g->emit(std::move(resp2));
  EXPECT_EQ(counter, 5);
}

TEST(GorTest, SimpleGorRun) {
  auto g = std::make_unique<SimpleGor>();
  callback_fn inc = [](Gor *g, std::shared_ptr<GorMessage> msg, std::string id,
                       std::shared_ptr<GorMessage> request,
                       std::shared_ptr<GorMessage> response,
                       void *extra) -> std::shared_ptr<GorMessage> {
    int *p_counter = static_cast<int *>(extra);
    *p_counter += 1;
    return nullptr;
  };
  int counter = 0;
  g->on("message", inc, &counter);
  g->on("request", inc, &counter);
  g->on("response", inc, &counter, "2");

  std::string payload =
      Utils::str_to_hex("1 2 3\nGET / HTTP/1.1\r\n\r\n") + "\n" +
      Utils::str_to_hex("2 2 3\nHTTP/1.1 200 OK\r\n\r\n") + "\n" +
      Utils::str_to_hex("2 3 3\nHTTP/1.1 200 OK\r\n\r\n");
  std::istringstream sin{payload};
  // Temporarily redirect cin to string streams to allow control
  // of input and observation of output.
  auto cin_rdbuf = std::cin.rdbuf(sin.rdbuf());

  g->run();
  EXPECT_EQ(counter, 5);

  // Reset cin to normal.
  std::cin.rdbuf(cin_rdbuf);
}
