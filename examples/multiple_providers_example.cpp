#include <agentixx/agentixx.hpp>
#include <iostream>
#include <memory>
#include <vector>

// Удобные константы для популярных OpenAI-совместимых провайдеров
namespace agentixx {
namespace providers {
const std::string OPENAI = "https://api.openai.com/v1";
const std::string DEEPSEEK = "https://api.deepseek.com/v1";
const std::string TOGETHER = "https://api.together.xyz/v1";
const std::string PERPLEXITY = "https://api.perplexity.ai";
const std::string GROQ = "https://api.groq.com/openai/v1";
}  // namespace providers
}  // namespace agentixx

// Функция для создания конфигурации провайдера
agentixx::Config create_provider_config(const std::string& provider_name,
                                        const std::string& api_key,
                                        const std::string& base_url) {
  agentixx::Config config;
  config.SetApiKey(api_key);
  config.SetBaseUrl(base_url);
  return config;
}

int main() {
  try {
    std::cout << "=== AgentCpp Multiple Providers Example ===" << std::endl;
    std::cout << "Демонстрация работы с разными OpenAI-совместимыми API"
              << std::endl;
    std::cout << std::endl;

    // Конфигурации для разных провайдеров
    std::vector<std::pair<std::string, agentixx::Config>> providers = {
        {"OpenAI", create_provider_config("OpenAI", "your-openai-key",
                                          agentixx::providers::OPENAI)},
        {"DeepSeek", create_provider_config("DeepSeek", "your-deepseek-key",
                                            agentixx::providers::DEEPSEEK)},
        {"Together.ai", create_provider_config("Together", "your-together-key",
                                               agentixx::providers::TOGETHER)},
        {"Groq", create_provider_config("Groq", "your-groq-key",
                                        agentixx::providers::GROQ)}};

    // Модели для каждого провайдера
    std::map<std::string, std::string> provider_models = {
        {"OpenAI", "gpt-3.5-turbo"},
        {"DeepSeek", "deepseek-chat"},
        {"Together.ai", "meta-llama/Llama-2-7b-chat-hf"},
        {"Groq", "llama3-8b-8192"}};

    std::string question =
        "Объясни принцип работы нейронных сетей простыми словами";

    // Опрашиваем каждый провайдер
    for (const auto& [provider_name, config] : providers) {
      std::cout << "--- " << provider_name << " ---" << std::endl;
      std::cout << "URL: " << config.base_url << std::endl;
      std::cout << "Модель: " << provider_models[provider_name] << std::endl;

      try {
        // Создаем адаптер для провайдера
        auto llm = std::make_unique<agentixx::OpenAIAdapter>(
            config, provider_models[provider_name]);

        // Отправляем одинаковый вопрос
        std::vector<agentixx::Message> messages = {
            {"system", "Ты полезный AI ассистент. Отвечай кратко и понятно."},
            {"user", question}};

        auto response = llm->Chat(messages);

        if (response.IsSuccess()) {
          std::cout << "Ответ: " << response.text().substr(0, 200) << "..."
                    << std::endl;

          // Показываем информацию об использовании если доступна
          if (response.has("usage")) {
            auto usage = response.get<agentixx::Json>("usage");
            if (usage.has_value() && usage->contains("total_tokens")) {
              std::cout << "Токены: " << (*usage)["total_tokens"] << std::endl;
            }
          }
        } else {
          std::cout << "Ошибка: " << response.error_message() << std::endl;
        }

      } catch (const agentixx::AgentCppException& e) {
        std::cout << "Ошибка с " << provider_name << ": " << e.what()
                  << std::endl;
      }

      std::cout << std::endl;
    }

    std::cout << "=== Пример кастомного провайдера ===" << std::endl;

    // Пример создания кастомной конфигурации
    agentixx::Config custom_config;
    custom_config.SetApiKey("your-custom-key");
    custom_config.SetBaseUrl(
        "https://your-custom-endpoint.com/v1");  // Ваш кастомный endpoint
    custom_config.SetTimeout(60000);  // Увеличенный таймаут

    // Можно добавить дополнительные заголовки
    custom_config.default_headers["X-Custom-Header"] = "custom-value";

    std::cout << "Кастомная конфигурация:" << std::endl;
    std::cout << "URL: " << custom_config.base_url << std::endl;
    std::cout << "Таймаут: " << custom_config.timeout_ms << "мс" << std::endl;
    std::cout << "Дополнительные заголовки: "
              << custom_config.default_headers.size() << std::endl;

    std::cout << std::endl;
    std::cout << "✅ AgentCpp поддерживает любой OpenAI-совместимый API!"
              << std::endl;
    std::cout << "Просто измените base_url и при необходимости модель."
              << std::endl;

  } catch (const std::exception& e) {
    std::cerr << "Ошибка: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}