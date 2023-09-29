#include <iostream>
#include <vector> 
#include <fstream>
#include <addm/cpp_toolbox.h>

// Location of the aDDM simulations. 
const std::string SIMS = "results/addm_simulations.csv";
// Location to save the computed likelihoods to. 
const std::string SAVE = "results/addm_mle.csv";
// Parameter ranges. Change as desired. 
const std::vector<float> rangeD = {0.0035, 0.005, 0.0065, 0.008};
const std::vector<float> rangeSigma = {0.06, 0.065, 0.07, 0.075};
const std::vector<float> rangeTheta = {0.35, 0.5, 0.65, 0.8};
const std::vector<float> rangeK = {0, 0.5, 1};

int main() {
    // Load trials from a CSV. 
    std::vector<aDDMTrial> trials = aDDMTrial::loadTrialsFromCSV(SIMS);
    // Add additional arguments to specify computation mode, etc.. if desired. 
    MLEinfo info = aDDM::fitModelMLE(trials, rangeD, rangeSigma, rangeTheta, rangeK, "thread");
    std::cout << 
    "  Optimal Parameters  \n" << 
    "======================\n" <<
    "d      : " << info.optimal.d << "\n" << 
    "sigma  : " << info.optimal.sigma << "\n" << 
    "theta  : " << info.optimal.theta << "\n" << 
    "k      : " << info.optimal.k << std::endl;

    // Save computed likelihoods to a CSV. 
    std::ofstream fp; 
    fp.open(SAVE); 
    fp << "d,sigma,theta,p\n"; 
    for (auto &i : info.likelihoods) {
        fp << i.first.d << "," << i.first.sigma << "," << i.first.theta << "," << i.second << "\n"; 
    }
    fp.close();
}