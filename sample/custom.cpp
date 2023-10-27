#include <addm/cpp_toolbox.h>
#include <iostream>

// Location of the aDDM simulations. 
const std::string SIMS = "results/addm_simulations.csv";

int main() {
    /**
     * Create a model and add a custom parameter "W" (this has no real meaning and is for example
     * purposes). 
     * 
     */
    aDDM addm = aDDM(0.001, 0.02, 0.3);
    addm.addParameter("W", 5);
    std::cout << "W = " << addm["W"] << std::endl; 
    double likelihood = addm.getLikelihoodAlternative(aDDMTrial()); 
    std::cout << "Likelihood = " << likelihood << std::endl; 

    /**
     * We can perform model fitting over a space of custom parameters as well.
     * 
     */
    std::map<string, vector<float>> rangeOptional = {
        {"A", {0.1, 0.2, 0.3, 0.4}}, 
        {"B", {0.5, 0.6, 0.7}}, 
        {"C", {0.8, 0.9}}
    };
    // Load trials from a CSV. 
    std::vector<aDDMTrial> trials = aDDMTrial::loadTrialsFromCSV(SIMS);
    // rangeOp informs the model fitting algorithm to increase the dimension of the grid space to include
    // the possible values for our custom variables. 
    MLEinfo info = aDDM::fitModelMLE(
        trials, {0.005}, {0.07}, {0.5}, {0}, "thread", false, 1, 0, 10, 0.1, {0}, {0}, true, rangeOptional);

}