"""
@brief Model the RDV over time for a single data trial. 

Provided the JSON output from exportTrial(...) in the DDM or aDDM classes, this 
script will plot the RDV value over time. To save the resulting graph, pass 
'save' as a command line argument. The JSON file can be declared with the 
FILE_PATH variable. 
"""
import matplotlib.pyplot as plt
import numpy as np
import json
from datetime import datetime
import sys

FILE_PATH = "results/data.json"


with open(FILE_PATH) as file:
    data = json.load(file)

rdvs = data["RDVs"]
x = np.arange(len(rdvs)) * data["timeStep"]
plt.plot(x, rdvs, label="RDVs")   

barrier = float(data["barrier"])
bias = float(data["bias"])
plt.axhline(barrier, color="grey")
plt.axhline(-barrier, color="grey")
plt.axhline(bias, color="grey")

plt.text(0, -barrier * 0.95, f"d: {round(data['d'], 3)}\n"\
            f"RT: {data['RT']}\nsigma: {round(data['sigma'], 3)}")

plt.xlabel("Time (ms)")
plt.ylabel("RDV")

if (len(sys.argv) > 1 and "save" in sys.argv):
    currTime = datetime.now().strftime(u"%Y-%m-%d_%H:%M:%S")
    plt.savefig("imgs/rdv_time_" + currTime + ".png")

plt.show()
