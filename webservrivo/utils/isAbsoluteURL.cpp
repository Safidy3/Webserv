/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   isAbsoluteURL.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/12/21 15:05:02 by rhanitra          #+#    #+#             */
/*   Updated: 2025/12/21 15:05:04 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

bool isAbsoluteURL(const std::string &p)
{
    return (p.rfind("http://", 0) == 0 || p.rfind("https://", 0) == 0);
}
