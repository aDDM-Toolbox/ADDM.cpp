#include <iostream>
#include <vector> 
#include <random>
#include <fstream>
#include <addm/cpp_toolbox.h>

// Verify absence of any scaling issues. 

const std::string EXP_DATA = "data/expdata.csv";
const std::string FIX_DATA = "data/fixations.csv"; 

int timestep = 1; 
float ass = 0.001; 


int main() {
    std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(EXP_DATA, FIX_DATA);
    FixationData fixationData = getEmpiricalDistributions(data);
    aDDM addm = aDDM(0.005, 0.05, 0.5);
    aDDM addm2 = aDDM(0.0025, 0.05, 0.5);
    aDDM addm05 = aDDM(0.01, 0.05, 0.5);
    aDDMTrial trial = addm.simulateTrial(1, 3, fixationData, 1, 3, {}, {}, 50);
    aDDMTrial dbld_trial = trial; 
    aDDMTrial half_trial = trial; 
    dbld_trial.valueLeft *= 2; 
    dbld_trial.valueRight *= 2; 
    half_trial.valueLeft /= 2; 
    half_trial.valueRight /= 2; 
    
    double llh = addm.getTrialLikelihood(trial, timestep, ass);
    cout << "baseline likelihood         " << llh << endl; 

    double llhdbld_expected = addm2.getTrialLikelihood(dbld_trial, timestep, ass);
    cout << "doubled VD, expected model  " <<  llhdbld_expected << endl; 

    double llhhalf_expected = addm05.getTrialLikelihood(half_trial, timestep, ass);
    cout << "halved VD, expected model   " << llhhalf_expected << endl; 
}