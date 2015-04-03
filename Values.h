#pragma once

class Values : public unordered_map<string, list<string>>
{
public:
    Values() = default;
    virtual ~Values() = default;

    bool has(const string& key) const;

    bool getBool(const string& key, bool defaultValue = false) const;
    string getString(const string& key, const string& defaultValue = string()) const;
    int getInt(const string& key, int defaultValue = 0) const;

    const list<string>& operator[](const string& key) const;
    using unordered_map<string, list<string>>::operator[];
};
