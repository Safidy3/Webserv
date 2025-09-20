
#include "../include/utils.hpp"

std::vector<std::string> BitcoinExchange::ftSplitToFloat(const std::string& str, char delimiter)
{
    std::deque<float> output;
    std::istringstream iss(str);
    std::string token;
    
    while (std::getline(iss, token, delimiter))
    {
        if (!token.empty())
        {
            std::istringstream iss_token(token);
            float n;
            char extra;
            
            if (!(iss_token >> n) || (iss_token >> extra))
                throw std::runtime_error("Token invalide: '" + token + "'");
            
            output.push_back(fromFloat(token));
        }
    }

    return (output);
}