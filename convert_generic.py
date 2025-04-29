# Python Script: convert_generic.py
# Converts any PNG in input folder to a .h file based on filename.

import os
import glob
from PIL import Image
import re

# --- Configuration ---
# Folder containing generic PNGs (backgrounds, UI, etc.)
input_folder = "generic_input" # <<< USE A DEDICATED FOLDER FOR THIS SCRIPT

# Output folder for .h files (same assets folder)
output_folder = "../DigiviceRefactor/assets"

# Magenta Color Key for Transparency
key_color_rgb = (255, 0, 255)
alpha_threshold = 128
# ---------------------

def sanitize_for_c(name_part):
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name_part)
    if name and name[0].isdigit(): name = "_" + name
    return name

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir_path = os.path.join(script_dir, input_folder)
    output_dir_path = os.path.join(script_dir, output_folder)

    os.makedirs(input_dir_path, exist_ok=True) # Create input folder if missing
    os.makedirs(output_dir_path, exist_ok=True)
    print(f"Looking for generic PNGs in: {os.path.abspath(input_dir_path)}")
    print(f"Output folder: {os.path.abspath(output_dir_path)}")

    search_pattern = os.path.join(input_dir_path, "*.png")
    png_files = glob.glob(search_pattern)

    if not png_files: print(f"No PNG files found in '{input_dir_path}'")
    else: print(f"Found {len(png_files)} PNG files. Starting conversion...")

    key_color_565 = ((key_color_rgb[0] >> 3) << 11) | ((key_color_rgb[1] >> 2) << 5) | (key_color_rgb[2] >> 3)

    for image_path in png_files:
        base_filename = os.path.basename(image_path)
        sanitized_base = sanitize_for_c(os.path.splitext(base_filename)[0]) # Sanitize name part only

        # --- Define output names based purely on sanitized input filename ---
        output_h_filename = f"{sanitized_base}.h"
        variable_name = f"{sanitized_base}_data"
        define_prefix = sanitized_base.upper()
        output_path = os.path.join(output_dir_path, output_h_filename)
        # ------------------------------------------------------------------

        print(f"\nProcessing '{base_filename}' ==>")
        print(f"  -> Output File: '{output_h_filename}'")

        try:
            img = Image.open(image_path)
            if img.mode != 'RGBA': img = img.convert('RGBA')
            width, height = img.size
            pixels_rgba = list(img.getdata())
            print(f"  Image size: {width}x{height}")

            with open(output_path, "w") as f:
                f.write("#pragma once\n\n#include <cstdint>\n\n")
                f.write(f"#define {define_prefix}_WIDTH {width}\n")
                f.write(f"#define {define_prefix}_HEIGHT {height}\n\n")
                f.write(f"// RGB565 format, Converted from {base_filename}\n")
                f.write(f"const uint16_t {variable_name}[{width * height}] = {{\n  ")
                count = 0
                for i, p in enumerate(pixels_rgba):
                    r, g, b, a = p
                    rgb565 = key_color_565 if a < alpha_threshold else ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)
                    f.write(f"0x{rgb565:04X}, ")
                    count += 1
                    if count % 12 == 0: f.write("\n  ")
                    elif i < len(pixels_rgba) - 1: f.write(" ")
                f.write("\n}; // End of array\n")
            print(f"  Successfully generated '{output_h_filename}'")

        except Exception as e: print(f"  ERROR processing '{base_filename}': {e}")

    print("\nGeneric conversion finished.")
except Exception as e: print(f"A critical error occurred: {e}")