#include "agentixx/core/response.hpp"

#include <nlohmann/json.hpp>

namespace agentixx {

std::string Response::text() const {
  if (has_error_) {
    return error_message_;
  }

  // Попробуем разные способы извлечения текста из OpenAI ответа
  try {
    // Для chat completions
    if (raw_data_.contains("choices") && raw_data_["choices"].is_array() &&
        !raw_data_["choices"].empty()) {
      const auto& first_choice = raw_data_["choices"][0];

      // Новый формат (message)
      if (first_choice.contains("message") &&
          first_choice["message"].contains("content")) {
        return first_choice["message"]["content"].get<std::string>();
      }

      // Старый формат (text)
      if (first_choice.contains("text")) {
        return first_choice["text"].get<std::string>();
      }
    }

    // Если это простой текстовый ответ
    if (raw_data_.is_string()) {
      return raw_data_.get<std::string>();
    }

    // Если есть поле "content"
    if (raw_data_.contains("content")) {
      return raw_data_["content"].get<std::string>();
    }

    // Возвращаем весь JSON как строку если не смогли распарсить
    return raw_data_.dump(2);

  } catch (const nlohmann::json::exception& e) {
    return "Error parsing response: " + std::string(e.what());
  }
}

}  // namespace agentixx