#include "trace_reader.h"
#include <sstream>
#include <iostream>

TraceReader::TraceReader(const std::string& filepath)
    : filepath_(filepath), file_(filepath)
{
    if (!file_.is_open())
        std::cerr << "[TraceReader] Cannot open: " << filepath << "\n";
}

TraceReader::~TraceReader() { file_.close(); }

void TraceReader::reset() {
    file_.clear();
    file_.seekg(0);
}

MemAccess TraceReader::next() {
    MemAccess acc{};
    std::string line;

    while (std::getline(file_, line)) {
        if (line.empty() || line[0] == '#' || line[0] == '=') continue;

        char type = line[0];

        // ── Valgrind lackey format:  "I 0xADDR,size" ──
        if (type == 'I' || type == 'L' || type == 'S' || type == 'M') {
            // skip instruction fetches
            if (type == 'I') continue;

            std::istringstream ss(line.substr(2));
            std::string addr_str;
            std::getline(ss, addr_str, ',');
            try {
                acc.address  = std::stoull(addr_str, nullptr, 16);
                acc.is_write = (type == 'S');  // M = modify (load+store)
                acc.valid    = true;
                return acc;
            } catch (...) { continue; }
        }

        // ── Simple format:  "R 0xADDR"  or  "W 0xADDR" ──
        if (type == 'R' || type == 'W' || type == 'r' || type == 'w') {
            std::istringstream ss(line.substr(2));
            std::string addr_str;
            ss >> addr_str;
            try {
                acc.address  = std::stoull(addr_str, nullptr, 16);
                acc.is_write = (type == 'W' || type == 'w');
                acc.valid    = true;
                return acc;
            } catch (...) { continue; }
        }
    }

    acc.valid = false;
    return acc;
}
