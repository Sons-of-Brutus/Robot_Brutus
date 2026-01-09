#!/usr/bin/env python3
import argparse
import os
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# Columnas esperadas (según tu logger actual)
ELBOW_COLS = ["fr_elbow_tau","fl_elbow_tau","br_elbow_tau","bl_elbow_tau"]
BOTTOM_COLS = ["fr_bottom_tau","fl_bottom_tau","br_bottom_tau","bl_bottom_tau"]

def moving_average(x, window):
    if window is None or window <= 1:
        return x
    return x.rolling(window=window, min_periods=1, center=False).mean()

def plot_group(df, t, cols, title, outfile, smooth_window=None):
    plt.figure()
    for c in cols:
        y = df[c]
        if smooth_window and smooth_window > 1:
            y = moving_average(y, smooth_window)
        plt.plot(t, y, label=c)
    plt.xlabel("t_seconds")
    plt.ylabel("tau [N·m]")
    plt.title(title)
    plt.legend()
    plt.tight_layout()
    plt.savefig(outfile, dpi=160)
    plt.close()
    print(f"[OK] Guardado: {outfile}")

def main():
    parser = argparse.ArgumentParser(description="Graficar torques desde CSV del robot.")
    parser.add_argument("csv", help="Ruta al CSV (ej: logs/torques_20251103_155105.csv)")
    parser.add_argument("--smooth", type=int, default=0,
                        help="Ventana de suavizado (media móvil). 0 o 1 = sin suavizado. Ej: --smooth 5")
    parser.add_argument("--outdir", default="plots",
                        help="Directorio de salida para las imágenes (por defecto: plots)")
    args = parser.parse_args()

    os.makedirs(args.outdir, exist_ok=True)

    # Carga CSV
    df = pd.read_csv(args.csv)
    if "t_seconds" not in df.columns:
        raise ValueError("El CSV no tiene columna 't_seconds'.")

    # Verifica columnas presentes según tu logger actual
    missing = [c for c in ELBOW_COLS + BOTTOM_COLS if c not in df.columns]
    if missing:
        raise ValueError(f"Faltan columnas en el CSV: {missing}")

    t = df["t_seconds"]

    # 1) Elbows
    plot_group(
        df, t, ELBOW_COLS,
        title="Elbow torques vs time",
        outfile=os.path.join(args.outdir, "elbows_tau.png"),
        smooth_window=args.smooth
    )

    # 2) Bottoms
    plot_group(
        df, t, BOTTOM_COLS,
        title="Bottom torques vs time",
        outfile=os.path.join(args.outdir, "bottoms_tau.png"),
        smooth_window=args.smooth
    )

    # 3) Gasto total = suma de |tau| de todas las juntas por instante
    tau_cols = ELBOW_COLS + BOTTOM_COLS
    gasto = df[tau_cols].abs().sum(axis=1)
    if args.smooth and args.smooth > 1:
        gasto = moving_average(gasto, args.smooth)

    plt.figure()
    plt.plot(t, gasto)
    plt.xlabel("t_seconds")
    plt.ylabel("Σ |tau| [N·m]")
    plt.title("Gasto instantáneo de torque")
    plt.tight_layout()
    plt.savefig(os.path.join(args.outdir, "gasto_tau_abs.png"), dpi=160)
    plt.close()
    print(f"[OK] Guardado: {os.path.join(args.outdir, 'gasto_tau_abs.png')}")

    print("\nListo. Revisa la carpeta:", os.path.abspath(args.outdir))

if __name__ == "__main__":
    main()
