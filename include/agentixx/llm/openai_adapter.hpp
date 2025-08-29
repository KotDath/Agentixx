#pragma once

#include <memory>

#include "../core/http_client.hpp"
#include "llm_interface.hpp"

namespace agentixx {

class OpenAIAdapter : public LLMInterface {
 private:
  Config config_;
  std::unique_ptr<HttpClient> http_client_;
  std::string model_;

  // Private methods for building requests
  Headers BuildHeaders() const;
  std::string BuildCompletionRequest(const std::string& prompt,
                                     bool stream = false) const;
  std::string BuildChatRequest(const std::vector<Message>& messages,
                               bool stream = false) const;
  std::string BuildChatRequestWithOptions(const std::vector<Message>& messages,
                                          double temperature, int max_tokens,
                                          bool stream = false) const;
  Response ParseOpenaiResponse(const HttpResponse& http_response) const;

 public:
  explicit OpenAIAdapter(const Config& config,
                         const std::string& model = "gpt-3.5-turbo");
  ~OpenAIAdapter() override = default;

  // Disable copying, allow moving
  OpenAIAdapter(const OpenAIAdapter&) = delete;
  OpenAIAdapter& operator=(const OpenAIAdapter&) = delete;
  OpenAIAdapter(OpenAIAdapter&&) = default;
  OpenAIAdapter& operator=(OpenAIAdapter&&) = default;

  // LLMInterface implementation
  Response Complete(const std::string& prompt) override;
  Response Chat(const std::vector<Message>& messages) override;
  StreamingResponse CompleteStream(const std::string& prompt) override;
  StreamingResponse ChatStream(const std::vector<Message>& messages) override;
  std::string ModelName() const override { return model_; }

  // OpenAI specific methods
  void SetModel(const std::string& model) { model_ = model; }
  Response ChatWithOptions(const std::vector<Message>& messages,
                           double temperature = 1.0, int max_tokens = -1) const;
  StreamingResponse ChatStreamWithOptions(const std::vector<Message>& messages,
                                          double temperature = 1.0,
                                          int max_tokens = -1) const;

  // Real-time streaming with callbacks
  void ChatStreamRealtime(const std::vector<Message>& messages,
                          StreamCallback on_chunk,
                          std::function<void()> on_complete = nullptr,
                          StreamErrorCallback on_error = nullptr) const;
};

}  // namespace agentixx