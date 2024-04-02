import pandas as pd 
import numpy as np
import matplotlib.pyplot as plt


pd1 = pd.read_csv("run_4condi.csv")
pd2 = pd.read_csv("run_2.csv")
pd05 = pd.read_csv("run_05.csv")

STEP_SIZE2 = 0.0001
STEP_SIZE05 = 0.001

subjects = []
d2_steps_err = []
d05_steps_err = []

plt.figure(figsize=(8, 8))

for (i, row), (j, row2), (k, row05) in zip(pd1.iterrows(), pd2.iterrows(), pd05.iterrows()): 
    subjects.append(i)
    baseline = row["d"]
    expected_d05 = baseline * 2
    expected_d2 = baseline * 0.5
    d2_steps_err.append(
        round((row2["d"] - expected_d2) / STEP_SIZE2)
    )
    d05_steps_err.append(
        round((row05["d"] - expected_d05) / STEP_SIZE05)
    )

plt.vlines(len(subjects) / 2, -8, 8, "gainsboro")
plt.plot(subjects, [0] * len(subjects), '--', color="gainsboro")

plt.plot(subjects, d2_steps_err, label="VD*2 num steps off")
plt.plot(subjects, d05_steps_err, label="VD*0.5 num steps off")

plt.text(15, 5, "low VD")
plt.text(30, 5, "high VD")

plt.legend()
plt.title("Comparison of step miscalculation")
plt.savefig("step_error_comparison.png")