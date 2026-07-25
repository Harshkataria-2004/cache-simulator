#pragma once
#include <string>
#include <fstream>
#include <cstdint>

struct MemAccess {
    uint64_t address;
    bool     is_write;   // true = store, false = load
    bool     valid;      // false means EOF
};

// Supports two trace formats:
//   1. Valgrind lackey:   "I 0x...,size"  "L 0x...,size"  "S 0x...,size"
//   2. Simple format:     "R 0x..."  or  "W 0x..."
class TraceReader {
public:
    explicit TraceReader(const std::string& filepath);
    ~TraceReader();

    MemAccess next();      // call repeatedly; valid=false when done
    void      reset();     // rewind to start

private:
    std::string   filepath_;
    std::ifstream file_;
};
