# ADDM.cpp

C++ implementation of the aDDM-Toolbox. 


## Requirements ##

This library requires g++ version 11.3.0, as well as three third-party C++ packages for thread pools, JSON processing, and statistical distributions:

* [BS::thread_pool](https://github.com/bshoshany/thread-pool)
* [JSON for Modern C++](https://github.com/nlohmann/json)
* [Boost Math/Statistical Distributions](https://www.boost.org/doc/libs/?view=category_math)

These dependencies can be installed using the following commands: 

```shell
$ wget -O /usr/include/c++/11/BS_thread_pool.hpp https://raw.githubusercontent.com/bshoshany/thread-pool/master/include/BS_thread_pool.hpp
$ mkdir -p /usr/include/c++/11/nlohmann
$ wget -O /usr/include/c++/11/nlohmann/json.hpp https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp
$ apt-get install libboost-math-dev libboost-math1.74-dev
```

*Note that the installation directory /usr/include/c++/11 may be modified to support newer versions of C++. In the event of a __Permission Denied__ error, precede the above commands with __sudo__.*

## Installation and Usage ## 

TThe aDDM-Toolbox-CPP library can then be built and installed in one step: 

```shell
$ make install
```

*In the event of a __Permission Denied__ error, precede the above command with __sudo__.*

This will install the libaddm.so shared library as well as the corresponding header files. Although there are multiple header files corresponding to the aDDM and DDM programs, simply adding `#include <addm/cpp_toolbox.h>` to a C++ program will include all necessary headers. A simple usage example is described below: 

`main.cpp`:
```C++
#include <addm/gpu_toolbox.h>
#include <iostream>

int main() {
    aDDM addm = aDDM(0.005, 0.07, 0.5);
    std::cout << "d: " << addm.d << std::endl; 
    std::cout << "sigma: " << addm.sigma << std::endl; 
    std::cout << "theta: " << addm.theta << std::endl; 
}
```

When compiling any code using the toolbox, include the `-laddm` flag to link with the installed shared object library.

```
$ g++ -o main main.cpp -laddm
$ ./main
d: 0.005
sigma: 0.07
theta: 0.5
```

## Tutorial ##

In the [data](data/) directory, we have included two test files to demonstrate how to use the toolbox. [expdata.csv](data/expdata.csv) contains experimental data and [fixations.csv](data/fixations.csv) contains the corresponding fixation data. A description of how to fit models corresponding to these subjects is located in [tutorial.cpp](sample/tutorial.cpp). We also detail the process below. 

`sample/tutorial.cpp`
```cpp
#include <addm/cpp_toolbox.h>
#include <iostream> 

int main() {
    // Load trial and fixation data
    std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV("data/expdata.csv", "data/fixations.csv");
    // Iterate through each SubjectID and its corresponding vector of trials. 
    for (const auto& [subjectID, trials] : data) {
        std::cout << subjectID << ": "; 
        // Compute the most optimal parameters to generate 
        MLEinfo info = aDDM::fitModelMLE(trials, {0.001, 0.002, 0.003}, {0.0875, 0.09, 0.0925}, {0.1, 0.3, 0.5}, {0, 0.5}, "thread");
        std::cout << "d: " << info.optimal.d << " "; 
        std::cout << "sigma: " << info.optimal.sigma << " "; 
        std::cout << "theta: " << info.optimal.theta << " "; 
        std::cout << "k: " << info.optimal.k << std::endl; 
    }
}
```

Let's break this down piece by piece: 

```cpp
#include <addm/cpp_toolbox.h>
#include <iostream> 
```

This tells the C++ pre-processor to find the `addm` library and the main header file `cpp_toolbox.h`. The main header file includes all sub-headers for the `DDM` and `aDDM` classes and utility methods, so there is no need to include any other files. If you haven't already, run `make install` to install the `addm` library on your machine. This also tells the pre-processor to compile with the `<iostream>` library, which provides functionality for printing to the console. 

```cpp
// Load trial and fixation data
std::map<int, std::vector<aDDMTrial>> data = loadDataFromCSV("data/expdata.csv", "data/fixations.csv");
```

This uses the `loadDataFromCSV` function included in `util.h` to load the experimental data and fixation data from CSV files. Both files need to be structured in a specific format to be properly loaded. These formats are described below, with sample experimental data in `data/expdata.csv` and fixation data in `data/fixations.csv`. 

__Experimental Data__

| parcode | trial | rt | choice | valueLeft | valueRight |  
| :-:     | :-:   |:-: | :-:    | :-:       | :-:        | 
| 0       | 0     |1962| -1     | 15        | 0          | 
| 0       | 1     |873 | 1      | -15       | 5          |  
| 0       | 2     |1345| 1      | 10        | -5         |  

__Fixation Data__

| parcode | trial | fixItem | fixTime |
| :-:     | :-:   | :-:      | :-:      |
| 0       | 0     | 3        | 176      | 
| 0       | 0     | 0        | 42       | 
| 0       | 0     | 1        | 188      | 

Note that data can also be loaded from a single CSV as well. To do this, use the `loadDataFromSingleCSV` function. This format is described below: 

__Single CSV__

|  trial 	|choice |   rt	|  valueLeft 	|  valueRight 	|  fixItem 	|  fixTime 	|
|:-:	|:-:	|:-:	|:-:	        |:-:	        |:-:	    |:-:	    |
|   0	|  1 	|  350 	|   3	        |   0           |   0	    |   200	    |
|   0	|   1	|  350 	|   3	        |   0	        |   1	    |   150	    |
|   1	|   -1	|  400 	|   4	        |  5            |   0	    |   300	    |
|   1	|   -1	|  400 	|   4	        |   5	        |   2	    |   100	    |

The `loadDataFromCSV` function returns a `std::map<int, std::vector<aDDMTrial>>`. This is a mapping from subjectIDs to their corresponding list of trials. A single trial (choice, response time, fixations) is represented in the `aDDMTrial` object. 

```cpp
for (const auto& [subjectID, trials] : data) ...
```

Iterate through each individual subjectID and its list of aDDMTrials. 

```cpp
std::cout << subjectID << ": "; 
// Compute the most optimal parameters to generate 
MLEinfo info = aDDM::fitModelMLE(trials, {0.001, 0.002, 0.003}, {0.0875, 0.09, 0.0925}, {0.1, 0.3, 0.5}, {0, 0.5}, "thread");
std::cout << "d: " << info.optimal.d << " "; 
std::cout << "sigma: " << info.optimal.sigma << " "; 
std::cout << "theta: " << info.optimal.theta << std::endl; 
```

Perform model fitting via Maximum Likelihood Estimation (MLE) to find the optimal parameters for each subject. The arguments for this function are as follows: 

* `trials` - The list of trials for the given subjectID. 
* `{0.001, 0.002, 0.003}` - Range to test for the drift rate (d).
* `{0.0875, 0.09, 0.0925}` - Range to test for noise (sigma).
* `{0.1, 0.3, 0.5}` - Range to test for the fixation discount (theta).
* `{0, 0.5}` - Range to test for additive fixation factor (k). 
* `"thread"` - indicates whether to use the standard or multithreaded implementation. Must be selected between `"basic"` and `"thread"`. 

## Modifying the Toolbox ## 

Some users may want to modify this codebase for their own purposes. Below are some examples of what users may want to do and some tips on altering the code. 

### Alternative Optimization Algorithms ###

For some use-cases, MLE may not be optimal for model fitting. So, some users may want to change how the model fitting algorithm identifies the optimal parameters. The portion of the `fitModelMLE` function in the aDDM class where MLE is actually performed is highlighted in the `addm.cpp` file, but is also described below. 

```cpp
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
```

Key Variables: 
* `potentialModels`: Vector of all possible aDDM models and is created by iterating through the entire parameter grid-space. 
* `posteriors`: Mapping from individual aDDM models to their NLL or marginalized posteriors, depending on input conditions. If the marginal posteriors are to be computed, these calculations are performed at the end of computations. 
* `allTrialLikelihoods`: Mapping from individual aDDM models to their computed `ProbabilityData` objects. For reference, this object contains information regarding the computed proabilities for a vector of `aDDMTrial` objects. It is comprised of the sum of Negative Log Likelihoods, sum of likelihoods, and a list of all likelihoods for each trial. 
* `optimal`: The most optimal model to fit the given trials. 

When redesigning this segment of code, the minimum requirement is that some `aDDM` object is selected as the __optimal__ model. All other computational features can be determined by the user. The decision to compute the marginal posteriors or include code to add to the mappings can also be determined by the user if they inted on using that feature. The code should still compile and run if the `posteriors` and `allTrialLikelihoods` maps are left empty. 

### Adding Parameters and Alternative Likelihood Calculators ###

Some users may want to fit models that have different parameters than those built into the standard model. Steps to use this toolbox with these modifications are described below. An example using a custom toolbox design is described in [custom.cpp](sample/custom.cpp) as well.  

__Adding New Parameters__: The `aDDM` class has a built-in field `optionalParameters` that allows users to easily add different parameters to a model. This field is a mapping from strings to floats, so named parameters can be mapped to their specific values. See example below: 

```cpp
#include <addm/cpp_toolbox.h>
#include <iostream>

int main() {
    aDDM addm = aDDM(0.001, 0.02, 0.3);
    addm.addParameter("W", 5);
    std::cout << "W = " << addm["W"] << std::endl; 
}
```
Output: 
```
W = 5
```

The `aDDM` class also contains a starter method, `getAlternativeLikelihood`, for users who want to define their own likelihood computations using custom variables. A very simple example is provided in the function now, simply returning `1 / this->optionalParams["W"]`. This function should be updated depending on the user's chosen parameters and needs for the calculations. A good starting point could be copying the existing code in `getTrialLikelihood` and modifying it to their needs. Or, for users who know their own likelihood function will be similar to the existing `getTrialLikelihood` function, it may be more worth it to just modify that code instead. An example of a call to `getAlternativeLikelihood` is described below:

`alternative.cpp`:
```cpp
double aDDM::getLikelihoodAlternative(aDDMTrial trial, int timeStep, float approxStateStep) {
    // EXAMPLE CODE! 
    try {
        return 1 / this->optionalParams["W"];
    } catch (exception e) {
        return 1; 
    }
}
```
Function call:
```cpp
#include <addm/cpp_toolbox.h>
#include <iostream>

int main() {
    aDDM addm = aDDM(0.001, 0.02, 0.3);
    addm.addParameter("W", 5);
    double likelihood = addm.getLikelihoodAlternative(aDDMTrial()); 
    std::cout << "Likelihood = " << likelihood << std::endl;     
}
```
Output: 
```
Likelihood = 0.2
```

## Python Bindings ## 

Python bindings are also provided for users who prefer to work with a Python codebase over C++. The provided bindings are located in [lib/bindings.cpp](lib/bindings.cpp). Note that [pybind11](https://github.com/pybind/pybind11) and Python version 3.10 (at a minimum) are __strict__ prerequisites for installation and usage of the Python code. These can be installed with 

```shell
apt-get install python3.10
pip3 install pybind11
```

Once `pybind11` and Python3.10 are installed, the module can be built with:

```
make pybind
```

This will create a shared library object in the repository's root directory containing the `addm_toolbox_cpp` module. Although function calls remain largely analogous with the original C++ code, an example is described below that can be used to ensure the code is working properly: 

`main.py`: 

```Python
import addm_toolbox_cpp

ddm = addm_toolbox_cpp.DDM(0.005, 0.07)
print(f"d = {ddm.d}")
print(f"sigma = {ddm.sigma}")

trial = ddm.simulateTrial(3, 7, 10, 540)
print(f"RT = {trial.RT}")
print(f"choice = {trial.choice}")
```
To run the code: 
```
$ python3 main.py
d = 0.005
sigma = 0.07
RT = 850
choice = 1
```

### Optional: Python Syntax Highlighting ###

For users working in a user interface, such as Visual Studio Code, a Python stub is provided to facilitate features including syntax highlighting, type-hinting, auto-complete. Although the `addm_toolbox_cpp.pyi` stub is built-in, the file can be dynamically generated using the [mypy stubgen](https://mypy.readthedocs.io/en/stable/stubgen.html) tool. The `mypy` module can be installed using: 

```shell
pip install mypy
```

For users who plan to modify the library for their own use and want the provided features, the stub file can be built as follows: 

```shell
stubgen -m addm_toolbox_cpp -o .
```
*Note that the `pybind11` shared library file should be built before running `stubgen`.*


## Authors ## 

* Jake Goldman - jgoldman@caltech.edu, [jakegoldm](https://github.com/jakegoldm)
* Zeynep Enkavi - zenkavi@caltech.edu, [zenkavi](https://github.com/zenkavi)

## Acknowledgements ##

This toolbox was developed as part of a resarch project in the [Rangel Neuroeconomics Lab](http://www.rnl.caltech.edu/) at the California Institute of Technology. Special thanks to Antonio Rangel and Zeynep Enkavi for your help with this project. 