import time
from datetime import datetime, timedelta
import psutil
import argparse

# --- Parsing degli argomenti ---
parser = argparse.ArgumentParser(description="Real-time periodic task simulator.")
parser.add_argument("--N", type=int, required=True, help="Intensità del carico di lavoro (moltiplicatore).")
args = parser.parse_args()

N = args.N  # Moltiplicatore carico

# --- Funzione workload ---
def do_workload():
    acc = 0
    for _ in range(500000 * N):
        acc += 1

# --- Parametri ---
log_file = "real_time_task_log.txt"
period = 0.1  # seconds (100ms)
frame_count = 100

process = psutil.Process()
time_zero = datetime.now()
all_log_entries = []

for frame_id in range(frame_count):
    release_time = time_zero + timedelta(seconds=frame_id * period)
    start_time = datetime.now()

    do_workload()

    finish_time = datetime.now()

    execution_time = (finish_time - start_time).total_seconds()
    jitter = (start_time - release_time).total_seconds()
    deadline = release_time + timedelta(seconds=period)
    sleep_delay = (deadline - datetime.now()).total_seconds()
    overrun = sleep_delay < 0

    cpu_usage = (execution_time / period) * 100
    current_core = process.cpu_num()

    log_entry = "\n".join([
        f"[Frame {frame_id}]",
        f"CPU core: {current_core}",
        f"Estimated CPU usage: {cpu_usage:.2f}%",
        f"Start Time: {start_time}",
        f"Finish Time: {finish_time}",
        f"Release Time: {release_time}",
        f"Execution time: {execution_time:.6f} sec",
        f"Jitter: {jitter:+.6f} sec",
        f"{f'[OVERRUN] {abs(sleep_delay):.6f} sec late' if overrun else '[ON TIME]'}",
        "-" * 40
    ])
    all_log_entries.append(log_entry)

    if not overrun:
        time.sleep(sleep_delay)

with open(log_file, "w") as f:
    f.write("Real-time task execution log\n\n")
    f.write("\n".join(all_log_entries))

print(f"Log salvato in: {log_file}")
