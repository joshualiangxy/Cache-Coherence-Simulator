#include "Logger.hpp"

#include <iostream>

Logger::Logger(int blockSize)
    : blockSize{blockSize}
    , numExecutionCycles{0}
    , numComputeCycles{0}
    , numIdleCycles{0}
    , numLoadStoreInstructions{0}
    , numCacheMiss{0}
    , numBusTrafficInBytes{0}
    , numBusInvalidateUpdateEvents{0}
    , numPrivateDataAccess{0}
    , numPublicDataAccess{0}
    , numUniqueAddresses{0} {}

void Logger::logResults() {
    long double cacheMissRate = (long double) numCacheMiss / numLoadStoreInstructions;

    std::cout << "1. Overall Execution Cycle: " << numExecutionCycles
        << "\n2. Number of compute cycles: " << numComputeCycles
        << "\n3. Number of load/store instructions: " << numLoadStoreInstructions
        << "\n4. Number of idle cycles: " << numIdleCycles
        << "\n5. Data cache miss rate: " << cacheMissRate
        << "\n6. Data traffic in bytes: " << numBusTrafficInBytes
        << "\n7. Number of invalidations or updates on bus: " << numBusInvalidateUpdateEvents
        << "\n8. Number of private data access: " << numPrivateDataAccess
        << "\n9. Number of public data access: " << numPublicDataAccess
        << "\n10: Number of unique data access: " << numUniqueAddresses
        << std::endl << std::endl;
}

void Logger::addExecutionCycles(int numCycles) {
    this->numExecutionCycles += numCycles;
}

void Logger::addComputeCycles(int numCycles) {
    this->numComputeCycles += numCycles;
}

void Logger::addIdleCycles(int numCycles) {
    this->numIdleCycles += numCycles;
}

void Logger::incrementNumLoadStoreInstructions() {
    ++this->numLoadStoreInstructions;
}

void Logger::incrementNumCacheMiss() {
    ++this->numCacheMiss;
}

void Logger::incrementBusTraffic() {
    this->numBusTrafficInBytes += this->blockSize;
}

void Logger::incrementBusInvalidateUpdateEvents() {
    ++this->numBusInvalidateUpdateEvents;
}

void Logger::incrementPrivateDataAccess() {
    ++this->numPrivateDataAccess;
}

void Logger::incrementPublicDataAccess() {
    ++this->numPublicDataAccess;
}

void Logger::incrementUniqueAddresses() {
    ++this->numUniqueAddresses;
}

long long Logger::getNumExecutionCycles() {
    return this->numExecutionCycles;
}

long long Logger::getNumComputeCycles() {
    return this->numComputeCycles;
}

long long Logger::getNumIdleCycles() {
    return this->numIdleCycles;
}

long long Logger::getNumLoadStoreInstructions() {
    return this->numLoadStoreInstructions;
}

long long Logger::getNumCacheMiss() {
    return this->numCacheMiss;
}

long long Logger::getNumBusTrafficInBytes() {
    return this->numBusTrafficInBytes;
}

long long Logger::getNumBusInvalidateUpdateEvents() {
    return this->numBusInvalidateUpdateEvents;
}

long long Logger::getNumPrivateDataAccess() {
    return this->numPrivateDataAccess;
}

long long Logger::getNumPublicDataAccess() {
    return this->numPublicDataAccess;
}

