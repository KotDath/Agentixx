# Agentixx

Современная C++ библиотека для взаимодействия с AI агентами и LLM интеграции.

## Возможности

- 🔧 **Модульная архитектура** - используйте только то, что нужно
- 🚀 **Множественные провайдеры** - OpenAI, DeepSeek, Together.ai, Groq и любые OpenAI-совместимые API
- 📦 **Гибкие ответы** - получайте данные как объекты или сырой JSON
- ⚡ **Современный C++20** - типобезопасность и производительность
- 🔐 **Безопасность** - правильная обработка API ключей и ошибок
- 🌐 **Кастомизируемые endpoints** - легко переключайтесь между провайдерами

## Быстрый старт

### Установка зависимостей

```bash
# Ubuntu/Debian
sudo apt install libcurl4-openssl-dev nlohmann-json3-dev cmake build-essential

# macOS
brew install curl nlohmann-json cmake
```

### Сборка

```bash
git clone <repository-url>
cd AgentCpp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Простой пример

```cpp
#include <agentcpp/agentcpp.hpp>

int main() {
    // Настройка
    agentixx::Config config;
    config.set_api_key("your-openai-api-key");
    
    // Создание LLM адаптера
    auto llm = std::make_unique<agentixx::OpenAIAdapter>(config);
    
    // Простой запрос
    auto response = llm->complete("Привет! Как дела?");
    
    if (response.is_success()) {
        std::cout << "Ответ: " << response.text() << std::endl;
        std::cout << "JSON: " << response.raw().dump(2) << std::endl;
    } else {
        std::cout << "Ошибка: " << response.error_message() << std::endl;
    }
    
    return 0;
}
```

### Чат с историей

```cpp
std::vector<agentixx::Message> messages = {
    {"system", "Ты полезный ассистент"},
    {"user", "Расскажи о C++"}
};

auto response = llm->chat(messages);
std::cout << response.text() << std::endl;
```

### Использование других провайдеров

```cpp
// DeepSeek
agentixx::Config deepseek_config;
deepseek_config.set_api_key("your-deepseek-key");
deepseek_config.set_base_url("https://api.deepseek.com/v1");
auto deepseek_llm = std::make_unique<agentixx::OpenAIAdapter>(deepseek_config, "deepseek-chat");

// Together.ai
agentixx::Config together_config;
together_config.set_api_key("your-together-key");  
together_config.set_base_url("https://api.together.xyz/v1");
auto together_llm = std::make_unique<agentixx::OpenAIAdapter>(together_config, "meta-llama/Llama-2-7b-chat-hf");

// Groq
agentixx::Config groq_config;
groq_config.set_api_key("your-groq-key");
groq_config.set_base_url("https://api.groq.com/openai/v1");  
auto groq_llm = std::make_unique<agentixx::OpenAIAdapter>(groq_config, "llama3-8b-8192");
```

## API

### Основные классы

- **`Config`** - конфигурация подключения
- **`Response`** - унифицированный ответ с поддержкой JSON/объектов
- **`OpenAIAdapter`** - адаптер для OpenAI API
- **`Message`** - структура сообщения для чата

### Методы Response

```cpp
std::string text() const;                      // Получить текст ответа
const Json& raw() const;                       // Получить сырой JSON  
template<typename T> T as() const;             // Преобразовать в тип
bool is_error() const;                         // Проверить на ошибку
template<typename T> std::optional<T> get(const std::string& key) const;
```

## Конфигурация

```cpp
agentixx::Config config;
config.set_api_key("your-api-key");                    // API ключ (обязательно)
config.set_base_url("https://api.openai.com/v1");      // Базовый URL (по умолчанию OpenAI)
config.set_organization("org-...");                    // Организация (опционально)
config.set_project("proj_...");                        // Проект (опционально)
config.set_timeout(30000);                            // Таймаут в мс (по умолчанию 30сек)
```

## Поддерживаемые провайдеры

AgentCpp работает с **любым OpenAI-совместимым API**:

| Провайдер | Base URL | Примечания |
|-----------|----------|------------|
| OpenAI | `https://api.openai.com/v1` | Официальный API |
| DeepSeek | `https://api.deepseek.com/v1` | Китайская модель |
| Together.ai | `https://api.together.xyz/v1` | Открытые модели |
| Groq | `https://api.groq.com/openai/v1` | Быстрые инференс |
| Perplexity | `https://api.perplexity.ai` | Поисковые возможности |
| Кастомный | `https://your-endpoint/v1` | Ваш собственный API |

## Примеры

- `basic_example.cpp` - базовое использование с OpenAI
- `deepseek_example.cpp` - пример работы с DeepSeek API  
- `multiple_providers_example.cpp` - демонстрация разных провайдеров

## Требования

- C++20 совместимый компилятор (GCC 10+, Clang 12+, MSVC 2019+)
- CMake 3.15+
- libcurl
- nlohmann/json