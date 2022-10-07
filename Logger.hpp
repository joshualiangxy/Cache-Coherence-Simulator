#ifndef GUARD_LOGGER_H
#define GUARD_LOGGER_H

class Logger {
public:
    Logger();

    void logResults();

    void addExecutionCycles(int numCycles);
    void addComputeCycles(int numCycles);
    void addIdleCycles(int numCycles);

    void incrementNumLoadStoreInstructions();
    void incrementNumCacheMiss();
    void incrementBusTraffic();
    void incrementBusInvalidateUpdateEvents();

    void incrementPrivateDataAccess();
    void incrementPublicDataAccess();

    long long getNumExecutionCycles();
    long long getNumComputeCycles();
    long long getNumIdleCycles();
    long long getNumLoadStoreInstructions();
    long long getNumCacheMiss();
    long long getNumBusTrafficInBytes();
    long long getNumBusInvalidateUpdateEvents();
    long long getNumPrivateDataAccess();
    long long getNumPublicDataAccess();

private:
    long long numExecutionCycles;
    long long numComputeCycles;
    long long numIdleCycles;
    long long numLoadStoreInstructions;
    long long numCacheMiss;
    long long numBusTrafficInBytes;
    long long numBusInvalidateUpdateEvents;
    long long numPrivateDataAccess;
    long long numPublicDataAccess;
};

#endif

