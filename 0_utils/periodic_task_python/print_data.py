import re
from datetime import datetime
import matplotlib.pyplot as plt

# --- Parameters ---
log_file = "real_time_task_log.txt"
period = 0.1  # must match the logger's value

# --- Log parsing ---
frames = []
with open(log_file, "r") as f:
    lines = f.readlines()

frame_data = {}
for line in lines:
    line = line.strip()
    if line.startswith("[Frame"):
        if frame_data:
            frames.append(frame_data)
        frame_data = {"frame_id": int(re.findall(r'\d+', line)[0])}
    elif line.startswith("Start Time"):
        frame_data["start"] = datetime.fromisoformat(line.split(": ", 1)[1])
    elif line.startswith("Finish Time"):
        frame_data["finish"] = datetime.fromisoformat(line.split(": ", 1)[1])
    elif line.startswith("Release Time"):
        frame_data["release"] = datetime.fromisoformat(line.split(": ", 1)[1])
    elif line.startswith("Execution time"):
        frame_data["execution_time"] = float(re.findall(r"[-+]?\d*\.\d+|\d+", line)[0])
    elif line.startswith("Jitter"):
        frame_data["jitter"] = float(re.findall(r"[-+]?\d*\.\d+|\d+", line)[0])
    elif line.startswith("Estimated CPU usage"):
        frame_data["cpu_usage"] = float(re.findall(r"[-+]?\d*\.\d+|\d+", line)[0])
    elif "OVERRUN" in line:
        frame_data["overrun_flag"] = True
    elif "ON TIME" in line:
        frame_data["overrun_flag"] = False

if frame_data:
    frames.append(frame_data)

# --- Compute relative times and deadline check ---
t0 = frames[0]["release"]
for f in frames:
    f["r_rel"] = (f["release"] - t0).total_seconds()
    f["s_rel"] = (f["start"] - t0).total_seconds()
    f["f_rel"] = (f["finish"] - t0).total_seconds()
    f["deadline"] = f["r_rel"] + period
    f["overrun"] = f["f_rel"] > f["deadline"]

# --- Plot execution bars and deadline misses ---
plt.figure(figsize=(14, 2))

for f in frames:
    start = f["s_rel"]
    finish = f["f_rel"]
    deadline = f["deadline"]
    height = 0.01  # slim bar

    # Vertical release line
    plt.axvline(x=f["r_rel"], color='red', linestyle='--', linewidth=0.5)

    if not f["overrun"]:
        plt.barh(y=0, width=finish - start, left=start, height=height, color='royalblue', edgecolor='black')
    else:
        ok_part = max(0, deadline - start)
        overrun_part = finish - deadline
        if ok_part > 0:
            plt.barh(y=0, width=ok_part, left=start, height=height, color='royalblue', edgecolor='black')
        plt.barh(y=0, width=overrun_part, left=deadline, height=height, color='firebrick', edgecolor='black')

plt.ylim(0, 0.01)
plt.yticks([])
plt.xlabel("Time (s)")
plt.grid(axis='x', linestyle=':', alpha=0.6)
plt.tight_layout()
plt.show()

# --- Summary statistics ---
exec_times = [f["execution_time"] for f in frames]
jitters = [abs(f["jitter"]) for f in frames]
cpu_usages = [f["cpu_usage"] for f in frames]
num_overruns = sum(f["overrun_flag"] for f in frames)

print("\n=== REAL-TIME TASK ANALYSIS ===")
print(f"Total frames executed      : {len(frames)}")
print(f"Period (s)                 : {period:.3f}")
print(f"Worst-case execution time  : {max(exec_times):.6f} s")
print(f"Average execution time     : {sum(exec_times)/len(exec_times):.6f} s")
print(f"Maximum absolute jitter    : {max(jitters):.6f} s")
print(f"Average estimated CPU usage: {sum(cpu_usages)/len(cpu_usages):.2f}%")
print(f"Peak estimated CPU usage   : {max(cpu_usages):.2f}%")
print(f"Deadline misses            : {num_overruns}")
print(f"Deadline miss rate         : {(num_overruns / len(frames)) * 100:.2f}%")
