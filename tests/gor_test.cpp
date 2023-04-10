#include "gor.h"
#include <gtest/gtest.h>

using namespace gor;

TEST(GorTest, Hexlify) {
  GorMessage msg("id", "type", {"meta1", "meta2"}, "raw_meta", "http");
  EXPECT_EQ(msg.hexlify(), "7261775f6d6574610a68747470\n");
}

TEST(GorTest, DecodeChunked) {
  string chunked_data = "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r\nchunks.\r\n0\r\n\r\n";
  EXPECT_EQ(Utils::decode_chunked(chunked_data), "Wikipedia in \r\n\r\nchunks.");
}