#include <stdexcept>
#include <functional>
#include <iostream>
#include <chrono>
#include <cstddef>
#include <string>
#include <random>
#include <fstream>
#include <iomanip>
#include <BS_thread_pool.hpp>
#include "util.h"
#include "ddm.h"
#include "stats.h"

DDMTrial::DDMTrial(unsigned int RT, int choice, int valueLeft, int valueRight) {
    this->RT = RT;
    this->choice = choice;
    this->valueLeft = valueLeft;
    this->valueRight = valueRight;
}

DDM::DDM(float d, float sigma, float barrier, unsigned int nonDecisionTime, float bias, float decay) {
    if (barrier <= 0) {
        throw std::invalid_argument("barrier parameter must be larger than 0.");
    }
    if (bias >= barrier) {
        throw std::invalid_argument("bias parameter must be smaller than barrier parameter.");
    }
    this->d = d;
    this->sigma = sigma;
    this->barrier = barrier;
    this->nonDecisionTime = nonDecisionTime;
    this->bias = bias;
    this->decay = decay;
}

void DDM::exportTrial(DDMTrial dt, std::string filename) {
    std::ofstream o(filename);
    json j;
    j["d"] = d;
    j["sigma"] = sigma;
    j["barrier"] = barrier;
    j["NDT"] = nonDecisionTime;
    j["bias"] = bias;
    j["RT"] = dt.RT;
    j["choice"] = dt.choice;
    j["vl"] = dt.valueLeft;
    j["vr"] = dt.valueRight;
    j["RDVs"] = dt.RDVs;
    j["timeStep"] = dt.timeStep;
    o << std::setw(4) << j << std::endl;
}

double DDM::getTrialLikelihood(DDMTrial trial, bool debug, int timeStep, float approxStateStep) {
    int numTimeSteps = trial.RT / timeStep;
    if (numTimeSteps < 1) {
        throw std::invalid_argument("trial response time is smaller than time step.");
    }
    if (debug) {
        std::cout << std::setprecision(6) << std::fixed;
    }

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

    int halfNumStateBins = ceil(this->barrier / approxStateStep);
    float stateStep = this->barrier / (halfNumStateBins + 0.5);
    std::vector<float> states;
    for (float ss = barrierDown.at(0) + (stateStep / 2); ss <= barrierUp.at(0) - (stateStep / 2); ss += stateStep) {
        states.push_back(ss);
    }

    if (debug) {
        std::cout << "STATES" << std::endl;
        for (float s : states) {
            std::cout << s << " " << std::endl;
        }
        std::cout << "------" << std::endl;
    }

    // Get index of state corresponding to the bias
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

    int elapsedNDT = 0;
    bool recomputePDCM = true;
    float prevMean = 0;
    std::vector<std::vector<double>> probDistChangeMatrix(states.size(), std::vector<double>(states.size()));

    for (int time = 1; time < numTimeSteps; time++) {
        if (debug) {
            std::cout << "============" << std::endl;
            std::cout << "TIMESTEP " << time << std::endl;
            std::cout << "============" << std::endl;
        }
        float mean;
        if (elapsedNDT < this->nonDecisionTime / timeStep) {
            mean = 0;
            elapsedNDT += 1;
        } else {
            mean = this->d * (trial.valueLeft - trial.valueRight);
        }
        if (debug) {
            std::cout << "mean: " << mean << std::endl;
        }

        if (mean != prevMean) {
            recomputePDCM = true;
        } else {
            recomputePDCM = false;
        }

        // Compute the likelihood of each change in the matrix using a probability density function with parameters mean and sigma.
        // Only necessary when:
        //     -mean of the normal distribution has changed
        //     -first timestep
        if (recomputePDCM || time == 1) {
            for (size_t i = 0; i < states.size(); i++) {
                for (size_t j = 0; j < states.size(); j++) {
                    float x = changeMatrix[i][j];
                    probDistChangeMatrix[i][j] = probabilityDensityFunction(mean, this->sigma, x);
                }
            }
        }

        if (debug) {
            pmat<double>(probDistChangeMatrix, "PROBABILITY CHANGE MATRIX");
        }

        // Fetch the probability states for the previous timeStep
        std::vector<double> prTimeSlice(states.size());
        for (size_t i = 0; i < states.size(); i++) {
            prTimeSlice[i] = prStates[i][time - 1];
        }

        if (debug) {
            std::cout << "PREV TIME SLICE" << std::endl;
            for (double d : prTimeSlice) {
                std::cout << d << std::endl;
            }
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

        std::vector<float> currChangeUp;
        for (auto s : changeUp) {
            currChangeUp.push_back(s.at(time));
        }
        std::vector<double> changeUpCDFs;
        for (int i = 0; i < currChangeUp.size(); i++) {
            float x = currChangeUp[i];
            changeUpCDFs.push_back(
                1 - cumulativeDensityFunction(mean, this->sigma, x)
            );
        }
        if (debug) {
            std::cout << "CURR CHANGE UP CDFs" << std::endl;
            for (float f : changeUpCDFs) {
                std::cout << f << std::endl;
            }
            std::cout << "------" << std::endl;
        }
        assert(changeUpCDFs.size() == prTimeSlice.size());
        double tempUpCross = 0;
        for (int i = 0; i < prTimeSlice.size(); i++) {
            tempUpCross += changeUpCDFs[i] * prTimeSlice[i];
        }
        if (debug) {
            std::cout << "temp up cross: " << tempUpCross << std::endl;
        }

        std::vector<float> currChangeDown;
        for (auto s: changeDown) {
            currChangeDown.push_back(s.at(time));
        }
        std::vector<double> changeDownCDFs;
        for (int i = 0; i < currChangeDown.size(); i++) {
            float x = currChangeDown[i];
            changeDownCDFs.push_back(
                cumulativeDensityFunction(0, 1, (x - mean) / this->sigma)
            );
        }
        assert(changeDownCDFs.size() == prTimeSlice.size());
        double tempDownCross = 0;
        for (int i = 0; i < prTimeSlice.size(); i++) {
            tempDownCross += changeDownCDFs[i] * prTimeSlice[i];
        }
        if (debug) {
            std::cout << "temp down cross: " << tempDownCross << std::endl;
        }

        double sumIn = 0;
        for (double prob : prTimeSlice) {
            sumIn += prob;
        }
        double sumCurrent = tempUpCross + tempDownCross;
        for (double prob : prStatesNew) {
            sumCurrent += prob;
        }
        double normFactor = sumIn / sumCurrent;

        if (debug) {
            std::cout << "norm factor " << normFactor << std::endl;
        }
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

        prevMean = mean;
    }

    double likelihood = 0;
    if (trial.choice == -1) {
        if (probUpCrossing[probUpCrossing.size() - 1] > 0) {
            likelihood = probUpCrossing[probUpCrossing.size() - 1];
        }
    } else if (trial.choice == 1) {
        if (probDownCrossing[probDownCrossing.size() - 1] > 0) {
            likelihood = probDownCrossing[probDownCrossing.size() - 1];
        }
    }
    assert(likelihood < 1);
    if (likelihood == 0) {
        likelihood = pow(10, -20);
    }
    return likelihood;
}


DDMTrial DDM::simulateTrial(int valueLeft, int valueRight, int timeStep, int seed) {
    float RDV = this->bias;
    int time = 0;
    int elapsedNDT = 0;
    int RT;
    int choice;
    float mean;
    std::vector<float>RDVs = {RDV};

    std::random_device rd;
    std::mt19937 gen(seed == -1 ? rd() : seed);

    while (true) {
        if (RDV >= this->barrier || RDV <= -this->barrier) {
            RT = time * timeStep;
            if (RDV >= this->barrier) {
                choice = -1;
            } else {
                choice = 1;
            }
            break;
        }
        if (elapsedNDT < this->nonDecisionTime / timeStep) {
            mean = 0;
            elapsedNDT += 1;
        }
        else {
            mean = this->d * (valueLeft - valueRight);
        }
        std::normal_distribution<float> dist(mean, this->sigma);
        float inc = dist(gen);
        RDV += inc;
        RDVs.push_back(RDV);
        time += 1;
    }
    DDMTrial trial = DDMTrial(RT, choice, valueLeft, valueRight);
    trial.RDVs = RDVs;
    trial.timeStep = timeStep;
    return trial;
}


ProbabilityData DDM::computeParallelNLL(std::vector<DDMTrial> trials, bool debug, int timeStep, float approxStateStep) {
    ProbabilityData datasetTotals = ProbabilityData(0, 0);
    BS::thread_pool pool;
    std::vector<double> trialLikelihoods(trials.size());
    BS::multi_future<ProbabilityData> futs = pool.parallelize_loop(
                0, trials.size(),
    [this, &trials, debug, timeStep, approxStateStep, &trialLikelihoods](const int a, const int b) {
        ProbabilityData aux = ProbabilityData(0, 0);
        for (int i = a; i < b; ++i) {
            double prob = this->getTrialLikelihood(trials[i], debug, timeStep, approxStateStep);
            trialLikelihoods[i] = prob;
            aux.likelihood += prob;
            aux.NLL += -log(prob);
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

void DDMTrial::writeTrialsToCSV(std::vector<DDMTrial> trials, std::string filename) {
    std::ofstream fp;
    fp.open(filename);
    fp << "choice,rt,valueLeft,valueRight\n";
    for (DDMTrial t : trials) {
        fp << t.choice << "," << t.RT << "," << t.valueLeft << "," << t.valueRight << "\n";
    }
    fp.close();
}

std::vector<DDMTrial> DDMTrial::loadTrialsFromCSV(std::string filename) {
    std::vector<DDMTrial> trials;
    std::ifstream file(filename);
    std::string line;
    std::getline(file, line);
    int choice;
    int RT;
    int valDiff;
    int valueLeft;
    int valueRight;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string field;
        std::getline(ss, field, ',');
        choice = std::stoi(field);
        std::getline(ss, field, ',');
        RT = std::stoi(field);
        std::getline(ss, field, ',');
        valueLeft = std::stoi(field);
        std::getline(ss, field, ',');
        valueRight = std::stoi(field);
        DDMTrial dt = DDMTrial(RT, choice, valueLeft, valueRight);
        trials.push_back(dt);
    }
    file.close();

    return trials;
}

MLEinfo<DDM> DDM::fitModelMLE(
    vector<DDMTrial> trials,
    vector<float> rangeD,
    vector<float> rangeSigma,
    string computeMethod,
    bool normalizePosteriors,
    float barrier,
    unsigned int nonDecisionTime,
    vector<float> bias,
    vector<float> decay) {

    if (std::find(validComputeMethods.begin(), validComputeMethods.end(), computeMethod) == validComputeMethods.end()) {
        throw std::invalid_argument("Input computeMethod is invalid.");
    }

    sort(rangeD.begin(), rangeD.end());
    sort(rangeSigma.begin(), rangeSigma.end());
    sort(bias.begin(), bias.end());
    sort(decay.begin(), decay.end());

    std::vector<DDM> potentialModels;
    for (float d : rangeD) {
        for (float sigma : rangeSigma) {
            for (float b : bias) {
                for (float dec : decay) {
                    DDM ddm = DDM(d, sigma, barrier, nonDecisionTime, b, dec);
                    potentialModels.push_back(ddm);
                }
            }
        }
    }

    std::function<ProbabilityData(DDM)> NLLcomputer;
    if (computeMethod == "basic") {
        NLLcomputer = [trials](DDM ddm) -> ProbabilityData {
            ProbabilityData data = ProbabilityData();
            for (DDMTrial trial : trials) {
                double prob = ddm.getTrialLikelihood(trial);
                data.likelihood += prob;
                data.trialLikelihoods.push_back(prob);
                data.NLL += -log(prob);
            }
            return data;
        };
    }
    else if (computeMethod == "thread") {
        NLLcomputer = [trials](DDM ddm) -> ProbabilityData {
            return ddm.computeParallelNLL(trials);
        };
    }

    double minNLL = __DBL_MAX__;
    std::map<DDM, ProbabilityData> allTrialLikelihoods;
    std::map<DDM, float> posteriors;
    double numModels = rangeD.size() * rangeSigma.size() * bias.size() * decay.size();

    DDM optimal = DDM();
    for (DDM ddm : potentialModels) {
        ProbabilityData aux = NLLcomputer(ddm);
        if (normalizePosteriors) {
            allTrialLikelihoods.insert({ddm, aux});
            posteriors.insert({ddm, 1 / numModels});
        } else {
            posteriors.insert({ddm, aux.NLL});
        }
        std::cout << "testing d=" << ddm.d << " sigma=" << ddm.sigma;
        if (bias.size() > 1) {
            std::cout << " bias=" << ddm.bias;
        }
        if (decay.size() > 1) {
            std::cout << " decay=" << ddm.decay;
        }
        std::cout << " NLL=" << aux.NLL << std::endl;
        if (aux.NLL < minNLL) {
            minNLL = aux.NLL;
            optimal = ddm;
        }
    }
    if (normalizePosteriors) {
        for (int tn = 0; tn < trials.size(); tn++) {
            double denominator = 0;
            for (const auto &ddmPD : allTrialLikelihoods) {
                DDM curr = ddmPD.first;
                ProbabilityData data = ddmPD.second;
                double likelihood = data.trialLikelihoods[tn];
                denominator += posteriors[curr] * likelihood;
            }
            double sum = 0;
            for (const auto &ddmPD : allTrialLikelihoods) {
                DDM curr = ddmPD.first;
                ProbabilityData data = ddmPD.second;
                double prior = posteriors[curr];
                double newLikelihood = data.trialLikelihoods[tn] * prior / denominator;
                posteriors[curr] = newLikelihood;
                sum += newLikelihood;
            }
            if (sum != 1) {
                double normalizer = 1 / sum;
                for (auto &p : posteriors) {
                    p.second *= normalizer;
                }
            }
        }
    }
    MLEinfo<DDM> info;
    info.optimal = optimal;
    info.likelihoods = posteriors;
    return info;
}
