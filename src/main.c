#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "config.h"
#include "markov.h"
#include "logging.h"
#include "markovgraph.h"
#include "markovnetwork.h"
#include "utils.h"

void printIntro() {
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("---------------------------- TIME SERIES FORECAST WITH MARKOV CHAINS ----------------------------\n");
    printf("-------------------------------------------------------------------------------------------------\n\n");
}

void printHelp() {
    printf("Usage: ./proj [-h] [-d data_file] [-m] [-c config_file] [-w] [-s steps] [-p] [-o order]\n");
    printf("=> [-h]: show this message and exit.\n");
    printf("=> [-d data_file]: use data file in path data_file.\n");
    printf("=> [-m]: insert data manually value by value. If this flag and '-d data_file' is provided, ignore the data file.\n");
    printf("=> [-c config]: use config file in path confg_file.\n");
    printf("=> [-w]: wait for user input before advancing to next sections.\n");
    printf("=> [-s steps]: predict next 'steps' instead of what's in the configuration file.\n");
    printf("=> [-p]: print details from loaded data. Useful for making sure the program has loaded things correctly.\n");
    printf("=> [-o order]: use 'order' for the system, instead of what's set in the configuration file.\n");
    printf("!! All file paths must be relative to current working directory -- the one you're at right now.\n");
    printf("!! You can change the default data file path in the config file. If no '-c config_file' is provided, it uses 'config.ini' as default.\n");
}

/* ---------------------------------------------- DEFAULT MARKOV CHAIN ---------------------------------------------- */
TransitionMatrix* runDefaultMarkov(const int* train, const size_t trainSize, const int* valid, const size_t validSize,
                                    const int* test, const size_t testSize, MarkovState* states, const ContextConfiguration* cfg, double* outAcc) {
    printf("\n=====> INITIATING DEFAULT MARKOV FORECAST RUN <=====\n");
    printf("=====> USING ORDER: %u\n", states->order);

    // Join train and valid data for default markov, since there's no validation step
    size_t n = trainSize + validSize;
    int* data = malloc(sizeof(int) * (n + testSize));
    if (!data) {
        LOG_ERROR("malloc failed for 'data' allocation in runDefaultMarkov");
        return NULL;
    }
    memcpy(data, train, sizeof(int) * trainSize);
    memcpy(data+trainSize, valid, sizeof(int) * validSize);

    TransitionMatrix* tm = markovBuildTransMatrix(data, n, states);
    if (!tm) {
        LOG_ERROR("Unable to build transition matrix in runDefaultMarkov");
        free(data);
        return NULL;
    }

    if (cfg->showTransMatrix) {
        printf("=====> MARKOV TRANSITION MATRIX WITH ORDER = %u\n", states->order);
        markovPrintTransMatrix(tm);
        putchar('\n');
    }

    // Run test predictions
    int* predictions = malloc(sizeof(int) * testSize);
    double* conf = malloc(sizeof(double) * testSize);
    if (!predictions || !conf) {
        LOG_ERROR("malloc failed for either predictions or confOut");
        markovFreeTransMatrix(&tm);
        free(data);
        if (predictions)
            free(predictions);
        if (conf)
            free(conf);
        return NULL;
    }

    clock_t time = clock();
    markovPredict(tm, testSize, data, n, predictions, conf);
    time = clock() - time;
    double delta = ((double)time)/CLOCKS_PER_SEC; // time in seconds
    printf("=====> TIME TAKEN IN PREDICTIONS (%lu steps): %lf s\n", testSize, delta);

    if (cfg->showConfMatrix) {
        printf("=====> CONFUSION MATRIX:\n");
        showConfusionMatrix(test, predictions, testSize);
    }

    if (cfg->showConfidence) {
        double propagated = 1.0;
        for (size_t i = 0; i < testSize; i++)
            propagated *= conf[i];
        printf("Pred. confidence (%lu): ", testSize);
        printArr_d(conf, testSize);
        printf("Final propagated confidence: %lf\n", propagated);
    }

    double acc = calcAccuracy(test, predictions, testSize);
    printf("=====> ACCURACY: %lf\n", acc);

    if (outAcc)
        *outAcc = acc;

    putchar('\n');

    free(predictions);
    free(conf);
    free(data);
    printf("=====> ENDING DEFAULT MARKOV FORECAST RUN <=====\n");
    return tm;
}
/* ------------------------------------------------------------------------------------------------------------------ */

/* -------------------------------------------------- MARKOV GRAPH -------------------------------------------------- */
MarkovGraph* runMarkovGraph(const TransitionMatrix* tm, const int* valid, const size_t validSize, const int* test,
                            const size_t testSize, const ContextConfiguration* cfg, double* outAcc) {
    printf("\n=====> INITIATING MARKOV GRAPH RUN <=====\n");

    MarkovGraph* graph = mkGraphInit(tm->state);
    if (!graph) {
        LOG_ERROR("Unable to initialize graph in runMarkovGraph");
        return NULL;
    }
    mkGraphBuildTransitions(graph, tm);

    if (cfg->doRandomWalk) {
        // Predict values doing random walk in the graph
        int* predictions = malloc(sizeof(int) * testSize);
        double* conf = malloc(sizeof(double) * testSize);

        // Last state is the last 'order' values of the valid set (because we use train+valid to train the TransitionMatrix)
        const int* lastState = &valid[validSize - graph->order];

        clock_t time = clock();
        mkGraphRandWalk(graph, lastState, testSize, predictions, conf);
        time = clock() - time;
        double delta = ((double)time)/CLOCKS_PER_SEC; // time in seconds
        printf("=====> TIME TAKEN IN PREDICTIONS (%lu steps): %lf s\n", testSize, delta);

        if (cfg->showConfMatrix) {
            printf("=====> CONFUSION MATRIX:\n");
            showConfusionMatrix(test, predictions, testSize);
        }

        if (cfg->showConfidence) {
            double propagated = 1.0;
            for (size_t i = 0; i < testSize; i++)
                propagated *= conf[i];
            printf("Pred. confidence (%lu): ", testSize);
            printArr_d(conf, testSize);
            printf("Final propagated confidence: %lf\n", propagated);
        }

        double acc = calcAccuracy(test, predictions, testSize);
        printf("=====> ACCURACY: %lf\n", acc);

        if (outAcc)
            *outAcc = acc;

        free(predictions);
        free(conf);
    }

    if (cfg->findDisconnected) {
        size_t count = 0;
        size_t* discIDs = mkGraphFindDisconnected(graph, &count);
        if (!discIDs || count == 0)
            printf("Couldn't find disconnected nodes in the graph\n");
        else {
            for (size_t i = 0; i < count; i++) {
                printf("=======> DISCONNECTED ID: %lu, STATE: ", discIDs[i]);
                printArr_i(mkNodeState(mkGraphGetNode(graph, discIDs[i])), graph->order);
            }
        }

        free(discIDs);
    }

    if (cfg->exportGraph)
        mkGraphExport(graph, "graph.dot");

    printf("\n=====> ENDING MARKOV GRAPH RUN <=====\n");

    return graph;
}
/* ------------------------------------------------------------------------------------------------------------------ */

/* ------------------------------------------------- MARKOV NETWORK ------------------------------------------------- */
MarkovNetwork* runMarkovNetwork(MarkovState* states, int* train, size_t trainSize, int* valid, size_t validSize, int* test,
                        size_t testSize, const ContextConfiguration* cfg, double* outAcc) {
    printf("\n=====> INITIATING MARKOV NETWORK RUN <=====\n");

    // Calculate error factor for each matrix node
    double* errFactors = malloc(cfg->netNodes * sizeof(double));
    for (size_t n = 0; n < cfg->netNodes; n++) {
        if (cfg->minErrFactor * (double)n > 0.95)
            errFactors[n] = 0.95;
        else
            errFactors[n] = cfg->minErrFactor * (double)n;
    }

    static const MKErrFuncT map[] = {randomBinarySwap, binarySegmentNoise, randomSwap};
    const size_t funcMapSize = sizeof(map) / sizeof(map[0]);
    if (cfg->errFuncID >= funcMapSize) {
        LOG_ERROR("Invalid error function id:");
        printf("%u\n", cfg->errFuncID);
        free(errFactors);
        return NULL;
    }

    MarkovNetwork* net = mkNetInit(states, cfg->netNodes, errFactors, map[cfg->errFuncID]);
    if (!net) {
        LOG_ERROR("Unable to initialize Markov Network in runMarkovNetwork");
        free(errFactors);
        return NULL;
    }

    clock_t time = clock();
    mkNetTrain(net, train, trainSize, valid, validSize, cfg->lr);
    time = clock() - time;
    double delta = ((double)time)/CLOCKS_PER_SEC;
    printf("=====> TIME TAKEN IN TRAINING (%lu nodes): %lf s\n", cfg->netNodes, delta);

    int* predictions = malloc(sizeof(int) * testSize);
    double* conf = malloc(sizeof(double) * testSize);
    if (!predictions || !conf) {
        LOG_ERROR("malloc failed for either predictions or confOut");
        mkNetFree(&net);
        free(errFactors);
        if (predictions)
            free(predictions);
        if (conf)
            free(conf);
        return NULL;
    }

    time = clock();
    mkNetPredict(net, testSize, predictions, conf);
    time = clock() - time;
    delta = ((double)time)/CLOCKS_PER_SEC; // time in seconds
    printf("=====> TIME TAKEN IN PREDICTIONS (%lu steps): %lf s\n", testSize, delta);

    if (cfg->showConfMatrix) {
        printf("=====> CONFUSION MATRIX:\n");
        showConfusionMatrix(test, predictions, testSize);
    }

    if (cfg->showConfidence) {
        double propagated = 1.0;
        for (size_t i = 0; i < testSize; i++)
            propagated *= conf[i];
        printf("Pred. confidence (%lu): ", testSize);
        printArr_d(conf, testSize);
        printf("Final propagated confidence: %lf\n", propagated);
    }
    double acc = calcAccuracy(test, predictions, testSize);
    printf("=====> ACCURACY: %lf\n", acc);

    if (outAcc)
        *outAcc = acc;

    if (cfg->getMostOptimalNode) {
        double score = 0.0;
        size_t optID = mkNetOptimalNode(net, cfg->scoreAlpha, &score);
        if (optID >= cfg->netNodes) {
            LOG_ERROR("Couldn't get most optimal node ID from Markov Network");
        } else {
            printf("=====> NETWORK MOST OPTIMAL NODE: ID %lu, WEIGHT: %lf, ERROR FACTOR: %lf, SCORE %lf, NODE TRANSITION MATRIX:\n",
                optID, net->output[optID]->weight, net->input[optID]->errFac, score);
            markovPrintTransMatrix(net->input[optID]->dest->matrix);
        }
    }

    if (cfg->exportNetwork)
        mkNetExport(net, "network.dot");

    putchar('\n');

    free(predictions);
    free(conf);
    free(errFactors);
    printf("\n=====> ENDING MARKOV NETWORK RUN <=====\n");

    return net;
}
/* ------------------------------------------------------------------------------------------------------------------ */

void manualInsertion(int** data, size_t* n) {
    const size_t BUCKET = 100;
    uint nBuckets = 1;
    *data = malloc(sizeof(int) * nBuckets * BUCKET);
    *n = 0;
    char buffer[256];
    printf("MANUAL DATA INSERTION (type non-numeric character to stop): ");
    while (true) {
        if (scanf("%255s", buffer) != 1)
            break;

        int val = 0;
        if (sscanf(buffer, "%d", &val) == 1) {
            (*data)[*n] = val;
            (*n)++;
            if (*n > BUCKET*nBuckets) {
                nBuckets++;
                int* temp = realloc(*data, sizeof(int) * nBuckets*BUCKET);
                if (!temp) {
                    LOG_ERROR("Failed to increase size for data input");
                    break;
                }
                *data = temp;
            }
        }
        else
            break;
    }
}

char* getArg(int argc, char* argv[], const char* key) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], key) == 0) {
            if (i+1 < argc)
                return argv[i+1];
            return argv[i];
        }
    }
    return NULL;
}

void enterWait() {
    printf("\n=====> PRESS ENTER TO CONTINUE (wait enabled)");
    getc(stdin);
}

int main(int argc, char* argv[]) {
    printIntro();

    if (getArg(argc,argv,"-h")) {
        printHelp();
        return 0;
    }

    bool wait = (getArg(argc, argv, "-w") != NULL);

    char* cfgFile = getArg(argc, argv, "-c");
    if (!cfgFile)
        cfgFile = "config.ini";

    // Get configuration
    ContextConfiguration* cfg = configInit();
    if (!configRead(cfg, cfgFile)) {
        LOG_FATAL("Unable to read config file: ");
        printf("%s\n", cfgFile);
        configFree(&cfg);
        return -1;
    }
    srand(cfg->randSeed);

    // Load data
    int* data = NULL;
    size_t dataSize = 0;
    if (argc > 2) {
        // Insert data manually
        if (getArg(argc, argv, "-m"))
            manualInsertion(&data, &dataSize);
        // Load data from file
        else
            data = loadData_i(getArg(argc,argv,"-d"), &dataSize);
    }
    else
        data = loadData_i(cfg->defaultFile, &dataSize);
    if (!data || dataSize == 0) {
        LOG_FATAL("Unable to open data file. The path must be relative to the current working directory");
        return -1;
    }

    // Get unique values from data
    int* unique = NULL;
    size_t uniqueSize = 0;
    findDistinct_i(data, dataSize, &unique, &uniqueSize);
    if (!unique) {
        LOG_FATAL("Unable to get unique values from data to build states");
        return -1;
    }

    // Split train, valid, test
    int *train, *valid, *test;
    size_t trainSize, testSize, validSize;
    splitTrainValTest_i(data, dataSize, &train, &valid, &test, &trainSize, &validSize, &testSize, cfg->validRatio, cfg->testRatio);
    if (!train || !valid || !test) {
        LOG_FATAL("Unable to split train, valid, test");
        return -1;
    }
    if (validSize <= 2 || testSize <= 2) {
        LOG_FATAL("There must be enough data to split between train, valid and test. But either valid or test are too small (the minimum is 2 for both of them).");
        printf("Valid size: %lu, Test size: %lu\n", validSize, testSize);
        return -1;
    }

    // Show data details
    if (getArg(argc, argv, "-p")) {
        printf("DATA DETAILS:\n");
        printf("Data (%lu): ", dataSize);
        printArr_i(data, dataSize);
        printf("Unique values (%lu): ", uniqueSize);
        printArr_i(unique, uniqueSize);
        printf("Train set (%lu): ", trainSize);
        printArr_i(train, trainSize);
        printf("Valid set (%lu): ", validSize);
        printArr_i(valid, validSize);
        printf("Test set (%lu): ", testSize);
        printArr_i(test, testSize);
        putchar('\n');
    }

    char* ord = getArg(argc, argv, "-o");
    if (ord) {
        long spec = strtol(ord, NULL, 10);
        if (errno == ERANGE || spec < 0 || spec > UINT_MAX) {
            LOG_FATAL("Unable to read specified order, or invalid value entered.");
            return -1;
        }
        cfg->order = (uint)spec;
    }

    // Build markov states
    MarkovState* states = markovBuildStates(cfg->order, unique, uniqueSize);
    if (!states) {
        LOG_FATAL("Unable to build markov states");
        return -1;
    }

    // Run forecast with default markov chain
    double mkAcc = 0.0;
    TransitionMatrix* tm = runDefaultMarkov(train, trainSize, valid, validSize, test, testSize, states, cfg, &mkAcc);
    if (!tm) {
        LOG_FATAL("Unable to get transition matrix from default run");
        return -1;
    }

    if (wait)
        enterWait();

    // Build graph if requested
    MarkovGraph* graph = NULL;
    double gAcc = 0.0;
    if (cfg->useMarkovGraph)
        graph = runMarkovGraph(tm, valid, validSize, test, testSize, cfg, &gAcc);

    if (wait)
        enterWait();

    // Build network if requested
    MarkovNetwork* net = NULL;
    double nAcc = 0.0;
    if (cfg->useMarkovNetwork)
        net = runMarkovNetwork(states, train, trainSize, valid, validSize, test, testSize, cfg, &nAcc);

    if (wait)
        enterWait();

    // Finally, run requested forecast
    const char* argSteps = getArg(argc, argv, "-s");
    if (argSteps)
        cfg->predictSteps = (size_t)strtol(argSteps, NULL, 10);

    printf("\n----------------------------------- RUNNING REQUESTED FORECAST -----------------------------------\n");
    printf("=====> STEPS TO PREDICT: %lu\n", cfg->predictSteps);
    if (cfg->predictSteps == 0) {
        printf("No steps to predict.\n");
        return 0;
    }

    int* predictions = malloc(sizeof(int) * cfg->predictSteps);
    double* conf = malloc(sizeof(double) * cfg->predictSteps);
    if (!predictions || !conf) {
        LOG_FATAL("malloc failed for either predictions or conf");
        return -1;
    }

    // Keep track of the lastState only
    int* lastState = malloc(sizeof(int) * states->order);
    memcpy(lastState, test + testSize - states->order, sizeof(int) * states->order);
    printf("Starting from last state (based on test set): ");
    printArr_i(lastState, states->order);

    // Predictions using Default Markov Chain
    markovPredict(tm, cfg->predictSteps, lastState, states->order, predictions, conf);
    printf("\n====> PREDICTIONS USING DEFAULT MARKOV CHAIN (acc: %lf): ", mkAcc);
    printArr_i(predictions, cfg->predictSteps);
    if (cfg->showConfidence) {
        double prop = 1.0;
        for (size_t i = 0; i < cfg->predictSteps; i++)
            prop *= conf[i];
        printf("=====> CONFIDENCE: ");
        printArr_d(conf, cfg->predictSteps);
        printf("=====> FINAL PROPAGATED CONFIDENCE: %lf\n", prop);
    }

    if (wait)
        enterWait();

    // Predictions using Markov Graph random walk
    if (cfg->useMarkovGraph && graph) {
        mkGraphRandWalk(graph, lastState, cfg->predictSteps, predictions, conf);

        printf("\n====> PREDICTIONS USING RANDOM WALK IN MARKOV GRAPH (acc: %lf): ", gAcc);
        printArr_i(predictions, cfg->predictSteps);
        if (cfg->showConfidence) {
            double prop = 1.0;
            for (size_t i = 0; i < cfg->predictSteps; i++)
                prop *= conf[i];
            printf("=====> CONFIDENCE: ");
            printArr_d(conf, cfg->predictSteps);
            printf("=====> FINAL PROPAGATED CONFIDENCE: %lf\n", prop);
        }
    }

    if (wait)
        enterWait();

    // Predictions using Markov Network
    if (cfg->useMarkovNetwork && net) {
        mkNetSetLastState(net, lastState);
        mkNetPredict(net, cfg->predictSteps, predictions, conf);

        printf("\n====> PREDICTIONS USING MARKOV NETWORK (acc: %lf): ", nAcc);
        printArr_i(predictions, cfg->predictSteps);
        if (cfg->showConfidence) {
            double prop = 1.0;
            for (size_t i = 0; i < cfg->predictSteps; i++)
                prop *= conf[i];
            printf("=====> CONFIDENCE: ");
            printArr_d(conf, cfg->predictSteps);
            printf("=====> FINAL PROPAGATED CONFIDENCE: %lf\n", prop);
        }
    }

    configFree(&cfg);
    mkGraphFree(&graph);
    mkNetFree(&net);
    markovFreeTransMatrix(&tm);
    markovFreeState(&states);
    free(predictions);
    free(conf);
    free(lastState);
    free(train);
    free(test);
    free(valid);
    free(unique);
    free(data);
    return 0;
}
