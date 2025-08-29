#include <agentixx/agentixx.hpp>
#include <cassert>
#include <iostream>

int main() {
  std::cout << "=== AgentCpp Conan Package Test ===" << std::endl;

  try {
    // Test 1: Config creation and basic functionality
    std::cout << "Test 1: Config creation... ";
    agentixx::Config config;
    config.SetApiKey("test-key");
    config.SetBaseUrl("https://test.example.com/v1");
    config.SetTimeout(30000);

    assert(config.api_key == "test-key");
    assert(config.base_url == "https://test.example.com/v1");
    assert(config.timeout_ms == 30000);
    std::cout << "✓ PASSED" << std::endl;

    // Test 2: Message creation and JSON conversion
    std::cout << "Test 2: Message handling... ";
    agentixx::Message msg("user", "Hello, AI!");
    auto json_msg = msg.ToJson();

    assert(msg.role == "user");
    assert(msg.content == "Hello, AI!");
    assert(json_msg["role"] == "user");
    assert(json_msg["content"] == "Hello, AI!");

    auto msg_from_json = agentixx::Message::FromJson(json_msg);
    assert(msg_from_json.role == msg.role);
    assert(msg_from_json.content == msg.content);
    std::cout << "✓ PASSED" << std::endl;

    // Test 3: Response object functionality
    std::cout << "Test 3: Response handling... ";
    agentixx::Json test_json = {
        {"choices", {{{"message", {{"content", "Test response"}}}}}},
        {"usage", {{"total_tokens", 42}}}};

    agentixx::Response response(test_json);
    assert(response.IsSuccess());
    assert(response.text() == "Test response");
    assert(response.has("usage"));

    auto usage = response.get<agentixx::Json>("usage");
    assert(usage.has_value());
    assert(usage->at("total_tokens") == 42);
    std::cout << "✓ PASSED" << std::endl;

    // Test 4: Exception types
    std::cout << "Test 4: Exception handling... ";
    try {
      throw agentixx::NetworkError("Test network error");
    } catch (const agentixx::AgentCppException& e) {
      std::string error_msg = e.what();
      assert(error_msg.find("Network error: Test network error") !=
             std::string::npos);
    }

    try {
      throw agentixx::ApiError(404, "Not found");
    } catch (const agentixx::ApiError& e) {
      assert(e.status_code == 404);
      std::string error_msg = e.what();
      assert(error_msg.find("API error (404): Not found") != std::string::npos);
    }
    std::cout << "✓ PASSED" << std::endl;

    // Test 5: HttpClient instantiation (without actual network calls)
    std::cout << "Test 5: HttpClient creation... ";
    agentixx::HttpClient client(config);
    // We don't make actual HTTP calls in the test to avoid network dependencies
    std::cout << "✓ PASSED" << std::endl;

    // Test 6: Version macros
    std::cout << "Test 6: Version information... ";
    assert(AGENTCPP_VERSION_MAJOR >= 0);
    assert(AGENTCPP_VERSION_MINOR >= 1);
    assert(AGENTCPP_VERSION_PATCH >= 0);
    std::cout << "✓ PASSED" << std::endl;

    std::cout << std::endl << "🎉 All tests passed successfully!" << std::endl;
    std::cout << "AgentCpp v" << AGENTCPP_VERSION_MAJOR << "."
              << AGENTCPP_VERSION_MINOR << "." << AGENTCPP_VERSION_PATCH
              << " is working correctly." << std::endl;

    return 0;

  } catch (const std::exception& e) {
    std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "❌ Test failed with unknown exception" << std::endl;
    return 1;
  }
}