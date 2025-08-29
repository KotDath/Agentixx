#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

#include "../core/response.hpp"
#include "../core/streaming.hpp"
#include "../core/types.hpp"

namespace agentixx {

// Структура сообщения для чата
struct Message {
  std::string role;     // "user", "assistant", "system"
  std::string content;  // текст сообщения

  Message() = default;
  Message(const std::string& r, const std::string& c) : role(r), content(c) {}

  // Конвертация в JSON
  Json ToJson() const { return Json{{"role", role}, {"content", content}}; }

  // Создание из JSON
  static Message FromJson(const Json& json) {
    return Message(json["role"], json["content"]);
  }
};

// Базовый интерфейс для LLM адаптеров
class LLMInterface {
 public:
  virtual ~LLMInterface() = default;

  // Простое завершение текста
  virtual Response Complete(const std::string& prompt) = 0;

  // Чат с историей сообщений
  virtual Response Chat(const std::vector<Message>& messages) = 0;

  // Streaming версии (опциональные, по умолчанию выбрасывают исключение)
  virtual StreamingResponse CompleteStream(const std::string& prompt) {
    throw std::runtime_error(
        "Streaming completion not supported by this adapter");
  }

  virtual StreamingResponse ChatStream(const std::vector<Message>& messages) {
    throw std::runtime_error("Streaming chat not supported by this adapter");
  }

  // Получить информацию о модели
  virtual std::string ModelName() const = 0;
};

// SFINAE проверка для LLM адаптера
template <typename T>
struct is_llm_adapter {
 private:
  template <typename U>
  static auto test_complete(int)
      -> decltype(std::declval<U>().Complete(std::declval<std::string>()),
                  std::true_type{});
  template <typename>
  static std::false_type test_complete(...);

  template <typename U>
  static auto test_chat(int)
      -> decltype(std::declval<U>().Chat(std::declval<std::vector<Message>>()),
                  std::true_type{});
  template <typename>
  static std::false_type test_chat(...);

  template <typename U>
  static auto test_model_name(int) -> decltype(std::declval<U>().ModelName(),
                                               std::true_type{});
  template <typename>
  static std::false_type test_model_name(...);

 public:
  static constexpr bool value = decltype(test_complete<T>(0))::value &&
                                decltype(test_chat<T>(0))::value &&
                                decltype(test_model_name<T>(0))::value;
};

template <typename T>
constexpr bool is_llm_adapter_v = is_llm_adapter<T>::value;

// Фабрика для создания LLM адаптеров
template <typename AdapterType>
typename std::enable_if<is_llm_adapter_v<AdapterType>,
                        std::unique_ptr<LLMInterface>>::type
create_llm_adapter(const Config& config) {
  return std::make_unique<AdapterType>(config);
}

}  // namespace agentixx