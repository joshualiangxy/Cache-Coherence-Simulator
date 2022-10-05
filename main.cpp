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

void simulate(const int, std::filesystem::path, CacheType, int, int, int);
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

    for (int threadID = 0; threadID < 4; ++threadID) {
        threads.emplace_back(
            simulate,
            threadID,
            dataPath,
            cacheType,
            cacheSize,
            associativity,
            blockSize
        );
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

void simulate(
    const int threadID,
    std::filesystem::path dataPath,
    CacheType cacheType,
    int cacheSize,
    int associativity,
    int blockSize
) {
    dataPath += "_" + std::to_string(threadID) + ".data";

    std::ifstream fileStream{dataPath, std::ios::in};

    if (!fileStream.is_open()) {
        std::cerr << "Cannot read file: " << dataPath << std::endl;
        return;
    }

    std::unique_ptr<Cache> cache;

    switch (cacheType) {
        case CacheType::MESI:
            cache = std::make_unique<MESICache>(cacheSize, associativity, blockSize);
            break;
        case CacheType::DRAGON:
            cache = std::make_unique<DragonCache>(cacheSize, associativity, blockSize);
            break;
    }

    return;

    std::string line, hex;
    int label;
    uint32_t parsedHex;
    long long numCycles = 0,
        numComputeCycles = 0,
        numLoadInstructions = 0,
        numStoreInstructions = 0;

    while (std::getline(fileStream, line)) {
        std::istringstream lineStream{line};

        lineStream >> label >> hex;
        parsedHex = std::stoi(hex, nullptr, 16);

        switch ((InstructionType) label) {
            case InstructionType::READ:
                numCycles += cache->read(parsedHex);
                ++numLoadInstructions;
                break;
            case InstructionType::WRITE:
                numCycles += cache->write(parsedHex);
                ++numStoreInstructions;
                break;
            case InstructionType::OTHER:
                numCycles += parsedHex;
                numComputeCycles += parsedHex;
                break;
        }

        // std::cout << label << ' ' << hex << ' ' << parsedHex << std::endl;
    }
}

// Bithack: http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
bool isPowOfTwo(int i) {
    return i > 0 && !(i & (i - 1));
}

