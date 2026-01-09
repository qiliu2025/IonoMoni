import numpy as np
import matplotlib.pyplot as plt
import os
import cartopy.crs as ccrs
import cartopy.feature as cfeature
import matplotlib.ticker as mticker

def load_vtec_txt(filepath):
    data = []
    with open(filepath, 'r') as f:
        for idx, line in enumerate(f):
            parts = line.strip().split()
            if idx == 0 or not parts:
                continue
            values = parts[2:]
            row = []
            for val in values:
                try:
                    row.append(float(val))
                except:
                    row.append(np.nan)
            data.append(row)
    return np.array(data)

def get_existing_file(folder, patterns):
    for pattern in patterns:
        fpath = os.path.join(folder, pattern)
        if os.path.exists(fpath):
            return fpath
    return None

def get_global_vtec_range(data_folder, sitelists, system):
    vtec_min, vtec_max = np.inf, -np.inf
    for site in sitelists:
        vtec_file = get_existing_file(
            data_folder,
            [f"{site}_VTEC_{system}.txt", f"{site}_{system}_VTEC.txt"]
        )
        if not vtec_file:
            continue
        vtec = load_vtec_txt(vtec_file)
        valid = vtec[~np.isnan(vtec) & (vtec != 0)]
        if valid.size > 0:
            vtec_min = min(vtec_min, np.min(valid))
            vtec_max = max(vtec_max, np.max(valid))
    return vtec_min, vtec_max

def deg_label(val, axis='lat'):
    abs_val = abs(int(val))
    if axis == 'lat':
        return f"{abs_val}N" if val >= 0 else f"{abs_val}S"
    else:
        return f"{abs_val}E" if val >= 0 else f"{abs_val}W"

def auto_tick_interval(deg_min, deg_max, max_ticks=8):
    span = abs(deg_max - deg_min)
    candidates = [1, 2, 3, 5, 10, 15, 20, 30, 45, 60, 90, 180]
    for interval in candidates:
        if span / interval <= max_ticks:
            return interval
    return candidates[-1]

def plot_multi_station_vtec(
        data_folder, sitelists, system="GPS",
        output_base=".", year=2023, doy=1
):
    vmin, vmax = get_global_vtec_range(data_folder, sitelists, system)
    if not np.isfinite(vmin) or not np.isfinite(vmax):
        print("[×] No valid VTEC data found. Plot aborted.")
        return

    fig, axes = plt.subplots(nrows=6, ncols=4, figsize=(20, 23), subplot_kw={'projection': ccrs.PlateCarree()})
    axes = axes.reshape(6, 4)
    plt.subplots_adjust(left=0.05, right=0.89, top=0.98, bottom=0.05, wspace=0.15, hspace=0.15)

    all_lons, all_lats = [], []

    for hour in range(24):
        lats_all, lons_all, vtecs_all = [], [], []
        for site in sitelists:
            vtec_file = get_existing_file(
                data_folder,
                [f"{site}_VTEC_{system}.txt", f"{site}_{system}_VTEC.txt"]
            )
            lat_file = get_existing_file(
                data_folder,
                [f"{site}_IPP_lat_{system}.txt"]
            )
            lon_file = get_existing_file(
                data_folder,
                [f"{site}_IPP_lon_{system}.txt"]
            )
            if not (vtec_file and lat_file and lon_file):
                continue
            vtec = load_vtec_txt(vtec_file)
            lat = np.loadtxt(lat_file, skiprows=1)
            lon = np.loadtxt(lon_file, skiprows=1)
            if vtec.shape != lat.shape or vtec.shape != lon.shape:
                continue
            num_epochs, num_sats = vtec.shape
            epoch_start = hour * 120
            epoch_end = min((hour + 1) * 120, num_epochs)
            for epoch in range(epoch_start, epoch_end):
                for sat in range(num_sats):
                    val = vtec[epoch, sat]
                    if not np.isnan(val) and val != 0:
                        lats_all.append(lat[epoch, sat])
                        lons_all.append(lon[epoch, sat])
                        vtecs_all.append(val)
        all_lats.extend(lats_all)
        all_lons.extend(lons_all)

        row, col = divmod(hour, 4)
        ax = axes[row, col]
        ax.add_feature(cfeature.COASTLINE, linewidth=0.8)
        ax.add_feature(cfeature.BORDERS, linestyle=':', linewidth=0.6)
        ax.add_feature(cfeature.LAND, edgecolor='black', facecolor='white', zorder=0)

        if lons_all and lats_all:
            lon_min = np.floor(np.min(lons_all)) - 0.5
            lon_max = np.ceil(np.max(lons_all)) + 0.5
            lat_min = np.floor(np.min(lats_all)) - 0.5
            lat_max = np.ceil(np.max(all_lats)) + 0.5
            ax.set_extent([lon_min, lon_max, lat_min, lat_max], crs=ccrs.PlateCarree())
            ax.scatter(lons_all, lats_all, c=vtecs_all, cmap='jet', vmin=vmin, vmax=vmax, s=18, marker='o')
        else:
            lon_min, lon_max, lat_min, lat_max = 10, 40, 45, 55
            ax.set_extent([lon_min, lon_max, lat_min, lat_max], crs=ccrs.PlateCarree())

        # Dynamic ticks
        lon_interval = auto_tick_interval(lon_min, lon_max)
        lat_interval = auto_tick_interval(lat_min, lat_max)
        xticks = np.arange(np.ceil(lon_min//lon_interval)*lon_interval,
                           np.floor(lon_max//lon_interval)*lon_interval + lon_interval, lon_interval)
        yticks = np.arange(np.ceil(lat_min//lat_interval)*lat_interval,
                           np.floor(lat_max//lat_interval)*lat_interval + lat_interval, lat_interval)
        ax.set_xticks(xticks, crs=ccrs.PlateCarree())
        ax.set_yticks(yticks, crs=ccrs.PlateCarree())
        ax.xaxis.set_major_locator(mticker.MaxNLocator(nbins=4))
        ax.yaxis.set_major_locator(mticker.MaxNLocator(nbins=5))

        ax.tick_params(axis='x', labelbottom=True, labeltop=False)
        ax.tick_params(axis='y', labelleft=True, labelright=False)
        lon_formatter = mticker.FuncFormatter(lambda x, _: f"{int(x)}°E" if x >= 0 else f"{int(-x)}°W")
        lat_formatter = mticker.FuncFormatter(lambda y, _: f"{int(y)}°N" if y >= 0 else f"{int(-y)}°S")
        ax.xaxis.set_major_formatter(lon_formatter)
        ax.yaxis.set_major_formatter(lat_formatter)
        gl = ax.gridlines(draw_labels=False, linewidth=0.4, color='gray', alpha=0.3, linestyle='--')
        ax.set_title(f"UT: {hour:02d}:00", fontsize=11)

    cbar_ax = fig.add_axes([0.92, 0.12, 0.02, 0.76])
    sm = plt.cm.ScalarMappable(cmap='jet', norm=plt.Normalize(vmin=vmin, vmax=vmax))
    cbar = fig.colorbar(sm, cax=cbar_ax)
    cbar.set_label('VTEC (TECU)', fontsize=16)
    cbar.ax.tick_params(labelsize=13)

    # Compute global lon/lat range (integer)
    if all_lons and all_lats:
        lon_min = int(np.floor(np.min(all_lons)))
        lon_max = int(np.ceil(np.max(all_lons)))
        lat_min = int(np.floor(np.min(all_lats)))
        lat_max = int(np.ceil(np.max(all_lats)))
    else:
        lon_min, lon_max, lat_min, lat_max = 0, 0, 0, 0

    # Assemble output filename
    lon_min_label = deg_label(lon_min, 'lon')
    lon_max_label = deg_label(lon_max, 'lon')
    lat_min_label = deg_label(lat_min, 'lat')
    lat_max_label = deg_label(lat_max, 'lat')
    output_name_final = f"VTEC_{system}_map_lon{lon_min_label}-{lon_max_label}_lat{lat_min_label}-{lat_max_label}.png"

    out_dir = os.path.join(output_base, "vtec_map_plot", f"{year}_{doy:03d}")
    os.makedirs(out_dir, exist_ok=True)
    output_path = os.path.join(out_dir, output_name_final)
    plt.savefig(output_path, dpi=600)
    plt.close()
    print(f"[✓] Image saved: {output_path}")

# ======== Main entry point ========
if __name__ == "__main__":
    input_folder = r"../data_vtec"  # Input data directory
    output_base = r"../output"      # Output root directory
    sitelists = ["GANP","PENC","SULP"]
    year = 2023
    doy = 100
    system = "GPS"
    plot_multi_station_vtec(
        input_folder, sitelists, system,
        output_base=output_base, year=year, doy=doy
    )
