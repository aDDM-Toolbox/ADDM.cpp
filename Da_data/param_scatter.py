import matplotlib.pyplot as plt
import pandas as pd 
import numpy as np
import sys

from labellines import labelLines

high_vd = "o"
low_vd = "x"

version = sys.argv[1]
labelPos = float(sys.argv[2])
try: 
    suffix = sys.argv[3]
except IndexError: 
    suffix = ""

BEST_FIT_MARGIN = 0.01


pd1 = pd.read_csv(f"v{version}_output/run_4condi{suffix}.csv")
pd2 = pd.read_csv(f"v{version}_output/run_2{suffix}.csv")
pd05 = pd.read_csv(f"v{version}_output/run_05{suffix}.csv")
plt.figure(figsize=(8, 8))

bases = pd1["d"].to_numpy()
dbls = pd2["d"].to_numpy()
hlvs = pd05["d"].to_numpy()

assert len(bases) == len(dbls)
assert len(dbls) == len(hlvs)
m = len(bases) // 2
plt.scatter(bases[:m], dbls[:m], label="VD*2 High", zorder=10, color="blue", marker=high_vd)
plt.scatter(bases[m:], dbls[m:], label="VD*2 Low", zorder=11, color="blue", marker=low_vd)
plt.scatter(bases[:m], hlvs[:m], label="VD/2 High", zorder=12, color="red", marker=high_vd)
plt.scatter(bases[m:], hlvs[m:], label="VD/2 Low", zorder=13, color="red", marker=low_vd)

x = np.linspace(0, np.max(bases), 100)
plt.plot(x, 2 * x, color="darkgrey", zorder=0, label="y=2x", lw=3)
plt.plot(x, x / 2, color="darkgrey", zorder=1, label="y=x/2", lw=3)

b1 = bases[:m]
h1 = hlvs[:m]
b1 = b1[: ,np.newaxis]
a1, _, _, _ = np.linalg.lstsq(b1, h1, rcond=None)
plt.plot(x, a1[0]*x, color="gainsboro", label=f"y={round(a1[0], 2)}x")

b2 = bases[m:]
h2 = hlvs[m:]
b2 = b2[: ,np.newaxis]
a2, _, _, _ = np.linalg.lstsq(b2, h2, rcond=None)
plt.plot(x, a2[0]*x, color="gainsboro", label=f"y={round(a2[0], 2)}x")

lines = plt.gca().get_lines()
labelLines(lines, align=True, xvals=[labelPos] * len(lines), drop_label=True, color='k')



plt.legend()
add = ""
if suffix == "_fixed_st":
    add = "(Fixed Sigma & Theta)"
elif suffix == "_fixed": 
    add = "(Fixed parameter space)"
plt.title(f"Comparison of Expected Drift Rate to Recovered {add}")

plt.xlabel("Baseline Drift Rate")
plt.ylabel("Recovered Drift Rate for VD Change")

plt.savefig(f"v{version}_output/scatter_params{suffix}.png")