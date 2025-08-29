#pragma once

// Main AgentCpp header - includes all necessary components

// Core components
#include "core/http_client.hpp"
#include "core/response.hpp"
#include "core/streaming.hpp"
#include "core/types.hpp"

// LLM adapters
#include "llm/llm_interface.hpp"
#include "llm/openai_adapter.hpp"

// Main namespace
namespace agentixx {
// All types and classes are already defined in corresponding headers

// Convenient aliases for commonly used types
using LLM = std::unique_ptr<LLMInterface>;
using OpenAI = OpenAIAdapter;
}  // namespace agentixx

// Library version
#define AGENTCPP_VERSION_MAJOR 0
#define AGENTCPP_VERSION_MINOR 0
#define AGENTCPP_VERSION_PATCH 1