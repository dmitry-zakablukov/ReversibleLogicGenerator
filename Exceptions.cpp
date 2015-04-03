#include "std.hpp"

AssertionError::AssertionError(string&& message)
    : message(move(message))
{
}

const char* AssertionError::what() const
{
    return message.c_str();
}

InvalidFormatException::InvalidFormatException(string&& message)
    : message(move(message))
{
}

void InvalidFormatException::setMessage(string&& newMessage)
{
    message = move(newMessage);
}

const char* InvalidFormatException::what() const
{
    return message.c_str();
}
