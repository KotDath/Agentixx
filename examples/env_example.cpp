#include <agentixx/agentixx.hpp>
#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int main() {
  try {
    // Создаем конфигурацию
    agentixx::Config config;

    std::cout << "=== AgentCpp Interactive Chat with DeepSeek ===" << std::endl;

    // Показываем значения по умолчанию
    std::cout << "Значения по умолчанию:" << std::endl;
    std::cout << "API Key: "
              << (config.api_key.empty() ? "(пустой)" : "***скрыт***")
              << std::endl;
    std::cout << "Base URL: " << config.base_url << std::endl << std::endl;

    // Загружаем из переменных среды
    std::cout
        << "Загружаем из переменных среды AGENT_API_KEY и AGENT_BASE_URL..."
        << std::endl;
    config.UseEnv();

    // Показываем обновленные значения
    std::cout << "После загрузки из переменных среды:" << std::endl;
    std::cout << "API Key: "
              << (config.api_key.empty() ? "(не установлен)" : "***скрыт***")
              << std::endl;
    std::cout << "Base URL: " << config.base_url << std::endl << std::endl;

    // Проверяем что API ключ установлен
    if (config.api_key.empty()) {
      std::cout << "⚠ API ключ не установлен. Установите переменную среды "
                   "AGENT_API_KEY"
                << std::endl;
      std::cout << "Пример:" << std::endl;
      std::cout << "export AGENT_API_KEY=\"your-deepseek-api-key\""
                << std::endl;
      std::cout << "export AGENT_BASE_URL=\"https://api.deepseek.com\""
                << std::endl;
      return 1;
    }

    // Создаем LLM адаптер с deepseek-chat моделью
    auto llm =
        std::make_unique<agentixx::OpenAIAdapter>(config, "deepseek-chat");

    std::cout << "✓ DeepSeek чат готов!" << std::endl;
    std::cout << "Модель: " << llm->ModelName() << std::endl;
    std::cout << "URL: " << config.base_url << std::endl;
    std::cout << std::endl;
    std::cout << "Введите ваши сообщения (или 'exit'/'quit' для выхода):"
              << std::endl;
    std::cout << "================================================"
              << std::endl;

    std::string user_input;
    while (true) {
      std::cout << std::endl << "Вы: ";
      std::getline(std::cin, user_input);

      // Проверяем команды выхода
      if (user_input == "exit" || user_input == "quit" ||
          user_input == "выход") {
        std::cout << "До свидания!" << std::endl;
        break;
      }

      // Пропускаем пустые сообщения
      if (user_input.empty()) {
        continue;
      }

      try {
        // Отправляем сообщение в DeepSeek
        std::vector<agentixx::Message> messages = {{"user", user_input}};

        std::cout << "DeepSeek: ";
        std::cout.flush();

        // Используем реал-тайм стриминг для показа ответа по мере получения
        llm->ChatStreamRealtime(
            messages,
            [](const agentixx::StreamChunk& chunk) {
              // Печатаем каждый chunk сразу как получили
              if (!chunk.done() && !chunk.text().empty()) {
                std::cout << chunk.text();
                std::cout.flush();
              }
            },
            []() {
              // Когда стриминг завершен
              std::cout << std::endl;
            },
            [](const std::string& error) {
              // При ошибке стриминга
              std::cout << std::endl
                        << "🔴 Streaming error: " << error << std::endl;
            });

      } catch (const agentixx::NetworkError& e) {
        std::cout << "🔴 Ошибка сети: " << e.what() << std::endl;
        std::cout << "Проверьте подключение к интернету и API ключ."
                  << std::endl;
      } catch (const agentixx::ApiError& e) {
        std::cout << "🔴 Ошибка API: " << e.what() << std::endl;
        std::cout << "Проверьте правильность API ключа и лимиты." << std::endl;
      } catch (const std::exception& e) {
        std::cout << "🔴 Ошибка: " << e.what() << std::endl;
      }
    }

  } catch (const agentixx::AgentCppException& e) {
    std::cerr << "Ошибка AgentCpp: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "Общая ошибка: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}