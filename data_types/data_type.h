#pragma once
#include <bits/stdc++.h>
using namespace std;

class DataType {
public:
    virtual ~DataType() = default;

    [[nodiscard]] virtual string to_string() const = 0;
};
