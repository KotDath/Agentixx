#include <agentixx/agentixx.hpp>
#include <iostream>
#include <vector>

int main() {
  try {
    // Конфигурация для DeepSeek API
    agentixx::Config config;

    // Способ 1: Явная установка (как было раньше)
    config.SetApiKey(
        "your-api-key");  // Замените на ваш DeepSeek API
                                                 // ключ
    config.SetBaseUrl("https://api.deepseek.com");  // DeepSeek URL

    // Способ 2: Загрузка из переменных среды (переопределяет настройки выше,
    // если переменные установлены) export AGENT_API_KEY="your-deepseek-api-key"
    // export AGENT_BASE_URL="https://api.deepseek.com"
    config.UseEnv();

    // Создание адаптера (использует тот же OpenAIAdapter, но с другим URL)
    auto llm =
        std::make_unique<agentixx::OpenAIAdapter>(config, "deepseek-chat");

    std::cout << "=== AgentCpp DeepSeek Example ===" << std::endl;
    std::cout << "Используемая модель: " << llm->ModelName() << std::endl;
    std::cout << "Base URL: " << config.base_url << std::endl;
    std::cout << std::endl;

    // Пример 1: Простой чат с DeepSeek
    std::cout << "1. Чат с DeepSeek:" << std::endl;

    std::vector<agentixx::Message> messages = {
        {"system",
         "Ты полезный AI ассистент от DeepSeek. Отвечай кратко и точно."},
        {"user", "Привет! Можешь рассказать о своих возможностях?"}};

    auto chat_response = llm->Chat(messages);

    if (chat_response.IsSuccess()) {
      std::cout << "DeepSeek: " << chat_response.text() << std::endl;
      std::cout << std::endl;
    } else {
      std::cout << "Ошибка чата: " << chat_response.error_message()
                << std::endl;
    }

    // Пример 2: Программирование с DeepSeek
    std::cout << "2. Вопрос по программированию:" << std::endl;

    messages.push_back({"assistant", chat_response.text()});
    messages.push_back(
        {"user", "Напиши простую функцию сортировки пузырьком на C++"});

    auto code_response = llm->Chat(messages);

    if (code_response.IsSuccess()) {
      std::cout << "Код от DeepSeek:" << std::endl;
      std::cout << code_response.text() << std::endl;
      std::cout << std::endl;
    } else {
      std::cout << "Ошибка при генерации кода: "
                << code_response.error_message() << std::endl;
    }

    // Пример 3: Использование с настройками
    std::cout << "3. Креативная задача с высокой температурой:" << std::endl;

    std::vector<agentixx::Message> creative_messages = {
        {"user",
         "Придумай оригинальную метафору для объяснения рекурсии в "
         "программировании"}};

    auto creative_response = llm->ChatWithOptions(creative_messages, 1.2, 200);

    if (creative_response.IsSuccess()) {
      std::cout << "Метафора: " << creative_response.text() << std::endl;
    } else {
      std::cout << "Ошибка: " << creative_response.error_message() << std::endl;
    }

    std::cout << std::endl << "---" << std::endl;
    std::cout << "Пример демонстрирует что AgentCpp работает с любым "
                 "OpenAI-совместимым API!"
              << std::endl;

  } catch (const agentixx::AgentCppException& e) {
    std::cerr << "Ошибка AgentCpp: " << e.what() << std::endl;
    return 1;
  } catch (const std::exception& e) {
    std::cerr << "Общая ошибка: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}