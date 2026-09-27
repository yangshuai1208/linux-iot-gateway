import os
import pty
import re
import time

master, slave = pty.openpty()
port = os.ttyname(slave)

print("Mock STM32 port:", port, flush=True)
print("Port exists:", os.path.exists(port), flush=True)

buffer = b""
attempts = {}

while True:
    data = os.read(master, 256)
    buffer += data

    while b"\n" in buffer:
        raw, buffer = buffer.split(b"\n", 1)
        line = raw.decode("ascii", errors="replace").strip()

        print("RX:", line, flush=True)

        match = re.fullmatch(
            r"SEQ:(\d+) CMD:HAND_(OPEN|GRAB|RELEASE|STOP)",
            line
        )

        if match is None:
            continue

        seq = int(match.group(1))
        attempts[seq] = attempts.get(seq, 0) + 1

        if attempts[seq] == 1:
            print("Simulated ACK loss:", seq, flush=True)
            continue

        progress = f"ACK:{seq} IN_PROGRESS\r\n"
        os.write(master, progress.encode("ascii"))
        print("TX:", progress.strip(), flush=True)

        time.sleep(0.2)

        completed = f"ACK:{seq} OK\r\n"
        os.write(master, completed.encode("ascii"))
        print("TX:", completed.strip(), flush=True)
