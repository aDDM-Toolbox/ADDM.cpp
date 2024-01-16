#include <iostream>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <time.h>
#include <cstdlib>
#include <random> 
#include "ddm.h"
#include "util.h"
#include "addm.h"
#include "stats.h"


std::function<ProbabilityData(aDDM)> NLLcomputer; 


FixationData::FixationData(float probFixLeftFirst, std::vector<int> latencies, 
    std::vector<int> transitions, fixDists fixations) {

    this->probFixLeftFirst = probFixLeftFirst;
    this->latencies = latencies;
    this->transitions = transitions;
    this->fixations = fixations;
}


aDDMTrial::aDDMTrial(
    unsigned int RT, int choice, int valueLeft, int valueRight, 
    std::vector<int> fixItem, std::vector<int> fixTime, 
    std::vector<float> fixRDV, float uninterruptedLastFixTime) :
    DDMTrial(RT, choice, valueLeft, valueRight) {
        this->fixItem = fixItem;
        this->fixTime = fixTime;
        this->fixRDV = fixRDV;
        this->uninterruptedLastFixTime = uninterruptedLastFixTime;
}


aDDM::aDDM(float d, float sigma, float theta, float k, float barrier, 
    unsigned int nonDecisionTime, float bias, float decay) : 
    DDM(d, sigma, barrier, nonDecisionTime, bias, decay) {
        this->theta = theta;
        this->k = k; 
}

void aDDM::exportTrial(aDDMTrial adt, std::string filename) {
    std::ofstream o(filename);
    json j;
    j["d"] = d;
    j["sigma"] = sigma;
    j["theta"] = theta;
    j["k"] = k; 
    j["barrier"] = barrier;
    j["NDT"] = nonDecisionTime;
    j["bias"] = bias;
    j["RT"] = adt.RT;
    j["choice"] = adt.choice;
    j["vl"] = adt.valueLeft;
    j["vr"] = adt.valueRight;
    j["RDVs"] = adt.RDVs;
    j["fixItem"] = adt.fixItem;
    j["fixTime"] = adt.fixTime;
    j["timeStep"] = adt.timeStep;
    o << std::setw(4) << j << std::endl;        
}


double aDDM::getTrialLikelihood(aDDMTrial trial, int timeStep, float approxStateStep) {
    bool debug = false; 
    if (debug) {
        std::cout << std::setprecision(6) << std::fixed;
    }
    // Discount any non-decision time if greater than 0. 
    std::vector<int> correctedFixItem = trial.fixItem;
    std::vector<int> correctedFixTime = trial.fixTime;
    if (this->nonDecisionTime > 0) {
        int remainingNDT = this->nonDecisionTime;
        assert(trial.fixItem.size() == trial.fixTime.size());
        for (int i = 0; i < trial.fixItem.size(); i++) { 
            int fItem = trial.fixItem[i];
            int fTime = trial.fixTime[i];
            if (remainingNDT > 0) {
                correctedFixItem.push_back(0);
                correctedFixTime.push_back(min(remainingNDT, fTime));
                fTime -= remainingNDT; 
                correctedFixItem.push_back(fItem);
                correctedFixTime.push_back(max(fTime, 0));
                remainingNDT -= fTime; 
            } else {
                correctedFixItem.push_back(fItem);
                correctedFixTime.push_back(fTime);
            }
        }
    }

    if (debug) {
        std::cout << "CFI" << std::endl;
        for (int i : correctedFixItem) {
            std::cout << i << std::endl;
        }
        std::cout << "------" << std::endl;
        std::cout << "CFT" << std::endl;
        for (int t : correctedFixTime) {
            std::cout << t << std::endl;
        }
        std::cout << "------" << std::endl;
    }
    
    // Get number of timesteps for this trial 
    int numTimeSteps = 0;
    for (int fTime : correctedFixTime) {
        numTimeSteps += fTime / timeStep;
    }
    if (numTimeSteps < 1) {
        throw std::invalid_argument("Trial response time is smaller than time step");
    }
    numTimeSteps++;

    // Values of barriers over time 
    std::vector<float> barrierUp(numTimeSteps);
    std::vector<float> barrierDown(numTimeSteps);
    std::fill(barrierUp.begin(), barrierUp.end(), this->barrier);
    std::fill(barrierDown.begin(), barrierDown.end(), -this->barrier);
    if (this->decay != 0) {
        for (int i = 1; i < numTimeSteps; i++) {
            barrierUp.at(i) = this->barrier / (1 + (this->decay * i));
            barrierDown.at(i) = -this->barrier / (1 + (this->decay * i));
        }
    } 
    
    // Obtain the correct state step
    int halfNumStateBins = ceil(this->barrier / approxStateStep); 
    float stateStep = this->barrier / (halfNumStateBins + 0.5);
    std::vector<float> states;
    for (float ss = barrierDown.at(0) + (stateStep / 2); ss <= barrierUp.at(0) - (stateStep / 2); ss += stateStep) {
        states.push_back(ss);
    }

    // Get index of the bias state
    float biasStateVal = MAXFLOAT;
    int biasState = 0;
    for (int i = 0; i < states.size(); i++) {
        float r = abs(states.at(i) - this->bias);
        if (r < biasStateVal) {
            biasState = i;
            biasStateVal = r;
        }
    }

    // Initialize an empty probability state grid
    std::vector<std::vector<double>> prStates; // prStates[state][timeStep]
    for (int i = 0; i < states.size(); i++) {
        prStates.push_back({});
        for (int j = 0; j < numTimeSteps; j++) {
            prStates.at(i).push_back(0);
        }
    }

    // Initialize vectors corresponding to the probability of crossing the 
    // top or bottom barriers at each timestep. 
    std::vector<double> probUpCrossing; 
    std::vector<double> probDownCrossing;
    for (int i = 0; i < numTimeSteps; i++) {
        probUpCrossing.push_back(0);
        probDownCrossing.push_back(0);
    }
    prStates.at(biasState).at(0) = 1; 

    int time = 1;

    // Initialize a change matrix where each value at (i, j) 
    // indicates the difference between states[i] and states[j] 
    std::vector<std::vector<float>> changeMatrix(states.size(), std::vector<float>(states.size())); 
    for (size_t i = 0; i < states.size(); i++) {
        for (size_t j = 0; j < states.size(); j++) {
            changeMatrix[i][j] = states[i] - states[j];
        }
    }
    if (debug) {
        pmat<float>(changeMatrix, "CHANGE MATRIX");
    }

    // Distance from every state to the top barrier at each timestep
    std::vector<std::vector<float>> changeUp(states.size(), std::vector<float>(numTimeSteps));
    for (size_t i = 0; i < states.size(); i++) {
        for (size_t j = 0; j < numTimeSteps; j++) {
            changeUp[i][j] = barrierUp[j] - states[i];
        }
    }
    if (debug) {
        pmat<float>(changeUp, "CHANGE UP");
    }


    // Distance from every state to the bottom barrier at each timestep
    std::vector<std::vector<float>> changeDown(states.size(), std::vector<float>(numTimeSteps));
    for (size_t i = 0; i < states.size(); i++) {
        for (size_t j = 0; j < numTimeSteps; j++) {
            changeDown[i][j] = barrierDown[j] - states[i];
        }
    }
    if (debug) {
        pmat<float>(changeDown, "CHANGE DOWN");
    }

    assert(correctedFixItem.size() == correctedFixTime.size());

    std::map<float, std::vector<std::vector<double>>> meansToPDCMs; 

    // Iterate over all fixations in the trial 
    std::vector<std::vector<double>> probDistChangeMatrix(states.size(), std::vector<double>(states.size()));
    std::vector<float> currChangeUp;            
    std::vector<double> changeUpCDFs;
    std::vector<float> currChangeDown;
    std::vector<double> changeDownCDFs;
    for (int c = 0; c < correctedFixItem.size(); c++) {
        int fItem = correctedFixItem[c];
        int fTime = correctedFixTime[c];

        if (debug) {
            std::cout << "============" << std::endl;
            std::cout << "fItem " << c << ": " << fItem << std::endl;
            std::cout << "fTime " << c << ": " << fTime << std::endl;
            std::cout << "============" << std::endl;
        }

        float mean;
        if (fItem == 1) {
            mean = this->d * ((trial.valueLeft + this->k) - (this->theta * trial.valueRight));
        }
        else if (fItem == 2) {
            mean = this->d * ((this->theta * trial.valueLeft) - (trial.valueRight + this->k));
        }
        else {
            mean = 0; 
        }

        if (meansToPDCMs.count(mean)) {
            probDistChangeMatrix = meansToPDCMs.at(mean);
        } else {
            for (size_t i = 0; i < states.size(); i++) {
                for (size_t j = 0; j < states.size(); j++) {
                    float x = changeMatrix[i][j];
                    probDistChangeMatrix[i][j] = probabilityDensityFunction(mean, this->sigma, x);
                }
            }
            meansToPDCMs.insert({mean, probDistChangeMatrix});
        }
        if (debug) {
            pmat<double>(probDistChangeMatrix, "PROBABILITY CHANGE MATRIX");
        }

        // Compute the probabilities of crossing the up and down barriers. This is given by: 
        // sum over all states s (
        //   probability of being in s at [t - 1] * probability of crossing barrier at [t]
        // )
        if (this->decay == 0) {
            for (auto s : changeUp) {
                currChangeUp.push_back(s.at(time));
            }
            for (int i = 0; i < currChangeUp.size(); i++) {
                float x = currChangeUp[i];
                changeUpCDFs.push_back(
                    1 - cumulativeDensityFunction(mean, this->sigma, x)
                );
            }
            for (auto s: changeDown) {
                currChangeDown.push_back(s.at(time));
            }
            for (int i = 0; i < currChangeDown.size(); i++) {
                float x = currChangeDown[i];
                changeDownCDFs.push_back(
                    cumulativeDensityFunction(mean, this->sigma, x)
                );
            }
        }

        for (int t = 0; t < fTime / timeStep; t++) {
            // Fetch the probability states for the previous timeStep
            std::vector<double> prTimeSlice(states.size());
            for (size_t i = 0; i < states.size(); i++) {
                prTimeSlice[i] = prStates[i][time - 1];
            }

            // Compute the dot product between the change matrix and previous timeStep's probabilities
            std::vector<double> prStatesNew(states.size());
            for (size_t i = 0; i < states.size(); i++) {
                double row_sum = 0; 
                for (size_t j = 0; j < states.size(); j++) {
                    row_sum += stateStep * probDistChangeMatrix[i][j] * prTimeSlice[j];
                }
                prStatesNew[i] = row_sum;
            }

            // Check for states that are now out-of-bounds based on decay
            for (int i = 0; i < states.size(); i++) {
                if (states[i] > barrierUp[time] || states[i] < barrierDown[time]) {
                    prStatesNew[i] = 0;
                }
            }
            if (debug) {
                std::cout << "PR STATES NEW" << std::endl;
                for (double d : prStatesNew) {
                    std::cout << d << std::endl;
                }
                std::cout << "------" << std::endl;
            }

            if (this->decay != 0) {
                for (auto s : changeUp) {
                    currChangeUp.push_back(s.at(time));
                }
                for (int i = 0; i < currChangeUp.size(); i++) {
                    float x = currChangeUp[i];
                    changeUpCDFs.push_back(
                        1 - cumulativeDensityFunction(mean, this->sigma, x)
                    );
                }            
                for (auto s: changeDown) {
                    currChangeDown.push_back(s.at(time));
                }
                for (int i = 0; i < currChangeDown.size(); i++) {
                    float x = currChangeDown[i];
                    changeDownCDFs.push_back(
                        cumulativeDensityFunction(mean, this->sigma, x)
                    );
                }
            }

            double tempUpCross = 0;
            for (int i = 0; i < prTimeSlice.size(); i++) {
                tempUpCross += changeUpCDFs[i] * prTimeSlice[i];
            }
            double tempDownCross = 0;
            for (int i = 0; i < prTimeSlice.size(); i++) {
                tempDownCross += changeDownCDFs[i] * prTimeSlice[i];
            }

            // Renormalize to cope with numerical approximations. 
            double sumIn = 0; 
            for (double prob : prTimeSlice) {
                sumIn += prob; 
            }
            double sumCurrent = tempUpCross + tempDownCross; 
            for (double prob : prStatesNew) {
                sumCurrent += prob;
            }
            double normFactor = sumIn / sumCurrent; 
            for (int i = 0; i < prStatesNew.size(); i++) {
                prStatesNew[i] *= normFactor;
            }
            tempUpCross *= normFactor;
            tempDownCross *= normFactor; 
            for (int i = 0; i < prStates.size(); i++) {
                prStates[i][time] = prStatesNew[i];
            }
            probUpCrossing[time] = tempUpCross; 
            probDownCrossing[time] = tempDownCross;

            time++;
        }
    }
    // Compute the likelihood based on the final choice. 
    double likelihood = 0;
    if (trial.choice == -1) {
        if (probUpCrossing[probUpCrossing.size() - 1] > 0) {
            likelihood = probUpCrossing[probUpCrossing.size() - 1];
        }
    }
    else if (trial.choice == 1) {
        if (probDownCrossing[probDownCrossing.size() - 1] > 0) {
            likelihood = probDownCrossing[probDownCrossing.size() - 1];
        }
    }
    // Return a non-zero value to prevent log(0)
    if (likelihood == 0) {
        likelihood = pow(10, -20);
    }
    return likelihood;
}


aDDMTrial aDDM::simulateTrial(
    int valueLeft, int valueRight, FixationData fixationData, int timeStep, 
    int numFixDists, fixDists fixationDist, vector<int> timeBins, int seed) {

    std::vector<int> fixItem;
    std::vector<int> fixTime;
    std::vector<float> fixRDV;

    // If no seed is provided (default -1), use a random generator. Otherwise, use the seed
    std::random_device rd;
    std::mt19937 gen(seed == -1 ? rd() : seed); 

    float RDV = this->bias;
    int time = 0;
    int choice; 
    int uninterruptedLastFixTime;
    int RT;

    std::vector<float>RDVs = {RDV};

    std::uniform_int_distribution<std::size_t> ludist(0, fixationData.latencies.size() - 1);
    int rIDX = ludist(gen);
    int latency = fixationData.latencies.at(rIDX);
    int remainingNDT = this->nonDecisionTime - latency;

    // Iterate over latency
    for (int t = 0; t < latency / timeStep; t++) {
        std::normal_distribution<float> ndist(0, this->sigma);
        float inc = ndist(gen);
        RDV += inc;
        RDVs.push_back(RDV);

        if(RDV >= this->barrier || RDV <= -this->barrier) {
            if (RDV >= this->barrier) {
                choice = -1;
            } else {
                choice = 1; 
            }
            fixRDV.push_back(RDV);
            fixItem.push_back(0);
            int dt = (t + 1) * timeStep;
            fixTime.push_back(dt);
            time += dt;
            RT = time;
            uninterruptedLastFixTime = latency;
            return aDDMTrial(
                RT, choice, valueLeft, valueRight, 
                fixItem, fixTime, fixRDV, uninterruptedLastFixTime);
        }
    }

    // Add latency to this trial's data
    fixRDV.push_back(RDV);
    RDVs.push_back(RDV);
    fixItem.push_back(0);
    int dt = latency - (latency % timeStep);
    fixTime.push_back(dt);
    time += dt;

    int fixNumber = 1;
    int prevFixatedItem = -1;
    int currFixLocation = 0;
    bool decisionReached = false;
    float currFixTime;

    while (true) {
        if (currFixLocation == 0) {
            // Sample based off of item location
            if (prevFixatedItem == -1) {
                std::discrete_distribution<> ddist({fixationData.probFixLeftFirst, 1 - fixationData.probFixLeftFirst});
                currFixLocation = ddist(gen) + 1;
            } else if (prevFixatedItem == 1) {
                currFixLocation = 2;
            } else if (prevFixatedItem == 2) {
                currFixLocation = 1;
            }
            prevFixatedItem = currFixLocation;
            if (fixationDist.empty()) {
                vector<float> fixTimes = fixationData.fixations.at(fixNumber);
                std::uniform_int_distribution<std::size_t> fudist(0, fixTimes.size() - 1);
                rIDX = fudist(gen);
                currFixTime = fixTimes.at(rIDX);
            }
            if (fixNumber < numFixDists) {
                fixNumber++;
            }
        }
        // Transition
        else {
            currFixLocation = 0;
            rIDX = rand() % fixationData.transitions.size();
            currFixTime = fixationData.transitions.at(rIDX);
        }
        // Iterate over any remaining non-decision time 
        if (remainingNDT > 0)  {
            for (int t = 0; t < remainingNDT / timeStep; t++) {
                std::normal_distribution<float> ndist(0, this->sigma);
                float inc = ndist(gen);
                RDV += inc;
                RDVs.push_back(RDV);

                if(RDV >= this->barrier || RDV <= -this->barrier) {
                    if (RDV >= this->barrier) {
                        choice = -1;
                    } else {
                        choice = 1; 
                    }
                    fixRDV.push_back(RDV);
                    fixItem.push_back(currFixLocation);
                    int dt = (t + 1) * timeStep;
                    fixTime.push_back(dt);
                    time += dt;
                    RT = time;
                    uninterruptedLastFixTime = currFixTime;
                    decisionReached = true;
                    break;
                }
            }
        }
        if (decisionReached) {
            break;
        }
        float remainingFixTime = max(0.0f, currFixTime - max(0, remainingNDT));
        remainingNDT -= currFixTime;

        for (int t = 0; t < round(remainingFixTime / timeStep); t++) {
            float mean;
            if (currFixLocation == 0) {
                mean = 0;
            } else if (currFixLocation == 1) {
                mean = this->d * ((valueLeft + this->k) - (this->theta * valueRight));
            } else if (currFixLocation == 2) {
                mean = this->d * ((this->theta * valueLeft) - (valueRight + this->k));
            }
            std::normal_distribution<float> ndist(mean, this->sigma);
            float inc = ndist(gen);
            RDV += inc;
            RDVs.push_back(RDV); 

            if(RDV >= this->barrier || RDV <= -this->barrier) {
                if (RDV >= this->barrier) {
                    choice = -1;
                } else {
                    choice = 1; 
                }
                fixRDV.push_back(RDV);
                fixItem.push_back(currFixLocation);
                int dt = (t + 1) * timeStep;
                fixTime.push_back(dt);
                time += dt;
                RT = time;
                uninterruptedLastFixTime = currFixTime;
                decisionReached = true;
                break;
            }                
        }

        if (decisionReached) {
            break;
        }

        fixRDV.push_back(RDV);
        fixItem.push_back(currFixLocation);
        int cft = round(currFixTime);
        int dt = cft - (cft % timeStep);
        fixTime.push_back(dt);
        time += dt;
    } 

    aDDMTrial trial = aDDMTrial(RT, choice, valueLeft, valueRight, fixItem, fixTime, fixRDV, uninterruptedLastFixTime);
    trial.RDVs = RDVs;
    trial.timeStep = timeStep;
    return trial;
}


ProbabilityData aDDM::computeParallelNLL(std::vector<aDDMTrial> trials, int timeStep, float approxStateStep, bool useAlternative) {
    ProbabilityData datasetTotals = ProbabilityData(0, 0);
    BS::thread_pool pool;
    std::vector<double> trialLikelihoods(trials.size());
    BS::multi_future<ProbabilityData> futs = pool.parallelize_loop(
        0, trials.size(), 
        [this, &trials, timeStep, approxStateStep, &trialLikelihoods, useAlternative](const int a, const int b) {
            ProbabilityData aux = ProbabilityData(0, 0);
            if (!useAlternative) {
                for (int i = a; i < b; ++i) {
                    double prob = this->getTrialLikelihood(trials[i], timeStep, approxStateStep);
                    trialLikelihoods[i] = prob; 
                    aux.likelihood += prob; 
                    aux.NLL += -log(prob);
                }
            } else {
                for (int i = a; i < b; ++i) {
                    double prob = this->getLikelihoodAlternative(trials[i], timeStep, approxStateStep);
                    trialLikelihoods[i] = prob; 
                    aux.likelihood += prob; 
                    aux.NLL += -log(prob);
                }                
            }
            return aux;
        }
    );
    std::vector<ProbabilityData> totals = futs.get();
    for (const ProbabilityData t : totals) {
        datasetTotals.NLL += t.NLL;
        datasetTotals.likelihood += t.likelihood; 
    }
    datasetTotals.trialLikelihoods = trialLikelihoods; 
    return datasetTotals;
}


void aDDMTrial::writeTrialsToCSV(std::vector<aDDMTrial> trials, string filename) {
    std::ofstream fp;
    fp.open(filename);
    fp << "trial,choice,rt,valueLeft,valueRight,fixItem,fixTime\n";
    int id = 0; 

    for (aDDMTrial adt : trials) {
        assert(adt.fixItem.size() == adt.fixTime.size());
        for (int i = 0; i < adt.fixItem.size(); i++) {
            fp << id << "," << adt.choice << "," << adt.RT << "," << 
                adt.valueLeft << "," << adt.valueRight << "," <<
                adt.fixItem[i] << "," << adt.fixTime[i] << "\n";
        }
        id++;
    }
    fp.close();    
}


vector<aDDMTrial> aDDMTrial::loadTrialsFromCSV(string filename) {
    std::vector<aDDMTrial> trials; 
    std::vector<aDDM> addms;
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);

    int ID;
    int choice; 
    int RT; 
    int valueLeft;
    int valueRight;
    int prevID;
    int fItem;
    int fTime; 
    bool firstIter = true; 
    std::vector<int> fixItem;
    std::vector<int> fixTime; 

    aDDMTrial adt = aDDMTrial();
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string field;
        std::getline(ss, field, ',');
        ID = std::stoi(field);
        std::getline(ss, field, ',');
        choice = std::stoi(field);
        std::getline(ss, field, ',');
        RT = std::stoi(field);
        std::getline(ss, field, ',');
        valueLeft = std::stoi(field);
        std::getline(ss, field, ',');
        valueRight = std::stoi(field);
        std::getline(ss, field, ',');
        fItem = std::stoi(field);
        std::getline(ss, field, ',');
        fTime = std::stoi(field);
        if (ID == prevID && !firstIter) {
            adt.fixItem.push_back(fItem);
            adt.fixTime.push_back(fTime);
        } else {
            if (firstIter) {
                firstIter = false; 
            } else {
                trials.push_back(adt);
            }
            adt = aDDMTrial(RT, choice, valueLeft, valueRight);
            adt.fixItem.push_back(fItem);
            adt.fixTime.push_back(fTime);

        }
        prevID = ID;
    }
    trials.push_back(adt);
    file.close();
    return trials;
}

MLEinfo<aDDM> aDDM::computeFitAndPosteriors(
    std::vector<aDDM> potentialModels, bool normalizePosteriors, int numTrials) {

    std::map<aDDM, ProbabilityData> allTrialLikelihoods; 
    std::map<aDDM, float> posteriors; 
    double numModels = potentialModels.size();
    /**
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * MLE IS PERFORMED HERE! 
     * Feel free to disregard the posteriors when making modifications
     * if you do not plan on using them in analyses. 
     * 
     * Example code without posterior computation for MLE: 
     * 
     * double minNLL = __DBL_MAX__; 
     * aDDM optimal = aDDM(); 
     * for (aDDM addm : potentialModels) {
     *     ProbabilityData aux = NLLcomputer(addm);
     *     if (aux.NLL < minNLL) {
     *         minNLL = aux.NLL; 
     *         optimal = addm; 
     *     }
     * }
     * 
     */
    double minNLL = __DBL_MAX__; 
    aDDM optimal = aDDM(); 
    for (aDDM addm : potentialModels) {
        ProbabilityData aux = NLLcomputer(addm);
        if (normalizePosteriors) {
            allTrialLikelihoods.insert({addm, aux});
            posteriors.insert({addm, 1 / numModels});
        } else {
            posteriors.insert({addm, aux.NLL});
        }
        if (aux.NLL < minNLL) {
            minNLL = aux.NLL; 
            optimal = addm; 
        }
    }
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    if (normalizePosteriors) {
        for (int tn = 0; tn < numTrials; tn++) {
            double denominator = 0; 
            for (const auto &addmPD : allTrialLikelihoods) {
                aDDM curr = addmPD.first; 
                ProbabilityData data = addmPD.second; 
                double likelihood = data.trialLikelihoods[tn];
                denominator += posteriors[curr] * likelihood; 
            }
            double sum = 0; 
            for (const auto &addmPD : allTrialLikelihoods) {
                aDDM curr = addmPD.first; 
                ProbabilityData data = addmPD.second; 
                double prior = posteriors[curr];
                double newLikelihoood = data.trialLikelihoods[tn] * prior / denominator; 
                posteriors[curr] = newLikelihoood; 
                sum += newLikelihoood;
            }
            if (sum != 1) {
                double normalizer = 1 / sum; 
                for (auto &p : posteriors) {
                    p.second *= normalizer; 
                }
            }
        }
    }
    MLEinfo<aDDM> info;
    info.optimal = optimal; 
    info.likelihoods = posteriors; 
    return info;   
}

MLEinfo<aDDM> aDDM::fitModelCSV(
    std::vector<aDDMTrial> trials, 
    std::string filename, 
    std::string computeMethod, 
    bool normalizePosteriors, 
    float barrier, 
    unsigned int nonDecisionTime, 
    int timeStep, 
    float approxStateStep, 
    bool useAlternative) {

    if (std::find(
        validComputeMethods.begin(), validComputeMethods.end(), 
        computeMethod) == validComputeMethods.end()) {
        throw std::invalid_argument("Input computeMethod is invalid.");
    }

    // default parameters handled in built-in aDDM implementation
    vector<string> validParams = {"d", "sigma", "theta", "k", "bias", "decay"};

    // trim whitespace from headers
    auto ltrim = [](string &str) {
        str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](char ch) {
            return !std::isspace<char>(ch, std::locale::classic());
        }));
    };
    auto rtrim = [](string &str) {
        str.erase(std::find_if(str.rbegin(), str.rend(), [](char ch) {
            return !std::isspace<char>(ch, std::locale::classic());
        }).base(), str.end());
    };

    // read in the headers
    std::ifstream paramsFile(filename);
    string paramNames; 
    std::getline(paramsFile, paramNames);
    std::stringstream ss(paramNames);    
    
    // convert headers to an array and check if they are implemented
    vector<string> headers;
    vector<bool> isBuiltInParam; 
    while(ss.good()) {
        string curr;
        getline(ss, curr, ',');
        ltrim(curr);
        rtrim(curr);
        headers.push_back(curr);      

        std::transform(curr.begin(), curr.end(), curr.begin(),
            [](unsigned char c){ return std::tolower(c); });
        if (std::find(
            validParams.begin(), validParams.end(), curr) != validParams.end()) {
            isBuiltInParam.push_back(true);
        }
        else {
            isBuiltInParam.push_back(false);
        }
    }

    string line; 
    vector<float> currParams; 
    vector<aDDM> potentialModels; 
    while (std::getline(paramsFile, line)) { // Read each line in the CSV
        ss.clear(); 
        ss.str(line);
        aDDM addm = aDDM(0, 0, 0, 0, barrier, nonDecisionTime, 0, 0);

        int col = 0; 
        while (ss.good()) {
            string curr; 
            float curr_f; 
            getline(ss, curr, ',');
            try {
               curr_f = stof(curr);
            }
            catch (invalid_argument &e) {
                throw std::invalid_argument(
                    string("Non-float entry provided: ") + curr);
            }
            if (col >= headers.size()) {
                throw std::invalid_argument("Row with too many entries provided");
            }
            if (isBuiltInParam[col]) {
                string currHeader = headers[col];
                if (currHeader == "d") {
                    addm.d = curr_f;
                } else if (currHeader == "sigma") {
                    addm.sigma = curr_f;
                } else if (currHeader == "theta") {
                    addm.theta = curr_f;
                } else if (currHeader == "k") {
                    addm.k = curr_f; 
                } else if (currHeader == "bias") {
                    addm.bias = curr_f;
                } else if (currHeader == "decay") {
                    addm.decay = curr_f;
                }
            } else {
                addm.optionalParams[headers[col]] = curr_f; 
            }
            col++; 
        }
        potentialModels.push_back(addm);
        if (col != headers.size()) {
            throw std::invalid_argument("Row with insufficient entries provided");            
        }
    }

    // for (string head : headers) {
    //     std::cout << head << " ";
    // }
    // std::cout << std::endl; 
    // for (bool f : isBuiltInParam) {
    //     std::cout << f << " ";
    // }
    // std::cout << std::endl; 

    // for (aDDM addm : potentialModels) {
    //     std::cout << "d " << addm.d << " sigma " << addm.sigma << " theta " << addm.theta << " k " << addm.k << " decay " << addm.decay << " bias " << addm.bias << " W " << addm["W"] << std::endl; 
    // }

    // select method for computing likelihoods
    if (computeMethod == "basic") {
        NLLcomputer = [trials, timeStep, approxStateStep, useAlternative](aDDM addm) -> ProbabilityData {
            ProbabilityData data = ProbabilityData(); 
            if (!useAlternative) {
                for (aDDMTrial trial : trials) {
                    double prob = addm.getTrialLikelihood(trial, timeStep, approxStateStep);
                    data.likelihood += prob; 
                    data.trialLikelihoods.push_back(prob);
                    data.NLL += -log(prob);
                }
            } else {
                for (aDDMTrial trial : trials) {
                    double prob = addm.getLikelihoodAlternative(trial, timeStep, approxStateStep);
                    data.likelihood += prob; 
                    data.trialLikelihoods.push_back(prob);
                    data.NLL += -log(prob);
                }
            }
            return data; 
        };
    }
    else if (computeMethod == "thread") {
        NLLcomputer = [
            trials, timeStep, approxStateStep, useAlternative](aDDM addm) -> ProbabilityData {
            return addm.computeParallelNLL(trials, timeStep, approxStateStep, useAlternative);
        };
    }

    return computeFitAndPosteriors(potentialModels, normalizePosteriors, trials.size());
}

MLEinfo<aDDM> aDDM::fitModelMLE(
    std::vector<aDDMTrial> trials, 
    std::vector<float> rangeD, 
    std::vector<float> rangeSigma, 
    std::vector<float> rangeTheta, 
    std::vector<float> rangeK,
    std::string computeMethod, 
    bool normalizePosteriors,
    float barrier,
    unsigned int nonDecisionTime,
    int timeStep, 
    float approxStateStep,
    std::vector<float> bias, 
    std::vector<float> decay,
    bool useAlternative, 
    map<string, vector<float>>rangeOptional) {

    if (std::find(
        validComputeMethods.begin(), validComputeMethods.end(), 
        computeMethod) == validComputeMethods.end()) {
        throw std::invalid_argument("Input computeMethod is invalid.");
    }

    std::function<void(int)> generate; 

    sort(rangeD.begin(), rangeD.end());
    sort(rangeSigma.begin(), rangeSigma.end());
    sort(rangeTheta.begin(), rangeTheta.end()); 
    sort(rangeK.begin(), rangeK.end());
    sort(bias.begin(), bias.end());
    sort(decay.begin(), decay.end());

    std::vector<std::vector<float>> allParams = {
        rangeD, 
        rangeSigma, 
        rangeTheta, 
        rangeK, 
        bias, 
        decay
    };

    /** On every combination of default parameters, 
     * iterate through a list of all combinations for each parameter value. 
     * {"A":{1, 2, 3}, "B":{4, 5, 6}} ==> {{1, 4}, {1, 5}, {1, 6}, 
     *                                     {2, 4}, {2, 5}, {2, 6}, 
     *                                     {3, 4}, {3, 5}, {3, 6}}
     */    
    std::vector<string> optionalKeys; // {"A", "B"}
    std::vector<std::vector<float>> optionalUnlabeledRanges; // {{1, 2, 3}, {4, 5, 6}}
    std::vector<std::vector<float>> optionalCombinations; // {{1, 4}, {1, 5}, ...}
    if (!rangeOptional.empty()) {
        for (auto it = rangeOptional.begin(); it != rangeOptional.end(); ++it) {
            optionalKeys.push_back(it->first);
            optionalUnlabeledRanges.push_back(it->second);
        }
        std::vector<float> currentOpt(optionalUnlabeledRanges.size(), 0);
        generate = [&](int index) {
            if (index == optionalUnlabeledRanges.size()) {
                optionalCombinations.push_back(currentOpt);
            } else {
                for (float element : optionalUnlabeledRanges[index]) {
                    currentOpt[index] = element; 
                    generate(index + 1);
                }
            }
        };
        generate(0);
        
        for (auto combination : optionalCombinations) {
            assert(combination.size() == optionalKeys.size());
        }  
    } 

    // generate all possible parameter combinations
    std::vector<float> current(allParams.size(), 0);
    std::vector<std::vector<float>> paramCombinations;
    generate = [&](int index) {
        if (index == allParams.size()) {
            paramCombinations.push_back(current); 
        } else {
            for (float element : allParams[index]) {
                current[index] = element; 
                generate(index + 1);
            }
        }
    }; 
    generate(0);

    // create all potential models
    std::vector<aDDM> potentialModels; 
    for (std::vector<float> combination : paramCombinations) {
        assert(combination.size() == 6);
        float d = combination[0]; 
        float sigma = combination[1];
        float theta = combination[2];
        float k = combination[3];
        float b = combination[4];
        float dec = combination[5];

        if (optionalCombinations.empty()) {
            aDDM addm = aDDM(d, sigma, theta, k, barrier, nonDecisionTime, b, dec);
            potentialModels.push_back(addm);
        } else {
            // Iterate through each possible combination of parameters. 
            // Create a new aDDM for each combination. 
            for (auto combination : optionalCombinations) {
                aDDM addm = aDDM(d, sigma, theta, k, barrier, nonDecisionTime, b, dec);
                for (int i = 0; i < combination.size(); i++) {
                    addm.addParameter(optionalKeys[i], combination[i]);
                }
                potentialModels.push_back(addm);
            }
        }        
    }

    // select method for computing likelihoods
    if (computeMethod == "basic") {
        NLLcomputer = [trials, timeStep, approxStateStep, useAlternative](aDDM addm) -> ProbabilityData {
            ProbabilityData data = ProbabilityData(); 
            if (!useAlternative) {
                for (aDDMTrial trial : trials) {
                    double prob = addm.getTrialLikelihood(trial, timeStep, approxStateStep);
                    data.likelihood += prob; 
                    data.trialLikelihoods.push_back(prob);
                    data.NLL += -log(prob);
                }
            } else {
                for (aDDMTrial trial : trials) {
                    double prob = addm.getLikelihoodAlternative(trial, timeStep, approxStateStep);
                    data.likelihood += prob; 
                    data.trialLikelihoods.push_back(prob);
                    data.NLL += -log(prob);
                }
            }
            return data; 
        };
    }
    else if (computeMethod == "thread") {
        NLLcomputer = [
            trials, timeStep, approxStateStep, useAlternative](aDDM addm) -> ProbabilityData {
            return addm.computeParallelNLL(trials, timeStep, approxStateStep, useAlternative);
        };
    }

    return computeFitAndPosteriors(potentialModels, normalizePosteriors, trials.size());
}

