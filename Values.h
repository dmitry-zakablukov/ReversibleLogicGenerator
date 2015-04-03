#pragma once

class Values : public unordered_map<string, list<string>>
{
public:
    Values() = default;
    virtual ~Values() = default;

    bool getBool(const string& key, bool defaultValue = false) const;
    string getString(const string& key, const string& defaultValue = string()) const;
    int getInt(const string& key, int defaultValue = 0) const;
};
