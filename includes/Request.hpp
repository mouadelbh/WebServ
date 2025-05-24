/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:06:02 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 17:14:03 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// #include "include.hpp"
#include <string>
#include <unordered_map>
#include <sstream>
#include <iostream>

class Request {
	public:
		std::string method;
		std::string path;
		std::string version;
		std::unordered_map<std::string, std::string> headers;
		std::string body;

		Request();
		void parse(const std::string &request_str);
		std::string toString() const;
};
