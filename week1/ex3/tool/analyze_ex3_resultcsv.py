#!/usr/bin/env python3
"""Analyze GPIO toggle timing from a PulseView CSV export.

Counts transitions (edges), not repeated samples. Measures time between every
consecutive transition, so a GPIO toggled every 17 ms should yield about 17 ms,
not 34 ms (which is the rising-edge-to-rising-edge blink period).

Results are written automatically to result.csv (same folder as this script;
created if missing). Delay/Tick are inferred from the capture file name, e.g.
vtaskdelay_1000hz.csv -> vTaskDelay, 1000.

Example (run from project root):
    python tool/analyze_ex3.py captures/vtaskdelay_1000hz.csv \
        --period-ms 17 --sample-count 800
"""

import argparse
import csv
import math
import re
import statistics
import sys
import unicodedata
from pathlib import Path


def norm(s):
    # Bỏ dấu tiếng Việt để so khớp tên cột cũ (vd. "Chu kỳ TB (ms)") với tên mới.
    s = unicodedata.normalize("NFKD", (s or "").strip().lower().replace("đ", "d"))
    s = "".join(c for c in s if not unicodedata.combining(c))
    return re.sub(r"[^a-z0-9]+", "", s)


# Tên cột chuẩn (tiếng Anh, ASCII) của result.csv
RESULT_HEADERS = ["Delay", "Tick", "Avg Period (ms)", "Jitter (us)", "Total Drift (ms)"]
# Tên cột được chấp nhận khi đọc (gồm cả tên tiếng Việt cũ) - đã chuẩn hóa bằng norm()
HEADER_ALIASES = [
    ["delay"],
    ["tick", "tickrate", "configtickratehz"],
    ["avgperiodms", "chukytbms", "periodms"],
    ["jitterus", "jitter"],
    ["totaldriftms", "drifttongsau800canhms", "drifttongms", "driftms"],
]


def infer_from_filename(name):
    """vtaskdelay_1000hz.csv -> ("vTaskDelay", 1000); không suy ra được thì trả None."""
    n = name.lower()
    delay = None
    if "xtaskdelayuntil" in n or "delayuntil" in n:
        delay = "xTaskDelayUntil"
    elif "vtaskdelay" in n:
        delay = "vTaskDelay"
    m = re.search(r"(\d+)\s*hz", n)
    tick = int(m.group(1)) if m else None
    return delay, tick


def update_result_csv(result_path, delay, tick, avg_ms, jitter_us, drift_ms):
    """Ghi kết quả vào dòng (delay, tick). Tạo file nếu chưa có; đổi header sang tiếng Anh."""
    if result_path.is_file():
        delimiter = detect_delimiter(result_path)
        with result_path.open("r", encoding="utf-8-sig", newline="") as f:
            rows = list(csv.reader(f, delimiter=delimiter))
        if not rows:
            raise ValueError(f"{result_path} rỗng.")
    else:
        delimiter = ","
        rows = [RESULT_HEADERS[:]]
        for t in (1000, 100):
            for d in ("vTaskDelay", "xTaskDelayUntil"):
                rows.append([d, str(t), "", "", ""])

    normalized = [norm(x) for x in rows[0]]
    idx = []
    for aliases in HEADER_ALIASES:
        found = next((normalized.index(a) for a in aliases if a in normalized), None)
        idx.append(found)
    if None in idx:
        raise ValueError(
            "Không nhận diện đủ cột trong result.csv. Cần: " + ", ".join(RESULT_HEADERS)
            + f". Header hiện có: {rows[0]}"
        )
    delay_idx, tick_idx, avg_idx, jitter_idx, drift_idx = idx

    def key(x):
        return norm(x).replace("vtaskdelayuntil", "xtaskdelayuntil")

    target = None
    for row in rows[1:]:
        if len(row) <= max(delay_idx, tick_idx):
            continue
        try:
            row_tick = int(float(row[tick_idx]))
        except (ValueError, TypeError):
            continue
        if key(row[delay_idx]) == key(delay) and row_tick == tick:
            target = row
            break
    if target is None:
        raise ValueError(f"Không tìm thấy dòng Delay={delay}, Tick={tick} trong {result_path}.")

    width = len(rows[0])
    while len(target) < width:
        target.append("")
    target[avg_idx] = f"{avg_ms:.6f}"
    target[jitter_idx] = f"{jitter_us:.3f}"
    target[drift_idx] = f"{drift_ms:.6f}"
    for i, name in zip(idx, RESULT_HEADERS):   # chuyển header sang tên tiếng Anh
        rows[0][i] = name

    with result_path.open("w", encoding="utf-8-sig", newline="") as f:
        csv.writer(f, delimiter=delimiter).writerows(rows)


def detect_delimiter(path):
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        sample = f.read(8192)
    try:
        return csv.Sniffer().sniff(sample, delimiters=",;\t").delimiter
    except csv.Error:
        return ","


def read_csv(path, delimiter):
    with path.open("r", encoding="utf-8-sig", newline="") as f:
        lines = [line for line in f if line.strip() and not line.lstrip().startswith(";")]
    if not lines:
        raise ValueError("CSV rỗng hoặc chỉ có dòng chú thích.")
    reader = csv.DictReader(lines, delimiter=delimiter)
    if not reader.fieldnames:
        raise ValueError("Không đọc được header CSV. Hãy bật Output header khi export.")
    # CSV may contain repeated headers such as "logic, logic, logic".
    # DictReader normally overwrites earlier columns with the same name, so
    # give duplicate headers unique internal names while preserving their order.
    original_headers = [(h or "").strip() for h in reader.fieldnames]
    headers = []
    seen = {}
    for h in original_headers:
        count = seen.get(h, 0) + 1
        seen[h] = count
        headers.append(h if count == 1 else f"{h}__{count}")
    rows = []
    for values in csv.reader(lines[1:], delimiter=delimiter):
        row = {}
        for i, h in enumerate(headers):
            row[h] = values[i] if i < len(values) else ""
        rows.append(row)
    return headers, rows


def select_column(headers, requested, kind):
    if requested:
        for h in headers:
            if h == requested or norm(h) == norm(requested):
                return h
        raise ValueError(f"Không tìm thấy cột '{requested}'. Các cột: {', '.join(headers)}")

    if kind == "time":
        for h in headers:
            n = norm(h)
            if n in {"microseconds", "time", "timestamp", "times", "seconds",
                     "milliseconds", "nanoseconds"} or "timestamp" in n or n.startswith("time"):
                return h
    else:
        # Prefer the first column named exactly "logic" (PulseView can export
        # several channels with the same header), then fall back to D0-style names.
        for h in headers:
            if norm(h) == "logic":
                return h
        for h in headers:
            if norm(h) in {"d0", "channel0", "logic0", "ch0"}:
                return h
        for h in headers:
            n = norm(h)
            if n.startswith("d") and n[1:].isdigit():
                return h
    return None


def parse_time(value, header):
    s = (value or "").strip().replace("µ", "u").replace("μ", "u")
    m = re.fullmatch(r"([-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?)\s*([A-Za-z]*)", s)
    if not m:
        return None
    x = float(m.group(1))
    unit = m.group(2).lower()
    h = norm(header)
    if unit in {"s", "sec", "second", "seconds"}:
        scale = 1
    elif unit in {"ms", "msec", "millisecond", "milliseconds"}:
        scale = 1e-3
    elif unit in {"us", "usec", "microsecond", "microseconds"}:
        scale = 1e-6
    elif unit in {"ns", "nsec", "nanosecond", "nanoseconds"}:
        scale = 1e-9
    elif unit:
        raise ValueError(f"Đơn vị thời gian không hỗ trợ: {unit}")
    elif "microsecond" in h or h.endswith("us"):
        scale = 1e-6
    elif "millisecond" in h or h.endswith("ms"):
        scale = 1e-3
    elif "nanosecond" in h or h.endswith("ns"):
        scale = 1e-9
    else:
        scale = 1.0  # số không kèm đơn vị trong cột thời gian được hiểu là giây
    return x * scale


def parse_logic(value):
    s = (value or "").strip().lower()
    if s in {"0", "low", "false", "off", "l"}:
        return 0
    if s in {"1", "high", "true", "on", "h"}:
        return 1
    if re.match(r"^0(?:\D|$)", s):
        return 0
    if re.match(r"^1(?:\D|$)", s):
        return 1
    return None


def main():
    p = argparse.ArgumentParser(description="Tính chu kỳ toggle, jitter và drift từ CSV PulseView.")
    p.add_argument("csv_file", help="Đường dẫn file CSV PulseView")
    p.add_argument("--period-ms", type=float, required=True, help="Chu kỳ toggle mục tiêu (ms), ví dụ 17")
    p.add_argument("--sample-count", type=int, required=True,
                   help="Số khoảng toggle cần phân tích (theo SAMPLE_COUNT của đề)")
    p.add_argument("--time-column", default=None, help="Tên cột thời gian, ví dụ microseconds")
    p.add_argument("--channel", default=None, help="Tên cột GPIO, ví dụ D0")
    p.add_argument("--delimiter", choices=["auto", "comma", "semicolon", "tab"], default="auto")
    p.add_argument("--result-csv", default=None,
                   help="Đường dẫn result.csv (mặc định: result.csv cùng thư mục với script)")
    p.add_argument("--no-result", action="store_true", help="Chỉ in ra màn hình, không ghi result.csv")
    p.add_argument("--delay", choices=["vTaskDelay", "xTaskDelayUntil"], default=None,
                   help="Loại delay (mặc định: suy ra từ tên file capture)")
    p.add_argument("--tick", type=int, choices=[100, 1000], default=None,
                   help="Tick rate (mặc định: suy ra từ tên file capture, vd. *_1000hz.csv)")
    args = p.parse_args()

    path = Path(args.csv_file)
    if not path.is_file():
        print(f"ERROR: không tìm thấy file: {path}", file=sys.stderr)
        return 2
    if args.period_ms <= 0 or args.sample_count <= 0:
        print("ERROR: period-ms và sample-count phải lớn hơn 0.", file=sys.stderr)
        return 2

    try:
        dmap = {"comma": ",", "semicolon": ";", "tab": "\t"}
        delimiter = detect_delimiter(path) if args.delimiter == "auto" else dmap[args.delimiter]
        headers, rows = read_csv(path, delimiter)
        time_col = select_column(headers, args.time_column, "time")
        signal_col = select_column(headers, args.channel, "signal")
        if time_col is None or signal_col is None:
            print("Không tự nhận diện được cột thời gian/GPIO.")
            print("Các cột hiện có:", ", ".join(headers))
            print("Dùng --time-column TEN_COT nếu cần chỉ định cột thời gian; GPIO sẽ tự ưu tiên cột 'logic' đầu tiên hoặc D0.")
            return 2

        # Read timestamped logic values. Repeated values are samples, not edges.
        samples = []
        for row in rows:
            t = parse_time(row.get(time_col, ""), time_col)
            v = parse_logic(row.get(signal_col, ""))
            if t is not None and v is not None and math.isfinite(t):
                samples.append((t, v))
        if len(samples) < 2:
            raise ValueError("Không đủ mẫu hợp lệ trong hai cột đã chọn.")

        samples.sort(key=lambda x: x[0])
        # If duplicate timestamps exist, retain the last value at that timestamp.
        by_time = {}
        for t, v in samples:
            by_time[t] = v
        samples = sorted(by_time.items())

        edges = []
        prev_t, prev_v = samples[0]
        for t, v in samples[1:]:
            if v != prev_v:
                edges.append(t)
            prev_t, prev_v = t, v

        # SAMPLE_COUNT means number of detected transitions (edges), including rising
        # and falling edges. N edges provide N-1 toggle intervals.
        needed_edges = args.sample_count
        if len(edges) < needed_edges:
            raise ValueError(
                f"Chỉ tìm thấy {len(edges)} cạnh chuyển trạng thái, nhưng cần "
                f"{needed_edges} cạnh theo SAMPLE_COUNT. "
                "Hãy ghi capture dài hơn hoặc giảm --sample-count."
            )
        if needed_edges < 2:
            raise ValueError("SAMPLE_COUNT phải >= 2 để tính khoảng toggle.")

        used_edges = edges[:needed_edges]
        periods = [b - a for a, b in zip(used_edges, used_edges[1:])]
        expected = args.period_ms / 1000.0
        avg = statistics.fmean(periods)
        jitter = max(periods) - min(periods)
        # N edges contain N-1 intervals, so compare against (N-1) ideal periods.
        drift = (used_edges[-1] - used_edges[0]) - (needed_edges - 1) * expected

        print(f"Chu kỳ toggle min / max: {min(periods) * 1000:.6f} / {max(periods) * 1000:.6f} ms")
        print(f"Chu kỳ toggle trung bình: {avg * 1000:.6f} ms")
        print(f"Jitter (max khoảng toggle - min khoảng toggle): {jitter * 1000:.6f} ms")
        print(f"Drift tổng sau {needed_edges} cạnh: {drift * 1000:.6f} ms")

        if not args.no_result:
            inf_delay, inf_tick = infer_from_filename(path.name)
            delay_name = args.delay or inf_delay
            tick_rate = args.tick or inf_tick
            if not delay_name or not tick_rate:
                raise ValueError(
                    f"Không suy ra được Delay/Tick từ tên file '{path.name}'. "
                    "Đặt tên dạng vtaskdelay_1000hz.csv hoặc dùng --delay và --tick."
                )
            result_path = (Path(args.result_csv) if args.result_csv
                           else Path(__file__).resolve().parent / "result.csv")
            update_result_csv(result_path, delay_name, tick_rate,
                              avg * 1000, jitter * 1e6, drift * 1000)
            print(f"Đã cập nhật dòng {delay_name}, Tick={tick_rate} trong: {result_path}")

        print()
        print(f"(Đã xử lý {len(samples)} mẫu hợp lệ; phát hiện {len(edges)} cạnh trong capture; "
              f"dùng {len(used_edges)} cạnh đầu tiên, tương ứng {len(periods)} khoảng toggle.)")
        print("Lưu ý: drift được tính bằng thời gian thực từ cạnh đầu đến cạnh cuối trừ N × chu kỳ mục tiêu.")
        return 0
    except (OSError, csv.Error, ValueError) as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())