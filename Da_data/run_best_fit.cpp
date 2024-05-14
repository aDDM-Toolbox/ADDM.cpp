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


std::vector<float> generateRange(float start, float end, float step) {
    std::vector<float> result;
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
        "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compE.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compE.csv",
        "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compH.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compH.csv"
    };
    std::vector<std::string> dataPairs2 = {
        "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compE2.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compE2.csv",
        "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compH2.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compH2.csv"
    };
    std::vector<std::string> dataPairs05 = {
       "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compE05.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compE05.csv",
       "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/b_compH05.csv", "/central/groups/rnl/jgoldman/ADDM.cpp/Da_data/data/f_compH05.csv"
    };

    std::vector<float>optimal_s = {}; 
    std::vector<float>optimal_theta = {}; 
    std::vector<float> range_d1 = generateRange(0.0001f, 0.01f, 0.0001f);
    std::vector<float> range_d2 = generateRange(0.0001f, 0.03f, 0.0001f);

    for (size_t i = 0; i < dataPairs.size(); i += 2) {
        std::vector<float> range_theta = generateRange(0.0f, 1.0f, 0.05f);
        std::vector<float> range_s = generateRange(0.01f, 0.1f, 0.005f);;

        // Print the generated ranges
        std::cout << "RUN 4CONDI" << std::endl; 
        printRange(range_d1, "Range_d");
        printRange(range_s, "Range_s");
        printRange(range_theta, "Range_theta");

        std::vector<std::tuple<int, double, double, double, double>> outputData;
        std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(dataPairs[i], dataPairs[i + 1]);
        std::string prefix = dataPairs[i].substr(7, dataPairs[i].find(".")-dataPairs[i].find("_")-1);
        for (const auto& [subjectID, trials] : data) {
            std::cout << subjectID << ": ";
            MLEinfo<aDDM> info = aDDM::fitModelMLE(trials, range_d1, range_s, range_theta, {0}, "thread", false);

            std::cout << "d: " << info.optimal.d << " ";
            std::cout << "sigma: " << info.optimal.sigma << " ";
            std::cout << "theta: " << info.optimal.theta << " ";
            std::cout << "k: " << info.optimal.k << std::endl;

            optimal_s.push_back(info.optimal.sigma);
            optimal_theta.push_back(info.optimal.theta);

            outputData.emplace_back(subjectID, info.optimal.d, info.optimal.sigma, info.optimal.theta, info.optimal.k);
        }
        std::string outputFileName = "/central/groups/rnl/jgoldman/out/output_" + prefix + ".csv";
        saveToCSV(outputFileName, outputData);
    }


    int j = 0;
    for (size_t i = 0; i < dataPairs2.size(); i += 2) {
        // Print the generated ranges
        std::cout << "RUN 2" << std::endl; 
        printRange(range_d1, "Range_d");

        std::vector<std::tuple<int, double, double, double, double>> outputData;
        std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(dataPairs2[i], dataPairs2[i + 1]);
        std::string prefix = dataPairs2[i].substr(7, dataPairs2[i].find(".")-dataPairs2[i].find("_")-1);

        for (const auto& [subjectID, trials] : data) {
            std::cout << subjectID << ": ";
            MLEinfo<aDDM> info = aDDM::fitModelMLE(trials, range_d1, {optimal_s[j]}, {optimal_theta[j]}, {0}, "thread", false);

            std::cout << "d: " << info.optimal.d << " ";
            std::cout << "sigma: " << info.optimal.sigma << " ";
            std::cout << "theta: " << info.optimal.theta << " ";
            std::cout << "k: " << info.optimal.k << std::endl;

            outputData.emplace_back(subjectID, info.optimal.d, info.optimal.sigma, info.optimal.theta, info.optimal.k);

            j++;
        }
        std::string outputFileName = "/central/groups/rnl/jgoldman/out/output2_" + prefix + ".csv";
        saveToCSV(outputFileName, outputData);
    }


    j = 0;  
    for (size_t i = 0; i < dataPairs05.size(); i += 2) {
        // Print the generated ranges
        std::cout << "RUN 05" << std::endl; 
        printRange(range_d2, "Range_d");

        std::vector<std::tuple<int, double, double, double, double>> outputData;
        std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(dataPairs05[i], dataPairs05[i + 1]);
        std::string prefix = dataPairs05[i].substr(7, dataPairs05[i].find(".")-dataPairs05[i].find("_")-1);

        for (const auto& [subjectID, trials] : data) {
            std::cout << subjectID << ": ";
            MLEinfo<aDDM> info = aDDM::fitModelMLE(trials, range_d2, {optimal_s[j]}, {optimal_theta[j]}, {0}, "thread", false);

            std::cout << "d: " << info.optimal.d << " ";
            std::cout << "sigma: " << info.optimal.sigma << " ";
            std::cout << "theta: " << info.optimal.theta << " ";
            std::cout << "k: " << info.optimal.k << std::endl;

            outputData.emplace_back(subjectID, info.optimal.d, info.optimal.sigma, info.optimal.theta, info.optimal.k);

            j++;
        }
        std::string outputFileName = "/central/groups/rnl/jgoldman/out/output2_" + prefix + ".csv";
        saveToCSV(outputFileName, outputData);
    }

    return 0;
}
