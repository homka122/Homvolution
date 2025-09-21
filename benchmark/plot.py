import matplotlib.pyplot as plt
import numpy as np

methods = []
data = []

filename = "results3"

with open(filename + ".csv") as f:
    for line in f:
        parts = line.strip().split(",")
        if not parts or len(parts) < 2:
            continue
        method = parts[0]
        values = [float(x) for x in parts[1:]]
        methods.append(method)
        data.append(values)

fig, ax = plt.subplots(figsize=(10, 6))

# boxplot
bp = ax.boxplot(
    data,
    positions=np.arange(len(data)) + 1,
    widths=0.6,
    patch_artist=True,
    showfliers=False,
)

colors = ["#FF9999", "#99FF99", "#9999FF", "#FFCC99", "#CC99FF", "#66CCCC"]
for patch, color in zip(bp["boxes"], colors * 10):
    patch.set_facecolor(color)
    patch.set_alpha(0.6)

for i, d in enumerate(data, start=1):
    x = np.random.normal(i, 0.04, size=len(d))
    ax.plot(x, d, "o", markersize=4, alpha=0.7, color="black")

ax.set_xticks(np.arange(1, len(methods) + 1))
ax.set_xticklabels(methods, rotation=20)
ax.set_ylabel("Time (sec)")
ax.set_title("Benchmark results")

plt.grid(True, axis="y", linestyle="--", alpha=0.6)
plt.tight_layout()

plt.savefig(filename + ".png", dpi=200)
plt.show()
