#include <addm/cpp_toolbox.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <tuple>

template<typename... Args>
void saveToCSV(const std::string& filename, const std::vector<std::tuple<Args...>>& data) {
    std::ofstream file(filename);
    if (file.is_open()) {
		file << "index, subj,d,s,theta,k\n";  // Updated header
        
        for (const auto& row : data) {
            file << std::get<0>(row); // Writing subjectID

            std::apply([&file](const auto&... args) {
                ((file << ',' << args), ...); // Writing data from tuples
            }, row);

            file << '\n';
        }
        file.close();
    }
}

// std::vector<float> generateRange(float start, float end, float step) {
//     std::vector<float> result;
//     for (float value = start; value < end+step; value += step) {
//         result.push_back(value);
//     }
//     return result;
// }
std::vector<float> generateRange(float start, float end, float step) {
    std::vector<float> result;
    // Convert everything to integer
    int startInt = static_cast<int>(start / step);
    int endInt = static_cast<int>(end / step);

    for (int i = startInt; i <= endInt; ++i) {
        float value = i * step;
        result.push_back(value);
    }
    return result;
}



void printRange(const std::vector<float>& range, const std::string& name) {
    std::cout << name << ": ";
    for (float value : range) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<std::string> dataPairs = {
        //"data/b_encC.csv", "data/f_encC.csv",
        //"data/b_encF.csv", "data/f_encF.csv",
        "data/b_compE.csv", "data/f_compE.csv",
        "data/b_compH.csv", "data/f_compH.csv"
    };

    std::cout << "hello" << std::endl; 

    for (size_t i = 0; i < dataPairs.size(); i += 2) {
        // Generate sets of values
        std::vector<float> range_theta = generateRange(0.0f, 1.0f, 0.05f);
        std::vector<float> range_d = generateRange(0.001f, 0.01f, 0.0005f);
        std::vector<float> range_s = generateRange(0.01f, 0.1f, 0.005f);

        // Print the generated ranges
        printRange(range_d, "Range_d");
        printRange(range_s, "Range_s");
        printRange(range_theta, "Range_theta");

        std::vector<std::tuple<int, double, double, double, double>> outputData;

        // Load trial and fixation data
        std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(dataPairs[i], dataPairs[i + 1]);

        // Extract the prefix for the output file name
        std::string prefix = dataPairs[i].substr(7, dataPairs[i].find(".")-dataPairs[i].find("_")-1);

        // Iterate through each SubjectID and its corresponding vector of trials.
        for (const auto& [subjectID, trials] : data) {
            std::cout << subjectID << ": ";
            // Compute the most optimal parameters to generate
            MLEinfo<aDDM> info = aDDM::fitModelMLE(trials, range_d, range_s, range_theta, {0}, "thread");

            std::cout << "theta: " << info.optimal.theta << " ";
            std::cout << "d: " << info.optimal.d << " ";
            std::cout << "sigma: " << info.optimal.sigma << " ";
            std::cout << "k: " << info.optimal.k << std::endl;
            outputData.emplace_back(subjectID, info.optimal.d, info.optimal.sigma, info.optimal.theta, info.optimal.k);
        }

        // Save output to CSV with a specific filename
        std::string outputFileName = "out/output_" + prefix + ".csv";
        saveToCSV(outputFileName, outputData);
    }

    return 0;
}

/**
 * hello
Range_d: 0.001 0.0015 0.002 0.0025 0.003 0.0035 0.004 0.0045 0.005 0.0055 0.006 0.0065 0.007 0.0075 0.008 0.0085 0.009 0.0095 
Range_s: 0.01 0.015 0.02 0.025 0.03 0.035 0.04 0.045 0.05 0.055 0.06 0.065 0.07 0.075 0.08 0.085 0.09 0.095 0.1 
Range_theta: 0 0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7 0.75 0.8 0.85 0.9 0.95 1 
0: theta: 0.25 d: 0.0065 sigma: 0.07 k: 0
1: theta: 0.3 d: 0.007 sigma: 0.065 k: 0
2: theta: 0.25 d: 0.0035 sigma: 0.065 k: 0
3: theta: 0.35 d: 0.007 sigma: 0.06 k: 0
4: theta: 0.1 d: 0.0055 sigma: 0.075 k: 0
5: theta: 0.15 d: 0.005 sigma: 0.08 k: 0
6: theta: 0.45 d: 0.002 sigma: 0.055 k: 0
7: theta: 0.05 d: 0.0045 sigma: 0.075 k: 0
8: theta: 0.75 d: 0.0055 sigma: 0.075 k: 0
9: theta: 0.6 d: 0.008 sigma: 0.075 k: 0
10: theta: 0.05 d: 0.0075 sigma: 0.1 k: 0
11: theta: 0.6 d: 0.0055 sigma: 0.07 k: 0
12: theta: 0.35 d: 0.0045 sigma: 0.07 k: 0
13: theta: 0.4 d: 0.003 sigma: 0.065 k: 0
14: theta: 0.35 d: 0.0025 sigma: 0.07 k: 0
15: theta: 0.25 d: 0.0055 sigma: 0.085 k: 0
16: theta: 0.1 d: 0.008 sigma: 0.09 k: 0
17: theta: 0.15 d: 0.004 sigma: 0.055 k: 0
18: theta: 0.25 d: 0.006 sigma: 0.08 k: 0
19: theta: 0.5 d: 0.002 sigma: 0.075 k: 0
20: theta: 0.5 d: 0.0035 sigma: 0.07 k: 0
21: theta: 0.3 d: 0.003 sigma: 0.065 k: 0
22: theta: 0 d: 0.0085 sigma: 0.085 k: 0
23: theta: 0 d: 0.0035 sigma: 0.06 k: 0
24: theta: 0.25 d: 0.005 sigma: 0.09 k: 0
Range_d: 0.001 0.0015 0.002 0.0025 0.003 0.0035 0.004 0.0045 0.005 0.0055 0.006 0.0065 0.007 0.0075 0.008 0.0085 0.009 0.0095 
Range_s: 0.01 0.015 0.02 0.025 0.03 0.035 0.04 0.045 0.05 0.055 0.06 0.065 0.07 0.075 0.08 0.085 0.09 0.095 0.1 
Range_theta: 0 0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7 0.75 0.8 0.85 0.9 0.95 1 
0: theta: 0.35 d: 0.0075 sigma: 0.085 k: 0
1: theta: 0.4 d: 0.008 sigma: 0.075 k: 0
2: theta: 0.45 d: 0.0035 sigma: 0.065 k: 0
3: theta: 0.5 d: 0.0065 sigma: 0.06 k: 0
4: theta: 0.25 d: 0.006 sigma: 0.08 k: 0
5: theta: 0.25 d: 0.006 sigma: 0.08 k: 0
6: theta: 0.4 d: 0.002 sigma: 0.06 k: 0
7: theta: 0.35 d: 0.0045 sigma: 0.07 k: 0
8: theta: 0.65 d: 0.0065 sigma: 0.085 k: 0
9: theta: 0.55 d: 0.0065 sigma: 0.08 k: 0
10: theta: 0.3 d: 0.008 sigma: 0.1 k: 0
11: theta: 0.65 d: 0.007 sigma: 0.085 k: 0
12: theta: 0.65 d: 0.0055 sigma: 0.07 k: 0
13: theta: 0.4 d: 0.004 sigma: 0.065 k: 0
*/