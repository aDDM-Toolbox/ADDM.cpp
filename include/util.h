#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <map>
#include <string>
#include <BS_thread_pool.hpp>
#include <nlohmann/json.hpp>
#include "addm.h"
#include "ddm.h"

using json = nlohmann::json;


extern vector<string> validComputeMethods;

/**
 * @brief Single entry in the experimental data CSV file. 
 * 
 */
struct expEntry {
    int parcode;
    int trial;
    int rt;
    int choice;
    int item_left;
    int item_right;
};

/**
 * @brief Single entry in the fixations CSV file. 
 * 
 */
struct fixEntry {
    int parcode;
    int trial;
    int fix_item;
    int fix_time;
};


/**
 * @brief Load experimental data from a single CSV file. Format expected is: 
 * 
 * trial, choice, rt, valueLeft, valueRight, fixItem, fixTime
 * 
 * @param filename Name of the CSV file. 
 * @return std::map<int, std::vector<aDDMTrial>> mapping subject IDs to each subject's 
 * corresponding aDDMTrials. 
 */
std::map<int, std::vector<aDDMTrial>> loadDataFromSingleCSV(
    std::string filename);

/**
 * @brief Load experimental data from two CSV files: an experimental data file and a fixations 
 * file. Format expected for experimental data file:
 * 
 * parcode, trial, rt, choice, valueLeft, valueRight
 * 
 * Format expected for fixations file: 
 * 
 * parcode, trial, fixItem, fixTime
 * 
 * @param expDataFilename Name of the experimental data trial. 
 * @param fixDataFilename Name of the fixations file. 
 * @return std::map<int, std::vector<aDDMTrial>> mapping subject IDs to each subject's 
 * corresponding aDDMTrials. 
 */
std::map<int, std::vector<aDDMTrial>> loadDataFromCSV(
    std::string expDataFilename,
    std::string fixDataFilename);

/**
 * @brief Create empirical distributions fro the data ot be used when generating model simulations.
 * 
 * @param data A mapping of subject IDs to vectors of each subject's corresponding aDDMTrials. 
 * @param timeStep Minimum duration of a fixation to be considered in milliseconds. 
 * @param maxFixTime Maximum duration of a fixation to be considered, in milliseconds. 
 * @param numFixDists Integer indicating the number of fixation types to use in the fixation 
 * distributions. I.e. if set to 3, then three separate fixation typyes will be used, 
 * corresponding to the first, second, and third fixation in each trial. 
 * @param valueDiffs List of integers corresponding to the available value differences between 
 * item. 
 * @param subjectIDs List of subject IDs to consider in the empirical data. If left empty, all 
 * subjectIDs will be used. 
 * @param useOddTrials Boolean indicating whether or not to use odd trials when creating the 
 * distributions. 
 * @param useEvenTrials Boolean indicating whether or not to use even trials when creating the 
 * distributions. 
 * @param useCisTrials Boolean indiciating whether or not to use cis trials when creating the 
 * distributions. Cis trials are defined as trials in which both items have either positive or 
 * negative values. 
 * @param useTransTrials Boolean indicating whether or not to use trans trials when creating the 
 * distributions. Trans trials are defined as trials in which one item has a positive value and 
 * one item has a negative value. 
 * @return FixationData object serving as a record of empirical fixation distributions. 
 */
FixationData getEmpiricalDistributions(
    std::map<int, std::vector<aDDMTrial>> data, 
    int timeStep=10, int maxFixTime=3000,
    int numFixDists=3, 
    std::vector<int> valueDiffs={-3,-2,-1,0,1,2,3},
    std::vector<int> subjectIDs={},
    bool useOddTrials=true, 
    bool useEvenTrials=true, 
    bool useCisTrials=true, 
    bool useTransTrials=true
    );

/**
 * @brief Print a matrix stored in nested-vector format. Utility function for debugging purposes.
 * 
 * @tparam T numerical type (double, float, int, etc...)
 * @param mat Nested vector to print.
 * @param name Name to print as a header for the matrix. 
 */
template <class T> 
void pmat(std::vector<std::vector<T>> mat, std::string name) {
    std::cout << name << std::endl;
    for (auto row : mat) {
        for (auto f : row) {
            std::cout << f;
            if (f >= 0 && f < 10) {
                std::cout << "  ";
            } else {
                std::cout << " ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "------" << std::endl;    
}

#endif