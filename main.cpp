#include "Bus.hpp"
#include "Cache.hpp"
#include "Logger.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <unordered_set>
#include <vector>
#include <string>

const int NUM_THREADS = 4;

enum class InstructionType {
    READ = 0,
    WRITE = 1,
    OTHER = 2
};

void simulate(
    const int threadID,
    std::filesystem::path dataPath,
    CacheType cacheType,
    int cacheSize,
    int associativity,
    int blockSize,
    std::shared_ptr<Logger> logger,
    std::shared_ptr<Bus> bus
);
bool isPowOfTwo(int i);
int parseInt(const std::string& toParse, const std::string& inputType);

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
    } else if (args[1] == "MESIF") {
        cacheType = CacheType::MESIF;
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

    for (int i = 0; i < argc; ++i) {
        std::cout << argv[i] << ' ';
    }
    std::cout << std::endl << std::endl;

    std::vector<std::thread> threads;
    std::vector<std::shared_ptr<Logger>> loggers;
    std::shared_ptr<Bus> bus = std::make_shared<Bus>();

    for (int threadID = 0; threadID < NUM_THREADS; ++threadID) {
        loggers.emplace_back(std::make_shared<Logger>(blockSize));
        threads.emplace_back(
            simulate,
            threadID,
            dataPath,
            cacheType,
            cacheSize,
            associativity,
            blockSize,
            loggers.back(),
            bus
        );
    }

    long long numExecutionCycles = 0,
        numBusTrafficInBytes = 0,
        numBusInvalidateUpdateEvents = 0;

    for (int threadID = 0; threadID < NUM_THREADS; ++threadID) {
        threads[threadID].join();

        numExecutionCycles += loggers[threadID]->getNumExecutionCycles();
        numBusTrafficInBytes += loggers[threadID]->getNumBusTrafficInBytes();
        numBusInvalidateUpdateEvents += loggers[threadID]
            ->getNumBusInvalidateUpdateEvents();
    }

    std::cout << "Overall Execution Cycles for entire trace: " << numExecutionCycles << std::endl
        << "Total data traffic on the bus in bytes: " << numBusTrafficInBytes << std::endl
        << "Total invalidations/updates on the bus: " << numBusInvalidateUpdateEvents << std::endl << std::endl;

    for (int threadID = 0; threadID < NUM_THREADS; ++threadID) {
        std::cout << "Core " << threadID << std::endl;
        loggers[threadID]->logResults();
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
    int blockSize,
    std::shared_ptr<Logger> logger,
    std::shared_ptr<Bus> bus
) {
    dataPath += "_" + std::to_string(threadID) + ".data";

    std::ifstream fileStream{dataPath, std::ios::in};

    if (!fileStream.is_open()) {
        std::cerr << "Cannot read file: " << dataPath << std::endl;
        return;
    }

    std::unique_ptr<Cache> cache = std::make_unique<Cache>(
        threadID,
        cacheSize,
        associativity,
        blockSize,
        cacheType,
        logger
    );

    std::string line, hex;
    int label, cyclesLeft = 1;
    uint32_t parsedHex;

    while (cyclesLeft-- >= 0) {
        std::queue<BusEvent> eventQueue = bus->getEventsInQueue(threadID);
        cache->handleBusEvents(eventQueue, bus);

        if (cyclesLeft == 0 && std::getline(fileStream, line)) {
            std::istringstream lineStream{line};

            lineStream >> label >> hex;
            parsedHex = std::stoi(hex, nullptr, 16);

            switch ((InstructionType) label) {
                case InstructionType::READ:
                    cyclesLeft += cache->read(parsedHex, bus);
                    break;
                case InstructionType::WRITE:
                    cyclesLeft += cache->write(parsedHex, bus);
                    break;
                case InstructionType::OTHER:
                    cyclesLeft += parsedHex;

                    logger->addExecutionCycles(parsedHex);
                    logger->addComputeCycles(parsedHex);
                    break;
            }
        }
    }
}

// Bithack: http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
bool isPowOfTwo(int i) {
    return i > 0 && !(i & (i - 1));
}

