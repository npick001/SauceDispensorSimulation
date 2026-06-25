"""Animate SauceDispensorSimulation execution from runtime logs.

This script reads the simulation logs and animates:
- bowls moving through 6 conveyor slots
- left and right dispensing heads operating over upstream/downstream slots
- a shared 10-cartridge carousel
- inventory levels as each cartridge is used
- successful and failed dispense attempts per bowl

Usage:
    python animate_logs.py --log-dir SauceDispensorSimulation/logs
"""

import argparse
import os
import re
from collections import defaultdict, OrderedDict

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from matplotlib.animation import FuncAnimation, FFMpegWriter


LOG_LINE_RE = re.compile(r"^\[[^\]]+\] \[[A-Z]+\] - (.*)$")
ATTEMPT_RE = re.compile(r"Attempted to dispense ([^\s]+) on Bowl (\d+) that did not need it\.")
FULFILL_RE = re.compile(r"Bowl (\d+) ([^ ]+) fulfilled\.")
EXPECT_RE = re.compile(r"Bowl (\d+): expecting (\d+) ingredients:")
EXPECT_ING_RE = re.compile(r"\s*- Ingredient: (.+)")
DISPENSE_RE = re.compile(r"(Left|Right) Dispensor dispensing (.+)")
INVENTORY_RE = re.compile(r"Reduced level of (.+) from ([0-9\.]+) to ([0-9\.]+)")

CAROUSEL_INGREDIENTS = [
    "Ranch",
    "BBQ",
    "Mayo",
    "Hot Sauce",
    "Mustard",
    "Teriyaki",
    "Caeser",
    "Buffalo",
    "Chipotle",
    "Vinaigrette",
]

HEAD_SLOTS = {
    "Left": 1,
    "Right": 4,
}

NUM_SLOTS = 6
CONVEYOR_MOVE_FRAMES = 10


def parse_log_file(path):
    with open(path, "r", encoding="utf-8", errors="ignore") as f:
        for raw in f:
            raw = raw.strip()
            if not raw:
                continue
            match = LOG_LINE_RE.match(raw)
            yield match.group(1) if match else raw


def parse_results_log(path):
    expectations = OrderedDict()
    current_section = None
    current_bowl = None

    for line in parse_log_file(path):
        expect_match = EXPECT_RE.match(line)
        if expect_match:
            current_section = "expect"
            current_bowl = int(expect_match.group(1))
            expectations.setdefault(current_bowl, [])
            continue

        expect_ing_match = EXPECT_ING_RE.match(line)
        if expect_ing_match and current_section == "expect" and current_bowl is not None:
            expectations[current_bowl].append(expect_ing_match.group(1))
            continue

    return expectations


def parse_conveyor_log(path):
    events = []
    event_index = 0
    for line in parse_log_file(path):
        attempt_match = ATTEMPT_RE.match(line)
        if attempt_match:
            ingredient = attempt_match.group(1)
            bowl_id = int(attempt_match.group(2))
            events.append({
                "type": "attempt",
                "bowl": bowl_id,
                "ingredient": ingredient,
                "success": False,
                "index": event_index,
            })
            event_index += 1
            continue

        fulfill_match = FULFILL_RE.match(line)
        if fulfill_match:
            bowl_id = int(fulfill_match.group(1))
            ingredient = fulfill_match.group(2)
            events.append({
                "type": "fulfill",
                "bowl": bowl_id,
                "ingredient": ingredient,
                "success": True,
                "index": event_index,
            })
            event_index += 1
            continue

    return events


def parse_dispensing_head_log(path):
    events = []
    event_index = 0
    for line in parse_log_file(path):
        dispense_match = DISPENSE_RE.match(line)
        if dispense_match:
            head = dispense_match.group(1)
            ingredient = dispense_match.group(2)
            events.append({
                "head": head,
                "ingredient": ingredient,
                "index": event_index,
            })
            event_index += 1
    return events


def parse_carousel_log(path):
    inventory = OrderedDict((name, 10.0) for name in CAROUSEL_INGREDIENTS)
    events = []
    for line in parse_log_file(path):
        inv_match = INVENTORY_RE.match(line)
        if inv_match:
            ingredient = inv_match.group(1)
            level = float(inv_match.group(3))
            inventory[ingredient] = level
            events.append({
                "ingredient": ingredient,
                "level": level,
                "frame": len(events),
            })
    return inventory, events


def build_frames(head_events, conveyor_events):
    num_frames = min(len(head_events), len(conveyor_events))
    frames = []

    for i in range(num_frames):
        head_event = head_events[i]
        conveyor_event = conveyor_events[i]
        frames.append({
            "head": head_event["head"],
            "ingredient": head_event["ingredient"],
            "bowl": conveyor_event["bowl"],
            "success": conveyor_event["success"],
            "type": conveyor_event["type"],
        })

    return frames


def init_conveyor_state(bowl_order):
    slots = [None] * NUM_SLOTS
    queue = list(bowl_order)
    if queue:
        slots[0] = queue.pop(0)
    return slots, queue


def step_conveyor(slots, queue):
    for i in reversed(range(1, NUM_SLOTS)):
        slots[i] = slots[i - 1]
    slots[0] = queue.pop(0) if queue else None
    return slots, queue


def draw_carousel(ax):
    ax.clear()
    ax.set_aspect("equal")
    ax.axis("off")

    radius = 1.9
    for i, ingredient in enumerate(CAROUSEL_INGREDIENTS):
        theta = 2 * 3.14159 * i / len(CAROUSEL_INGREDIENTS)
        x = radius * np.cos(theta)
        y = radius * np.sin(theta)
        circle = patches.Circle((x, y), 0.22, edgecolor="black", facecolor="#f7f7f7")
        ax.add_patch(circle)
        ax.text(x, y, ingredient, ha="center", va="center", fontsize=7)

    ax.text(0, 0, "Carousel\n10 cartridges", ha="center", va="center", fontsize=9, fontweight="bold")
    ax.set_xlim(-2.5, 2.5)
    ax.set_ylim(-2.5, 2.5)


def draw_conveyor(ax, slots, bowl_requirements, fulfillment_counts, current_frame):
    ax.clear()
    ax.set_title("Conveyor and Bowls", fontsize=12)
    ax.set_xlim(-0.5, NUM_SLOTS + 0.5)
    ax.set_ylim(-1, 4)
    ax.axis("off")

    for i in range(NUM_SLOTS):
        rect = patches.Rectangle((i, 1), 0.9, 0.8, edgecolor="black", facecolor="#e4e4e4")
        ax.add_patch(rect)
        ax.text(i + 0.45, 1.4, f"Slot {i}", ha="center", va="center", fontsize=8)

    for head, slot in HEAD_SLOTS.items():
        ax.text(slot + 0.45, 2.3, f"{head}\nHead", ha="center", va="center", fontsize=9, fontweight="bold")
        arrow = patches.FancyArrow(slot + 0.45, 2.2, 0, -0.35, width=0.08, length_includes_head=True, head_width=0.18)
        ax.add_patch(arrow)

    for i, bowl in enumerate(slots):
        if bowl is None:
            continue
        y = 1.05
        color = "#add8e6"
        rect = patches.FancyBboxPatch((i + 0.05, y), 0.8, 0.6, boxstyle="round,pad=0.05", edgecolor="black", facecolor=color)
        ax.add_patch(rect)
        req_count = len(bowl_requirements.get(bowl, []))
        fulfilled = fulfillment_counts.get(bowl, 0)
        status = f"Bowl {bowl}\n{fulfilled}/{req_count} filled"
        ax.text(i + 0.45, y + 0.35, status, ha="center", va="center", fontsize=7)

    ax.text(0, 0, f"Frame: {current_frame}", fontsize=10, fontweight="bold")


def draw_inventory(ax, inventory):
    ax.clear()
    ax.set_title("Carousel Inventory", fontsize=12)
    ingredients = list(inventory.keys())
    levels = list(inventory.values())
    bars = ax.bar(range(len(ingredients)), levels, color="#82c6e2")
    ax.set_xticks(range(len(ingredients)))
    ax.set_xticklabels(ingredients, rotation=45, ha="right", fontsize=8)
    ax.set_ylim(0, 10)
    ax.set_ylabel("Remaining level")
    for bar, level in zip(bars, levels):
        ax.text(bar.get_x() + bar.get_width() / 2, level + 0.15, f"{level:.1f}", ha="center", va="bottom", fontsize=7)


def draw_head_status(ax, frame):
    ax.clear()
    ax.set_title("Current Head Activity", fontsize=12)
    ax.axis("off")
    if frame is None:
        ax.text(0.5, 0.5, "Waiting for events...", ha="center", va="center", fontsize=10)
        return

    ax.text(0.05, 0.75, f"Head: {frame['head']}", fontsize=10)
    ax.text(0.05, 0.55, f"Ingredient: {frame['ingredient']}", fontsize=10)
    ax.text(0.05, 0.35, f"Bowl: {frame['bowl']}", fontsize=10)
    ax.text(0.05, 0.15, "Result: " + ("fulfilled" if frame["success"] else "attempted"), fontsize=10, color=("green" if frame["success"] else "red"))


def draw_carousel_alignment(ax, head_state):
    ax.clear()
    ax.set_title("Carousel Alignment", fontsize=12)
    ax.set_xlim(-1.5, 1.5)
    ax.set_ylim(-1.5, 1.5)
    ax.axis("off")

    circle = patches.Circle((0, 0), 1.0, edgecolor="black", facecolor="#f2f2f2")
    ax.add_patch(circle)
    highlighted = {ingredient for ingredient in head_state.values() if ingredient}

    for i, ingredient in enumerate(CAROUSEL_INGREDIENTS):
        theta = 2 * 3.14159 * i / len(CAROUSEL_INGREDIENTS)
        x = 0.8 * np.cos(theta)
        y = 0.8 * np.sin(theta)
        color = "#fffacd"
        if ingredient in highlighted:
            color = "#ff9999"
        ax.add_patch(patches.Circle((x, y), 0.15, edgecolor="black", facecolor=color))
        label = ingredient if ingredient in highlighted else ""
        ax.text(x, y, label, ha="center", va="center", fontsize=6)

    ax.text(0, 0, "Carousel\n10 cartridges", ha="center", va="center", fontsize=9, fontweight="bold")
    left_label = head_state.get("Left", "none")
    right_label = head_state.get("Right", "none")
    ax.text(0, -1.2, f"Left: {left_label}\nRight: {right_label}", ha="center", va="center", fontsize=8)


def animate(frames, bowl_order, bowl_requirements, inventory_events, output_file=None):
    fig = plt.figure(figsize=(14, 12))
    grid = fig.add_gridspec(3, 2, height_ratios=[1, 0.8, 0.8])

    ax_carousel = fig.add_subplot(grid[0, :])
    ax_conveyor = fig.add_subplot(grid[1, :])
    ax_head = fig.add_subplot(grid[2, 0])
    ax_inventory = fig.add_subplot(grid[2, 1])

    slots, queue = init_conveyor_state(bowl_order)
    fulfillment_counts = defaultdict(int)
    current_level_map = {name: 10.0 for name in CAROUSEL_INGREDIENTS}
    head_state = {"Left": None, "Right": None}

    def update(frame_idx):
        nonlocal slots, queue
        if frame_idx > 0 and frame_idx % CONVEYOR_MOVE_FRAMES == 0:
            slots, queue = step_conveyor(slots, queue)

        frame = frames[frame_idx]
        if frame["success"]:
            fulfillment_counts[frame["bowl"]] += 1

        head_state[frame["head"]] = frame["ingredient"]

        if frame["type"] == "fulfill":
            current_level_map[frame["ingredient"]] = max(0.0, current_level_map[frame["ingredient"]] - 0.2)

        draw_conveyor(ax_conveyor, slots, bowl_requirements, fulfillment_counts, frame_idx)
        draw_head_status(ax_head, frame)
        draw_carousel_alignment(ax_carousel, head_state)
        draw_inventory(ax_inventory, current_level_map)

    ani = FuncAnimation(fig, update, frames=len(frames), interval=250, repeat=False)
    plt.tight_layout()

    if output_file:
        writer = FFMpegWriter(fps=4, metadata={"artist": "SauceDispensorSimulation"})
        ani.save(output_file, writer=writer)
        print(f"Animation saved to {output_file}")
    else:
        plt.show()


def main(log_dir, args):
    results_path = os.path.join(log_dir, "results_log.txt")
    conveyor_path = os.path.join(log_dir, "conveyor_log.txt")
    dispensing_path = os.path.join(log_dir, "dispensing_head_log.txt")

    bowl_requirements = parse_results_log(results_path)
    conveyor_events = parse_conveyor_log(conveyor_path)
    dispensing_events = parse_dispensing_head_log(dispensing_path)
    frames = build_frames(dispensing_events, conveyor_events)

    bowl_order = list(bowl_requirements.keys())
    print(f"Loaded {len(bowl_order)} bowls and {len(frames)} animation frames.")

    animate(frames, bowl_order, bowl_requirements, None, output_file=args.output)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Animate Sauce Dispensor Simulation logs.")
    parser.add_argument("--log-dir", default="SauceDispensorSimulation/logs", help="Path to the log directory")
    parser.add_argument("--output", default=None, help="Path to save the animation as an MP4 file")
    args = parser.parse_args()
    main(args.log_dir, args)
