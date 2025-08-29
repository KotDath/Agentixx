#include <agentixx/agentixx.hpp>
#include <iostream>
#include <vector>

int main() {
  try {
    // Настройка конфигурации
    agentixx::Config config;
    config.SetApiKey("your-openai-api-key");  // Замените на ваш API ключ

    // Создание OpenAI адаптера
    auto llm =
        std::make_unique<agentixx::OpenAIAdapter>(config, "gpt-3.5-turbo");

    std::cout << "=== AgentCpp Basic Example ===" << std::endl;
    std::cout << "Используемая модель: " << llm->ModelName() << std::endl;
    std::cout << std::endl;

    // Пример 1: Простое завершение текста
    std::cout << "1. Простое завершение текста:" << std::endl;
    std::cout << "Промпт: 'Объясни что такое искусственный интеллект'"
              << std::endl;

    auto completion_response =
        llm->Complete("Объясни что такое искусственный интеллект");

    if (completion_response.IsSuccess()) {
      std::cout << "Ответ (текст): " << completion_response.text() << std::endl;
      std::cout << "Ответ (JSON): " << completion_response.raw().dump(2)
                << std::endl;
    } else {
      std::cout << "Ошибка: " << completion_response.error_message()
                << std::endl;
    }

    std::cout << std::endl << "---" << std::endl << std::endl;

    // Пример 2: Чат с историей сообщений
    std::cout << "2. Чат с историей сообщений:" << std::endl;

    std::vector<agentixx::Message> messages = {
        {"system",
         "Ты полезный AI ассистент, который отвечает кратко и по делу."},
        {"user", "Привет! Как дела?"},
    };

    auto chat_response = llm->Chat(messages);

    if (chat_response.IsSuccess()) {
      std::cout << "Ответ AI: " << chat_response.text() << std::endl;

      // Демонстрация работы с JSON
      if (chat_response.has("usage")) {
        auto usage = chat_response.get<agentixx::Json>("usage");
        if (usage.has_value()) {
          std::cout << "Использование токенов: " << usage->dump() << std::endl;
        }
      }
    } else {
      std::cout << "Ошибка чата: " << chat_response.error_message()
                << std::endl;
    }

    std::cout << std::endl << "---" << std::endl << std::endl;

    // Пример 3: Продолжение беседы
    std::cout << "3. Продолжение беседы:" << std::endl;

    // Добавляем ответ ассистента и новый вопрос пользователя
    messages.push_back({"assistant", chat_response.text()});
    messages.push_back({"user", "Расскажи шутку про программистов"});

    auto joke_response = llm->Chat(messages);

    if (joke_response.IsSuccess()) {
      std::cout << "Шутка от AI: " << joke_response.text() << std::endl;
    } else {
      std::cout << "Ошибка при запросе шутки: " << joke_response.error_message()
                << std::endl;
    }

    std::cout << std::endl << "---" << std::endl << std::endl;

    // Пример 4: Использование с настройками
    std::cout << "4. Чат с кастомными настройками:" << std::endl;

    std::vector<agentixx::Message> creative_messages = {
        {"user", "Напиши креативную короткую историю о роботе"}};

    // Используем высокую температуру для креативности
    auto creative_response = llm->ChatWithOptions(creative_messages, 1.5, 200);

    if (creative_response.IsSuccess()) {
      std::cout << "Креативная история: " << creative_response.text()
                << std::endl;
    } else {
      std::cout << "Ошибка при генерации истории: "
                << creative_response.error_message() << std::endl;
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