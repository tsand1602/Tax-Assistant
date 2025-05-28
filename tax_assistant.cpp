#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <curl/curl.h>
#include <json.hpp>
#include <fstream>
#include "gemini_llm.hpp"

using json = nlohmann::json;
using namespace std;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

class Pan {
    string Pan_number;
    string name;
    string father_name;
    string DOB;
    string phone_number;
public:
    Pan(string Pan_number, string name, string father_name, string DOB, string phone_number)
        : Pan_number(Pan_number), name(name), father_name(father_name), DOB(DOB), phone_number(phone_number) {}
    string getPanNumber() const { return Pan_number; }
    string getname() const { return name; }
    string getfatherName() const { return father_name; }
    string getDOB() const { return DOB; }
    char getEntity() const { return Pan_number[3]; }
    string getphoneNumber() const { return phone_number; }
};

class company;

class person {
    vector<long long> income;
    string phone_number;
    Pan individual_pan;
    company* employer;
    bool is_Certificate_issued;
    string us_state;
    string filing_status;
public:
    person(vector<long long> income, Pan issuedPan, company* employer, string phone_number, const string& state, const string& filing_status)
        : income(income), individual_pan(issuedPan), employer(employer), phone_number(phone_number), is_Certificate_issued(false), us_state(state), filing_status(filing_status) {}
    company* getcompany() const { return employer; }
    Pan getPan() const { return individual_pan; }
    string getPhoneNumber() const { return phone_number; }
    long long getIncome(int category) const {
        if (category >= 0 && category < income.size())
            return income[category];
        return 0;
    }
    vector<long long> getIncomeVector() const { return income; }
    void get_certificate() { is_Certificate_issued = true; }
    bool certificate_already_issued() const { return is_Certificate_issued; }
    string getState() const { return us_state; }
    string getFilingStatus() const { return filing_status; }
};

class company {
    Pan company_pan;
public:
    vector<person> employees;
    company(Pan company_pan) : company_pan(company_pan) {}

    void addEmployee(const person& employee) {
        employees.push_back(employee);
    }
    bool check_PanDetails(const Pan& user, string phone_number, char actual_entity) const {
        return user.getEntity() == actual_entity && user.getphoneNumber() == phone_number;
    }
    void issue_certificate(person& employee) {
        employee.get_certificate();
    }
};

class government {
protected:
    unordered_map<string, Pan> earning_people;
    long long treasury;
public:
    government() : treasury(0) { srand(time(0)); }
    Pan add_pan(char entity, string name, string surname, string father_name, string DOB, string phone_number) {
        string pan_id;
        do  {
            pan_id.clear();
            for (int i = 0; i < 3; i++) {
                pan_id.push_back('A' + rand() % 26);
            }
            pan_id.push_back(entity);
            if (surname.length() != 0)
                pan_id.push_back(surname[0]);
            else
                pan_id.push_back(name[0]);
            for (int i = 0; i < 5; i++) {
                pan_id.push_back('0' + rand() % 10);
            }
        } while (earning_people.find(pan_id) != earning_people.end());
        Pan newPan(pan_id, name + " " + surname, father_name, DOB, phone_number);
        earning_people.insert({ pan_id,newPan });
        return newPan;
    }
    void accept_TDS(long long TDS) {
        treasury += TDS;
    }
    long long getTreasury() const {
        return treasury;
    }
};

class Assistant {
public:
    string loadApiNinjasKey(const string& config_filename = "config.json") {
        std::ifstream configFile(config_filename);
        if (!configFile.is_open()) {
            cerr << "Error: Could not open config file: " << config_filename << endl;
            return "";
        }
        json config;
        try {
            configFile >> config;
        } catch (const json::parse_error& e) {
            cerr << "Error parsing config file: " << e.what() << endl;
            return "";
        }
        if (!config.contains("api_ninjas_key") || !config["api_ninjas_key"].is_string()) {
            cerr << "Error: 'api_ninjas_key' not found in config file!" << endl;
            return "";
        }
        return config["api_ninjas_key"];
    }

    string callApiNinjas(const string& endpoint, const string& query_params) {
        string api_key = loadApiNinjasKey();
        if (api_key.empty()) {
            cerr << "API key not loaded. Aborting API call." << endl;
            return "Error: API key not loaded";
        }
        string api_url = endpoint + "?" + query_params;

        CURL* curl;
        CURLcode res;
        string response_string;

        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl = curl_easy_init();
        if (!curl) {
            cerr << "Error: curl_easy_init() failed" << endl;
            curl_global_cleanup();
            return "Error initializing libcurl";
        }

        curl_easy_setopt(curl, CURLOPT_URL, api_url.c_str());
        curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, ("X-Api-Key: " + api_key).c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
        curl_global_cleanup();

        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return "Error calling API Ninjas";
        } else {
            return response_string;
        }
    }

    long long calculate_income_tax(const person& employee) {
        vector<long long> incomes = employee.getIncomeVector();
        long long total_income = 0;
        for (long long income : incomes) {
            total_income += income;
        }
        string state = employee.getState();
        string filing_status = employee.getFilingStatus();

        stringstream query_params;
        query_params << "country=US"
                     << "&region=" << state
                     << "&income=" << total_income
                     << "&filing_status=" << filing_status;

        string endpoint = "https://api.api-ninjas.com/v1/incometaxcalculator";
        string tax_response = callApiNinjas(endpoint, query_params.str());

        cout << "API Ninjas response: " << tax_response << endl;

        long long tax_amount = 0;
        try {
            json response_json = json::parse(tax_response);
            
            if (response_json.contains("federal_taxes_owed") && response_json["federal_taxes_owed"].is_number()) {
                tax_amount += static_cast<long long>(response_json["federal_taxes_owed"].get<double>());
            }
            if (response_json.contains("fica_total") && response_json["fica_total"].is_number()) {
                tax_amount += static_cast<long long>(response_json["fica_total"].get<double>());
            }
            if (response_json.contains("region_taxes_owed") && response_json["region_taxes_owed"].is_string() &&
                response_json["region_taxes_owed"] == "premium subscription required") {
                cout << "(State/regional tax calculation requires a premium API Ninjas subscription.)" << endl;
            }
            if (response_json.contains("total_taxes_owed") && response_json["total_taxes_owed"].is_string() &&
                response_json["total_taxes_owed"] == "premium subscription required") {
                cout << "(Total tax calculation requires a premium API Ninjas subscription.)" << endl;
            }
        } catch (const json::parse_error& e) {
            cerr << "Error parsing API Ninjas response: " << e.what() << endl;
            return 0;
        } catch (const std::exception& e) {
            cerr << "Error extracting tax amount from API Ninjas response: " << e.what() << endl;
            return 0;
        }
        return tax_amount;
    }

    void Filing_Tax(government& Government, company& employer) {
        cout << "Starting Tax Filing..." << endl;
        for (auto& employee : employer.employees) {
            cout << "Processing: " << employee.getPan().getname() << endl;
            if (employee.certificate_already_issued()) {
                cout << "TDS already Deducted from employee " << employee.getPan().getname() << endl;
                continue;
            }

            if (!employer.check_PanDetails(employee.getPan(), employee.getPhoneNumber(), 'P')) {
                cout << "PAN details don't match for employee " << employee.getPan().getname() << endl;
                continue;
            }
            long long tax = calculate_income_tax(employee);
            Government.accept_TDS(tax);
            cout << "TDS deducted for " << employee.getPan().getname() << ". Tax amount: " << tax << endl;
        }
        cout << "Finished Tax Filing." << endl;
    }
};

int main() {
    government Government;
    Assistant tax_assistant;

    Pan companyPan("ABCDE1234F", "MyCompany", "FounderName", "01/01/1990", "9876543210");
    company MyCompany(companyPan);

    Pan person1Pan = Government.add_pan('P', "John", "Doe", "FatherDoe", "02/02/1980", "1234567890");
    Pan person2Pan = Government.add_pan('P', "Jane", "Smith", "FatherSmith", "03/03/1995", "9087654321");

    vector<long long> income1 = { 100000, 0, 0, 0, 0 };
    vector<long long> income2 = { 200000, 0, 0, 0, 0 };

    person person1(income1, person1Pan, &MyCompany, "1234567890", "CA", "single");
    person person2(income2, person2Pan, &MyCompany, "9087654321", "NY", "head_of_household");

    MyCompany.addEmployee(person1);
    MyCompany.addEmployee(person2);

    tax_assistant.Filing_Tax(Government, MyCompany);

    cout << "Total treasury amount: " << Government.getTreasury() << endl;

    string user_question_gemini; 
    while (true) {
        cout << "\nAsk the Gemini tax LLM a question (type 'exit' to quit): ";
        getline(std::cin, user_question_gemini);
    
        if (std::cin.eof()) { 
            cout << "\nEOF detected. Exiting Gemini chat." << std::endl;
            break;
        }
        if (user_question_gemini.empty()) 
            continue;
        if (user_question_gemini == "exit" || user_question_gemini == "quit" || user_question_gemini == "q") 
            break;
    
        string llm_answer = ask_gemini_tax_llm(user_question_gemini); 
    
        cout << "\nGemini LLM says:\n" << llm_answer << std::endl;
    }
    return 0;
}
