#pragma once

#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <vector>

#include "response.hpp"
#include "types.hpp"

namespace agentixx {

// Chunk данных в потоке
struct StreamChunk {
  std::string content;   // Текстовое содержимое chunk
  Json raw_data;         // Сырые данные chunk
  bool is_done = false;  // Признак завершения потока

  StreamChunk() = default;
  StreamChunk(const std::string& text) : content(text) {}
  StreamChunk(const Json& data) : raw_data(data) {
    // Извлекаем контент из delta
    if (data.contains("choices") && data["choices"].is_array() &&
        !data["choices"].empty()) {
      const auto& choice = data["choices"][0];
      if (choice.contains("delta") && choice["delta"].contains("content")) {
        content = choice["delta"]["content"].get<std::string>();
      }
    }
  }

  // Получить текст chunk
  const std::string& text() const { return content; }

  // Получить сырые данные
  const Json& raw() const { return raw_data; }

  // Проверить завершение
  bool done() const { return is_done; }
};

// Callback типы для streaming
using StreamCallback = std::function<void(const StreamChunk&)>;
using StreamErrorCallback = std::function<void(const std::string&)>;

// Класс для работы с потоковыми ответами
class StreamingResponse {
 private:
  std::vector<StreamChunk> chunks_;
  size_t current_index_ = 0;
  bool is_complete_ = false;
  std::string accumulated_text_;

 public:
  // Конструктор
  StreamingResponse() = default;

  // Добавить chunk
  void AddChunk(const StreamChunk& chunk) {
    chunks_.push_back(chunk);
    if (!chunk.is_done) {
      accumulated_text_ += chunk.content;
    } else {
      is_complete_ = true;
    }
  }

  // Завершить поток
  void finish() {
    is_complete_ = true;
    if (chunks_.empty() || !chunks_.back().is_done) {
      StreamChunk done_chunk;
      done_chunk.is_done = true;
      chunks_.push_back(done_chunk);
    }
  }

  // Получить полный накопленный текст
  std::string full_text() const { return accumulated_text_; }

  // Проверить завершение
  bool IsComplete() const { return is_complete_; }

  // Получить все chunks
  const std::vector<StreamChunk>& chunks() const { return chunks_; }

  // Iterator для range-based for loops
  class iterator {
   private:
    const StreamingResponse* response_;
    size_t index_;

   public:
    using iterator_category = std::input_iterator_tag;
    using value_type = StreamChunk;
    using difference_type = std::ptrdiff_t;
    using pointer = const StreamChunk*;
    using reference = const StreamChunk&;

    iterator(const StreamingResponse* resp, size_t idx)
        : response_(resp), index_(idx) {}

    reference operator*() const { return response_->chunks_[index_]; }
    pointer operator->() const { return &response_->chunks_[index_]; }

    iterator& operator++() {
      ++index_;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp = *this;
      ++index_;
      return tmp;
    }

    bool operator==(const iterator& other) const {
      return response_ == other.response_ && index_ == other.index_;
    }

    bool operator!=(const iterator& other) const { return !(*this == other); }
  };

  iterator begin() const { return iterator(this, 0); }
  iterator end() const { return iterator(this, chunks_.size()); }
};

// Streaming HTTP ответ с callback
struct StreamingHttpResponse {
  int status_code = 0;
  Headers headers;
  std::function<void(const std::string&)> data_callback;
  std::function<void()> complete_callback;
  std::function<void(const std::string&)> error_callback;

  bool IsSuccess() const { return status_code >= 200 && status_code < 300; }
};

}  // namespace agentixx