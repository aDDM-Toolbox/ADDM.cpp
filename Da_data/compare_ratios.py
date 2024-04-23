import pandas as pd 
import numpy as np
import matplotlib.pyplot as plt
import sys

PARAM = sys.argv[1]

assert PARAM in ["d", "sigma", "theta"]

version = sys.argv[2]
try: 
    suffix = sys.argv[3]
except IndexError: 
    suffix = ""


pd1 = pd.read_csv(f"v{version}_output/run_4condi{suffix}.csv")
pd2 = pd.read_csv(f"v{version}_output/run_2{suffix}.csv")
pd05 = pd.read_csv(f"v{version}_output/run_05{suffix}.csv")

subjects = []
d1_to_d2 = []
d1_to_d05 = []

plt.figure(figsize=(8, 8))

for (i, row), (j, row2), (k, row05) in zip(pd1.iterrows(), pd2.iterrows(), pd05.iterrows()): 
    subjects.append(i)
    if row[PARAM] - row2[PARAM]== 0: 
        d1_to_d2.append(1)
    else:
        d1_to_d2.append(row[PARAM] / row2[PARAM])
    if row[PARAM] - row05[PARAM] == 0: 
        d1_to_d05.append(1)
    else: 
        if row05[PARAM] == 0: 
            row05[PARAM] = 0.1
        d1_to_d05.append(row[PARAM] / row05[PARAM])

plt.vlines(len(subjects) / 2, 0.25, 2.25, "gainsboro")
if PARAM == "d": 
    plt.plot(subjects, [0.5] * len(subjects), '--', color="gainsboro")
    plt.plot(subjects, [2] * len(subjects), '--', color="gainsboro")
else: 
    plt.plot(subjects, [1] * len(subjects), '--', color="gainsboro")

plt.plot(subjects, d1_to_d2, label="baseline : doubled")
plt.plot(subjects, d1_to_d05, label="baseline : half")

plt.text(15, 1.25, "high VD")
plt.text(30, 1.25, "low VD")

plt.ylim(top=2.5)

plt.xlabel("Subject ID")
plt.ylabel("Ratio")


plt.legend()

add = ""
if suffix == "_fixed_st":
    add = "(Fixed Sigma & Theta)"
elif suffix == "_fixed": 
    add = "(Fixed parameter space)"

plt.title(f"Comparison of {PARAM} ratios for value difference scaling {add}")
plt.savefig(f"v{version}_output/{PARAM}_comparison{suffix}.png")