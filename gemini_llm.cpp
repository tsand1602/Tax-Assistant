#include "gemini_llm.hpp" 

using json = nlohmann::json;

namespace GeminiLLM {

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t total_size = size * nmemb;
    output->append(static_cast<char*>(contents), total_size);
    return total_size;
}

std::string loadGeminiApiKey(const std::string& config_filename = "config.json") {
    std::ifstream configFile(config_filename);
    if (!configFile.is_open()) {
        std::cerr << "Gemini LLM Error: Could not open config file: " << config_filename << std::endl;
        return "";
    }
    json config;
    try {
        configFile >> config;
    } catch (const json::parse_error& e) {
        std::cerr << "Gemini LLM Error parsing config file: " << e.what() << std::endl;
        return "";
    }
    if (!config.contains("gemini_api_key") || !config["gemini_api_key"].is_string()) {
        std::cerr << "Gemini LLM Error: 'gemini_api_key' not found or not a string in config file!" << std::endl;
        return "";
    }
    return config["gemini_api_key"].get<std::string>();
}

std::string ask_gemini_tax_llm_internal(const std::string& user_question, const std::string& model_name) {
    std::string api_key = loadGeminiApiKey();
    if (api_key.empty()) {
        return "Error: Gemini API key not loaded. Please check config.json.";
    }

    std::string api_url = "https://generativelanguage.googleapis.com/v1beta/models/" + model_name + ":generateContent?key=" + api_key;

    CURL* curl;
    CURLcode res;
    std::string response_string;
    long http_code = 0;

    std::string system_prompt_text =
        "You are a specialized assistant. Your ONLY function is to answer questions and provide information strictly related to the Indian tax system. "
        "You must not answer questions or engage in discussions about any other topic, including but not limited to: current events, history, science, art, personal advice (not tax-related), creative writing, jokes, general knowledge, or taxes of other countries. "
        "If a user's query is not about Indian taxes, you MUST politely decline by saying something similar to: 'I'm sorry, but I can only provide information and answer questions about the Indian tax system. Do you have a Indian tax-related question?' "
        "Do not apologize excessively or try to steer the conversation back if the query is clearly off-topic. Simply state your limitation. "
        "Examples of off-topic queries you must decline: 'What's the capital of France?', 'Tell me a fun fact.', 'How do I bake a cake?'. "
        "Examples of on-topic queries you should answer: 'What are the income tax brackets for 2023 in India?', 'Can I deduct student loan interest in India?'. "
        "Always provide a disclaimer that your information is not professional tax advice and users should consult a qualified professional for specific situations.";

    json payload;

    payload["contents"] = json::array({
        {
            {"role", "user"},
            {"parts", json::array({
                {{"text", system_prompt_text}}
            })}
        },
        {
            {"role", "user"}, 
            {"parts", json::array({
                {{"text", user_question}}
            })}
        }
    });

    json safetySettings_array = json::array();
    std::vector<std::string> harm_categories = {
        "HARM_CATEGORY_HARASSMENT",
        "HARM_CATEGORY_HATE_SPEECH",
        "HARM_CATEGORY_SEXUALLY_EXPLICIT",
        "HARM_CATEGORY_DANGEROUS_CONTENT"
    };
    for (const auto& category : harm_categories) {
        json setting;
        setting["category"] = category;
        setting["threshold"] = "BLOCK_MEDIUM_AND_ABOVE";
        safetySettings_array.push_back(setting);
    }
    payload["safetySettings"] = safetySettings_array;

    std::string request_body = payload.dump();

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Gemini LLM Error: curl_easy_init() failed" << std::endl;
        curl_global_cleanup(); 
        return "Error initializing libcurl for Gemini";
    }

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request_body.length());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, GeminiLLM::WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem"); 

    res = curl_easy_perform(curl);

    std::string result_message;

    if (res != CURLE_OK) {
        std::cerr << "Gemini LLM curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        result_message = "Error calling Gemini API: " + std::string(curl_easy_strerror(res));
    } else {
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        if (http_code != 200) {
            std::cerr << "Gemini LLM API Error: HTTP " << http_code << " Response: " << response_string << std::endl;
            result_message = "Error from Gemini API (HTTP " + std::to_string(http_code) + "). Check console for details.";
            try {
                json error_json = json::parse(response_string);
                if (error_json.contains("error") && error_json["error"].contains("message")) {
                    result_message += " Message: " + error_json["error"]["message"].get<std::string>();
                }
            } catch (const json::parse_error&) {}
        } else {
            try {
                json json_response = json::parse(response_string);
                if (json_response.contains("candidates") && json_response["candidates"].is_array() && !json_response["candidates"].empty()) {
                    const auto& first_candidate = json_response["candidates"][0];
                    if (first_candidate.contains("content") && first_candidate["content"].contains("parts") &&
                        first_candidate["content"]["parts"].is_array() && !first_candidate["content"]["parts"].empty()) {
                        const auto& first_part = first_candidate["content"]["parts"][0];
                        if (first_part.contains("text") && first_part["text"].is_string()) {
                            result_message = first_part["text"].get<std::string>();
                        } else {
                             result_message = "Gemini LLM: Response part does not contain text.";
                        }
                    } else if (first_candidate.contains("finishReason") && first_candidate["finishReason"] != "STOP") {
                        std::string reason = first_candidate["finishReason"].get<std::string>();
                        std::string safety_msg = "";
                        if (first_candidate.contains("safetyRatings") && first_candidate["safetyRatings"].is_array()) {
                             for (const auto& rating : first_candidate["safetyRatings"]) {
                                 if (rating.contains("blocked") && rating["blocked"].is_boolean() && rating["blocked"].get<bool>()) {
                                     safety_msg += " Blocked due to: " + rating["category"].get<std::string>();
                                 }
                             }
                        }
                        result_message = "Gemini LLM: Content generation stopped. Reason: " + reason + "." + safety_msg;
                    } else {
                        result_message = "Gemini LLM: Response structure not as expected (no content/parts).";
                    }
                } else if (json_response.contains("promptFeedback") && json_response["promptFeedback"].contains("blockReason")) {
                     std::string reason = json_response["promptFeedback"]["blockReason"].get<std::string>();
                     std::string safety_msg = "";
                     if (json_response["promptFeedback"].contains("safetyRatings") && json_response["promptFeedback"]["safetyRatings"].is_array()) {
                         for (const auto& rating : json_response["promptFeedback"]["safetyRatings"]) {
                             if (rating.contains("blocked") && rating["blocked"].is_boolean() && rating["blocked"].get<bool>()) {
                                 safety_msg += " Blocked due to: " + rating["category"].get<std::string>();
                             }
                         }
                     }
                    result_message = "Gemini LLM Error: Prompt blocked. Reason: " + reason + "." + safety_msg;
                }
                else {
                    std::cerr << "Gemini LLM Error: Could not extract text. Response: " << response_string << std::endl;
                    result_message = "Error: Could not parse Gemini response structure. Raw: " + response_string;
                }
            } catch (const json::parse_error& e) {
                std::cerr << "Gemini LLM Error parsing JSON response: " << e.what() << std::endl;
                std::cerr << "Gemini LLM Raw Response: " << response_string << std::endl;
                result_message = "Error parsing Gemini JSON response.";
            }
        }
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();

    return result_message;
}
} 

std::string ask_gemini_tax_llm(const std::string& user_question, const std::string& model_name) {
    std::string llm_response = GeminiLLM::ask_gemini_tax_llm_internal(user_question, model_name);
    return llm_response;
}
