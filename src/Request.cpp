/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:07:51 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/26 15:46:56 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/Request.hpp"

Request::Request() {}

void	Request::parse(const std::string &str) {
	std::istringstream stream(str);
	stream >> method >> path >> version;
	std::cout << "Method: " << method << " Path: " << path << std::endl;
}

void	Request::clear() {
	method.clear();
	path.clear();
	version.clear();
	headers.clear();
	body.clear();
}
