#include "Cache.h"
#include "DragonCache.h"
#include "MESICache.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <vector>
#include <string>

enum class InstructionType {
    READ = 0,
    WRITE = 1,
    OTHER = 2
};

void simulate(const int, std::filesystem::path);
bool isPowOfTwo(int);
int parseInt(const std::string&, const std::string&);

int main(int argc, char* argv[]) {
    if (argc < 6) {
        std::cerr << "Usage: coherence <PROTOCOL> "
            "<BENCHMARK> <CACHE_SIZE> "
            "<ASSOCIATIVITY> <BLOCK_SIZE>" << std::endl;
        exit(1);
    }

    const std::unordered_set<std::string> validBenchmarks = {
        "blackscholes", "bodytrack", "fluidanimate"
    };

    std::vector<std::string> args;
    CacheType cacheType;

    args.assign(argv, argv + argc);

    if (args[1] == "Dragon") {
        cacheType = CacheType::DRAGON;
    } else if (args[1] == "MESI") {
        cacheType = CacheType::MESI;
    } else {
        std::cerr << "Expected <PROTOCOL>: MESI | Dragon" << std::endl
            << "Actual <PROTOCOL>: " << args[1] << std::endl;
        exit(1);
    }

    if (!validBenchmarks.contains(args[2])) {
        std::cerr << "Expected <INPUT_FILE>: "
            "blackscholes | bodytrack | fluidanimate" << std::endl
            << "Actual <INPUT_FILE>: " << args[2] << std::endl;
        exit(1);
    }

    std::filesystem::path dataPath = std::filesystem::current_path();
    dataPath /= "data";
    dataPath /= args[2];

    int cacheSize, associativity, blockSize;

    try {
        cacheSize = parseInt(args[3], "<CACHE_SIZE>");
        associativity = parseInt(args[4], "<ASSOCIATIVITY>");
        blockSize = parseInt(args[5], "<BLOCK_SIZE>");
    } catch (std::logic_error& err) {
        std::cerr << err.what() << std::endl;
        exit(1);
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(simulate, i, dataPath);
    }
    for (auto& thread : threads) {
        thread.join();
    }

    return 0;
}

int parseInt(const std::string& toParse, const std::string& inputType) {
    int parsedInput;
    try {
        parsedInput = std::stoi(toParse);
    } catch (std::logic_error& err) {
        throw std::logic_error(
            "Expected " + inputType + ": integer\n"
            "Actual " + inputType + ": " + toParse
        );
    }

    if (!isPowOfTwo(parsedInput)) {
        throw std::logic_error(
            "Expected " + inputType + ": to be power of 2\n"
            "Actual " + inputType + ": " + toParse
        );
    }

    return parsedInput;
}

void simulate(const int threadID, std::filesystem::path dataPath) {
    dataPath += "_" + std::to_string(threadID) + ".data";

    std::ifstream fileStream{dataPath, std::ios::in};

    if (!fileStream.is_open()) {
        std::cerr << "Cannot read file: " << dataPath << std::endl;
        return;
    }

    std::cout << dataPath << std::endl;
    return;

    std::string line, hex;
    int label;
    while (std::getline(fileStream, line)) {
        std::istringstream lineStream{line};

        lineStream >> label >> hex;

        switch ((InstructionType) label) {
        case InstructionType::READ:
            std::cout << "READ" << std::endl;
            break;
        case InstructionType::WRITE:
            std::cout << "WRITE" << std::endl;
            break;
        case InstructionType::OTHER:
            std::cout << "OTHER" << std::endl;
            break;
        }

        // std::cout << label << ' ' << hex << ' ' << std::stoi(hex, nullptr, 16) << std::endl;
    }
}

bool isPowOfTwo(int i) {
    return i > 0 && !(i & (i - 1));
}

