# from functools import reduce
import pandas as pd
import numpy as np
import os
import sys
import matplotlib.pyplot as plt
from collections import namedtuple

RESULTS_DIR = '../../../res'
OUTPUT_DIR = 'plots/'

Param = namedtuple('Param', ['name', 'short', 'long', 'default'])
Config = namedtuple('Config', ['support', 'processes', 'threads', 'density'])

DEFAULT_PARAMS = Config(
    Param('support',   'S', 'supports',                '0.0001'),
    Param('processes', 'P', 'number of processes',         '16'),
    Param('threads',   'T', 'number of threads',            '8'),
    Param('density',   'D', 'frequent itemset densities', '0.6'),
)


def load_dataset(dir_path):
    dataset = pd.DataFrame()
    for file_path in filter(lambda x: x.startswith('out_'), os.listdir(dir_path)):
        _, ts, _, s, p, t, d = file_path.split('_')
        tmp = pd.read_csv(os.path.join(dir_path, file_path), sep=', ', engine='python') \
            .assign(
                timestamp=ts,
                support=s,
                processes=p,
                threads=t,
                density=d
        )
        dataset = dataset.append(tmp, ignore_index=True, sort=False)

    return dataset


def aggregate_run_times(dataset):
    return dataset.groupby(['timestamp', 'support', 'processes', 'threads', 'density', 'rank']) \
        .agg({'time': np.sum}) \
        .groupby(['timestamp', 'support', 'processes', 'threads', 'density']) \
        .agg({'time': np.max}) \
        .reset_index()


def default_config(dataset, excluded_param):
    # mask = reduce(lambda x, y: x & (dataset[y.name] == y.default),
    #               filter(lambda x: x != excluded_param, DEFAULT_PARAMS),
    #               pd.Series((True for _ in range(dataset.shape[0]))))
    mask = pd.Series((True for _ in range(dataset.shape[0])))
    for param in filter(lambda x: x != excluded_param, DEFAULT_PARAMS):
        mask &= dataset[param.name] == param.default
    return dataset[mask]


def get_plot_title(excluded_param):
    params = ', '.join(f"{param.name}={param.default}" for param in DEFAULT_PARAMS
                       if param != excluded_param)
    return f"""Time with different {excluded_param.long}
({params}, nodes=16, cpu=8)"""


def plot_vary_param(dataset, param, out):
    # print(default_config(dataset, excluded_param=param).sort_values(
    #     by=[param.name], key=lambda c: pd.to_numeric(c)))

    default_config(dataset, excluded_param=param) \
        .groupby(param.name).agg({'time': np.mean}) \
        .sort_values(by=[param.name],
                     key=lambda c: pd.to_numeric(c)) \
        .plot(ylabel='seconds',
              title=get_plot_title(excluded_param=param)) \


    plt.savefig(os.path.join(out, f'{param.name}.png'))


def plot_count(dataset):
    dataset.groupby(['processes', 'threads', 'density']
                    ).agg({'timestamp': len}).plot(kind='bar')
    plt.show()


if __name__ == '__main__':
    basepath = os.path.dirname(sys.argv[0])
    dataset = load_dataset(os.path.join(basepath, RESULTS_DIR))
    dataset = aggregate_run_times(dataset)
    plot_count(dataset)
    # print(dataset[(dataset['processes'] != '1') & (
    #     dataset['threads'] != '1')]['time'].max())
    # print(dataset)
    # print(dataset[dataset['timestamp']
    #       == '2021-06-09-12-20-39'])
    out_dir = os.path.join(basepath, OUTPUT_DIR)
    plot_vary_param(dataset, DEFAULT_PARAMS.processes, out=out_dir)
    plot_vary_param(dataset, DEFAULT_PARAMS.threads,   out=out_dir)
    plot_vary_param(dataset, DEFAULT_PARAMS.density,   out=out_dir)
