#include <addm/cpp_toolbox.h>
#include <iostream> 
#include <random>
#include <fstream>

// Experimental and Fixation data. 
const std::string EXP_DATA = "data/expdata.csv";
const std::string FIX_DATA = "data/fixations.csv"; 
// Sample value differences.
const std::vector<int> vals = {-16, -14, -12, -10, -8, -6, -4, -2, 0, 2, 4, 6, 8, 10, 12, 14, 16};

int main() {
    int N = 1000; 
    float d = 0.004; 
    float sigma = 0.04; 
    float theta = 0.4; 

    std::vector<float> rangeD = {0.001, 0.002, 0.003, 0.004, 0.005, 0.006, 0.007, 0.008, 0.009, 0.010};
    std::vector<float> rangeSigma = {0.01, 0.02, 0.03, 0.04, 0.05, 0.06, 0.07, 0.08, 0.09, 0.1};
    std::vector<float> rangeTheta = {0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};

    std::vector<aDDMTrial> trials;
    std::vector<aDDMTrial> halfTrials; 
    std::vector<aDDMTrial> doubleTrials; 

    // Read the empirical fixation data. 
    cout << "reading empirical data..." << endl;
    std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(EXP_DATA, FIX_DATA);
    FixationData fixationData = getEmpiricalDistributions(data);
    
    // Create an aDDM with the specified parameters. 
    aDDM addm = aDDM(d, sigma, theta);

    // Create N trials witha random values.       
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<std::size_t> distribution(0, vals.size() - 1);
    for (int i = 0; i < N; i++) {
        int rIDX = distribution(generator);
        int lIDX = distribution(generator); 

        int valueRight = vals.at(rIDX);
        int valueLeft = vals.at(lIDX);

        aDDMTrial adt = addm.simulateTrial(valueLeft, valueRight, fixationData);
        aDDMTrial adtHalf = aDDMTrial(adt.RT, adt.choice, 
            int (adt.valueLeft * 0.5), int (adt.valueRight * 0.5), 
            adt.fixItem, adt.fixTime, adt.fixRDV, adt.uninterruptedLastFixTime);
        aDDMTrial adtDouble = aDDMTrial(adt.RT, adt.choice, 
            adt.valueLeft * 2, adt.valueRight * 2, 
            adt.fixItem, adt.fixTime, adt.fixRDV, adt.uninterruptedLastFixTime);
        trials.push_back(adt);
        halfTrials.push_back(adtHalf);
        doubleTrials.push_back(adtDouble);
    }

    cout << trials.size() << " trials." << endl; 

    MLEinfo<aDDM> info = aDDM::fitModelMLE(trials, rangeD, rangeSigma, rangeTheta, {0}, "thread", true, 1.0, 0, 10, 0.01); 
    cout << "STD done."  << endl; 
    MLEinfo<aDDM> halfInfo = aDDM::fitModelMLE(halfTrials, rangeD, rangeSigma, rangeTheta, {0}, "thread", true, 1.0, 0, 10, 0.01); 
    cout << "HLF done." << endl; 
    MLEinfo<aDDM> doubleInfo = aDDM::fitModelMLE(doubleTrials, rangeD, rangeSigma, rangeTheta, {0}, "thread", true, 1.0, 0, 10, 0.01);
    cout << "DBL done." << endl; 
    std::cout << info.optimal.d << " " << info.optimal.sigma << " " << info.optimal.theta << std::endl; 
    std::cout << halfInfo.optimal.d << " " << halfInfo.optimal.sigma << " " << halfInfo.optimal.theta << std::endl; 
    std::cout << doubleInfo.optimal.d << " " << doubleInfo.optimal.sigma << " " << doubleInfo.optimal.theta << std::endl; 

    std::string csvHeader = "d,sigma,theta,p\n"; 
    std::ofstream fp; 
    fp.open("results/scaling_std.csv"); 
    fp << csvHeader; 
    for (auto &i : info.likelihoods) {
        fp << i.first.d << "," << i.first.sigma << "," << i.first.theta << "," << i.second << "\n"; 
    }
    fp.close();

    fp.open("results/scaling_half.csv");
    fp << csvHeader; 
    for (auto &i : halfInfo.likelihoods) {
        fp << i.first.d << "," << i.first.sigma << "," << i.first.theta << "," << i.second << "\n";
    }
    fp.close(); 

    fp.open("results/scaling_double.csv");
    fp << csvHeader; 
    for (auto &i : doubleInfo.likelihoods) {
        fp << i.first.d << "," << i.first.sigma << "," << i.first.theta << "," << i.second << "\n";
    }
    fp.close(); 
}

// Run through DDM and not aDDM. 