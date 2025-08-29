#include "agentixx/core/http_client.hpp"

#include <curl/curl.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace agentixx {

// PIMPL реализация для скрытия libcurl деталей
class HttpClient::Impl {
 private:
  CURL* curl_;
  Config config_;

  // Callback для записи данных ответа
  static size_t WriteCallback(void* contents, size_t size, size_t nmemb,
                              std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
  }

  // Callback для записи заголовков
  static size_t HeaderCallback(void* contents, size_t size, size_t nmemb,
                               Headers* headers) {
    size_t totalSize = size * nmemb;
    std::string header((char*)contents, totalSize);

    // Найти разделитель ':'
    size_t colonPos = header.find(':');
    if (colonPos != std::string::npos) {
      std::string key = header.substr(0, colonPos);
      std::string value = header.substr(colonPos + 1);

      // Убрать пробелы
      key.erase(key.find_last_not_of(" \t\r\n") + 1);
      value.erase(0, value.find_first_not_of(" \t\r\n"));
      value.erase(value.find_last_not_of(" \t\r\n") + 1);

      (*headers)[key] = value;
    }

    return totalSize;
  }

  // Структура для streaming callback
  struct StreamingCallbackData {
    StreamCallback on_chunk;
    std::function<void()> on_complete;
    StreamErrorCallback on_error;
    std::string buffer;
    bool finished = false;
  };

  // Callback для streaming данных (Server-Sent Events)
  static size_t StreamingWriteCallback(void* contents, size_t size,
                                       size_t nmemb,
                                       StreamingCallbackData* data) {
    size_t totalSize = size * nmemb;
    std::string chunk((char*)contents, totalSize);
    data->buffer += chunk;

    // Обрабатываем SSE события построчно
    size_t pos = 0;
    while ((pos = data->buffer.find('\n')) != std::string::npos) {
      std::string line = data->buffer.substr(0, pos);
      data->buffer.erase(0, pos + 1);

      // Убираем \r если есть
      if (!line.empty() && line.back() == '\r') {
        line.pop_back();
      }

      // Парсим SSE формат
      if (line.length() >= 6 && line.substr(0, 6) == "data: ") {
        std::string json_data = line.substr(6);

        if (json_data == "[DONE]") {
          // Конец потока
          data->finished = true;
          if (data->on_complete) {
            data->on_complete();
          }
          break;
        } else if (!json_data.empty()) {
          // Парсим JSON chunk
          try {
            Json chunk_json = Json::parse(json_data);
            StreamChunk chunk(chunk_json);
            if (data->on_chunk) {
              data->on_chunk(chunk);
            }
          } catch (const nlohmann::json::parse_error& e) {
            if (data->on_error) {
              data->on_error("Failed to parse JSON chunk: " +
                             std::string(e.what()));
            }
          }
        }
      }
    }

    return totalSize;
  }

 public:
  explicit Impl(const Config& config) : config_(config) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    if (!curl_) {
      throw NetworkError("Failed to initialize CURL");
    }

    // Базовые настройки
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, config_.timeout_ms);
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);
  }

  ~Impl() {
    if (curl_) {
      curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
  }

  HttpResponse get(const std::string& url, const Headers& headers) {
    return make_request(url, "GET", "", headers);
  }

  HttpResponse post(const std::string& url, const std::string& body,
                    const Headers& headers) {
    return make_request(url, "POST", body, headers);
  }

  void SetTimeout(int timeout_ms) {
    config_.timeout_ms = timeout_ms;
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_ms);
  }

  StreamingResponse PostStream(const std::string& url, const std::string& body,
                               const Headers& headers) {
    StreamingResponse streaming_response;

    PostStreamAsync(
        url, body, headers,
        [&streaming_response](const StreamChunk& chunk) {
          streaming_response.AddChunk(chunk);
        },
        [&streaming_response]() { streaming_response.finish(); },
        [](const std::string& error) {
          throw NetworkError("Streaming error: " + error);
        });

    return streaming_response;
  }

  void PostStreamAsync(const std::string& url, const std::string& body,
                       const Headers& headers, StreamCallback on_chunk,
                       std::function<void()> on_complete,
                       StreamErrorCallback on_error) {
    struct curl_slist* curl_headers = nullptr;
    StreamingCallbackData callback_data{on_chunk, on_complete, on_error, "",
                                        false};

    try {
      // Настройка URL и POST метода
      curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
      curl_easy_setopt(curl_, CURLOPT_POST, 1L);
      curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
      curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.length());

      // Установка заголовков
      Headers all_headers = config_.default_headers;
      for (const auto& header : headers) {
        all_headers[header.first] = header.second;
      }

      // Добавляем Accept для SSE
      all_headers["Accept"] = "text/event-stream";
      all_headers["Cache-Control"] = "no-cache";

      for (const auto& header : all_headers) {
        std::string header_str = header.first + ": " + header.second;
        curl_headers = curl_slist_append(curl_headers, header_str.c_str());
      }
      curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, curl_headers);

      // Настройка streaming callback
      curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, StreamingWriteCallback);
      curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &callback_data);

      // Выполнение запроса
      CURLcode res = curl_easy_perform(curl_);

      if (res != CURLE_OK) {
        throw NetworkError(std::string("CURL streaming error: ") +
                           curl_easy_strerror(res));
      }

      // Проверка статуса ответа
      long response_code;
      curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);

      if (response_code < 200 || response_code >= 300) {
        throw ApiError(static_cast<int>(response_code), "HTTP streaming error");
      }

    } catch (...) {
      if (curl_headers) {
        curl_slist_free_all(curl_headers);
      }
      throw;
    }

    if (curl_headers) {
      curl_slist_free_all(curl_headers);
    }

    // Убедимся что поток завершен
    if (!callback_data.finished && on_complete) {
      on_complete();
    }
  }

 private:
  HttpResponse make_request(const std::string& url, const std::string& method,
                            const std::string& body, const Headers& headers) {
    HttpResponse response;
    struct curl_slist* curl_headers = nullptr;

    try {
      // Настройка URL
      curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());

      // Настройка метода
      if (method == "POST") {
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        if (!body.empty()) {
          curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, body.c_str());
          curl_easy_setopt(curl_, CURLOPT_POSTFIELDSIZE, body.length());
        }
      } else if (method == "GET") {
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
      }

      // Установка заголовков
      Headers all_headers = config_.default_headers;
      for (const auto& header : headers) {
        all_headers[header.first] = header.second;
      }

      for (const auto& header : all_headers) {
        std::string header_str = header.first + ": " + header.second;
        curl_headers = curl_slist_append(curl_headers, header_str.c_str());
      }
      curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, curl_headers);

      // Настройка callbacks
      curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
      curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response.body);
      curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, HeaderCallback);
      curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &response.headers);

      // Выполнение запроса
      CURLcode res = curl_easy_perform(curl_);

      if (res != CURLE_OK) {
        throw NetworkError(std::string("CURL error: ") +
                           curl_easy_strerror(res));
      }

      // Получение кода ответа
      long response_code;
      curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);
      response.status_code = static_cast<int>(response_code);

    } catch (...) {
      if (curl_headers) {
        curl_slist_free_all(curl_headers);
      }
      throw;
    }

    if (curl_headers) {
      curl_slist_free_all(curl_headers);
    }

    return response;
  }
};

// Реализация публичных методов HttpClient

HttpClient::HttpClient(const Config& config)
    : pimpl_(std::make_unique<Impl>(config)) {}

HttpClient::~HttpClient() = default;

HttpResponse HttpClient::get(const std::string& url, const Headers& headers) {
  return pimpl_->get(url, headers);
}

HttpResponse HttpClient::post(const std::string& url, const std::string& body,
                              const Headers& headers) {
  return pimpl_->post(url, body, headers);
}

void HttpClient::SetTimeout(int timeout_ms) { pimpl_->SetTimeout(timeout_ms); }

void HttpClient::SetDefaultHeaders(const Headers& headers) {
  // Это метод можно реализовать позже, если понадобится
}

StreamingResponse HttpClient::PostStream(const std::string& url,
                                         const std::string& body,
                                         const Headers& headers) {
  return pimpl_->PostStream(url, body, headers);
}

void HttpClient::PostStreamAsync(const std::string& url,
                                 const std::string& body,
                                 const Headers& headers,
                                 StreamCallback on_chunk,
                                 std::function<void()> on_complete,
                                 StreamErrorCallback on_error) {
  pimpl_->PostStreamAsync(url, body, headers, on_chunk, on_complete, on_error);
}

}  // namespace agentixx