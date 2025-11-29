/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftToInt.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 13:42:35 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/08 19:39:50 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

int ftToInt(const std::string &s)
{
    std::stringstream ss(s);
    int val;
    if (!(ss >> val))
        throw std::runtime_error("Invalid number: " + s);
    return val;
}