#pragma once

#include <functional>
#include <memory>
#include <string>

#include "streaming.hpp"
#include "types.hpp"

namespace agentixx {

class HttpClient {
 private:
  class Impl;  // PIMPL идиома для скрытия libcurl деталей
  std::unique_ptr<Impl> pimpl_;

 public:
  explicit HttpClient(const Config& config = {});
  ~HttpClient();

  // Disable copying, allow moving
  HttpClient(const HttpClient&) = delete;
  HttpClient& operator=(const HttpClient&) = delete;
  HttpClient(HttpClient&&) = default;
  HttpClient& operator=(HttpClient&&) = default;

  // HTTP методы
  HttpResponse get(const std::string& url, const Headers& headers = {});
  HttpResponse post(const std::string& url, const std::string& body,
                    const Headers& headers = {});

  // Streaming HTTP методы
  StreamingResponse PostStream(const std::string& url, const std::string& body,
                               const Headers& headers = {});
  void PostStreamAsync(const std::string& url, const std::string& body,
                       const Headers& headers, StreamCallback on_chunk,
                       std::function<void()> on_complete = nullptr,
                       StreamErrorCallback on_error = nullptr);

  // Установить базовую конфигурацию
  void SetTimeout(int timeout_ms);
  void SetDefaultHeaders(const Headers& headers);
};

}  // namespace agentixx