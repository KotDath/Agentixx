#pragma once

#include <cstdlib>
#include <functional>
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <vector>

namespace agentixx {

// JSON alias для удобства
using Json = nlohmann::json;

// Типы для HTTP
using Headers = std::map<std::string, std::string>;

// Базовая конфигурация
struct Config {
  std::string api_key;
  std::string base_url = "https://api.openai.com/v1";
  std::string organization;
  std::string project;
  int timeout_ms = 90000;
  Headers default_headers;

  void SetApiKey(const std::string& key) { api_key = key; }
  void SetBaseUrl(const std::string& url) { base_url = url; }
  void SetOrganization(const std::string& org) { organization = org; }
  void SetProject(const std::string& proj) { project = proj; }
  void SetTimeout(int timeout) { timeout_ms = timeout; }

  // Загрузка конфигурации из переменных среды
  void UseEnv() {
    const char* env_api_key = std::getenv("AGENT_API_KEY");
    if (env_api_key) {
      api_key = env_api_key;
    }

    const char* env_base_url = std::getenv("AGENT_BASE_URL");
    if (env_base_url) {
      base_url = env_base_url;
    }
  }
};

// Структура HTTP ответа
struct HttpResponse {
  int status_code = 0;
  std::string body;
  Headers headers;
  bool IsSuccess() const { return status_code >= 200 && status_code < 300; }
};

// Базовые исключения
class AgentCppException : public std::exception {
 private:
  std::string message_;

 public:
  explicit AgentCppException(const std::string& message) : message_(message) {}
  const char* what() const noexcept override { return message_.c_str(); }
};

class NetworkError : public AgentCppException {
 public:
  explicit NetworkError(const std::string& message)
      : AgentCppException("Network error: " + message) {}
};

class ApiError : public AgentCppException {
 public:
  int status_code;
  explicit ApiError(int code, const std::string& message)
      : AgentCppException("API error (" + std::to_string(code) +
                          "): " + message),
        status_code(code) {}
};

class ParseError : public AgentCppException {
 public:
  explicit ParseError(const std::string& message)
      : AgentCppException("Parse error: " + message) {}
};

}  // namespace agentixx