#pragma once
#include <string>
#include <stdexcept>
	

class FAN_exception : public std::runtime_error
{
public:
  FAN_exception(std::string msg) : std::runtime_error(std::move(msg)) {}
};
