// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <fstream>
#include <iostream>

namespace FIRMWARE {
namespace TEST {
class TestFirmware {
public:
    struct Option {
        Option() = default;
        bool _runComponentTest = false;
    };

public:
    bool runFirmwareTest(void) const;
    Option& opt() { return _opt; }
    const Option& opt() const { return _opt; }

private:
    bool runComponentTest(void) const;

private:
    Option _opt;
};
} // namespace TEST
} // namespace FIRMWARE

