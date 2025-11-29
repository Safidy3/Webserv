/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftStrdup.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 18:00:10 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/12 19:40:02 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

std::string ftStrdup(const char* s)
{
    if (!s)
        return std::string();
    return std::string(s);
}

