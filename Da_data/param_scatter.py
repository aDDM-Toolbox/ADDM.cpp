import matplotlib.pyplot as plt
import pandas as pd 
import numpy as np

high_vd = "o"
low_vd = "x"


pd1 = pd.read_csv("run_4condi_fixed.csv")
pd2 = pd.read_csv("run_2_fixed.csv")
pd05 = pd.read_csv("run_05_fixed.csv")
plt.figure(figsize=(8, 8))

bases = pd1["d"].to_numpy()
dbls = pd2["d"].to_numpy()
hlvs = pd05["d"].to_numpy()

assert len(bases) == len(dbls)
assert len(dbls) == len(hlvs)
m = len(bases) // 2
plt.scatter(bases[:m], dbls[:m], label="VD*2 Low", zorder=10, color="blue", marker=low_vd)
plt.scatter(bases[m:], dbls[m:], label="VD*2 High", zorder=11, color="blue", marker=high_vd)
plt.scatter(bases[:m], hlvs[:m], label="VD/2 Low", zorder=12, color="red", marker=low_vd)
plt.scatter(bases[m:], hlvs[m:], label="VD/2 High", zorder=13, color="red", marker=high_vd)

x = np.linspace(0, np.max(bases), 100)
plt.plot(x, 2 * x, color="silver", zorder=0)
plt.plot(x, x / 2, color="silver", zorder=1)

plt.legend()
plt.title("Comparison of Expected Drift Rate to Recovered (Fixed)")

plt.xlabel("Baseline Drift Rate")
plt.ylabel("Recovered Drift Rate for VD Change")

plt.savefig("scatter_params_fixed.png")