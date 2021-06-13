# from functools import reduce
import pandas as pd
import numpy as np
import os
import sys
import matplotlib.pyplot as plt
from collections import namedtuple

RESULTS_DIR = 'pct/'
OUTPUT_DIR = 'plots/'

Param = namedtuple('Param', ['name', 'short', 'long', 'default'])
Config = namedtuple(
    'Config', ['support', 'processes', 'threads', 'density', 'nodes', 'cpus'])

DEFAULT_PARAMS = Config(
    Param('support',   'S', 'supports',                '0.0001'),
    Param('processes', 'P', 'number of processes',         '16'),
    Param('threads',   'T', 'number of threads',            '8'),
    Param('density',   'D', 'frequent itemset densities', '0.6'),
    Param('nodes',     'N', 'nodes',                       '16'),
    Param('cpus',      'C', 'cpus',                         '8')
)

SINGLE_PARAMS = Config(
    Param('support',   'S', 'supports',                '0.0001'),
    Param('processes', 'P', 'number of processes',          '1'),
    Param('threads',   'T', 'number of threads',            '1'),
    Param('density',   'D', 'frequent itemset densities', '0.6'),
    Param('nodes',     'N', 'nodes',                        '1'),
    Param('cpus',      'C', 'cpus',                         '1')
)


def load_dataset(dir_path):
    dataset = pd.DataFrame()
    for file_path in filter(lambda x: x.startswith('out_'), os.listdir(dir_path)):
        _, ts, _, s, p, t, d, sc = file_path.split('_')
        # print(file_path)
        tmp = pd.read_csv(os.path.join(dir_path, file_path), sep=', ', engine='python') \
            .assign(
                timestamp=ts,
                support=s,
                processes=p,
                threads=t,
                density=d
        )
        if (tmp[(tmp['rank'] == 0) & (tmp['msg'] == 'received global tree')].empty):
            print(f'Discarding {file_path}')
        else:
            dataset = dataset.append(tmp, ignore_index=True, sort=False)

    return dataset


def aggregate_rank(dataset):
    return dataset\
        .groupby(['timestamp', 'support', 'processes', 'threads', 'density', 'rank']) \
        .agg({'time': np.sum})\
        .groupby(['timestamp', 'support', 'processes', 'threads', 'density']) \
        .agg({'time': np.max}) \
        .reset_index()


def aggregate_step(dataset):
    return dataset\
        .groupby(['timestamp', 'support', 'processes', 'threads', 'density', 'msg']) \
        .agg({'time': np.max}) \
        .reset_index()


def load_config(dataset, config, excluded_param):
    # mask = reduce(lambda x, y: x & (dataset[y.name] == y.default),
    #               filter(lambda x: x != excluded_param, DEFAULT_PARAMS),
    #               pd.Series((True for _ in range(dataset.shape[0]))))
    mask = pd.Series((True for _ in range(dataset.shape[0])))
    for param in filter(lambda x: x != excluded_param and
                        x.name in dataset.columns, config):
        mask &= dataset[param.name] == param.default
    return dataset[mask]


def get_plot_title(excluded_param, config, additional_param=''):

    params = ', '.join(f"{param.name}={param.default}" for param in config
                       if param != excluded_param)
    return f"""Time with different {excluded_param.long} {additional_param}
({params})"""


def plot_vary_param(dataset, param, out, config=DEFAULT_PARAMS):
    # print(load_config(dataset, config, excluded_param=param).sort_values(
    #     by=[param.name], key=lambda c: pd.to_numeric(c)))

    load_config(dataset, config, excluded_param=param) \
        .groupby(param.name) \
        .agg({'time': np.mean}) \
        .sort_values(by=[param.name],
                     key=lambda c: pd.to_numeric(c)) \
        .plot(ylabel='seconds',
              title=get_plot_title(param, config))

    plt.savefig(os.path.join(
        out, f'{config.nodes.default}_{config.cpus.default}_{param.name}.png'))


def plot_count(dataset, out):
    dataset.groupby(['processes', 'threads', 'density'])\
        .agg({'timestamp': len})\
        .plot(kind='bar',
              title="Number of runs per configuration")
    plt.tight_layout()
    plt.savefig(os.path.join(out, f'count.png'))


def plot_vary_param_per_step(dataset, param, out, config=DEFAULT_PARAMS):
    load_config(dataset, config, excluded_param=param) \
        .groupby([param.name, 'msg']) \
        .agg({'time': np.mean}) \
        .unstack() \
        .sort_values(by=[param.name],
                     key=lambda c: pd.to_numeric(c)) \
        .plot(ylabel='seconds',
              title=get_plot_title(param, config, additional_param='per step'))
    plt.tight_layout()
    plt.savefig(os.path.join(
        out, f'{config.nodes.default}_{config.cpus.default}_{param.name}_step.png'))


def plot_configurations_time(dataset, out, fix_param):
    config_params = ['processes', 'threads']
    params = ', '.join(f"{param.name}={param.default}" for param in DEFAULT_PARAMS
                       if param.name not in config_params)
    title = f"""Time with different number of processes and threads
({params})"""

    dataset_agg = dataset[dataset[fix_param.name] == fix_param.default] \
        .groupby(config_params)\
        .agg({'time': np.mean})\
        .sort_values(by=config_params,
                     key=lambda c: pd.to_numeric(c))

    ticks = [f"({p},{t})" for p, t in dataset_agg.index.tolist()]
    ax = dataset_agg.plot(ylabel='seconds', rot=90,
                          title=title)
    ax.set_xticks(range(len(ticks)))
    ax.set_xticklabels(ticks)

    plt.tight_layout()
    plt.savefig(os.path.join(out, f'config_time.png'))


if __name__ == '__main__':
    basepath = os.path.dirname(sys.argv[0])
    out_dir = os.path.join(basepath, OUTPUT_DIR)
    # aggregate data
    dataset = load_dataset(os.path.join(basepath, RESULTS_DIR))
    dataset_agg_all = aggregate_rank(dataset)
    dataset_agg_step = aggregate_step(dataset)
    # plots with 16 proc 8 threads
    plot_count(dataset_agg_all, out_dir)
    plot_vary_param(dataset_agg_all,
                    DEFAULT_PARAMS.processes, out=out_dir)
    plot_vary_param(dataset_agg_all,
                    DEFAULT_PARAMS.threads,   out=out_dir)
    plot_vary_param(dataset_agg_all,
                    DEFAULT_PARAMS.density,   out=out_dir)

    # plots with 16 proc 8 threads for each step
    plot_vary_param_per_step(
        dataset_agg_step, DEFAULT_PARAMS.processes, out=out_dir)
    plot_vary_param_per_step(
        dataset_agg_step, DEFAULT_PARAMS.threads,   out=out_dir)
    plot_vary_param_per_step(
        dataset_agg_step, DEFAULT_PARAMS.density,   out=out_dir)

    # plots with 1 proc 1 thread
    plot_vary_param(dataset_agg_all,
                    DEFAULT_PARAMS.density,          out=out_dir, config=SINGLE_PARAMS)

    # plots with 1 proc 1 thread for each step
    plot_vary_param_per_step(
        dataset_agg_step, DEFAULT_PARAMS.density,   out=out_dir, config=SINGLE_PARAMS)

    plot_configurations_time(dataset_agg_all, out=out_dir,
                             fix_param=DEFAULT_PARAMS.density)
