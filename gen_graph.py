# NOTE: This is AI generated, just a nice little graphic

import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from datetime import datetime

# --- YOUR DATA HERE ---
# Format: ("YYYY-MM-DD", time_in_seconds, "Label/Commit Name")
data = [
    ("2026-01-04", 294.24, "Initial Parallel Model"),
    ("2026-01-05", 248.14, "Action Pruning"),
    ("2026-01-06", 65.3, "Wordle LUT"),
    ("2026-01-19", 55.89, "FastBitset Iterator"),
    ("2026-01-24", 40.77, "SIMD State Pruning"),
    ("2026-01-25", 109.23, "Locking Bugfix :(")
]
# ----------------------

# Unpack data
dates = [datetime.strptime(d[0], "%Y-%m-%d") for d in data]
times = [d[1] for d in data]
labels = [d[2] for d in data]

# Style: Dark background for that "Terminal" look
plt.style.use('dark_background')
fig, ax = plt.subplots(figsize=(10, 6))

# Plot the line
ax.plot(dates, times, color='#00ff41', marker='o', linewidth=2, markersize=8) # Matrix Green

# Add background grid
ax.grid(color='gray', linestyle='--', linewidth=0.5, alpha=0.3)

# Annotate points (The "Why")
for i, txt in enumerate(labels):
    ax.annotate(txt, 
                (dates[i], times[i]), 
                xytext=(10, 10), 
                textcoords='offset points',
                fontsize=9,
                color='white',
                arrowprops=dict(arrowstyle='-', color='gray'))

# Formatting
ax.set_title('Solver Optimization - 50 answers, full guesses', fontsize=16, fontweight='bold', pad=20)
ax.set_ylabel('Benchmark Time (seconds)', fontsize=12)
ax.set_xlabel('Date / Commit', fontsize=12)

# Format X-axis dates nicely
fig.autofmt_xdate()
ax.fmt_xdate = mdates.DateFormatter('%Y-%m-%d')

# Save transparent for easy README embedding
plt.savefig('performance_graph.png', transparent=True, dpi=100, bbox_inches='tight')
print("Graph saved as performance_graph.png")
