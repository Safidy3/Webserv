/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pars_utils.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/12 15:38:26 by safandri          #+#    #+#             */
/*   Updated: 2025/08/12 16:21:55 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

bool isSpace(char c)
{
	return (c == ' ' || c == '\n' || c == '\t');
}

bool isAllSpace(const std::string& str)
{
	for (std::string::const_iterator it = str.begin(); it != str.end(); it++)
		if (!isSpace(*it))
			return (false);
	return true;
}

void	insertMap(std::string& line, std::map<std::string, std::string>& dict)
{
	std::string	key;
	std::string	value;

	size_t	key_end = line.find(':');
	if (key_end == std::string::npos)
		value = "";
	else
		value = line.substr(key_end + 1, line.size());
	key = line.substr(0, key_end);
	std::cout << key << " -> " << value << std::endl;
	dict.insert(std::make_pair(key, value));
}

void	parsString(std::string& str, std::map<std::string, std::string>& dict)
{
	std::string	line;
	size_t		line_end;

	line_end = str.find('\n');
	while (line_end != std::string::npos)
	{
		line = str.substr(0, line_end);
		str = str.substr(line_end + 1, str.size());
		line_end = str.find('\n');
		if (isAllSpace(line))
			continue;
		if (line_end == std::string::npos)
		{
			insertMap(str, dict);
			break;
		}
		insertMap(line, dict);
	}
	std::cout << "*Dict size = " << dict.size() << std::endl;
}

void	print_map(std::map<std::string, std::string>& dict)
{
	for (std::map<std::string, std::string>::iterator it = dict.begin(); it != dict.end(); it++)
		std::cout << it->first << " ; " << it->second << std::endl;
	std::cout << std::endl;
}
