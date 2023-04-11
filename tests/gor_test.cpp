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

TEST(GorTest, HttpHeaders) {
  std::string payload = "GET / HTTP/1.1\r\nHost: localhost:3000\r\nUser-Agent: "
                        "Cpp\r\nContent-Length:5\r\n\r\nhello";
  auto headers = HttpUtils::http_headers(payload);
  EXPECT_EQ(headers["Host"], "localhost:3000");
  EXPECT_EQ(headers["User-Agent"], "Cpp");
  EXPECT_EQ(headers["Content-Length"], "5");
}

TEST(GorTest, HttpHeader) {
  std::string payload = "GET / HTTP/1.1\r\nHost: localhost:3000\r\nUser-Agent: "
                        "Cpp\r\nContent-Length:5\r\n\r\nhello";
  std::vector<std::pair<std::string, std::string>> cases{
      {"Host", "localhost:3000"},
      {"User-Agent", "Cpp"},
      {"Content-Length", "5"},
  };
  for (auto it = cases.begin(); it != cases.end(); ++it) {
    bool found;
    std::string key = it->first;
    std::string value = it->second;
    std::string header_value;
    std::tie(std::ignore, std::ignore, std::ignore, header_value) =
        HttpUtils::http_header(payload, key, &found);
    EXPECT_TRUE(found);
    EXPECT_EQ(header_value, value);
  }
}

TEST(GorTest, SetHttpHeader) {
  std::string payload = "GET / HTTP/1.1\r\nUser-Agent: Cpp\r\nContent-Length: "
                        "5\r\n\r\nhello";
  std::vector<std::pair<std::string, std::string>> cases{
      {"", "GET / HTTP/1.1\r\nUser-Agent: \r\nContent-Length: 5\r\n\r\nhello"},
      {"1",
       "GET / HTTP/1.1\r\nUser-Agent: 1\r\nContent-Length: 5\r\n\r\nhello"},
      {"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_13_0)",
       "GET / HTTP/1.1\r\nUser-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X "
       "10_13_0)\r\nContent-Length: 5\r\n\r\nhello"},
  };
  for (auto it = cases.begin(); it != cases.end(); ++it) {
    std::string key = "User-Agent";
    std::string value = it->first;
    std::string expected = it->second;
    std::string new_payload = HttpUtils::set_http_header(payload, key, value);
    EXPECT_EQ(new_payload, expected);
  }

  std::string new_payload =
      HttpUtils::set_http_header(payload, "X-Test", "test");
  EXPECT_EQ(
      new_payload,
      "GET / HTTP/1.1\r\nX-Test: test\r\nUser-Agent: Cpp\r\nContent-Length: "
      "5\r\n\r\nhello");

  new_payload = HttpUtils::set_http_header(new_payload, "X-Test2", "test2");
  EXPECT_EQ(new_payload, "GET / HTTP/1.1\r\nX-Test2: test2\r\nX-Test: "
                         "test\r\nUser-Agent: Cpp\r\nContent-Length: "
                         "5\r\n\r\nhello");
}

TEST(GorTest, DeleteHttpHeader) {
  std::string payload = "GET / HTTP/1.1\r\nUser-Agent: "
                        "Cpp\r\nContent-Length: 5\r\n\r\nhello";
  std::string new_payload =
      HttpUtils::delete_http_header(payload, "User-Agent");
  EXPECT_EQ(new_payload, "GET / HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello");
  new_payload = HttpUtils::delete_http_header(new_payload, "not-exists-header");
  EXPECT_EQ(new_payload, "GET / HTTP/1.1\r\nContent-Length: 5\r\n\r\nhello");
}

TEST(GorTest, HttpBody) {
  std::string payload =
      "GET / HTTP/1.1\r\nUser-Agent: Cpp\r\nContent-Length: 5\r\n\r\nhello";
  std::string body = HttpUtils::http_body(payload);
  EXPECT_EQ(body, "hello");

  std::string invalid_payload =
      "GET / HTTP/1.1\r\nUser-Agent: Cpp\r\nContent-Length: 5\r\nhello";
  body = HttpUtils::http_body(invalid_payload);
  EXPECT_EQ(body, "");

  // TODO: add gzip body test
}

TEST(GorTest, SetHttpBody) {
  std::string payload =
      "GET / HTTP/1.1\r\nUser-Agent: Cpp\r\nContent-Length: 5\r\n\r\nhello";
  std::string new_payload = HttpUtils::set_http_body(payload, "world");
  EXPECT_EQ(
      new_payload,
      "GET / HTTP/1.1\r\nUser-Agent: Cpp\r\nContent-Length: 5\r\n\r\nworld");

  std::string new_payload2 =
      HttpUtils::set_http_body(new_payload, "hello, world!");
  EXPECT_EQ(new_payload2, "GET / HTTP/1.1\r\nUser-Agent: "
                          "Cpp\r\nContent-Length: 13\r\n\r\nhello, world!");
}

TEST(GorTest, DecodeChunked) {
  std::string chunked_data =
      "4\r\nWiki\r\n6\r\npedia \r\nE\r\nin \r\n\r\nchunks.\r\n0\r\n\r\n";
  EXPECT_EQ(HttpUtils::decode_chunked(chunked_data),
            "Wikipedia in \r\n\r\nchunks.");
}