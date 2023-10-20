#include <addm/cpp_toolbox.h>
#include <iostream>

int main() {
    aDDM addm = aDDM(0.001, 0.02, 0.3);
    addm.addParameter("W", 5);
    std::cout << "W = " << addm["W"] << std::endl; 
    double likelihood = addm.getLikelihoodAlternative(aDDMTrial()); 
    std::cout << "Likelihood = " << likelihood << std::endl; 
}