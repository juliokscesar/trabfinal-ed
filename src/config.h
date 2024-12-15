#ifndef CONFIG_H
#define CONFIG_H

#include "typedefs.h"

typedef struct {
    // markov section
    uint order;
    uint randSeed;
    bool showTransMatrix;
    bool showConfidence;

    // data section
    char* defaultFile;
    size_t fileNameLen;
    double validRatio;
    double testRatio;

    // Network section
    bool useMarkovNetwork;
    bool exportNetwork;
    size_t netNodes;
    double lr;
    double minErrFactor;
    uint errFuncID;

    // Graph section
    bool useMarkovGraph;
    bool exportGraph;
    bool detectCycles;
    bool findDisconnected;
    bool findMostLikelyPath;
    bool doRandomWalk;

    // Predictions section
    size_t predictSteps;

} ContextConfiguration;

int iniHandler(void* user, const char* section, const char* name, const char* value);

ContextConfiguration* configInit();
bool configRead(ContextConfiguration* cfg, const char* file);
void configFree(ContextConfiguration** cfg);

#endif //CONFIG_H