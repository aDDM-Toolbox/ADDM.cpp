#ifndef MLE_INFO_H
#define MLE_INFO_H

#include <map> 
#include <vector>

/**
 * @brief Information returned by MLE computations containing the most optimal model and 
 * NLLs/marginalized posteriors, as specified. 
 * 
 * @tparam T DDM or aDDM. 
 */
template <typename T>
struct MLEinfo {
    T optimal; /**< Most optimal model. */
    std::map<T, float> likelihoods; /**< Either a mapping of models to NLLs or models to 
        marginalized posteriors. */
};

/**
 * @brief Information pertaining to the computation of likelihoods for a dataset of trials (either
 * DDM or aDDM).
 * 
 * The ProbabilityData class provides a wrapper for information that may be returned after computing
 * the likelihoods for a set of trials. The class is meant to provide output information that can 
 * be used when returning NLLs or marginalized posteriors in MLE. 
 * 
 */
class ProbabilityData {
    private:
    public: 
        double likelihood; /**< Sum of likelihoods for all trials. */
        double NLL; /**< Sum of negative log likelihoods for all trials. */
        std::vector<double> trialLikelihoods; /**< Vector containing all trial likelihoods in the 
            order of the input trials. */
        
        /**
         * @brief Construct a new Probability Data object. 
         * 
         * @param likelihood Initial likelihood.
         * @param NLL Initial negative log likelihood; 
         */
        ProbabilityData(double likelihood=0, double NLL=0) {
            this->likelihood = likelihood; 
            this->NLL = NLL;
        };
};

#endif