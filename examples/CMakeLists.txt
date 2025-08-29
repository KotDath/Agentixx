# Примеры для AgentCpp

# Базовый пример
add_executable(basic_example basic_example.cpp)
target_link_libraries(basic_example PRIVATE AgentCpp::AgentCpp)

# Пример с DeepSeek API
add_executable(deepseek_example deepseek_example.cpp)
target_link_libraries(deepseek_example PRIVATE AgentCpp::AgentCpp)

# Пример с несколькими провайдерами
add_executable(multiple_providers_example multiple_providers_example.cpp)
target_link_libraries(multiple_providers_example PRIVATE AgentCpp::AgentCpp)

# Пример со стримингом
add_executable(streaming_example streaming_example.cpp)
target_link_libraries(streaming_example PRIVATE AgentCpp::AgentCpp)

# Пример с переменными среды
add_executable(env_example env_example.cpp)
target_link_libraries(env_example PRIVATE AgentCpp::AgentCpp)

# Установка примеров (опционально)
install(TARGETS 
    basic_example 
    deepseek_example 
    multiple_providers_example
    streaming_example
    env_example
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/examples
)