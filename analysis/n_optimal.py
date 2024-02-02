from datetime import datetime
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
import sys

N = 5

df = pd.read_csv("results/addm_mle.csv")
df = df.sort_values("p")
df = df.head(N)

header_params = list(df.columns.values)[:-1]
hp_string = "("
for param in header_params: 
    hp_string += param + ", "
hp_string = hp_string[:-2] + ")"

x_vals = []
for i, row in df.iterrows(): 
    curr = "("
    for param in header_params: 
        curr += str(row[param]) + ", "
    curr = curr[:-2] + ")"
    x_vals.append(curr)
y_vals = np.asarray(df["p"])

plt.bar(x_vals, y_vals)
plt.xticks(rotation=45, ha='right')
height_delta = 0.01
plt.ylim([min(y_vals) * (1 - height_delta), max(y_vals) * (1 + height_delta)])
plt.tight_layout()

if (len(sys.argv) > 1 and "save" in sys.argv):
    currTime = datetime.now().strftime(u"%Y-%m-%d_%H:%M:%S")
    plt.savefig("imgs/n_optimal_" + currTime + ".png")

plt.show()
