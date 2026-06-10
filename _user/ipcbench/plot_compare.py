#!/usr/bin/env python3
"""
Compare two ipcbench CSV outputs — publication-quality (USENIX-grade) figures.

Usage:
    python3 plot_compare.py baseline.csv improved.csv [-o outdir] [--fmt pdf|png|svg]

Row types handled automatically:
  "raw <op> d=<D> p=<P> rr=<R>"  →  line chart per op-type (rd/wr/bidi)
                                      x-axis = payload size (p=),
                                      one line per d= value,
                                      solid = baseline / dashed = improved
  "posix <op> sz=<S>"            →  line chart, x-axis = transfer size (sz=),
                                      one line per op (read/write),
                                      solid = baseline / dashed = improved
  other throughput (avg_us set)  →  side-by-side bar chart
  latency rows (med_us set)      →  side-by-side bar chart per metric
"""

import argparse
import csv
import os
import re
import sys

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

# ---------------------------------------------------------------------------
# Publication-quality style settings
# ---------------------------------------------------------------------------
# Approximate USENIX double-column figure width; height scales per chart.
_PAPER_W = 7.0   # inches
_ROW_H   = 3.3   # inches

# Colorblind-friendly pair (blue / orange) that also distinguishes in B&W
# when combined with hatch patterns.
_C_BASE    = "#4878CF"   # muted blue
_C_IMP     = "#D65F0E"   # muted orange
_H_BASE    = ""          # no hatch on baseline (solid fill)
_H_IMP     = "////"      # diagonal hatch on improved
_EDGE_COL  = "#222222"
_EDGE_LW   = 0.5

# Line chart color cycle — qualitative, colorblind-tolerant, B&W-distinct
# via marker shape + fill (filled = baseline, open = improved).
_LC = [
    "#1f5fa6",  # dark blue
    "#3a9ad9",  # sky blue
    "#2e7d4f",  # dark green
    "#5cb85c",  # medium green
    "#a07c00",  # dark gold
    "#e8a020",  # amber
    "#8b3a3a",  # dark red
    "#d06060",  # salmon
]
_MARKERS = ["o", "s", "^", "v", "D", "p", "*", "h"]

plt.rcParams.update({
    # --- typography ---
    "font.family"         : "serif",
    "font.serif"          : ["Times New Roman", "Times", "DejaVu Serif"],
    "font.size"           : 9,
    "axes.labelsize"      : 9,
    "axes.titlesize"      : 10,
    "xtick.labelsize"     : 8,
    "ytick.labelsize"     : 8,
    "legend.fontsize"     : 7,
    "legend.framealpha"   : 0.92,
    "legend.edgecolor"    : "0.70",
    "legend.handlelength" : 2.2,
    # --- axes / ticks ---
    "axes.linewidth"      : 0.7,
    "xtick.major.width"   : 0.7,
    "ytick.major.width"   : 0.7,
    "xtick.major.size"    : 3.5,
    "ytick.major.size"    : 3.5,
    "xtick.direction"     : "in",
    "ytick.direction"     : "in",
    "axes.spines.top"     : False,
    "axes.spines.right"   : False,
    # --- grid (horizontal only by default; line charts override to both) ---
    "axes.grid"           : True,
    "axes.grid.axis"      : "y",
    "grid.linewidth"      : 0.4,
    "grid.linestyle"      : "--",
    "grid.alpha"          : 0.55,
    "grid.color"          : "#999999",
    # --- output ---
    "figure.dpi"          : 150,
    "savefig.dpi"         : 300,
    "savefig.bbox"        : "tight",
    "savefig.pad_inches"  : 0.04,
    # embed fonts so the file is camera-ready
    "pdf.fonttype"        : 42,
    "ps.fonttype"         : 42,
})


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def load_csv(path):
    rows = []
    with open(path, newline="") as f:
        reader = csv.DictReader(f)
        for r in reader:
            rows.append(r)
    return rows


def safe_int(val):
    if val is None or val.strip() == "":
        return None
    return int(val)


# Regex parsers for parametric row names
_RAW_RE     = re.compile(r"^raw\s+(?P<op>\w+)\s+d=(?P<d>\d+)\s+p=(?P<p>\d+)\s+rr=(?P<rr>\d+)$")
_POSIX_SZ_RE = re.compile(r"^posix\s+(?P<op>\w+)\s+sz=(?P<sz>\d+)$")


def _parse_raw(name):
    """Return (op, d, p, rr) ints/strings or None."""
    m = _RAW_RE.match(name.strip())
    if m:
        return m.group("op"), int(m.group("d")), int(m.group("p")), int(m.group("rr"))
    return None


def _parse_posix_sz(name):
    """Return (op, sz) or None."""
    m = _POSIX_SZ_RE.match(name.strip())
    if m:
        return m.group("op"), int(m.group("sz"))
    return None


def _fmt_bytes(n):
    if n == 0:
        return "0 B"
    if n >= 1024 * 1024 and n % (1024 * 1024) == 0:
        return f"{n // (1024 * 1024)} MB"
    if n >= 1024 and n % 1024 == 0:
        return f"{n // 1024} KB"
    return f"{n} B"


def classify_rows(rows):
    """Return (raw_param, posix_sz, throughput_plain, latency)."""
    raw_param, posix_sz, tp_plain, latency = [], [], [], []
    for r in rows:
        med = safe_int(r.get("med_us"))
        avg = safe_int(r.get("avg_us"))
        if med is not None:
            latency.append(r)
        elif avg is not None:
            if _parse_raw(r["name"]):
                raw_param.append(r)
            elif _parse_posix_sz(r["name"]):
                posix_sz.append(r)
            else:
                tp_plain.append(r)
    return raw_param, posix_sz, tp_plain, latency


def _ordered_names(*row_lists):
    seen = {}
    for rows in row_lists:
        for r in rows:
            seen.setdefault(r["name"], None)
    return list(seen)


def _annotate_bars(ax, bars, max_val, fontsize=6):
    threshold = max_val * 0.04 if max_val > 0 else 1
    for bar in bars:
        h = bar.get_height()
        if h >= threshold:
            ax.annotate(
                f"{h:,}",
                xy=(bar.get_x() + bar.get_width() / 2, h),
                xytext=(0, 2), textcoords="offset points",
                ha="center", va="bottom",
                fontsize=fontsize, color="#333333",
            )


def _make_bar_fig(n_bars):
    fw = max(_PAPER_W, n_bars * 0.62)
    return plt.subplots(figsize=(fw, _ROW_H))


def _make_line_fig(n_x):
    fw = max(_PAPER_W, n_x * 1.1)
    return plt.subplots(figsize=(fw, _ROW_H))


def _save(fig, path):
    fig.savefig(path)
    plt.close(fig)
    print(f"Saved {path}")


def _apply_line_grid(ax):
    """Enable subtle grid on both axes for line charts."""
    ax.grid(True, axis="both", linewidth=0.4, linestyle="--",
            alpha=0.55, color="#999999")
    ax.set_axisbelow(True)


def _plot_speedup_lines(ax, x_idx, speedup_series, x_labels, base_label, imp_label):
    """
    Draw speedup lines (baseline_time / improved_time) on *ax*.

    speedup_series: list of (series_label, color, marker, [speedup_or_None, ...])
    A horizontal reference line at y=1 is always drawn.
    """
    ax.axhline(1.0, color="#555555", linewidth=0.8, linestyle=":", zorder=2)
    for label, color, marker, y in speedup_series:
        ax.plot(x_idx, y, linestyle="-", marker=marker,
                color=color, linewidth=1.1, markersize=5,
                markerfacecolor=color, markeredgecolor=color,
                label=label, zorder=3)
    ax.set_ylabel(f"Speedup\n({base_label} / {imp_label})")
    ax.set_xticks(x_idx)
    ax.set_xticklabels(x_labels)
    _apply_line_grid(ax)
    # shade > 1 (improved is faster) with a faint green tint
    ax.axhspan(1.0, ax.get_ylim()[1] if ax.get_ylim()[1] > 1 else 2,
               alpha=0.04, color="green", zorder=0)


# ---------------------------------------------------------------------------
# Line charts — raw parametric  (x = p=, lines = d= values)
# ---------------------------------------------------------------------------

def plot_raw_lines(base_rows, imp_rows, base_label, imp_label, outdir, fmt):
    """One line chart per raw op-type (rd / wr / bidi)."""

    def _group(rows):
        # g[(op, rr)][d][p] = avg_us
        g = {}
        for r in rows:
            parsed = _parse_raw(r["name"])
            if parsed is None:
                continue
            op, d, p, rr = parsed
            g.setdefault((op, rr), {}).setdefault(d, {})[p] = safe_int(r["avg_us"]) or 0
        return g

    base_g = _group(base_rows)
    imp_g  = _group(imp_rows)
    all_keys = sorted(set(base_g) | set(imp_g))

    for (op, rr) in all_keys:
        bd  = base_g.get((op, rr), {})
        id_ = imp_g.get((op, rr), {})
        all_d = sorted(set(bd) | set(id_))
        all_p = sorted({p for dmap in (bd, id_) for dvals in dmap.values() for p in dvals})
        if not all_p:
            continue

        x_labels = [_fmt_bytes(p) for p in all_p]
        x_idx    = np.arange(len(all_p))
        fig, ax  = _make_line_fig(len(all_p))
        _apply_line_grid(ax)

        for di, d in enumerate(all_d):
            color  = _LC[di % len(_LC)]
            marker = _MARKERS[di % len(_MARKERS)]
            if d in bd:
                y = [bd[d].get(p, None) for p in all_p]
                ax.plot(x_idx, y, linestyle="-", marker=marker,
                        color=color, linewidth=1.1, markersize=5,
                        markerfacecolor=color, markeredgecolor=color,
                        label=f"{base_label} d={d}")
            if d in id_:
                y = [id_[d].get(p, None) for p in all_p]
                ax.plot(x_idx, y, linestyle="--", marker=marker,
                        color=color, linewidth=1.1, markersize=5,
                        markerfacecolor="white", markeredgecolor=color,
                        label=f"{imp_label} d={d}")

        rr_tag = f" rr={rr}" if rr != 0 else ""
        ax.set_title(f"Raw IPC \u2014 {op}{rr_tag}")
        ax.set_xlabel("Payload size (p=)")
        ax.set_ylabel(r"Average latency ($\mu$s / iter)")
        ax.set_xticks(x_idx)
        ax.set_xticklabels(x_labels)
        ax.legend(loc="upper left", ncol=2)
        fig.tight_layout()
        rr_sfx = f"_rr{rr}" if rr != 0 else ""
        _save(fig, os.path.join(outdir, f"raw_{op}{rr_sfx}_lines.{fmt}"))

        # --- speedup chart ---
        speedup_series = []
        for di, d in enumerate(all_d):
            if d not in bd or d not in id_:
                continue
            color  = _LC[di % len(_LC)]
            marker = _MARKERS[di % len(_MARKERS)]
            y = []
            for p in all_p:
                bv = bd[d].get(p)
                iv = id_[d].get(p)
                y.append(bv / iv if bv and iv else None)
            speedup_series.append((f"d={d}", color, marker, y))
        if speedup_series:
            fig2, ax2 = _make_line_fig(len(all_p))
            _plot_speedup_lines(ax2, x_idx, speedup_series, x_labels,
                                base_label, imp_label)
            ax2.set_title(f"Raw IPC Speedup \u2014 {op}{rr_tag}")
            ax2.set_xlabel("Payload size (p=)")
            ax2.legend(loc="upper left", ncol=2)
            fig2.tight_layout()
            _save(fig2, os.path.join(outdir, f"raw_{op}{rr_sfx}_speedup.{fmt}"))


# ---------------------------------------------------------------------------
# Line charts — posix sz= parametric  (x = sz=, lines = op type)
# ---------------------------------------------------------------------------

def plot_posix_sz_lines(base_rows, imp_rows, base_label, imp_label, outdir, fmt):
    """One combined line chart for all posix read/write sz= rows."""

    def _group(rows):
        # g[op][sz] = avg_us
        g = {}
        for r in rows:
            parsed = _parse_posix_sz(r["name"])
            if parsed is None:
                continue
            op, sz = parsed
            g.setdefault(op, {})[sz] = safe_int(r["avg_us"]) or 0
        return g

    base_g  = _group(base_rows)
    imp_g   = _group(imp_rows)
    all_ops = sorted(set(base_g) | set(imp_g))
    all_sz  = sorted({sz for g in (base_g, imp_g) for dmap in g.values() for sz in dmap})

    if not all_sz or not all_ops:
        return

    x_labels = [_fmt_bytes(s) for s in all_sz]
    x_idx    = np.arange(len(all_sz))
    fig, ax  = _make_line_fig(len(all_sz))
    _apply_line_grid(ax)

    for oi, op in enumerate(all_ops):
        color  = _LC[oi % len(_LC)]
        marker = _MARKERS[oi % len(_MARKERS)]
        if op in base_g:
            y = [base_g[op].get(sz, None) for sz in all_sz]
            ax.plot(x_idx, y, linestyle="-", marker=marker,
                    color=color, linewidth=1.1, markersize=5,
                    markerfacecolor=color, markeredgecolor=color,
                    label=f"{base_label} {op}")
        if op in imp_g:
            y = [imp_g[op].get(sz, None) for sz in all_sz]
            ax.plot(x_idx, y, linestyle="--", marker=marker,
                    color=color, linewidth=1.1, markersize=5,
                    markerfacecolor="white", markeredgecolor=color,
                    label=f"{imp_label} {op}")

    ax.set_title("POSIX I/O Latency vs. Transfer Size")
    ax.set_xlabel("Transfer size (sz=)")
    ax.set_ylabel(r"Average latency ($\mu$s / iter)")
    ax.set_xticks(x_idx)
    ax.set_xticklabels(x_labels)
    ax.legend(loc="upper left", ncol=2)
    fig.tight_layout()
    _save(fig, os.path.join(outdir, f"posix_sz_lines.{fmt}"))

    # --- speedup chart ---
    speedup_series = []
    for oi, op in enumerate(all_ops):
        if op not in base_g or op not in imp_g:
            continue
        color  = _LC[oi % len(_LC)]
        marker = _MARKERS[oi % len(_MARKERS)]
        y = []
        for sz in all_sz:
            bv = base_g[op].get(sz)
            iv = imp_g[op].get(sz)
            y.append(bv / iv if bv and iv else None)
        speedup_series.append((op, color, marker, y))
    if speedup_series:
        fig2, ax2 = _make_line_fig(len(all_sz))
        _plot_speedup_lines(ax2, x_idx, speedup_series, x_labels,
                            base_label, imp_label)
        ax2.set_title("POSIX I/O Speedup vs. Transfer Size")
        ax2.set_xlabel("Transfer size (sz=)")
        ax2.legend(loc="upper left", ncol=2)
        fig2.tight_layout()
        _save(fig2, os.path.join(outdir, f"posix_sz_speedup.{fmt}"))


# ---------------------------------------------------------------------------
# Bar charts — plain (non-parametric) throughput
# ---------------------------------------------------------------------------

def plot_throughput(base_rows, imp_rows, base_label, imp_label, outdir, fmt):
    base_map = {r["name"]: safe_int(r["avg_us"]) for r in base_rows}
    imp_map  = {r["name"]: safe_int(r["avg_us"]) for r in imp_rows}
    names    = _ordered_names(base_rows, imp_rows)
    if not names:
        return

    base_vals = [base_map.get(n) or 0 for n in names]
    imp_vals  = [imp_map.get(n)  or 0 for n in names]
    max_val   = max(max(base_vals), max(imp_vals), 1)

    x, width = np.arange(len(names)), 0.35
    fig, ax  = _make_bar_fig(len(names))

    bars1 = ax.bar(x - width / 2, base_vals, width,
                   label=base_label, color=_C_BASE, hatch=_H_BASE,
                   edgecolor=_EDGE_COL, linewidth=_EDGE_LW, zorder=3)
    bars2 = ax.bar(x + width / 2, imp_vals, width,
                   label=imp_label, color=_C_IMP, hatch=_H_IMP,
                   edgecolor=_EDGE_COL, linewidth=_EDGE_LW, zorder=3)

    ax.set_ylabel(r"Average latency ($\mu$s / iter)")
    ax.set_title("IPC Throughput Benchmark Comparison")
    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=38, ha="right")
    ax.set_ylim(bottom=0, top=max_val * 1.18)
    ax.legend(loc="upper right")
    ax.set_axisbelow(True)

    _annotate_bars(ax, bars1, max_val)
    _annotate_bars(ax, bars2, max_val)

    fig.tight_layout()
    _save(fig, os.path.join(outdir, f"throughput_compare.{fmt}"))


# ---------------------------------------------------------------------------
# Bar charts — latency (rt tests)
# ---------------------------------------------------------------------------

def plot_latency(base_rows, imp_rows, base_label, imp_label, outdir, fmt):
    base_map = {r["name"]: r for r in base_rows}
    imp_map  = {r["name"]: r for r in imp_rows}
    names    = _ordered_names(base_rows, imp_rows)
    if not names:
        return

    metrics = [
        ("min_us", r"Min latency ($\mu$s)",    "Min"),
        ("med_us", r"Median latency ($\mu$s)",  "Median"),
        ("avg_us", r"Average latency ($\mu$s)", "Average"),
        ("p99_us", r"P99 latency ($\mu$s)",     "P99"),
        ("max_us", r"Max latency ($\mu$s)",     "Max"),
    ]

    for metric_key, ylabel, short_label in metrics:
        base_vals = [safe_int(base_map[n].get(metric_key)) or 0
                     if n in base_map else 0 for n in names]
        imp_vals  = [safe_int(imp_map[n].get(metric_key)) or 0
                     if n in imp_map  else 0 for n in names]
        max_val   = max(max(base_vals), max(imp_vals), 1)

        x, width = np.arange(len(names)), 0.35
        fig, ax  = _make_bar_fig(len(names))

        bars1 = ax.bar(x - width / 2, base_vals, width,
                       label=base_label, color=_C_BASE, hatch=_H_BASE,
                       edgecolor=_EDGE_COL, linewidth=_EDGE_LW, zorder=3)
        bars2 = ax.bar(x + width / 2, imp_vals, width,
                       label=imp_label, color=_C_IMP, hatch=_H_IMP,
                       edgecolor=_EDGE_COL, linewidth=_EDGE_LW, zorder=3)

        ax.set_ylabel(ylabel)
        ax.set_title(f"IPC Latency Comparison \u2014 {short_label}")
        ax.set_xticks(x)
        ax.set_xticklabels(names, rotation=38, ha="right")
        ax.set_ylim(bottom=0, top=max_val * 1.18)
        ax.legend(loc="upper right")
        ax.set_axisbelow(True)

        _annotate_bars(ax, bars1, max_val)
        _annotate_bars(ax, bars2, max_val)

        fig.tight_layout()
        _save(fig, os.path.join(outdir, f"latency_{metric_key}_compare.{fmt}"))


# ---------------------------------------------------------------------------
# CLI
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Compare two ipcbench CSV results — publication-quality figures.")
    parser.add_argument("baseline", help="Path to baseline CSV")
    parser.add_argument("improved", help="Path to improved CSV")
    parser.add_argument("--base-label", default="baseline",
                        help="Legend label for the baseline dataset")
    parser.add_argument("--imp-label", default="improved",
                        help="Legend label for the improved dataset")
    parser.add_argument("-o", "--outdir", default=".",
                        help="Output directory for figures (default: cwd)")
    parser.add_argument("--fmt", default="pdf", choices=["pdf", "png", "svg"],
                        help="Output format — pdf (default, camera-ready), png, or svg")
    args = parser.parse_args()

    if not os.path.isdir(args.outdir):
        os.makedirs(args.outdir)

    base_rows = load_csv(args.baseline)
    imp_rows  = load_csv(args.improved)

    base_raw, base_psz, base_tp, base_lat = classify_rows(base_rows)
    imp_raw,  imp_psz,  imp_tp,  imp_lat  = classify_rows(imp_rows)

    plot_raw_lines(base_raw, imp_raw, args.base_label, args.imp_label, args.outdir, args.fmt)
    plot_posix_sz_lines(base_psz, imp_psz, args.base_label, args.imp_label, args.outdir, args.fmt)
    plot_throughput(base_tp, imp_tp, args.base_label, args.imp_label, args.outdir, args.fmt)
    plot_latency(base_lat, imp_lat, args.base_label, args.imp_label, args.outdir, args.fmt)


if __name__ == "__main__":
    main()
