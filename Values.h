#pragma once

class Values : public unordered_map<string, list<string>>
{
public:
    Values() = default;

    bool getBool(const string& key) const;
    string getString(const string& key) const;
    int getInt(const string& key) const;
};
