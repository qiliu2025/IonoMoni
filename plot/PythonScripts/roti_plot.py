import os
import numpy as np
import matplotlib.pyplot as plt


def plot_roti_or_rot(file_path, save_path, title_prefix="ROTI", max_sat=32, unit="min"):
    data = []
    with open(file_path, 'r', encoding='utf-8') as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("Epoch \\ PRN"):
                continue
            try:
                parts = line.split()
                if parts[0] == "Epoch" and ":" in parts[1]:
                    values = [float(x) for x in parts[2:]]
                else:
                    values = [float(x) for x in parts]
                data.append(values)
            except ValueError:
                print(f"[×] Skipped invalid line: {line}")
                continue

    data = np.array(data)
    if data.size == 0 or data.shape[1] == 0:
        print(f"[×] Empty data, skipped: {file_path}")
        return

    x = np.arange(data.shape[0]) / 120.0
    colors = plt.get_cmap("rainbow")

    plt.figure(figsize=(12, 6))
    for i in range(data.shape[1]):
        sat_data = data[:, i]
        if np.all(np.isnan(sat_data)):
            continue  # skip
        prn_label = f"{title_prefix}{i + 1:02d}"
        plt.scatter(x, sat_data, color=colors(i / data.shape[1]), s=10, label=prn_label, alpha=0.6)

    plt.xlabel("Time (hours)")
    plt.xlim(0, 24)
    plt.xticks(np.arange(0, 25, 4))

    filename_upper = os.path.basename(file_path).upper()
    if "ROTI" in filename_upper:
        plt.ylabel(f"ROTI (TECU/{unit})")
        plt.ylim(0, 5)
        plt.yticks(np.arange(0, 5.5, 0.5))
    elif "ROT" in filename_upper:
        plt.ylabel(f"ROT (TECU/{unit})")
        plt.ylim(-5, 5)
        plt.yticks(np.arange(-5, 5.5, 0.5))
    else:
        plt.ylabel(f"Value (TECU/{unit})")

    plt.title(os.path.basename(file_path).replace(".txt", ""))
    plt.legend(loc='upper left', bbox_to_anchor=(1.01, 1), ncol=2, fontsize=8)
    plt.grid(True)
    plt.tight_layout()

    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    plt.savefig(save_path, dpi=300)
    print(f"[✓] Image saved: {save_path}")
    plt.close()


def batch_plot_roti_rot(base_input_path, base_output_path, year, doy, rot_unit="min", site_list=None):
    """
    Batch process ROTI and ROT files for selected stations.
    rot_unit: "min" or "sec", sets Y-axis unit.
    site_list: list of station names to include (e.g., ["HKWS", "ABPO"]).
    """
    root_output = os.path.join(base_output_path, "roti_plot", f"{year}_{doy:03d}")
    all_files = os.listdir(base_input_path)

    for file in all_files:
        if not file.endswith(".txt") or ("ROTI" not in file and "ROT" not in file):
            continue

        parts = file.replace(".txt", "").split("_")
        if len(parts) < 3:
            print(f"[×] Invalid filename format, skipped: {file}")
            continue

        station, system, kind = parts[0], parts[1].upper(), parts[2].upper()
        if site_list and station not in site_list:
            continue

        full_input_path = os.path.join(base_input_path, file)
        subfolder = "roti" if kind == "ROTI" else "rot"
        save_folder = os.path.join(root_output, station, subfolder)
        save_path = os.path.join(save_folder, file.replace(".txt", ".png"))

        prefix_map = {"GPS": "G", "GLO": "R", "BDS": "C", "GAL": "E"}
        prefix = prefix_map.get(system, system[0])
        max_sat = {"GPS": 32, "GLO": 24, "BDS": 46, "GAL": 36}.get(system, 32)

        plot_roti_or_rot(full_input_path, save_path, title_prefix=prefix, max_sat=max_sat, unit=rot_unit)

# ======== Main entry point ========
if __name__ == "__main__":
    base_input_path = "../data_roti"
    base_output_path = "../output"
    year = 2023
    doy = 100
    rot_unit = "min"   # "sec" means ROT unit is TECU/sec; "min" means TECU/min
    site_list = ["CORD","POTS","HRAO"]  # Add your station names here

    batch_plot_roti_rot(base_input_path, base_output_path, year, doy, rot_unit, site_list)
