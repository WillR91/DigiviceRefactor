# Python Script: batch_convert_sprites_mapped.py (Reads Char_Num.png, writes Char_Action_Num.*)
import os
import glob
from PIL import Image
import re

# --- Configuration ---

# Folder containing your separated PNGs named like "CharacterName_FrameNumber.png"
# This folder should be in the same directory as this script.
input_folder = "separated_sprites_input" # <<< ADJUST if your input folder name is different

# Folder where the output .h files will be saved
# Should point correctly to your project's assets folder
output_folder = "../DigiviceRefactor/assets"

# --- Animation Mapping ---
# Maps the input frame index (0-9) to a tuple: (ActionName, ActionFrameIndex)
# !!! CONFIRM/ADJUST THIS MAPPING !!!
animation_mapping = {
    0: ("Idle", 0),
    1: ("Idle", 1),
    2: ("Walk", 0),
    3: ("Walk", 1),
    4: ("Run", 0),
    5: ("Run", 1),
    6: ("Happy", 0),
    7: ("Rest", 0),
    8: ("Attack", 0),
    9: ("Turn", 0) # Renamed from "BackViewTurn" for shorter names
}
# ---

# Magenta Color Key for Transparency
key_color_rgb = (255, 0, 255)
alpha_threshold = 128
# ---------------------

# --- Helper to sanitize for C names (remains the same) ---
def sanitize_for_c(name_part):
    name = re.sub(r'[^a-zA-Z0-9_]', '_', name_part)
    if name and name[0].isdigit():
        name = "_" + name
    return name
# ----------------------------------------------------------

try:
    script_dir = os.path.dirname(os.path.abspath(__file__))
    input_dir_path = os.path.join(script_dir, input_folder)
    output_dir_path = os.path.join(script_dir, output_folder)

    os.makedirs(output_dir_path, exist_ok=True)
    print(f"Input folder: {os.path.abspath(input_dir_path)}")
    print(f"Output folder: {os.path.abspath(output_dir_path)}")

    search_pattern = os.path.join(input_dir_path, "*.png")
    png_files = glob.glob(search_pattern)

    if not png_files:
        print(f"Error: No PNG files found in '{input_dir_path}'")
    else:
        print(f"Found {len(png_files)} PNG files. Starting conversion...")

    key_color_565 = ((key_color_rgb[0] >> 3) << 11) | ((key_color_rgb[1] >> 2) << 5) | (key_color_rgb[2] >> 3)

    processed_count = 0
    error_count = 0

    for image_path in png_files:
        base_filename = os.path.basename(image_path)

        # Parse filename like "CharacterName_FrameNumber.png"
        match = re.match(r'^(.*)_(\d+)\.png$', base_filename)
        if not match:
            print(f"  Skipping '{base_filename}': Does not match 'Name_Number.png' format.")
            continue

        character_name_raw = match.group(1)
        try:
            frame_number = int(match.group(2))
        except ValueError:
            print(f"  Skipping '{base_filename}': Frame number part '{match.group(2)}' not integer.")
            continue

        # Look up action and new index
        if frame_number in animation_mapping:
            action_name_raw, action_frame_index = animation_mapping[frame_number]

            # --- Sanitize names for C/Filesystem ---
            character_name_c = sanitize_for_c(character_name_raw)
            action_name_c = sanitize_for_c(action_name_raw)
            # ---

            # --- Define output names based on PARSED info ---
            output_base_name = f"{character_name_c}_{action_name_c}_{action_frame_index}"
            output_h_filename = f"{output_base_name}.h"
            variable_name = f"{output_base_name}_data"
            define_prefix = output_base_name.upper() # e.g., AGUMON_IDLE_0
            output_path = os.path.join(output_dir_path, output_h_filename)
            # ---------------------------------------------

            print(f"\nProcessing '{base_filename}' ==>")
            print(f"  -> Action: {action_name_raw}, Frame: {action_frame_index}")
            print(f"  -> Output File: '{output_h_filename}'")
            print(f"  -> Variable: '{variable_name}', Defines: '{define_prefix}_WIDTH/HEIGHT'")

            try:
                # --- Image processing ---
                img = Image.open(image_path)
                if img.mode != 'RGBA':
                    img = img.convert('RGBA')
                width, height = img.size
                pixels_rgba = list(img.getdata())
                print(f"  Image size: {width}x{height}")

                # --- File writing ---
                with open(output_path, "w") as f:
                    f.write("#pragma once\n\n")
                    f.write("#include <cstdint>\n\n")
                    # Use new define prefix convention
                    f.write(f"#define {define_prefix}_WIDTH {width}\n")
                    f.write(f"#define {define_prefix}_HEIGHT {height}\n\n")
                    f.write(f"// RGB565 format, Converted from {base_filename}\n")
                    f.write(f"// Action: {action_name_raw}, Frame Index: {action_frame_index}\n")
                    f.write(f"// Magenta (0x{key_color_565:04X}) is used as transparent color key\n")
                    # Use new variable name convention
                    f.write(f"const uint16_t {variable_name}[{width * height}] = {{\n  ")

                    count = 0
                    for i, p in enumerate(pixels_rgba):
                        r, g, b, a = p
                        if a < alpha_threshold:
                            rgb565 = key_color_565
                        else:
                            rgb565 = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

                        f.write(f"0x{rgb565:04X}")
                        if i < len(pixels_rgba) - 1: f.write(",")
                        count += 1
                        if count % 12 == 0: f.write("\n  ")
                        elif i < len(pixels_rgba) - 1: f.write(" ")

                    f.write("\n}; // End of array\n")
                print(f"  Successfully generated '{output_h_filename}'")
                processed_count += 1

            except Exception as e:
                print(f"  ERROR processing '{base_filename}': {e}")
                error_count += 1
        else:
            print(f"  Skipping '{base_filename}': Frame number {frame_number} not found in animation_mapping.")
            error_count +=1


    print(f"\nBatch conversion finished. Processed {processed_count} files. Encountered {error_count} errors or skipped files.")

except Exception as e:
    print(f"A critical error occurred: {e}")