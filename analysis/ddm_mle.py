"""
@brief Create a heatmap for Maximum Likelihood Estimation on the DDM. 

This script creates a heatmap representing the Negative Log Likelihoods 
following a run of MLE on the DDM. The expected input data format in CSV is as
follows: 

|  d  | sigma |  p  | 
+-----+-------+-----+
|  *  |   *   |  *  | 
...

The input path can be decalred in the FILE_PATH variable. To save the 
resulting heatmap in the imgs directory, pass 'save' as a command line argument. 
Note that the maximum for each heatmap should be toggled to allow for greater 
contrast around the minimum NLL. The maximum for each heatmap is computed as a 
percentage of the global minimum, specified by the MAX_PERCENTILE variable. 
This constant should be adjusted on a case-by-case basis. Usage is
as follows: 

python3 analysis/ddm_mle.py [save]
"""
import matplotlib.pyplot as plt 
import pandas as pd
import numpy as np
from datetime import datetime
import sys

FILE_PATH = "results/ddm_mle.csv"
MAX_PERCENTILE = 1.024

data = pd.read_csv(FILE_PATH)

# Create a pivot table to reshape the data
pivot_table = pd.pivot_table(data, values='NLL', index='sigma', columns='d')

# Convert the pivot table to a numpy array
heatmap_data = pivot_table.values
minimum = heatmap_data.min()

# Create the heatmap
_, ax = plt.subplots()
im = ax.imshow(
    heatmap_data, 
    cmap='cividis', 
    aspect='auto', 
    vmax=minimum * MAX_PERCENTILE)

xlabs = set()
for row in data['d']:
    xlabs.add(row)
xlabs = list(xlabs)
xlabs.reverse()

ylabs = set()
for row in data['sigma']:
    ylabs.add(row)
ylabs = list(ylabs)
ylabs.sort()

ax.set_xticks(np.arange(len(xlabs)), labels=xlabs)
ax.set_yticks(np.arange(len(ylabs)), labels=ylabs)
_ = ax.figure.colorbar(im, ax=ax)
ax.set_xlabel('d')
ax.set_ylabel('sigma')
ax.set_title('NLL Heatmap')

if (len(sys.argv) > 1 and "save" in sys.argv):
    currTime = datetime.now().strftime(u"%Y-%m-%d_%H:%M:%S")
    plt.savefig("imgs/ddm_mle_" + currTime + ".png")

plt.show()
