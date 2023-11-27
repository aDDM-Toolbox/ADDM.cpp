"""
@brief Plot reaction time against frequency for each value difference or choice. 

Plot the reaction time against frequency for each possible value difference. To 
use for the aDDM, pass 'addm' as a command line argument. To save the image in
the imgs directory, pass 'save' as a command line argument. Usage is as follows:

python3 analysis/vd_dists.py [addm] [save]
"""

import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
from typing import Dict, List
from datetime import datetime
import sys

DDM_FILE_PATH = "results/ddm_simulations.csv"
ADDM_FILE_PATH = "results/addm_simulations.csv"


val_diff_to_rts: Dict[int, List[int]] = {}
choice_to_rts: Dict[int, List[int]] = {-1 : list(), 1 : list()}
addm = False
if (len(sys.argv) > 1 and sys.argv[1] == "addm"):
    addm = True
    df = pd.read_csv(ADDM_FILE_PATH)
else:
    df = pd.read_csv(DDM_FILE_PATH)
df = df.reset_index()

for _, row in df.iterrows():
    choice = row['choice']
    rt = row['RT']
    val_diff = row['valueLeft'] - row['valueRight']
    if val_diff in val_diff_to_rts:
        val_diff_to_rts[val_diff].append(rt)
    else:
        val_diff_to_rts[val_diff] = [rt]
    if choice in choice_to_rts:
        choice_to_rts[choice].append(rt)
    else:
        choice_to_rts[choice] = [rt]

keys = list(val_diff_to_rts.keys())
keys.sort()
val_diff_to_rts = {i : val_diff_to_rts[i] for i in keys}

fig, axs = plt.subplots(len(val_diff_to_rts))
plt.rcParams['axes.titley'] = 1.0
plt.rcParams['axes.titlepad'] = -14
i = 0
for vd, rts in val_diff_to_rts.items():
    mean = np.mean(rts)
    axs[i].hist(rts, label=vd, bins=range(0, 5000, 100))
    axs[i].set_title(f"value difference={vd}")
    axs[i].set_ylim([0, 30])
    axs[i].axvline(mean, color="red")
    i+= 1
axs[6].set_xlabel("Reaction Time (ms)")
axs[3].set_ylabel('Count')
fig.set_figwidth(18)
fig.set_figheight(15)

if (len(sys.argv) > 1 and "save" in sys.argv):
    currTime = datetime.now().strftime(u"%Y-%m-%d_%H:%M:%S")
    if addm:
        plt.suptitle("aDDM Reaction Time Distribution for Value Differences")   
        plt.savefig("imgs/addm_rt_dist_" + currTime + ".png")
    else:
        plt.suptitle("DDM Reaction Time Distribution for Value Differences")  
        plt.savefig("imgs/ddm_rt_dist_" + currTime + ".png")
