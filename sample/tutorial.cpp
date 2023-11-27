#include <addm/cpp_toolbox.h>
#include <iostream> 

int main() {
    // Load trial and fixation data
    std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV(
        "data/expdata.csv", "data/fixations.csv");
    // Iterate through each SubjectID and its corresponding vector of trials. 
    for (const auto& [subjectID, trials] : data) {
        std::cout << subjectID << ": "; 
        // Compute the most optimal parameters to generate 
        MLEinfo info = aDDM::fitModelMLE(
            trials, {0.001, 0.002, 0.003}, {0.0875, 0.09, 0.0925}, 
            {0.1, 0.3, 0.5}, {0}, "thread");
        std::cout << "d: " << info.optimal.d << " "; 
        std::cout << "sigma: " << info.optimal.sigma << " "; 
        std::cout << "theta: " << info.optimal.theta << std::endl; 
    }
}  