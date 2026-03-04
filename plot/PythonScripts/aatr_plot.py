import os
import numpy as np
import matplotlib.pyplot as plt


def plot_iaatr_curve(iaatr_filename, save_path, title="IAATR Curve", rot_unit="min"):
    iaatr_values = []
    with open(iaatr_filename, 'r', encoding='utf-8') as f:
        for line in f:
            parts = line.strip().split(':')
            if len(parts) == 2:
                try:
                    iaatr_values.append(float(parts[1].strip()))
                except ValueError:
                    continue

    iaatr_values = np.array(iaatr_values)
    if iaatr_values.size == 0:
        print(f"[×] Empty data, skipping plot: {iaatr_filename}")
        return

    x_iaatr = np.linspace(0, 24, len(iaatr_values))

    plt.figure(figsize=(10, 5))
    plt.plot(x_iaatr, iaatr_values, color='black', linestyle='-', marker='o',
             linewidth=1.5, markersize=5, label="AATR")

    plt.xlabel("Time (hours)")
    unit_label = "TECU/sec" if rot_unit.lower() == "sec" else "TECU/min"
    plt.ylabel(f"AATR ({unit_label})")
    plt.title(title)
    plt.grid(True)
    plt.xticks(np.arange(0, 25, 4))  # 每4小时一个刻度
    plt.xlim(0, 24)                  # 限制横坐标范围为 0–24 小时
    plt.legend()
    plt.tight_layout()

    os.makedirs(os.path.dirname(save_path), exist_ok=True)
    plt.savefig(save_path, dpi=300)
    print(f"[✓] Image saved: {save_path}")
    plt.close()


def batch_plot_iaatr(base_input_path, base_output_path, year, doy, site_list, rot_unit="min"):
    """
    Batch plot IAATR curves for selected stations.
    rot_unit: choose 'min' (default) or 'sec' to control AATR unit label.
              'sec' means y-axis label is TECU/sec, 'min' means TECU/min
    """
    root_output = os.path.join(base_output_path, "aatr_plot", f"{year}_{doy:03d}")
    all_files = os.listdir(base_input_path)

    for file in all_files:
        if not file.endswith("_AATR.txt"):
            continue

        parts = file.replace(".txt", "").split("_")
        if len(parts) < 3:
            print(f"[×] Invalid filename format, skipping: {file}")
            continue

        station = parts[0]
        if station not in site_list:
            continue

        full_input_path = os.path.join(base_input_path, file)
        save_folder = os.path.join(root_output, station)
        save_path = os.path.join(save_folder, file.replace(".txt", ".png"))

        plot_iaatr_curve(full_input_path, save_path, title=file.replace(".txt", ""), rot_unit=rot_unit)


if __name__ == "__main__":
    base_input_path = "../data_aatr"
    base_output_path = "../output"
    year = 2023
    doy = 100
    site_list = ["HRAO", "POTS","CORD"]
    rot_unit = "min"  # "sec" means ROT unit is TECU/sec; "min" means TECU/min

    batch_plot_iaatr(base_input_path, base_output_path, year, doy, site_list, rot_unit)
