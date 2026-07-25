#include "branch_predictor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║    Branch Predictor  (2-bit saturating)  ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n\n";

    std::vector<std::string> files;
    if (argc >= 2) {
        for (int i = 1; i < argc; ++i)
            files.push_back(argv[i]);
    } else {
        files = {"traces/branch_matrix.txt",
                 "traces/branch_bfs.txt",
                 "traces/branch_sort.txt"};
    }

    for (auto& path : files) {
        BranchPredictor bp(1024);
        std::ifstream f(path);
        if (!f.is_open()) {
            std::cerr << "Cannot open: " << path << "\n";
            continue;
        }

        std::string line;
        uint64_t count = 0;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ss(line);
            std::string addr_str;
            int taken_int;
            ss >> addr_str >> taken_int;
            uint64_t addr = std::stoull(addr_str, nullptr, 16);
            bp.predict_and_update(addr, taken_int == 1);
            ++count;
        }

        // extract workload name from filename
        std::string label = path;
        auto pos = label.rfind('/');
        if (pos != std::string::npos) label = label.substr(pos + 1);

        std::cout << "Trace: " << label << "  branches=" << count << "\n";
        bp.print_stats(label);
        std::cout << "\n";
    }
    return 0;
}

