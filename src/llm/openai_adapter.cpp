#include "agentixx/llm/openai_adapter.hpp"

#include <nlohmann/json.hpp>
#include <sstream>

namespace agentixx {

OpenAIAdapter::OpenAIAdapter(const Config& config, const std::string& model)
    : config_(config), model_(model) {
  if (config_.api_key.empty()) {
    throw ApiError(401, "OpenAI API key is required");
  }

  // Создаем HTTP клиент
  http_client_ = std::make_unique<HttpClient>(config_);
}

Headers OpenAIAdapter::BuildHeaders() const {
  Headers headers;
  headers["Authorization"] = "Bearer " + config_.api_key;
  headers["Content-Type"] = "application/json";

  if (!config_.organization.empty()) {
    headers["OpenAI-Organization"] = config_.organization;
  }

  if (!config_.project.empty()) {
    headers["OpenAI-Project"] = config_.project;
  }

  return headers;
}

std::string OpenAIAdapter::BuildCompletionRequest(const std::string& prompt,
                                                  bool stream) const {
  Json request = {{"model", model_},
                  {"prompt", prompt},
                  {"max_tokens", 150},
                  {"temperature", 0.7}};

  if (stream) {
    request["stream"] = true;
  }

  return request.dump();
}

std::string OpenAIAdapter::BuildChatRequest(
    const std::vector<Message>& messages, bool stream) const {
  Json request = {{"model", model_}, {"messages", Json::array()}};

  // Конвертируем сообщения в JSON
  for (const auto& msg : messages) {
    request["messages"].push_back(msg.ToJson());
  }

  if (stream) {
    request["stream"] = true;
  }

  return request.dump();
}

Response OpenAIAdapter::ParseOpenaiResponse(
    const HttpResponse& http_response) const {
  if (!http_response.IsSuccess()) {
    try {
      Json error_json = Json::parse(http_response.body);
      std::string error_message = "OpenAI API error";

      if (error_json.contains("error") &&
          error_json["error"].contains("message")) {
        error_message = error_json["error"]["message"];
      }

      throw ApiError(http_response.status_code, error_message);
    } catch (const nlohmann::json::parse_error&) {
      throw ApiError(http_response.status_code,
                     "HTTP error: " + http_response.body);
    }
  }

  try {
    Json response_json = Json::parse(http_response.body);
    return Response(response_json);
  } catch (const nlohmann::json::parse_error& e) {
    throw ParseError("Failed to parse OpenAI response: " +
                     std::string(e.what()));
  }
}

Response OpenAIAdapter::Complete(const std::string& prompt) {
  std::string url = config_.base_url + "/completions";
  std::string body = BuildCompletionRequest(prompt, false);
  Headers headers = BuildHeaders();

  HttpResponse http_response = http_client_->post(url, body, headers);
  return ParseOpenaiResponse(http_response);
}

Response OpenAIAdapter::Chat(const std::vector<Message>& messages) {
  std::string url = config_.base_url + "/chat/completions";
  std::string body = BuildChatRequest(messages, false);
  Headers headers = BuildHeaders();

  HttpResponse http_response = http_client_->post(url, body, headers);
  return ParseOpenaiResponse(http_response);
}

Response OpenAIAdapter::ChatWithOptions(const std::vector<Message>& messages,
                                        double temperature,
                                        int max_tokens) const {
  Json request = {{"model", model_},
                  {"messages", Json::array()},
                  {"temperature", temperature}};

  if (max_tokens > 0) {
    request["max_tokens"] = max_tokens;
  }

  // Конвертируем сообщения в JSON
  for (const auto& msg : messages) {
    request["messages"].push_back(msg.ToJson());
  }

  std::string url = config_.base_url + "/chat/completions";
  std::string body = request.dump();
  Headers headers = BuildHeaders();

  HttpResponse http_response = http_client_->post(url, body, headers);
  return ParseOpenaiResponse(http_response);
}

std::string OpenAIAdapter::BuildChatRequestWithOptions(
    const std::vector<Message>& messages, double temperature, int max_tokens,
    bool stream) const {
  Json request = {{"model", model_},
                  {"messages", Json::array()},
                  {"temperature", temperature}};

  if (max_tokens > 0) {
    request["max_tokens"] = max_tokens;
  }

  if (stream) {
    request["stream"] = true;
  }

  // Конвертируем сообщения в JSON
  for (const auto& msg : messages) {
    request["messages"].push_back(msg.ToJson());
  }

  return request.dump();
}

StreamingResponse OpenAIAdapter::CompleteStream(const std::string& prompt) {
  std::string url = config_.base_url + "/completions";
  std::string body = BuildCompletionRequest(prompt, true);
  Headers headers = BuildHeaders();

  return http_client_->PostStream(url, body, headers);
}

StreamingResponse OpenAIAdapter::ChatStream(
    const std::vector<Message>& messages) {
  std::string url = config_.base_url + "/chat/completions";
  std::string body = BuildChatRequest(messages, true);
  Headers headers = BuildHeaders();

  return http_client_->PostStream(url, body, headers);
}

StreamingResponse OpenAIAdapter::ChatStreamWithOptions(
    const std::vector<Message>& messages, double temperature,
    int max_tokens) const {
  std::string url = config_.base_url + "/chat/completions";
  std::string body =
      BuildChatRequestWithOptions(messages, temperature, max_tokens, true);
  Headers headers = BuildHeaders();

  return http_client_->PostStream(url, body, headers);
}

void OpenAIAdapter::ChatStreamRealtime(const std::vector<Message>& messages,
                                       StreamCallback on_chunk,
                                       std::function<void()> on_complete,
                                       StreamErrorCallback on_error) const {
  std::string url = config_.base_url + "/chat/completions";
  std::string body = BuildChatRequest(messages, true);
  Headers headers = BuildHeaders();

  http_client_->PostStreamAsync(url, body, headers, on_chunk, on_complete,
                                on_error);
}

}  // namespace agentixx