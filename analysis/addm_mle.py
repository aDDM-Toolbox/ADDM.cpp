""" 
@brief Create heatmaps for Maximum Likelihood Estimation on the aDDM. 

This script creates a table of heatmaps for aDDM Maximum Likelihood Estimation
Data NLLs. The expected input data format in CSV is as follows:  

|  d  | sigma | theta |  p  |
+-----+-------+-------+-----+
|  *  |   *   |   *   |  *  |
... 

The input file can be declared in the FILE_PATH variable. To save the resulting 
heatmap in the imgs directory, pass 'save' as a command line argument. Note 
that the maximum for each heatmap should be toggled to allow for greater 
contrast around the minimum NLL. The maximum for each heatmap is computed as a 
percentage of the global minimum, specified by the MAX_PERCENTILE variable. 
This constant should be adjusted on a case-by-case basis. Usage is
as follows: 

python3 analysis/addm_mle.py [save]
"""
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import sys

from datetime import datetime

FILE_PATH = "results/addm_mle.csv"
MAX_PERCENTILE = 1.024
PROB_LABEL = 'p'

df = pd.read_csv(FILE_PATH)
thetas = df['theta'].unique()

num_thetas = len(thetas)
num_cols = 2  # Number of columns in the subplot grid
num_rows = (num_thetas - 1) // num_cols + 1  # Number of rows in the subplot grid

fig, axes = plt.subplots(num_rows, num_cols, figsize=(12, 9))

vmin = df[PROB_LABEL].min()

for i, theta in enumerate(thetas):
    data = df[df['theta'] == theta]

    row = i // num_cols
    col = i % num_cols
    ax = axes[row, col] if num_rows > 1 else axes[col]

    heatmap = ax.imshow(
        np.reshape(data[PROB_LABEL], (-1, len(data['d'].unique()))),
        cmap='gist_heat_r',
        extent=[0, len(data['d'].unique()) - 1, 0, len(data['sigma'].unique()) - 1],
        origin='lower',
        aspect='auto',
        vmin=vmin,
        vmax=(MAX_PERCENTILE * vmin)
    )
    ax.set_title(f'theta = {theta}')
    ax.set_xlabel('d')
    ax.set_ylabel('sigma')

    ax.set_xticks(np.arange(len(data['d'].unique())))
    ax.set_xticklabels(data['d'].unique())
    ax.set_yticks(np.arange(len(data['sigma'].unique())))
    ax.set_yticklabels(data['sigma'].unique())

    fig.colorbar(heatmap, ax=ax)

min_nll_idx = df[PROB_LABEL].idxmin()
min_d = df.loc[min_nll_idx, 'd']
min_sigma = df.loc[min_nll_idx, 'sigma']
min_theta = df.loc[min_nll_idx, 'theta']

annotation = f'Min NLL: d={min_d}, sigma={min_sigma}, theta={min_theta}'
fig.text(0.5, 0.02, annotation, ha='center', va='center', fontsize=12)

fig.tight_layout()

if (len(sys.argv) > 1 and "save" in sys.argv):
    currTime = datetime.now().strftime(u"%Y-%m-%d_%H:%M:%S")
    plt.savefig("imgs/addm_mle_" + currTime + ".png")

plt.show()

