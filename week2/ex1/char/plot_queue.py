#!/usr/bin/env python3
"""Ve do day queue_raw / queue_filtered theo thoi gian tu log Hercules.

Dong du lieu can co dang:  D,<tick>,<raw>,<filt>
Cac dong khac (seq,ax,ay,az, ky tu rac, ...) se bi bo qua.

Dung:
    python plot_queue.py normal_115200.txt
    python plot_queue.py slow_9600.txt --qlen 4 --title "UART 9600"
Dau ra (cung ten voi file log):
    <ten>_queue_fill.csv   du lieu: time_ms, queue_raw, queue_filtered
    <ten>_queue_fill.png   do thi
"""
import argparse
import csv
import os
import sys

import matplotlib.pyplot as plt


def parse_log(path):
    ticks, raw, filt = [], [], []
    with open(path, errors="ignore") as f:
        for line in f:
            line = line.strip()
            if not line.startswith("D,"):
                continue
            p = line.split(",")
            if len(p) != 4:
                continue
            try:
                t, r, q = int(p[1]), int(p[2]), int(p[3])
            except ValueError:
                continue  # dong bi cat do hoac rac
            ticks.append(t)
            raw.append(r)
            filt.append(q)
    return ticks, raw, filt


def unwrap_ticks(ticks):
    """tick la uint16 nen quay vong sau 65536 ms; cong bu va dua ve 0."""
    out, offset, prev = [], 0, None
    for t in ticks:
        if prev is not None and t < prev:
            offset += 65536
        out.append(t + offset)
        prev = t
    base = out[0]
    return [t - base for t in out]


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("log", help="file log luu tu Hercules")
    ap.add_argument("--qlen", type=int, default=4,
                    help="QUEUE_LENGTH, ve duong gioi han (mac dinh 4)")
    ap.add_argument("--title", default="Do day queue theo thoi gian")
    ap.add_argument("--show", action="store_true", help="hien cua so do thi")
    args = ap.parse_args()

    ticks, raw, filt = parse_log(args.log)
    if not ticks:
        sys.exit("Khong tim thay dong 'D,tick,raw,filt' nao trong file log.")

    t = unwrap_ticks(ticks)
    stem = os.path.splitext(args.log)[0]

    csv_path = stem + "_queue_fill.csv"
    with open(csv_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["time_ms", "queue_raw", "queue_filtered"])
        w.writerows(zip(t, raw, filt))

    fig, ax = plt.subplots(figsize=(10, 4.5))
    ax.step(t, raw, where="post", label="queue_raw", linewidth=1.8)
    ax.step(t, filt, where="post", label="queue_filtered", linewidth=1.8)
    ax.axhline(args.qlen, color="gray", linestyle="--", linewidth=1,
               label=f"QUEUE_LENGTH = {args.qlen}")
    ax.set_xlabel("Thoi gian (ms)")
    ax.set_ylabel("So phan tu trong queue")
    ax.set_yticks(range(0, args.qlen + 2))
    ax.set_ylim(-0.2, args.qlen + 0.8)
    ax.set_title(args.title)
    ax.grid(True, alpha=0.3)
    ax.legend(loc="upper left")
    fig.tight_layout()

    png_path = stem + "_queue_fill.png"
    fig.savefig(png_path, dpi=150)

    print(f"So mau: {len(t)} | khoang thoi gian: {t[-1]} ms")
    print(f"Max queue_raw = {max(raw)}, max queue_filtered = {max(filt)}")
    print(f"Da luu: {csv_path}")
    print(f"Da luu: {png_path}")

    if args.show:
        plt.show()


if __name__ == "__main__":
    main()
