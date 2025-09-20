/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ftStrdup.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: safandri <safandri@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 18:00:10 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/20 15:00:15 by safandri         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::string ftStrdup(const char* s)
{
    if (!s)
        return std::string();
    return std::string(s);
}

