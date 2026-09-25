// copyright 2026 All Rights Reserved
// Author : madusankaijayarathna@gmail.com (Madusanka Jayarathna)

#pragma once
#include <iostream>
#include <string_view>
#include <vector>

namespace FIRMWARE {
inline constexpr std::string_view TAG = "ComponentTests";

class ComponentTests {
public:
    void runBlinkTest( void );

private:
    void toggleFlag(bool& flag);
    bool configurePins(const std::vector<size_t>& pinList, const std::vector<bool>& dirList);
    void setPinValue(size_t pin, const bool flag);

private:
    bool    verbose = false;

};
} // namespace FIRMWARE



