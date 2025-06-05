# Tax-Assistant

A C++ Income Tax Calculator and PAN Verification tool with AI Assistant integration.

## Overview

Tax-Assistant is a C++ project for simulating an Income Tax Calculator system featuring:

- **PAN Format Verification** (checks if a PAN ID matches the required format)
- **Income Tax Calculation** (based on user/company input and US tax rules via API Ninja)
- **AI Assistant Integration** (powered by Gemini LLM for intelligent assistance; answers only Indian tax-related queries)
- **API Integration** (uses API Ninja for tax calculation, Gemini via Google Cloud Platform for AI)
- **Secure HTTP requests** (via cURL and bundled `cacert.pem`)

## Features

- Add companies and employees with details interactively
- Check validity of PAN IDs by format (not by API)
- Calculate income tax for each employee, based on state and filing status
- Get AI-powered answers and guidance using Gemini LLM via Google Cloud Platform (answers only Indian tax-related queries)
- Communicate securely with APIs using cURL

## Requirements

- C++17 or newer compiler (e.g., g++)
- API keys from:
  - [API Ninja](https://api-ninjas.com/) (For Income Tax Calculator)
  - [Google Cloud Platform](https://console.cloud.google.com/) (for Gemini LLM)
- Both of these APIs are free to use.
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
     - [API Ninja](https://api-ninjas.com/) (**Be sure to get the key for the "Income Tax Calculator" API**)
     - [Google Cloud Platform](https://console.cloud.google.com/) (for Gemini LLM)
   - Both API keys are free to obtain.
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

## Configuration

- Place your API keys in the `config.json` file in the project directory.
- No extra setup needed for cURL or `cacert.pem`—both are included.

## Input Flow and Prompts

**The program is fully interactive** and will guide you step-by-step, prompting you for each required input.  
The helper lines and prompts are printed directly to the terminal, indicating exactly what information you need to enter at each step.

Here is how you will interact with the program:

1. **Company Details**  
   You will be prompted for the following information for each company:
   - `Number of Companies :` — Enter the total number of companies you want to add.
   - For each company:
     - `Company Name :` — Enter the company's name.
     - `Founder Name :` — Enter the name of the company's founder.
     - `Date of Starting :` — Enter the starting date (format as you wish, e.g., YYYY-MM-DD).
     - `Phone Number :` — Enter the company's phone number.

2. **Employee Details**  
   After companies, you will be prompted for employee details:
   - `Number of People :` — Enter the total number of employees.
   - For each employee:
     - `Name :` — Enter the employee's first name.
     - `Surname :` — Enter the employee's surname.
     - `Company Name :` — Enter the name of the company the employee works for (must match a company entered above).
     - `Date of Birth :` — Enter the employee's date of birth.
     - `Phone Number :` — Enter the employee's phone number.
     - `State :` — Enter the US state abbreviation (e.g., CA, NY, TX).
     - `Filing Status :` — Enter the tax filing status (e.g., single, married).
     - `Income :` — Enter five (5) income values separated by spaces (e.g., 80000 5000 2000 0 0).

3. **Tax Filing and AI Assistant**  
   - The program will process the tax calculations for each employee and display results.
   - At the end, you can interactively ask questions to the Gemini AI Assistant:
     - You will see: `Ask the Gemini tax LLM a question (type 'exit' to quit):`
     - Type your question (Indian tax-related queries only) or type `exit` to finish.

**You do not need to remember the order of fields**; simply follow the prompts as displayed.

## Example Input/Session

Below is a sample session to help you understand the interactive flow.

### Input

```plaintext
Number of Companies : 2
Company Name : AcmeCorp
Founder Name : Alice
Date of Starting : 2000-01-01
Phone Number : 1234567890

Company Name : BetaTech
Founder Name : Bob
Date of Starting : 2010-05-12
Phone Number : 9876543210

Number of People : 2
Name : John
Surname : Doe
Company Name : AcmeCorp
Date of Birth : 1985-07-23
Phone Number : 1112223333
State : CA
Filing Status : single
Income : 80000 5000 2000 0 0

Name : Jane
Surname : Smith
Company Name : BetaTech
Date of Birth : 1990-02-15
Phone Number : 4445556666
State : NY
Filing Status : married
Income : 90000 6000 3000 0 0
```

### Output

```plaintext
Starting Tax Filing for company AcmeCorp...

Processing: John

API Ninjas response: {"federal_taxes_owed":12000,"fica_total":6120}
TDS deducted for John. Tax amount: 18120

Finished Tax Filing.

Starting Tax Filing for company BetaTech...

Processing: Jane

API Ninjas response: {"federal_taxes_owed":14000,"fica_total":6890}
TDS deducted for Jane. Tax amount: 20890

Finished Tax Filing.

Total treasury amount: 39010
```

### Asking the AI Assistant

After the tax filing output, you can interact with the AI assistant as follows:

```plaintext
Ask the Gemini tax LLM a question (type 'exit' to quit): What are the new slabs for Indian income tax in 2024?
Gemini LLM says:
For FY 2023-24, the Indian income tax slabs are as follows...
```

> **Note:** The Gemini LLM will answer only Indian tax-related queries.

```plaintext
Ask the Gemini tax LLM a question (type 'exit' to quit): exit
```

**Note:**  
- All input is prompted interactively; you only need to respond as indicated.
- You can enter data in any reasonable format as suggested by each prompt.
- If you make a mistake, you can restart the program and re-enter the data.

---
