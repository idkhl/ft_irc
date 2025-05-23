#pragma once

#include <iostream>

class	Error : public std::exception
{
	private:
		const std::string	_message;

	public:
					Error(const std::string& message) : _message(message) {}
					~Error(void) throw() {}

		const char		*what(void) const throw() { return _message.c_str(); }
};