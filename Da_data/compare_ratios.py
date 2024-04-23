import pandas as pd 
import numpy as np
import matplotlib.pyplot as plt

PARAM = "d"


pd1 = pd.read_csv("run_4condi_fixed_st.csv")
pd2 = pd.read_csv("run_2_fixed_st.csv")
pd05 = pd.read_csv("run_05_fixed_st.csv")

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
plt.plot(subjects, [0.5] * len(subjects), '--', color="gainsboro")
plt.plot(subjects, [2] * len(subjects), '--', color="gainsboro")

plt.plot(subjects, d1_to_d2, label="baseline : doubled")
plt.plot(subjects, d1_to_d05, label="baseline : half")

plt.text(15, 1.25, "low VD")
plt.text(30, 1.25, "high VD")

plt.ylim(top=2.5)

plt.xlabel("Trial No.")
plt.ylabel("Ratio")


plt.legend()
plt.title(f"Comparison of {PARAM} ratios for value difference scaling (fixed Sigma & Theta)")
plt.savefig(f"{PARAM}_comparison_fixed_st.png")