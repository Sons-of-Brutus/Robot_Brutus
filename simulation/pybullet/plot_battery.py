#!/usr/bin/env python3
import argparse
import os
import pandas as pd
import matplotlib.pyplot as plt

# Columnas esperadas
TIME_COL = "t_seconds"
Z_COL = "battery_z_m"

def moving_average(x, window):
    if window is None or window <= 1:
        return x
    return x.rolling(window=window, min_periods=1, center=False).mean()

def main():
    parser = argparse.ArgumentParser(description="Graficar altura de la batería desde CSV.")
    parser.add_argument("csv", help="Ruta al CSV (ej: logs/battery_height.csv o tu torques.csv con columna battery_z_m)")
    parser.add_argument("--smooth", type=int, default=0,
                        help="Ventana de suavizado (media móvil). 0 o 1 = sin suavizado. Ej: --smooth 5")
    parser.add_argument("--outdir", default="plots",
                        help="Directorio de salida para la imagen (por defecto: plots)")
    args = parser.parse_args()

    os.makedirs(args.outdir, exist_ok=True)

    # Cargar CSV
    df = pd.read_csv(args.csv)
    if TIME_COL not in df.columns:
        raise ValueError(f"El CSV no tiene columna '{TIME_COL}'.")
    if Z_COL not in df.columns:
        raise ValueError(f"El CSV no tiene columna '{Z_COL}'. "
                         f"Asegúrate de haberla guardado como '{Z_COL}' al loguear.")

    t = df[TIME_COL]
    z = df[Z_COL]
    if args.smooth and args.smooth > 1:
        z = moving_average(z, args.smooth)

    # Plot
    plt.figure()
    plt.plot(t, z, label=Z_COL)
    plt.xlabel(TIME_COL)
    plt.ylabel("altura batería [m]")
    plt.title("Altura de la batería vs tiempo")
    plt.legend()
    plt.tight_layout()

    outpath = os.path.join(args.outdir, "battery_height.png")
    plt.savefig(outpath, dpi=160)
    plt.close()
    print(f"[OK] Guardado: {outpath}")
    print("\nListo. Revisa la carpeta:", os.path.abspath(args.outdir))

if __name__ == "__main__":
    main()
