#include "gor.h"
#include "http_utils.h"
#include <gtest/gtest.h>

using namespace gor;

TEST(GorTest, Hexlify) {
  GorMessage msg("id", "type", {"meta1", "meta2"}, "raw_meta", "http");
  EXPECT_EQ(msg.hexlify(), "7261775f6d6574610a68747470\n");
}

TEST(GorTest, HttpMethod) {
  Gor *g = new (Gor);
  EXPECT_EQ(HttpUtils::http_method("GET / HTTP/1.1\r\n\r\n"), "GET");
  EXPECT_EQ(HttpUtils::http_method("POST / HTTP/1.1\r\n\r\n"), "POST");
  EXPECT_EQ(HttpUtils::http_method("PUT / HTTP/1.1\r\n\r\n"), "PUT");
  EXPECT_EQ(HttpUtils::http_method("DELETE / HTTP/1.1\r\n\r\n"), "DELETE");
}

TEST(GorTest, HttpPath) {
  std::string payload = "GET /test HTTP/1.1\r\n\r\n";
  EXPECT_EQ(HttpUtils::http_path(payload), "/test");
  payload = "GET /emoji/😊 HTTP/1.1\r\n\r\n";
  EXPECT_EQ(HttpUtils::http_path(payload), "/emoji/😊");

  std::string new_payload = HttpUtils::set_http_path(payload, "/new/test/path");
  EXPECT_EQ(new_payload, "GET /new/test/path HTTP/1.1\r\n\r\n");
}

bool vector_contains_value(const std::vector<std::string> &vec,
                           std::string value) {
  for (auto it = vec.begin(); it != vec.end(); ++it) {
    if (*it == value) {
      return true;
    }
  }
  return false;
}

TEST(GorTest, HttpParam) {
  std::string payload = "GET / HTTP/1.1\r\n\r\n";
  bool found;
  Gor *g = new (Gor);
  EXPECT_FALSE(found);
  EXPECT_TRUE(HttpUtils::http_path_param(payload, "test", &found).empty());

  payload = HttpUtils::set_http_path_param(payload, "test", "123");
  EXPECT_EQ(HttpUtils::http_path(payload), "/?test=123");
  auto v1 = HttpUtils::http_path_param(payload, "test", &found);
  EXPECT_TRUE(found);
  EXPECT_FALSE(v1.empty());
  EXPECT_TRUE(vector_contains_value(v1, "123"));

  payload = HttpUtils::set_http_path_param(payload, "qwer", "ty");
  EXPECT_EQ(HttpUtils::http_path(payload), "/?test=123&qwer=ty");
  auto v2 = HttpUtils::http_path_param(payload, "qwer", &found);
  EXPECT_TRUE(found);
  EXPECT_FALSE(v2.empty());
  EXPECT_TRUE(vector_contains_value(v2, "ty"));

  payload =
      HttpUtils::set_http_path_param(payload, "comp", "value !@#$%^&*()_+");
  EXPECT_EQ(
      HttpUtils::http_path(payload),
      "/?test=123&qwer=ty&comp=value%20%21%40%23%24%25%5E%26%2A%28%29_%2B");
}

TEST(GorTest, DecodeChunked) {
  std::string chunked_data =
      "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r\nchunks.\r\n0\r\n\r\n";
  EXPECT_EQ(HttpUtils::decode_chunked(chunked_data),
            "Wikipedia in \r\n\r\nchunks.");
}