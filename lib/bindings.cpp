#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "cpp_toolbox.h"
#include <string>

namespace py = pybind11; 

using namespace pybind11::literals; 
using Arg = py::arg; 

template<typename T> 
void declareMLEinfo(py::module &m, const std::string &typestr) {
    using Class = MLEinfo<T>; 
    std::string pyclass_name = std::string("MLEinfo") + typestr; 
    py::class_<Class>(m, pyclass_name.c_str())
        .def_readonly("optimal", &Class::optimal)
        .def_readonly("likelihoods", &Class::likelihoods);
}

PYBIND11_MODULE(addm_toolbox_cpp, m) {
    m.doc() = "aDDMToolbox developed for C++.";
    declareMLEinfo<DDM>(m, "DDM"); 
    declareMLEinfo<aDDM>(m, "aDDM");
    py::class_<ProbabilityData>(m, "ProbabilityData")
        .def(py::init<double, double>(), 
            Arg("likelihood")=0, 
            Arg("NLL")=0)
        .def_readonly("likelihood", &ProbabilityData::likelihood)
        .def_readonly("NLL", &ProbabilityData::NLL)
        .def_readonly("trialLikelihoods", &ProbabilityData::trialLikelihoods);
    py::class_<FixationData>(m, "FixationData")
        .def(py::init<float, vector<int>, vector<int>, fixDists>(), 
            Arg("probFixLeftFirst"), 
            Arg("latencies"), 
            Arg("transitions"), 
            Arg("fixations"))
        .def_readonly("probFixLeftFirst", &FixationData::probFixLeftFirst)
        .def_readonly("latencies", &FixationData::latencies)
        .def_readonly("transitions", &FixationData::transitions)
        .def_readonly("fixations", &FixationData::fixations);
    py::class_<DDMTrial>(m, "DDMTrial")
        .def(py::init<int, int, int, int>(),
            Arg("RT"), 
            Arg("choice"), 
            Arg("valueLeft"), 
            Arg("valueRight"))
        .def_readonly("RT", &DDMTrial::RT)
        .def_readonly("choice", &DDMTrial::choice)
        .def_readonly("valueLeft", &DDMTrial::valueLeft)
        .def_readonly("valueRight", &DDMTrial::valueRight)
        .def_readonly("RDVs", &DDMTrial::RDVs)
        .def_readonly("timeStep", &DDMTrial::timeStep)
        .def_static("writeTrialsToCSV", &DDMTrial::writeTrialsToCSV, 
            Arg("trials"), 
            Arg("filename"))
        .def_static("loadTrialsFromCSV", &DDMTrial::loadTrialsFromCSV, 
            Arg("filename"));
    py::class_<DDM>(m, "DDM")
        .def(py::init<float, float, float, unsigned int, float, float>(), 
            Arg("d"), 
            Arg("sigma"), 
            Arg("barrier")=1, 
            Arg("nonDecisionTime")=0, 
            Arg("bias")=0, 
            Arg("decay")=0)
        .def_readonly("d", &DDM::d)
        .def_readonly("sigma", &DDM::sigma)
        .def_readonly("barrier", &DDM::barrier)
        .def_readonly("nonDecisionTime", &DDM::nonDecisionTime)
        .def_readonly("bias", &DDM::bias)
        .def_readonly("decay", &DDM::decay)
        .def("exportTrial", &DDM::exportTrial, 
            Arg("dt"), 
            Arg("filename"))
        .def("getTrialLikelihood", &DDM::getTrialLikelihood, 
            Arg("trial"), 
            Arg("debug")=false, 
            Arg("timeStep")=10, 
            Arg("approxStateStep")=0.1)
        .def("simulateTrial", &DDM::simulateTrial, 
            Arg("valueLeft"), 
            Arg("valueRight"),
            Arg("timeStep")=10, 
            Arg("seed")=-1)
        .def("computeParallelNLL", &DDM::computeParallelNLL, 
            Arg("trials"), 
            Arg("debug")=false, 
            Arg("timeStep")=10, 
            Arg("approxStateStep")=0.1)
        .def_static("fitModelMLE", &DDM::fitModelMLE, 
            Arg("trials"), 
            Arg("rangeD"), 
            Arg("rangeSigma"), 
            Arg("computeMethod")="basic", 
            Arg("normalizePosteriors")=false,
            Arg("barrier")=1, 
            Arg("nonDecisionTime")=0,
            Arg("bias")=vector<float>{0}, 
            Arg("decay")=vector<float>{0});
    py::class_<aDDMTrial, DDMTrial>(m, "aDDMTrial")
        .def(py::init<unsigned int, int, int, int, vector<int>, vector<int>, vector<float>, float>(), 
            Arg("RT"), 
            Arg("choice"), 
            Arg("valueLeft"), 
            Arg("valueRight"), 
            Arg("fixItem")=vector<int>(), 
            Arg("fixTime")=vector<int>(), 
            Arg("fixRDV")=vector<float>(), 
            Arg("uninterruptedLastFixTime")=0)
        .def_readonly("fixItem", &aDDMTrial::fixItem)
        .def_readonly("fixTime", &aDDMTrial::fixTime)
        .def_readonly("fixRDV", &aDDMTrial::fixRDV)
        .def_readonly("uninterruptedLastFixTime", &aDDMTrial::uninterruptedLastFixTime)
        .def_static("writeTrialsToCSV", &aDDMTrial::writeTrialsToCSV, 
            Arg("trials"), 
            Arg("filename"))
        .def_static("loadTrialsFromCSV", &aDDMTrial::loadTrialsFromCSV, 
            Arg("filename"));
    py::class_<aDDM, DDM>(m, "aDDM")
        .def(py::init<float, float, float, float, float, unsigned int, float, float>(), 
            Arg("d"), 
            Arg("sigma"), 
            Arg("theta"), 
            Arg("k")=0,
            Arg("barrier")=1, 
            Arg("nonDecisionTime")=0, 
            Arg("bias")=0, 
            Arg("decay")=0)
        .def_readonly("theta", &aDDM::theta)
        .def("exportTrial", &aDDM::exportTrial, 
            Arg("adt"), 
            Arg("filename"))
        .def("getTrialLikelihood", &aDDM::getTrialLikelihood, 
            Arg("trial"), 
            Arg("debug")=false, 
            Arg("timeStep")=10, 
            Arg("approxStateStep")=0.1)
        .def("simulateTrial", &aDDM::simulateTrial, 
            Arg("valueLeft"), 
            Arg("valueRight"), 
            Arg("fixationData"), 
            Arg("timeStep")=10, 
            Arg("numFixDists")=3, 
            Arg("fixationDist")=fixDists(), 
            Arg("timeBins")=vector<int>(), 
            Arg("seed")=-1)
        .def("computeParallelNLL", &aDDM::computeParallelNLL, 
            Arg("trials"), 
            Arg("debug")=false, 
            Arg("timeStep")=10, 
            Arg("approxStateStep")=0.1)
        .def_static("fitModelMLE", &aDDM::fitModelMLE, 
            Arg("trials"), 
            Arg("rangeD"), 
            Arg("rangeSigma"), 
            Arg("rangeTheta"),
            Arg("rangeK")=vector<float>{0},
            Arg("computeMethod")="basic",
            Arg("normalizePosteriors")=false,
            Arg("barrier")=1, 
            Arg("nonDecisionTime")=0,
            Arg("bias")=vector<float>{0}, 
            Arg("decay")=vector<float>{0});
    m.def("loadDataFromSingleCSV", &loadDataFromSingleCSV, 
        Arg("filename"));
    m.def("loadDataFromCSV", &loadDataFromCSV, 
        Arg("expDataFilename"), 
        Arg("fixDataFilename"));
    m.def("getEmpiricalDistributions", &getEmpiricalDistributions, 
        Arg("data"), 
        Arg("timeStep")=10, 
        Arg("maxFixTime")=3000, 
        Arg("numFixDists")=3, 
        Arg("valueDiffs")=vector<int>{-3,-2,-1,0,1,2,3}, 
        Arg("subjectIDs")=vector<int>(), 
        Arg("useOddTrials")=true, 
        Arg("useEvenTrials")=true, 
        Arg("useCisTrials")=true, 
        Arg("useTransTrials")=true); 
}
