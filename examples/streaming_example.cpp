#include <agentixx/agentixx.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

int main() {
  try {
    // Конфигурация для DeepSeek API (можно заменить на OpenAI)
    agentixx::Config config;
    config.SetApiKey("your-api-key");  // Ваш API ключ
    config.SetBaseUrl(
        "https://api.deepseek.com");  // DeepSeek поддерживает стриминг

    auto llm =
        std::make_unique<agentixx::OpenAIAdapter>(config, "deepseek-chat");

    std::cout << "=== AgentCpp Streaming Example ===" << std::endl;
    std::cout << "Модель: " << llm->ModelName() << std::endl;
    std::cout << "URL: " << config.base_url << std::endl;
    std::cout << std::endl;

    // Пример 1: Простой streaming запрос
    std::cout << "1. Streaming completion:" << std::endl;
    std::cout << "Вопрос: 'Расскажи интересную историю о программисте'"
              << std::endl;
    std::cout << "Ответ (streaming): ";
    std::cout.flush();

    try {
      auto stream = llm->CompleteStream(
          "Расскажи короткую интересную историю о программисте");

      for (const auto& chunk : stream) {
        if (!chunk.done() && !chunk.text().empty()) {
          std::cout << chunk.text();
          std::cout.flush();
          // Небольшая задержка для демонстрации streaming эффекта
          std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
      }

      std::cout << std::endl << std::endl;
      std::cout << "Полный текст: " << stream.full_text() << std::endl;

    } catch (const std::exception& e) {
      std::cout << "Ошибка streaming completion: " << e.what() << std::endl;
    }

    std::cout << std::endl << "---" << std::endl << std::endl;

    // Пример 2: Streaming чат
    std::cout << "2. Streaming chat:" << std::endl;

    std::vector<agentixx::Message> messages = {
        {"system", "Ты креативный писатель. Пиши живо и интересно."},
        {"user", "Напиши короткое стихотворение о программировании"}};

    std::cout << "Вопрос: 'Напиши короткое стихотворение о программировании'"
              << std::endl;
    std::cout << "Ответ (streaming): ";
    std::cout.flush();

    try {
      auto chat_stream = llm->ChatStream(messages);

      for (const auto& chunk : chat_stream) {
        if (!chunk.done() && !chunk.text().empty()) {
          std::cout << chunk.text();
          std::cout.flush();
          std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }
      }

      std::cout << std::endl << std::endl;
      std::cout << "Chunks получено: " << chat_stream.chunks().size()
                << std::endl;

    } catch (const std::exception& e) {
      std::cout << "Ошибка streaming chat: " << e.what() << std::endl;
    }

    std::cout << std::endl << "---" << std::endl << std::endl;

    // Пример 3: Streaming с настройками
    std::cout << "3. Streaming с высокой креативностью:" << std::endl;

    std::vector<agentixx::Message> creative_messages = {
        {"user",
         "Придумай необычное объяснение что такое искусственный интеллект"}};

    std::cout << "Вопрос: 'Придумай необычное объяснение что такое ИИ'"
              << std::endl;
    std::cout << "Ответ (streaming, temperature=1.3): ";
    std::cout.flush();

    try {
      auto creative_stream =
          llm->ChatStreamWithOptions(creative_messages, 1.3, 200);

      std::string accumulated_text;
      for (const auto& chunk : creative_stream) {
        if (!chunk.done() && !chunk.text().empty()) {
          std::cout << chunk.text();
          std::cout.flush();
          accumulated_text += chunk.text();
          std::this_thread::sleep_for(std::chrono::milliseconds(40));
        }
      }

      std::cout << std::endl << std::endl;
      std::cout << "Собранный текст длиной: " << accumulated_text.length()
                << " символов" << std::endl;

    } catch (const std::exception& e) {
      std::cout << "Ошибка creative streaming: " << e.what() << std::endl;
    }

    std::cout << std::endl << "---" << std::endl << std::endl;

    // Пример 4: Сравнение обычного и streaming ответа
    std::cout << "4. Сравнение обычного и streaming ответа:" << std::endl;

    std::vector<agentixx::Message> simple_messages = {
        {"user", "Объясни рекурсию простыми словами"}};

    std::cout << "Обычный ответ:" << std::endl;
    auto normal_start = std::chrono::high_resolution_clock::now();
    auto normal_response = llm->Chat(simple_messages);
    auto normal_end = std::chrono::high_resolution_clock::now();
    auto normal_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(normal_end -
                                                              normal_start);

    std::cout << normal_response.text() << std::endl;
    std::cout << "Время выполнения: " << normal_duration.count() << "мс"
              << std::endl
              << std::endl;

    std::cout << "Streaming ответ:" << std::endl;
    auto stream_start = std::chrono::high_resolution_clock::now();
    auto streaming_response = llm->ChatStream(simple_messages);

    for (const auto& chunk : streaming_response) {
      if (!chunk.done() && !chunk.text().empty()) {
        std::cout << chunk.text();
        std::cout.flush();
      }
    }
    auto stream_end = std::chrono::high_resolution_clock::now();
    auto stream_duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(stream_end -
                                                              stream_start);

    std::cout << std::endl;
    std::cout << "Время выполнения streaming: " << stream_duration.count()
              << "мс" << std::endl;
    std::cout << "Полный текст совпадает: "
              << (normal_response.text() == streaming_response.full_text()
                      ? "Да"
                      : "Нет")
              << std::endl;

    std::cout
        << std::endl
        << "🎉 Streaming работает! Теперь ответы приходят в реальном времени."
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