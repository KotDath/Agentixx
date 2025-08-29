#pragma once

#include <nlohmann/json.hpp>
#include <optional>
#include <string>

#include "types.hpp"

namespace agentixx {

class Response {
 private:
  Json raw_data_;
  bool has_error_;
  std::string error_message_;

 public:
  // Конструкторы
  Response() : has_error_(false) {}
  explicit Response(const Json& data) : raw_data_(data), has_error_(false) {}
  explicit Response(const std::string& error)
      : has_error_(true), error_message_(error) {}

  // Получить сырые JSON данные
  const Json& raw() const { return raw_data_; }

  // Получить текстовое содержимое
  std::string text() const;

  // Преобразовать в конкретный тип
  template <typename T>
  T as() const {
    if (has_error_) {
      throw ParseError("Cannot parse response with error: " + error_message_);
    }
    try {
      return raw_data_.get<T>();
    } catch (const nlohmann::json::exception& e) {
      throw ParseError("Failed to parse JSON: " + std::string(e.what()));
    }
  }

  // Проверка на ошибку
  bool IsError() const { return has_error_; }
  std::string error_message() const { return error_message_; }

  // Проверка на успех
  bool IsSuccess() const { return !has_error_; }

  // Получить поле из JSON
  template <typename T>
  std::optional<T> get(const std::string& key) const {
    if (has_error_ || !raw_data_.contains(key)) {
      return std::nullopt;
    }
    try {
      return raw_data_[key].get<T>();
    } catch (const nlohmann::json::exception&) {
      return std::nullopt;
    }
  }

  // Проверить наличие поля
  bool has(const std::string& key) const {
    return !has_error_ && raw_data_.contains(key);
  }
};

}  // namespace agentixx