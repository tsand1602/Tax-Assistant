# Tax-Assistant

A C++ Income Tax Calculator and PAN Verification tool with AI Assistant integration.

## Overview

Tax-Assistant is a C++ project for simulating an Income Tax Calculator system featuring:

- **PAN Format Verification** (checks if a PAN ID matches the required format)
- **Income Tax Calculation** (based on user input)
- **AI Assistant Integration** (powered by Gemini LLM for intelligent assistance)
- **API Integration** (uses API Ninja for tax calculation, Gemini via Google Cloud Platform for AI)
- **Secure HTTP requests** (via cURL and bundled `cacert.pem`)

## Features

- Check validity of PAN IDs by format (not by API)
- Calculate income tax based on provided details
- Get AI-powered answers and guidance using Gemini LLM via Google Cloud Platform
- Communicate securely with APIs using cURL

## Requirements

- C++17 or newer compiler (e.g., g++)
- API keys from:
  - [API Ninja](https://api-ninjas.com/) (For Income Tax Calculator)
  - [Google Cloud Platform](https://console.cloud.google.com/) (for Gemini LLM)
- Bundled cURL libraries and `cacert.pem` (already included in this repo)
- Command prompt or any terminal

## Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/tsand1602/Tax-Assistant.git
   cd Tax-Assistant
   ```

2. **API Keys**
   - Sign up and get your API keys from:
     - [API Ninja](https://api-ninjas.com/)
     - [Google Cloud Platform](https://console.cloud.google.com/) (for Gemini LLM)
   - Add your API keys in `config.json` located in the project directory.  
     Refer to the code comments for the expected structure of `config.json`.

## Compilation

Open a terminal in the project directory and run:

```bash
g++ tax_assistant.cpp gemini_llm.cpp -o tax_assistant.exe -std=c++17 -I. -lcurl
```

- The `-I.` flag includes the current directory for headers.
- The `-lcurl` flag links the cURL library (already included in the repo).
- For Linux/Mac, change the output file name to remove `.exe` if desired.

## Usage

After compiling, run the executable:

```bash
tax_assistant.exe
```
or (on Linux/Mac):
```bash
./tax_assistant
```

Follow the prompts to:
- Enter and verify PAN format
- Input income details
- Interact with the AI assistant

## Configuration

- Place your API keys in the `config.json` file in the project directory.
- No extra setup needed for cURL or `cacert.pem`—both are included.

---
