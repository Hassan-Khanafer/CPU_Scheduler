import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Example data (replace this with your CSV or manually input data)
data = {
    "Scheduler": ["FCFS", "FCFS", "FCFS", "EP", "EP", "EP", "EP", "RR", "RR", "RR"],
    "Trace": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10],
    "Throughput": [43.8, 82.2, 195.6667, 38.75, 30.3333, 102.3333, 41.6667, 1546.5, 261.5, 1500.6667],
    "Mean TAT": [138, 112.2, 242.3333, 92.25, 54.6667, 255.6667, 77.3333, 2184, 474, 6496.5],
    "Wait Times": [
        "61, 109, 44, 62, 91", "0, 5, 36, 25, 64", "16, 39, 0",
        "54, 50, 47, 0", "19, 11, 8", "240, 108, 93",
        "10, 17, 15", "600, 567", "148, 221", "3875, 6074, 4200, 5724, 4525, 4978"
    ]
}

# Convert to DataFrame
df = pd.DataFrame(data)

# Plot Throughput Comparison
plt.figure(figsize=(10, 6))
sns.barplot(data=df, x="Scheduler", y="Throughput", ci=None, hue="Trace", dodge=True)
plt.title("Throughput Comparison by Scheduler")
plt.ylabel("Throughput (Jobs/)")
plt.xlabel("Scheduler")
plt.legend(title="Trace")
plt.show()

# Plot Mean TAT
plt.figure(figsize=(10, 6))
sns.lineplot(data=df, x="Trace", y="Mean TAT", hue="Scheduler", marker="o")
plt.title("Mean Turnaround Time (TAT) by Scheduler")
plt.ylabel("Mean TAT")
plt.xlabel("Trace")
plt.show()

# Calculate Average Wait Time for each Scheduler
df["Average Wait Time"] = df["Wait Times"].apply(
    lambda x: sum(map(int, x.split(","))) / len(x.split(","))
)

# Plot Average Wait Times Distribution as Bar Plot
plt.figure(figsize=(10, 6))
sns.barplot(data=df, x="Scheduler", y="Average Wait Time")
plt.title("Average Wait Times by Scheduler")
plt.ylabel("Average Wait Time")
plt.xlabel("Scheduler")
plt.show()
