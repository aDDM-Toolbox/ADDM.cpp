#include <iostream>
#include <vector> 
#include <fstream>
#include <addm/cpp_toolbox.h>

// Location of the aDDM simulations. 
const std::string SIMS = "results/addm_simulations.csv";

int main() {
    // Load trials from a CSV. 
    std::vector<aDDMTrial> trials = aDDMTrial::loadTrialsFromCSV(SIMS);
    // Add additional arguments to specify computation mode, etc.. if desired.
    MLEinfo<aDDM> info = aDDM::fitModelCSV(trials, "test.csv", "thread");
    std::cout << 
    "  Optimal Parameters  \n" << 
    "======================\n" <<
    "d      : " << info.optimal.d << "\n" << 
    "sigma  : " << info.optimal.sigma << "\n" << 
    "theta  : " << info.optimal.theta << "\n" << 
    "k      : " << info.optimal.k << std::endl;
}