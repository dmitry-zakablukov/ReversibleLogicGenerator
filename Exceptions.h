#pragma once

class AssertionError : public exception
{
public:
    AssertionError() = default;
    AssertionError(string&& message);
    virtual ~AssertionError() = default;

    virtual const char* what() const;

private:
    string message;
};

class InvalidFormatException : public exception
{
public:
    InvalidFormatException() = default;
    explicit InvalidFormatException(string&& message);
    virtual ~InvalidFormatException() = default;

    void setMessage(string&& newMessage);
    virtual const char* what() const;

private:
    string message;
};
