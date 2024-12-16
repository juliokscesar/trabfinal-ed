#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <ini.h>

#include "logging.h"

#define MATCH(s,n) strcmp(section,s) == 0 && strcmp(name, n) == 0

int iniHandler(void* user, const char* section, const char* name, const char* value) {
    ContextConfiguration* config = (ContextConfiguration*)user;

    if (MATCH("markov", "order"))
        config->order = atoi(value);
    else if (MATCH("markov", "seed"))
        config->randSeed = atoi(value);
    else if (MATCH("markov", "show_transition_matrix"))
        config->showTransMatrix = (bool)atoi(value);
    else if (MATCH("markov", "show_confidence"))
        config->showConfidence = (bool)atoi(value);
    else if (MATCH("markov", "show_confusion_matrix"))
        config->showConfMatrix = (bool)atoi(value);

    else if (MATCH("data", "default_file")) {
        config->fileNameLen = strlen(value);
        config->defaultFile = malloc(sizeof(char) * config->fileNameLen);
        strncpy(config->defaultFile, value, config->fileNameLen);
    }
    else if (MATCH("data", "valid_ratio"))
        config->validRatio = strtod(value, NULL);
    else if (MATCH("data", "test_ratio"))
        config->testRatio = strtod(value, NULL);

    else if (MATCH("network", "use"))
        config->useMarkovNetwork = (bool)atoi(value);
    else if (MATCH("network", "export"))
        config->exportNetwork = (bool)atoi(value);
    else if (MATCH("network", "nodes"))
        config->netNodes = (size_t)strtol(value, NULL, 10);
    else if (MATCH("network", "lr"))
        config->lr = strtod(value, NULL);
    else if (MATCH("network", "minimum_error_factor"))
        config->minErrFactor = strtod(value, NULL);
    else if (MATCH("network", "err_func_id"))
        config->errFuncID = (uint)atoi(value);
    else if (MATCH("network", "get_most_optimal_node"))
        config->getMostOptimalNode = (bool)atoi(value);
    else if (MATCH("network", "score_alpha"))
        config->scoreAlpha = strtod(value, NULL);

    else if (MATCH("graph", "use"))
        config->useMarkovGraph = (bool)atoi(value);
    else if (MATCH("graph", "export"))
        config->exportGraph = (bool)atoi(value);
    else if (MATCH("graph", "find_disconnected"))
        config->findDisconnected = (bool)atoi(value);
    else if (MATCH("graph", "random_walk"))
        config->doRandomWalk = (bool)atoi(value);

    else if(MATCH("predictions", "steps"))
        config->predictSteps = (size_t)strtol(value, NULL, 10);

    else
        return 0;

    return 1;
}

ContextConfiguration* configInit() {
    ContextConfiguration* cfg = calloc(1, sizeof(ContextConfiguration));
    return cfg;
}

bool configRead(ContextConfiguration* cfg, const char* file) {
    if (!cfg)
        return false;

    return ini_parse(file, iniHandler, cfg) >= 0;
}

void configFree(ContextConfiguration** cfg) {
    if (!cfg || !(*cfg))
        return;
    if ((*cfg)->defaultFile)
        free((*cfg)->defaultFile);
    free(*cfg);
    *cfg = NULL;
}
