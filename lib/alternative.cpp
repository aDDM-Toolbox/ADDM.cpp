#include "addm.h"
#include "util.h"
#include "stats.h"

double aDDM::getLikelihoodAlternative(aDDMTrial trial, int timeStep, float approxStateStep) {
    // EXAMPLE CODE! 
    try {
        return 1 / this->optionalParams["W"];
    } catch (exception e) {
        return 1; 
    }
}