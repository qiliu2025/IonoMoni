import numpy as np
import matplotlib.pyplot as plt
import os

def plot_pseudorange_data(filename, prefix="G", save_path=None):
    """
    Read pseudorange data and plot (optionally save as PNG).
    Automatically skips the header (first line) and processes the data into 2D format.
    """
    data = []

    with open(filename, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    for line in lines[1:]:
        line = line.strip()
        if not line:
            continue

        parts = line.split()
        if parts[0] == "Epoch":
            values = parts[2:]
        else:
            values = parts

        row_data = []
        for x in values:
            try:
                val = float(x)
                row_data.append(np.nan if val == 0.0 else val)
            except ValueError:
                row_data.append(np.nan)

        data.append(row_data)

    try:
        data = np.array(data, dtype=float)
    except:
        return

    if data.shape[0] == 0 or data.shape[1] == 0:
        return

    x = np.arange(data.shape[0])
    x_in_hours = x / 120.0

    num_satellites = data.shape[1]
    colors = plt.get_cmap('rainbow')(np.linspace(0, 1, num_satellites))

    plt.figure(figsize=(12, 6))

    label_prefix = "E" if prefix == "EX" else prefix
    has_plot = False
    for i in range(num_satellites):
        sat_data = data[:, i]
        if not np.all(np.isnan(sat_data)):
            prn = f"{label_prefix}{i + 1:02d}"
            plt.plot(x_in_hours, sat_data, color=colors[i], label=prn)
            has_plot = True

    plt.xlabel("Time (hours)")
    plt.ylabel("STEC (TECU)")
    plt.xlim(0, 24)
    plt.xticks(np.arange(0, 25, 2))
    title_name = os.path.splitext(os.path.basename(filename))[0]
    plt.title(title_name)
    plt.grid(True)
    if has_plot:
        plt.legend(loc='upper left', bbox_to_anchor=(1.01, 1), ncol=2, fontsize=8)
    plt.tight_layout()

    if save_path:
        os.makedirs(os.path.dirname(save_path), exist_ok=True)
        plt.savefig(save_path, dpi=600)
        print(f"[✓] Image saved: {save_path}")
    else:
        plt.show()

    plt.close()


def batch_plot(site_list, input_path, output_path, year, doy):
    """
    Batch plotting main function.
    input_path: Directory of input TXT files
    output_path: Directory to save output PNG images
    """
    systems = {
        "GPS": "G",
        "GLO": "R",
        "GAL": "E",
        "BDS": "C"
    }

    for site in site_list:
        for suffix, prefix in systems.items():
            txt_path = os.path.join(input_path, f"{site}_{suffix}.txt")
            subfolder = os.path.join(output_path, "stec_plot", f"{year}_{doy:03d}", site)
            png_path = os.path.join(subfolder, f"{site}_{suffix}.png")

            if os.path.exists(txt_path):
                plot_pseudorange_data(txt_path, prefix=prefix, save_path=png_path)

# ======== Main entry point ========
if __name__ == "__main__":
    site_list = ["CORD","POTS","HRAO"]
    input_path = "../data_stec"           # Relative input path
    output_path = "../output"            # Relative output path
    year = 2023
    doy = 100

    batch_plot(site_list, input_path, output_path, year, doy)
