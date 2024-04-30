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

num_subjects = pd1.shape[0]
print(f"num_subjects={num_subjects}")

for (i, row), (j, row2), (k, row05) in zip(pd1.iterrows(), pd2.iterrows(), pd05.iterrows()): 
    subjects.append(i)

    if row[PARAM] - row2[PARAM]== 0: 
        d1_to_d2.append(1)
    else:
        d1_to_d2.append(row2[PARAM]/ row[PARAM])

    if row[PARAM] - row05[PARAM] == 0: 
        d1_to_d05.append(1)
    else: 
        if row05[PARAM] == 0: 
            row05[PARAM] = 0.1
        d1_to_d05.append(row05[PARAM]/ row[PARAM] )

half_subjs = num_subjects // 2
subjects = subjects[:half_subjs]
for i in range(half_subjs): 
    d1_to_d05[i] = (d1_to_d05[i] + d1_to_d05[i + half_subjs]) / 2
for i in range(half_subjs): 
    d1_to_d2[i] = (d1_to_d2[i] + d1_to_d2[i + half_subjs]) / 2
d1_to_d05 = d1_to_d05[:half_subjs]
d1_to_d2 = d1_to_d2[:half_subjs]


lim = max(max(d1_to_d05), max(d1_to_d2))
if PARAM == "d": 
    plt.plot(subjects, [0.5] * len(subjects), '--', color="gainsboro")
    plt.plot(subjects, [2] * len(subjects), '--', color="gainsboro")
else: 
    plt.plot(subjects, [1] * len(subjects), '--', color="gainsboro")

plt.plot(subjects, d1_to_d2, label="doubled VD : baseline", color="blue")
plt.plot(subjects, d1_to_d05, label="half VD : baseline", color="red")



plt.xlabel("Subject ID")
plt.ylabel("Ratio")


plt.legend()

add = ""
if suffix == "_fixed_st":
    add = "(Fixed Sigma & Theta)"
elif suffix == "_fixed": 
    add = "(Fixed parameter space)"

plt.title(f"Averaged Comparison of {PARAM} ratios for value difference scaling {add}")
plt.savefig(f"v{version}_output/{PARAM}_avg_comparison{suffix}.png")