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
/* 

std::vector<float> generateRange(float start, float end, float step) {
    std::vector<float> result;
    for (float value = start; value < end+step; value += step) {
        result.push_back(value);
    }
    return result;
}
 */

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
        "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compE2.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compE2.csv",
        "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compH2.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compH2.csv"
//         "data/b_encC2.csv", "data/f_encC2.csv",
        // "data/b_encF2.csv", "data/f_encF2.csv"
    };

    for (size_t i = 0; i < dataPairs.size(); i += 2) {
        // Generate sets of values
        std::vector<float> range_d = generateRange(0.0005f, 0.005f, 0.0001f);
        std::vector<float> range_s = generateRange(0.01f, 0.1f, 0.005f);
        std::vector<float> range_theta = generateRange(0.0f, 1.0f, 0.05f);

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
            MLEinfo info = aDDM::fitModelMLE(trials, range_d, range_s, range_theta, {0}, "thread");

            std::cout << "d: " << info.optimal.d << " ";
            std::cout << "sigma: " << info.optimal.sigma << " ";
            std::cout << "theta: " << info.optimal.theta << " ";
            std::cout << "k: " << info.optimal.k << std::endl;
            outputData.emplace_back(subjectID, info.optimal.d, info.optimal.sigma, info.optimal.theta, info.optimal.k);
        }

        // Save output to CSV with a specific filename
        std::string outputFileName = "central/groups/rnl/jgoldman/output_" + prefix + ".csv";
        saveToCSV(outputFileName, outputData);
    }

    return 0;
}
