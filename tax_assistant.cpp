#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <cstdlib>
#include <ctime>
#include <curl/curl.h>  // Include libcurl
#include <nlohmann/json.hpp> // JSON library
#include <fstream> // For file reading/writing

using json = nlohmann::json; //Define json for the library

using namespace std;

// --- libcurl write callback function ---
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t total_size = size * nmemb;
    output->append((char*)contents, total_size);
    return total_size;
}

// Helper function to escape special characters in a string for JSON
string escapeJsonString(const string& input) {
    string output = "";
    for (char c : input) {
        if (c == '\\') {
            output += "\\\\";
        } else if (c == '"') {
            output += "\\\"";
        } else if (c == '\b') {
            output += "\\b";
        } else if (c == '\f') {
            output += "\\f";
        } else if (c == '\n') {
            output += "\\n";
        } else if (c == '\r') {
            output += "\\r";
        } else if (c == '\t') {
            output += "\\t";
        } else {
            output += c;
        }
    }
    return output;
}

class Pan {
    string Pan_number; // Pan ID
    string name; // Name in the Pan Card
    string father_name; // Father's Name in the Pan card
    string DOB; // Date of Birth mentioned
    string phone_number; // Phone number mentioned
public:
    // Constructor for PAN
    Pan(string Pan_number, string name, string father_name, string DOB, string phone_number) : Pan_number(Pan_number), name(name), father_name(father_name), DOB(DOB), phone_number(phone_number) {}
    // Getter functions
    string getPanNumber() const { return Pan_number; }
    string getname() const { return name; }
    string getfatherName() const { return father_name; }
    string getDOB() const { return DOB; }
    char getEntity() const { return Pan_number[3]; } // Entity is a person or company/organisation
    string getphoneNumber() const { return phone_number; }

    /*
    Setter functions are implemented when user wants to change his/her PAN card details

    void setname(string newName) { name = newName; }
    void setfatherName(string newFatherName) { father_name = newFatherName; }
    void setDOB(string newDOB) { DOB = newDOB; }

    */
};

class company;

class person {
    vector<long long> income; // several categories of income earned by the person, if no income in a particular category, then 0
    // Salaried income is type 0
    string phone_number; // Phone number of individual
    Pan individual_pan; // Pan Card
    company* employer; // Company in which he/she works
    bool is_Certificate_issued; // Checks if TDS Certificate is already issued
public:
    // Constructor for person
    person(vector<long long> income, Pan issuedPan, company* employer, string phone_number) : income(income), individual_pan(issuedPan), employer(employer), phone_number(phone_number), is_Certificate_issued(false) {}
    company* getcompany() const { return employer; } // Returns company
    Pan getPan() const { return individual_pan; } // Returns pan card of the person
    string getPhoneNumber() const { return phone_number; } // Returns phone number of the person
    long long getIncome(int category) const { return income[category]; } // Gets the income of the person in a particular category
    vector<long long> getIncomeVector() const { return income; } //returns the full vector
    void get_certificate() { is_Certificate_issued = true; } // Person gets TDS Certificate after Company deducts TDS
    bool certificate_already_issued() const { return is_Certificate_issued; } // Checks if TDS Certificate is already issued
};

class company {
    Pan company_pan; // Pan Card of the company
public:
    vector<person> employees; // Employees in the company
    // Constructor for company
    company(Pan company_pan) : company_pan(company_pan) {}

    void addEmployee(person employee) {
        employees.push_back(employee);
    }
    /*
    Company gets PAN Details from employees through third party sources..
    They need to check if the PAN details are genuine in order to deduct TDS carefully
    They take the phone number claimed by the employee and check if it is the same in the PAN card
    They also check the 4th digit to prevent malpractice
    */
    bool check_PanDetails(const Pan& user, string phone_number, char actual_entity) const {
        if (user.getEntity() == actual_entity && user.getphoneNumber() == phone_number)
            return true;
        return false;
    }
    // Company issues certificate to employee after TDS id deducted
    void issue_certificate(person& employee) {
        employee.get_certificate();
    }
};

class government {
protected:
    unordered_map<string, Pan> earning_people; // string datatype is used for PAN ID
    long long treasury;
public:
    government() : treasury(0) {
      srand(time(0)); 
    }
    // Creates a new PAN card when an user applies for it
    Pan add_pan(char entity, string name, string surname, string father_name, string DOB, string phone_number) {
        string pan_id;
        do  {
            pan_id.clear();
            for (int i = 0; i < 3; i++) {
                pan_id.push_back('A' + rand() % 26); // Generating first three digits in pan id
            }
            pan_id.push_back(entity); // setting 4th digit as entity
            if (surname.length() != 0)
                pan_id.push_back(surname[0]); // 5th digit as first letter of surname
            else
                pan_id.push_back(name[0]); // 5th digit as first letter of name
            for (int i = 0; i < 5; i++) {
                pan_id.push_back('0' + rand() % 10); // setting last 3 digits in pan id
            }
        } while (earning_people.find(pan_id) != earning_people.end());
        Pan newPan(pan_id, name + " " + surname, father_name, DOB, phone_number);
        earning_people.insert({ pan_id,newPan });
        return newPan;
    }
    // Government accepts TDS from the company
    void accept_TDS(long long TDS) {
        treasury += TDS;
    }
    // Returns amount in treasury
    long long getTreasury() const {
        return treasury;
    }
};

// Assistant is implemented with AI
class Assistant {
public:
    // Load configuration from a file
    bool loadConfig(const string& filename, json& config) {
        ifstream configFile(filename);
        if (!configFile.is_open()) {
            cerr << "Error: Could not open config file: " << filename << endl;
            return false;
        }

        try {
            configFile >> config;
        } catch (const json::parse_error& e) {
            cerr << "Error parsing config file: " << e.what() << endl;
            return false;
        }
        return true;
    }

    string callClearTaxAPI(const string& endpoint, const string& json_payload) {

        json config;
        if (!loadConfig("config.json", config)) {
            return "Error: Could not load config file.";
        }
        const string api_key = config["cleartax_api_key"];

        if (api_key.empty()) {
            cerr << "Error: CLEARTAX_API_KEY not set in config.json!" << endl;
            return "Error: API key not set.";
        }

        string api_url = endpoint;

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
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_payload.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, json_payload.length());

        // Set a callback function to receive the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);

        // Set headers to indicate JSON content
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        headers = curl_slist_append(headers, ("Authorization: Bearer " + api_key).c_str()); //Use config variable
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        res = curl_easy_perform(curl);

        // Always cleanup
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers); // Free the header list
        if (res != CURLE_OK) {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return "Error calling ClearTax API";
        }
        else {
            // Basic error handling
            if (response_string.find("error") != string::npos) {
                cerr << "ClearTax API returned an error: " << response_string << endl;
                return "ClearTax API returned an error";
            }
            return response_string;
        }
        curl_global_cleanup();
    }

    // New: Dedicated function to calculate income tax using the ClearTax API
    long long calculate_income_tax(const person& employee) {
        // 1. Prepare the data to send to the API (e.g., income, deductions, etc.)
        vector<long long> incomes = employee.getIncomeVector();
        long long total_income = 0;
        for (long long income: incomes) {
          total_income += income;
        }

        //In real implementation you would take vector of deductions as well

        // 2. Construct the JSON payload for the ClearTax API
        json payload = {
            {"income", total_income}
            // Add other required parameters based on the ClearTax API documentation
        };

        string json_payload_string = payload.dump(); // needs the #include <nlohmann/json.hpp>

        // 3. Call the ClearTax API (replace with the correct endpoint)
        json config;
        if (!loadConfig("config.json", config)) {
            return 0; // or throw exception
        }
        string endpoint = config["cleartax_tds_endpoint"]; // config file

        string tax_response = callClearTaxAPI(endpoint, json_payload_string);

        // 4. Parse the tax amount from the response (using nlohmann/json)
        long long tax_amount = 0;
        try {
            json response_json = json::parse(tax_response);

            // Extract tax amount from the JSON response.  Adjust the key based on the API documentation.
            tax_amount = response_json["tds_amount"].get<long long>(); // adjust "tds_amount"

        } catch (const json::parse_error& e) {
            cerr << "Error parsing ClearTax API response: " << e.what() << endl;
            return 0;
        } catch (const std::exception& e) {
            cerr << "Error extracting tax amount from ClearTax API response: " << e.what() << endl;
            return 0;
        }

        // 5. Return the calculated tax amount
        return tax_amount;
    }

    void Filing_Tax(government& Government, company& employer) {
        for (auto& employee : employer.employees) {
            // Check details
            if (employee.certificate_already_issued()) {
                cout << "TDS already Deducted from employee " << employee.getPan().getname() << endl;
                continue;
            }

            if (!employer.check_PanDetails(employee.getPan(), employee.getPhoneNumber(), 'P')) {
                cout << "PAN details don't match for employee " << employee.getPan().getname() << endl;
                continue; // inform the user if details don't match
            }
            // Calculate tax
            long long tax = calculate_income_tax(employee);  //Use new tax calculating method
            // Transfer TDS to government
            Government.accept_TDS(tax);
            // Generate certificate and send to user
            employer.issue_certificate(employee);

            cout << "TDS deducted for " << employee.getPan().getname() << ". Tax amount: " << tax << endl;
        }
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

    person person1(income1, person1Pan, &MyCompany, "1234567890");
    person person2(income2, person2Pan, &MyCompany, "9087654321");

    MyCompany.addEmployee(person1);
    MyCompany.addEmployee(person2);

    tax_assistant.Filing_Tax(Government, MyCompany);

    cout << "Total treasury amount: " << Government.getTreasury() << endl;

    return 0;
}
