import numpy as np
import numbers
import pandas as pd
import msgpack

import matplotlib

# matplotlib.use('Qt5Agg')
matplotlib.use('TkAgg')

import matplotlib.pyplot as plt

pd.set_option('display.max_rows', 1000)
pd.set_option('display.max_columns', 1000)
pd.set_option('display.width', 1000)
pd.set_option('display.max_colwidth', 1000)


def create_numpy_arrays(ldf):
    var_names = [k.replace(".dtype", "") for k in ldf.keys() if "dtype" in k]

    for var_name in var_names:
        ldf[var_name] = ldf.apply(lambda x: np.array(
            x[var_name + ".data"], dtype=np.dtype(x[var_name + ".dtype"])).reshape(x[var_name + ".shape"], order=x[var_name + ".order"]).squeeze(), axis=1)

    for var_name in var_names:
        ldf = ldf.drop(var_name + ".data", axis=1)
        ldf = ldf.drop(var_name + ".dtype", axis=1)
        ldf = ldf.drop(var_name + ".shape", axis=1)
        ldf = ldf.drop(var_name + ".order", axis=1)

    return ldf


def unwrap_numeric_indexes(ldf):
    for k in ldf.keys():
        if type(ldf[k].iloc[0]) == np.ndarray:
            if len(ldf[k].iloc[0].shape) == 2:
                for i in range(ldf[k].iloc[0].shape[0]):
                    for j in range(ldf[k].iloc[0].shape[1]):
                        ldf[k + "." + str(i) + str(j)] = ldf[k].apply(lambda x: x[i, j])
            elif len(ldf[k].iloc[0].shape) == 1:
                for i in range(ldf[k].iloc[0].shape[0]):
                    ldf[k + "." + str(i)] = ldf[k].apply(lambda x: x[i])
            else:
                pass
    return ldf


def load_msgpack_dataset(l_dump_path):
    
    with open(l_dump_path, "rb") as data_file:
        data = [unpacked for unpacked in msgpack.Unpacker(data_file)]

    lldf = pd.json_normalize(data)
    lldf = create_numpy_arrays(lldf)
    lldf = unwrap_numeric_indexes(lldf)

    return lldf


def plot_df_entry(df, plot_groups, skip_names=(), wrt_iloc=False, fig=None, tight=True, top=True):
    plt.style.use('default')
    # plt.style.use('dark_background')

    def get_figure(l_fig):
        if l_fig is None:
            lf = plt.figure(figsize=(18, 9), dpi=80)
        elif isinstance(l_fig, numbers.Number):
            lf = plt.figure(l_fig, figsize=(18, 9), dpi=80)
            lf.clf()
        else:
            lf = l_fig
            lf.clf()
        return lf

    def get_matches(patern, keys):
        if patern[-1] == "*":
            return [key for key in keys if patern[:-1] in key]
        else:
            return [key for key in keys if patern == key]

    def expand_plot(plot, df, ax):
        lplot = {}
        lplots = []
        if isinstance(plot, str):
            lplot = {"name": plot}
        elif isinstance(plot, dict):
            for k, v in plot.items():
                lplot[k] = v

        matches = get_matches(lplot["name"], df.columns)

        if len(matches) == 0:  # Backward compatibility
            print("len(matches) == 0:  # Enabling backward compatibility")
            lplot["name"] = lplot["name"] + "*"
            matches = get_matches(lplot["name"], df.columns)


        for match in matches:
            if any([skip_name in match for skip_name in skip_names]):
                continue
            if df[match].dtypes == np.dtype('O'):
                continue
            ddd = {k: v for k, v in lplot.items()}
            ddd["data"] = df[match].copy()
            ddd["plot_name"] = match
            ddd["ax"] = ax

            lplots.append(ddd)

        for i in range(len(lplots)):
            if lplots[i].get("lambda", None) is not None:
                lplots[i]["data"] = lplots[i]["data"].apply(lplots[i]["lambda"])

            if lplots[i].get("scale", None) is not None:
                lplots[i]["data"] = lplots[i]["data"] * lplots[i]["scale"]

        return lplots

    def plot_plot(plot):
        if wrt_iloc:
            plot["ax"].plot(plot["data"].values, '.-', label=str(plot["plot_name"]))
        else:
            plot["ax"].plot(plot["data"].index, plot["data"], '.-', label=str(plot["plot_name"]))

        if len(plot["ax"].get_lines()) > 0:
            plot["ax"].legend(loc='upper right')

        plot["ax"].grid(True, which="major")
        plot["ax"].minorticks_on()
        plot["ax"].grid(True, which="minor", linestyle=':', linewidth=1)

    def handle_plot_group(plot_group):
        if isinstance(plot_group, str):
            plot_group = [plot_group]
        return plot_group

    figure = get_figure(fig)

    for plot_group_idx, plot_group in enumerate(plot_groups):
        if plot_group_idx == 0:
            figure.add_subplot(len(plot_groups), 1, plot_group_idx + 1)
        else:
            figure.add_subplot(len(plot_groups), 1, plot_group_idx + 1, sharex=figure.axes[0])

    all_plots = []

    for plot_group_idx, plot_group in enumerate(plot_groups):

        plot_group = handle_plot_group(plot_group)

        for plot in plot_group:
            plots = expand_plot(plot, df, figure.axes[plot_group_idx])

            all_plots += plots

    for plot in all_plots:
        plot_plot(plot)

    if tight:
        figure.tight_layout()

    if top and matplotlib.get_backend() == 'TkAgg':
        figure.canvas.manager.window.attributes('-topmost', 1)

    return figure, figure.axes

