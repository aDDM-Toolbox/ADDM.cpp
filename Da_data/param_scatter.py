import matplotlib.pyplot as plt
import pandas as pd 
import numpy as np
import sys

from labellines import labelLines

high_vd = "o"
low_vd = "x"

version = sys.argv[1]
labelPos = sys.argv[2]
try: 
    suffix = sys.argv[3]
except IndexError: 
    suffix = ""


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

zh = np.polyfit(bases[:m], hlvs[:m], 1)
ph = np.poly1d(zh)
plt.plot(x, ph(x), color="gainsboro", label=f"y≈{round(ph.coef[0], 2)}x")

zl = np.polyfit(bases[m:], hlvs[m:], 1)
pl = np.poly1d(zl)
plt.plot(x, pl(x), color="gainsboro", label=f"y≈{round(pl.coef[0], 2)}x")

lines = plt.gca().get_lines()
labelLines(lines, align=True, xvals=[0.0085] * len(lines), drop_label=True, color='k')



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