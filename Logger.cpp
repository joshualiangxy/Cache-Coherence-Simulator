#include "Logger.h"

Logger::Logger()
    : numExecutionCycles{0}
    , numComputeCycles{0}
    , numIdleCycles{0}
    , numLoadStoreInstructions{0}
    , numCacheMiss{0}
    , numBusTrafficInBytes{0}
    , numBusInvalidateUpdateEvents{0}
    , numPrivateDataAccess{0}
    , numPublicDataAccess{0} {}

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
    this->numBusTrafficInBytes += 4;
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

