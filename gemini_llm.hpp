#ifndef GEMINI_LLM_HPP
#define GEMINI_LLM_HPP

#include <string>
#include <vector> 
#include <iostream> 
#include <fstream>  
#include <curl/curl.h> 
#include "json.hpp" 

std::string ask_gemini_tax_llm(const std::string& user_question, const std::string& model_name = "gemini-1.5-flash-latest");

#endif 
