#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
"""Render and compare deterministic effect/snapshot images from two backend builds."""

import argparse
import pathlib
import subprocess


# Mean RGB error, 99th-percentile per-pixel max-channel error, and fraction of
# pixels exceeding 16 levels. Soft shadows allow more drift for their different
# PCSS tap counts; scaled MSAA allows rasterizer-specific sample placement.
TOLERANCES = {
    "plain.rgba": (0.25, 2, 0.001),
    "ambient-occlusion.rgba": (1.0, 16, 0.02),
    "soft-shadow.rgba": (1.5, 24, 0.04),
    "average-transparency.rgba": (1.5, 24, 0.04),
    "average-textured-transparency.rgba": (1.5, 24, 0.04),
    "scaled-msaa.rgba": (1.5, 20, 0.04),
}

# OpenGL shades the virtual shadow receiver directly and writes its opacity to
# alpha; Vulkan composites a shadow overlay onto an opaque scene. Compare visible
# RGB for this effect, while bounding and reporting the known alpha divergence.
ALPHA_MISMATCH_FRACTION_LIMITS = {
    "soft-shadow.rgba": 0.04,
}


def read_rgba(path):
    with path.open("rb") as image:
        header = image.readline().decode("ascii").split()
        if len(header) != 4 or header[:2] != ["EASY3D_RGBA", "1"]:
            raise ValueError(f"{path}: invalid image header")
        width, height = (int(header[2]), int(header[3]))
        if width <= 0 or height <= 0:
            raise ValueError(f"{path}: invalid image dimensions")
        pixels = image.read()
    if len(pixels) != width * height * 4:
        raise ValueError(f"{path}: RGBA payload has the wrong size")
    return width, height, pixels


def compare(name, left_path, right_path):
    left_width, left_height, left = read_rgba(left_path)
    right_width, right_height, right = read_rgba(right_path)
    if (left_width, left_height) != (right_width, right_height):
        raise ValueError(
            f"{name}: dimensions differ: {left_width}x{left_height} vs "
            f"{right_width}x{right_height}"
        )

    pixel_errors = []
    channel_error = 0
    alpha_mismatches = 0
    for offset in range(0, len(left), 4):
        deltas = [abs(left[offset + c] - right[offset + c]) for c in range(3)]
        pixel_errors.append(max(deltas))
        channel_error += sum(deltas)
        alpha_mismatches += left[offset + 3] != right[offset + 3]

    pixel_count = len(pixel_errors)
    mean_error = channel_error / (pixel_count * 3)
    p99 = sorted(pixel_errors)[(pixel_count * 99 + 99) // 100 - 1]
    fraction_over_16 = sum(error > 16 for error in pixel_errors) / pixel_count
    mean_limit, p99_limit, fraction_limit = TOLERANCES[name]
    alpha_mismatch_fraction = alpha_mismatches / pixel_count
    alpha_limit = ALPHA_MISMATCH_FRACTION_LIMITS.get(name, 0.0)
    print(
        f"{name}: {left_width}x{left_height}, mean RGB error {mean_error:.3f}/{mean_limit:.3f}, "
        f"p99 {p99}/{p99_limit}, >16 fraction {fraction_over_16:.4%}/{fraction_limit:.4%}, "
        f"alpha mismatches {alpha_mismatch_fraction:.4%}/{alpha_limit:.4%}"
    )
    return not (
        mean_error > mean_limit
        or p99 > p99_limit
        or fraction_over_16 > fraction_limit
        or alpha_mismatch_fraction > alpha_limit
    )


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--opengl", required=True, type=pathlib.Path, help="OpenGL test executable")
    parser.add_argument("--vulkan", required=True, type=pathlib.Path, help="Vulkan test executable")
    parser.add_argument("--output-dir", required=True, type=pathlib.Path, help="fresh output directory")
    parser.add_argument("--validation", action="store_true", help="enable Vulkan validation")
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)
    opengl_output = args.output_dir / "opengl"
    vulkan_output = args.output_dir / "vulkan"
    opengl_output.mkdir()
    vulkan_output.mkdir()
    subprocess.run(
        [str(args.opengl), "--output-dir", str(opengl_output)],
        check=True,
    )
    vulkan_command = [str(args.vulkan), "--output-dir", str(vulkan_output)]
    if args.validation:
        vulkan_command.insert(1, "--validation")
    subprocess.run(vulkan_command, check=True)

    failures = [
        image_name
        for image_name in TOLERANCES
        if not compare(
            image_name,
            opengl_output / image_name,
            vulkan_output / image_name,
        )
    ]
    if failures:
        raise AssertionError(
            "cross-backend image differences exceed tolerance: " + ", ".join(failures)
        )


if __name__ == "__main__":
    main()
