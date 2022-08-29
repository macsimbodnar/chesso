#pragma once
#include <pixello.hpp>


class FAN_exception : public pixello_exception
{
public:
  FAN_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};


class input_exception : public pixello_exception
{
public:
  input_exception(std::string msg) : pixello_exception(std::move(msg)) {}
};