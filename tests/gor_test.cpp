#include "gor.h"
#include "utils.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

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