/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BitcoinExchange.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/07 19:31:09 by rhanitra          #+#    #+#             */
/*   Updated: 2025/08/04 16:30:24 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BITCOINEXCHANGE_HPP
#define BITCOINEXCHANGE_HPP

#include <iostream>
#include <exception>
# include <string>
#include <sstream>
# include <fstream>
#include <limits>
#include <cmath>
#include <iomanip>
#include <list>

class BitcoinExchange
{
    private:
    
        std::list<std::string> _fileContent;
        std::list<std::string> _dataBase;

        static float fromFloat(const std::string& literal);
        static bool isLeapYear(int year);
        static std::string removeSpaces(const std::string& str);
        static void findDuplicates(std::list<std::string>& dataBase);
       
    public:
       
        BitcoinExchange();
        BitcoinExchange(const BitcoinExchange& other);
        BitcoinExchange& operator=(const BitcoinExchange& other);
        ~BitcoinExchange();

        const std::list<std::string>& getDataBase() const;
        const std::list<std::string>& getFileContent() const;

        const std::string formatNumber(float);
        bool isValidDate(int day, int month, int year);
        const std::string& myRegexReplace(std::string& str, const std::string& reg, char c);
        std::list<float> ftSplitToFloat(const std::string& str, char delimiter);
        
        void putFileContent(const std::string& fileName);
        void putDataBase(const std::string& fileName);

              
};
    
template <typename T>
T& getListElement(std::list<T>& lst, size_t index)
{
    if (index >= lst.size())
        throw std::out_of_range("bad input => null");

    typename std::list<T>::iterator it = lst.begin();
    std::advance(it, index);
    return (*it);
}

template <typename T>
const T& getListElement(const std::list<T>& lst, size_t index)
{
    if (index >= lst.size())
        throw std::out_of_range("bad input => null");
        
    typename std::list<T>::const_iterator it = lst.begin();
    std::advance(it, index);
    return (*it);
}

void findValue(const std::string& dbName, char *inputFileName);

#endif