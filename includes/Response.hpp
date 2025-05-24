/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mel-bouh <mel-bouh@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/24 16:17:34 by mel-bouh          #+#    #+#             */
/*   Updated: 2025/05/24 17:14:18 by mel-bouh         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

// #include "include.hpp"
#include <string>
#include <unordered_map>
#include <sstream>
#include <iostream>

class Response {
	public:
		std::string version;
		int status_code;
		std::string status_message;
		std::unordered_map<std::string, std::string> headers;
		std::string body;

		Response();
		void setStatus(int code, const std::string &message);
		std::string toString() const;
};
