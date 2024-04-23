import pandas as pd 
import numpy as np
import matplotlib.pyplot as plt


pd1 = pd.read_csv("run_4condi_fixed_st.csv")
pd2 = pd.read_csv("run_2_fixed_st.csv")
pd05 = pd.read_csv("run_05_fixed_st.csv")

STEP_SIZE2 = 0.0001
STEP_SIZE05 = 0.0001

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

plt.vlines(len(subjects) / 2, -40, 10, "gainsboro")
plt.plot(subjects, [0] * len(subjects), '--', color="gainsboro")

plt.plot(subjects, d2_steps_err, label="VD*2")
plt.plot(subjects, d05_steps_err, label="VD*0.5")

plt.xlabel("Trial No.")
plt.ylabel("Number of Steps off of Expected Parameter")

plt.text(15, 5, "low VD")
plt.text(30, 5, "high VD")

plt.legend()
plt.title("Comparison of step miscalculation (Fixed Sigma & Theta)")
plt.savefig("step_error_comparison_fixed_st.png")

d2_err_lowVD = d2_steps_err[:len(d2_steps_err) // 2]
d2_err_highVD = d2_steps_err[len(d2_steps_err) // 2:]

d05_err_lowVD = d05_steps_err[:len(d05_steps_err) // 2]
d05_err_highVD = d05_steps_err[len(d05_steps_err) // 2:]


with open("step_error_stats_fixed_st.txt", "w") as fp: 
    fp.write("Doubled Value Difference number of steps off\n")
    fp.write(f"low VD range: [{np.min(d2_err_lowVD)}, {np.max(d2_err_lowVD)}]\n")
    fp.write(f"low VD mean: {np.mean(d2_err_lowVD)}\n")
    fp.write(f"low VD standard deviation: {np.std(d2_err_lowVD)}\n")

    fp.write(f"high VD range: [{np.min(d2_err_highVD)}, {np.max(d2_err_highVD)}]\n")
    fp.write(f"high VD mean: {np.mean(d2_err_highVD)}\n")
    fp.write(f"high VD standard deviation: {np.std(d2_err_highVD)}\n")

    fp.write("\n")

    fp.write("Halved Value Difference number of steps off\n")
    fp.write(f"low VD range: [{np.min(d05_err_lowVD)}, {np.max(d05_err_lowVD)}]\n")
    fp.write(f"low VD mean: {np.mean(d05_err_lowVD)}\n")
    fp.write(f"low VD standard deviation: {np.std(d05_err_lowVD)}\n")

    fp.write(f"high VD range: [{np.min(d05_err_highVD)}, {np.max(d05_err_highVD)}]\n")
    fp.write(f"high VD mean: {np.mean(d05_err_highVD)}\n")
    fp.write(f"high VD standard deviation: {np.std(d05_err_highVD)}\n")
