FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

# Utils
RUN apt-get update && \
    apt-get install -y vim &&\
    apt install -y python3-pip

# aDDM toolbox dependencies
RUN wget -O /usr/include/c++/11/BS_thread_pool.hpp https://raw.githubusercontent.com/bshoshany/thread-pool/master/include/BS_thread_pool.hpp && \
    mkdir /usr/include/c++/11/nlohmann/ && \
    wget -O /usr/include/c++/11/nlohmann/json.hpp https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp && \
    apt-get install -y libboost-math-dev libboost-math1.74-dev

# aDDM toolbox installations
RUN cd ~/  && \
    git clone https://github.com/aDDM-Toolbox/ADDM.cpp.git && \
    cd ADDM.cpp/ && \
    make install

# Python dependencies
# Make sure you're in correct directory with access to file
RUN pip install --upgrade pip && \
    pip install --no-cache-dir -r requirements.txt

# Add examples in README as tests to make sure installation worked?