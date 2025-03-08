#pragma once
#include <stdexcept>
#include <string>


class FAN_exception : public std::runtime_error
{
public:
  FAN_exception(std::string msg) : std::runtime_error(std::move(msg)) {}
};


class algebraic_exception : public std::runtime_error
{
public:
  algebraic_exception(std::string msg) : std::runtime_error(std::move(msg)) {}
};


class kin_not_on_board_exception : public std::runtime_error
{
public:
  kin_not_on_board_exception(std::string msg)
      : std::runtime_error(std::move(msg))
  {}
};
