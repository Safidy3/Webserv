#include <iostream>
#include <fstream>
#include "../include/handleErrors.hpp"

int main()
{
    // Préparer un ServerConfig factice avec errorPages
    ServerConfig sc;
    sc.errorPages[404] = "./tests/404_test.html";

    // Créer le fichier personnalisé
    std::ofstream ofs("./tests/404_test.html");
    ofs << "<html><body><h1>Custom 404</h1></body></html>";
    ofs.close();

    std::string resp = HandleErrors::generateErrorResponse(404, sc, NULL);
    if (resp.find("Custom 404") == std::string::npos) {
        std::cerr << "Test failed: custom 404 body not found" << std::endl;
        return 1;
    }
    std::cout << "Test passed: custom 404 included in response" << std::endl;
    return 0;
}
