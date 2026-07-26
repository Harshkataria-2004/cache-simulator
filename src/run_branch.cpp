#include "branch_predictor.h"
#include "gshare_predictor.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    std::cout << "╔══════════════════════════════════════════════╗\n";
    std::cout << "║   Branch Predictor Comparison                ║\n";
    std::cout << "║   2-bit Saturating Counter vs GShare         ║\n";
    std::cout << "╚══════════════════════════════════════════════╝\n\n";

    std::vector<std::string> files = {
        "traces/branch_matrix.txt",
        "traces/branch_bfs.txt",
        "traces/branch_sort.txt"
    };

    if (argc >= 2)
        files = {argv[1]};

    std::cout << std::left
              << std::setw(20) << "Workload"
              << std::setw(20) << "2-bit Accuracy"
              << std::setw(20) << "GShare Accuracy"
              << "Winner\n";
    std::cout << std::string(70, '-') << "\n";

    for (auto& path : files) {
        BranchPredictor  bp2(1024);
        GSharePredictor  gshare(10, 1024);

        std::ifstream f(path);
        if (!f.is_open()) {
            std::cerr << "Cannot open: " << path << "\n";
            continue;
        }

        std::string line;
        while (std::getline(f, line)) {
            if (line.empty() || line[0] == '#') continue;
            std::istringstream ss(line);
            std::string addr_str;
            int taken_int;
            ss >> addr_str >> taken_int;
            uint64_t addr = std::stoull(addr_str, nullptr, 16);
            bool taken    = (taken_int == 1);
            bp2.predict_and_update(addr, taken);
            gshare.predict_and_update(addr, taken);
        }

        // extract label from filename
        std::string label = path;
        auto pos = label.rfind('/');
        if (pos != std::string::npos) label = label.substr(pos + 1);
        pos = label.rfind('.');
        if (pos != std::string::npos) label = label.substr(0, pos);

        double acc2    = bp2.stats().accuracy();
        double acc_gs  = gshare.stats().accuracy();
        std::string winner = (acc_gs > acc2) ? "GShare ✓" :
                             (acc2 > acc_gs) ? "2-bit ✓" : "Tie";

        std::cout << std::left
                  << std::setw(20) << label
                  << std::setw(20) << (std::to_string((int)acc2)    + "." +
                                       std::to_string((int)(acc2*100)%100) + "%")
                  << std::setw(20) << (std::to_string((int)acc_gs)  + "." +
                                       std::to_string((int)(acc_gs*100)%100) + "%")
                  << winner << "\n";
    }

    return 0;
}

