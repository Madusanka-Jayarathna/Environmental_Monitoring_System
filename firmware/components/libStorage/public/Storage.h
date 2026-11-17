// Copyright (c) 2026 Madusanka Jayarathna. All rights reserved.
// Use of this source code is governed by the MIT license.

#pragma once

#include <cstddef>
#include <cstdint>
#include "components/libUtils/public/defines.h"

namespace FIRMWARE {

struct reading_t;
namespace STORAGE {

class Storage {
public:
    struct Option {
        Option()                = default;
        size_t _maxFileSize     = MAX_FILE_SIZE; // 1MB
    };
public:
    Storage();
    ~Storage();

    // Non-copyable (owns hardware SPI & file handle)
    Storage(const Storage &) = delete;
    Storage &operator=(const Storage &) = delete;

    Option& opt() {return _opt;}
    const Option& opt() const {return _opt; }

    bool init();
    uint32_t nextSeq();
    bool appendReading(const reading_t &reading);

    uint32_t lastPersistedSeq() const { return _persistedSeq; }
    bool isMounted() const { return _isMounted; }

private:
    bool initNvs();
    bool persistSeqToNvs(uint32_t seq);
    bool mountSdCard();
    bool openActiveFile();
    void rotateIfNeeded();

private:
    Option          _opt;
    uint32_t        _nvsHandle = 0;
    uint32_t        _nextSeq = 0;
    uint32_t        _persistedSeq = 0;
    bool            _isMounted = false;
    void            *_activeFile = nullptr;
    void            *_card = nullptr;
};

} // namespace STORAGE
} // namespace FIRMWARE
