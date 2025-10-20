// Simple test program to exercise appendToCSV concurrently
#include <iostream>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <fstream>

// forward declaration of appendToCSV from utils
void appendToCSV(const std::map<std::string, std::string> &fields,
                 const std::string &csvPath,
                 const std::string &filename);

int main()
{
    const std::string csv = "./tests/contacts_test.csv";
    // remove existing test csv
    unlink(csv.c_str());

    const int N = 10;
    std::vector<pid_t> kids;
    for (int i = 0; i < N; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            // child
            std::map<std::string, std::string> fields;
            std::ostringstream ss;
            ss << i;
            fields["FirstName"] = std::string("Child") + ss.str();
            fields["Name"] = "Tester";
            std::ostringstream se;
            se << i;
            fields["email"] = std::string("child") + se.str() + "@example.test";
            std::ostringstream sf;
            sf << i;
            appendToCSV(fields, csv, std::string("child_file_") + sf.str() + ".txt");
            _exit(0);
        } else if (pid > 0) {
            kids.push_back(pid);
        } else {
            std::cerr << "fork failed\n";
            return 1;
        }
    }

    for (size_t i = 0; i < kids.size(); ++i) {
        int status = 0;
        waitpid(kids[i], &status, 0);
    }

    // print resulting file for manual inspection
    std::cout << "--- Resulting CSV ---\n";
    std::ifstream in(csv.c_str());
    if (!in.is_open()) {
        std::cerr << "test: cannot open resulting csv\n";
        return 1;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::cout << line << "\n";
    }
    in.close();
    return 0;
}
