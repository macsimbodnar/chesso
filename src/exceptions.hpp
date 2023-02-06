#pragma once
#include <pixello.hpp>


class FAN_exception : public pixello_exception
{
public:
  FAN_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};
