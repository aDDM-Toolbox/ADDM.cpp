#include <iostream>
#include <vector> 
#include <random>
#include <fstream>
#include <addm/cpp_toolbox.h>

// Experimental and Fixation data. 
const std::string EXP_DATA = "data/expdata.csv";
const std::string FIX_DATA = "data/fixations.csv"; 
// Location to save the aDDM trials. 
const std::string SAVE = "results/addm_simulations.csv";
// Sample value differences. Change if deisred. 
const std::vector<float> valDiffs = {-3, -2, -1, 0, 1, 2, 3};

/**
 * Example Usage: 
 * 
 * bin/addm_simulate 1000 0.005 0.07 0.5
 */
int main(int argc, char** argv) {
    int N; 
    float d; 
    float sigma; 
    float theta; 

    // Check the input arguments are valid. 
    if (argc != 5) {
        std::cerr << "Provide 3 arguments." << std::endl;
        exit(1);
    }
    try {
        N = stoi(argv[1]);
    } catch (invalid_argument &e) {
        std::cerr << "Input N not convertable to int: " << argv[1] << std::endl; 
        exit(1);
    }
    try {
        d = stof(argv[2]);
    } catch (invalid_argument &e) {
        std::cerr << "Input d not convertable to float: " << argv[2] << std::endl; 
        exit(1);
    }
    try {
        sigma = stof(argv[3]);
    } catch (invalid_argument &e) {
        std::cerr << "Input sigma not convertable to float: " << argv[3] << std::endl;  
        exit(1);
    }
    try {
        theta = stof(argv[4]);
    } catch (invalid_argument &e) {
        std::cerr << "Input theta not convertable to float: " << argv[4] << std::endl; 
        exit(1); 
    }

    std::cout << "Performing " << N << " trials." << std::endl; 
    std::cout << "d=" << d << std::endl; 
    std::cout << "sigma=" << sigma << std::endl; 
    std::cout << "theta=" << theta << std::endl; 

    std::vector<aDDMTrial> trials;
    srand(time(NULL));

    // Read the empirical fixation data. 
    std::cout << "reading empirical data..." << std::endl;
    std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(EXP_DATA, FIX_DATA);
    FixationData fixationData = getEmpiricalDistributions(data);
    
    // Create an aDDM with the specified parameters. 
    aDDM addm = aDDM(d, sigma, theta);

    // Create N trials witha  random value difference.         
    std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<std::size_t> distribution(0, valDiffs.size() - 1);
    for (int i = 0; i < N; i++) {
        int rIDX = distribution(generator);

        int valDiff = valDiffs.at(rIDX);
        float valueLeft = 3;
        float valueRight = valueLeft - valDiff;
        aDDMTrial adt = addm.simulateTrial(valueLeft, valueRight, fixationData);
        trials.push_back(adt);
    }

    // Write trials to a CSV. 
    aDDMTrial::writeTrialsToCSV(trials, "results/addm_simulations.csv");
}