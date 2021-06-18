# from functools import reduce
import pandas as pd
import numpy as np
import os
import sys
import matplotlib.pyplot as plt
from collections import namedtuple

RESULTS_DIR = 'pct/'
OUTPUT_DIR = 'plots/'
EXT = 'pdf'

Param = namedtuple('Param', ['name', 'short', 'long', 'default'])
Config = namedtuple(
    'Config', ['schedule', 'processes', 'threads', 'density', 'nodes', 'cpus'])

DEFAULT_PARAMS_STAT = Config(
    # Param('support',   'S', 'supports',                '0.0001'),
    Param('schedule',   'S', 'schedule',               'static'),
    Param('processes', 'P', 'number of processes',           16),
    Param('threads',   'T', 'number of threads',              8),
    Param('density',   'D', 'frequent itemset densities', '0.6'),
    Param('nodes',     'N', 'nodes',                       '16'),
    Param('cpus',      'C', 'cpus',                         '8')
)

SINGLE_PARAMS = Config(
    # Param('support',   'S', 'supports',                '0.0001'),
    Param('schedule',   'S', 'schedule',              'dynamic'),
    Param('processes', 'P', 'number of processes',            1),
    Param('threads',   'T', 'number of threads',              1),
    Param('density',   'D', 'frequent itemset densities', '0.6'),
    Param('nodes',     'N', 'nodes',                        '1'),
    Param('cpus',      'C', 'cpus',                         '1')
)


DEFAULT_PARAMS_DYN = Config(
    # Param('support',   'S', 'supports',                '0.0001'),
    Param('schedule',   'S', 'schedule',              'dynamic'),
    Param('processes', 'P', 'number of processes',           16),
    Param('threads',   'T', 'number of threads',              8),
    Param('density',   'D', 'frequent itemset densities', '0.6'),
    Param('nodes',     'N', 'nodes',                       '16'),
    Param('cpus',      'C', 'cpus',                         '8')
)

DEFAULT_PARAMS_GUI = Config(
    # Param('support',   'S', 'supports',                '0.0001'),
    Param('schedule',   'S', 'schedule',               'guided'),
    Param('processes', 'P', 'number of processes',           16),
    Param('threads',   'T', 'number of threads',              8),
    Param('density',   'D', 'frequent itemset densities', '0.6'),
    Param('nodes',     'N', 'nodes',                       '16'),
    Param('cpus',      'C', 'cpus',                         '8')
)


def load_dataset(dir_path):
    dataset = pd.DataFrame()
    for file_path in filter(lambda x: x.startswith('out_'), os.listdir(dir_path)):
        # print(file_path)
        _, ts, _, s, p, t, d, sc = file_path.split('_')
        # print(file_path)
        tmp = pd.read_csv(os.path.join(dir_path, file_path), sep=', ', engine='python') \
            .assign(
                timestamp=ts,
                support=s,
                processes=int(p),
                threads=int(t),
                density=d,
                schedule=sc
        )
        if (tmp[(tmp['rank'] == 0) & (tmp['msg'] == 'received global tree')].empty):
            print(f'Discarding {file_path}')
        else:
            if p == '1' and t == '1':
                tmp.loc[tmp['msg'] == 'received global tree', 'time'] = 0.0
            dataset = dataset.append(tmp, ignore_index=True, sort=False)

    return dataset


def aggregate_rank(dataset):
    return dataset\
        .groupby(['timestamp', 'support', 'processes', 'threads', 'density', 'schedule', 'rank']) \
        .agg({'time': np.sum})\
        .groupby(['timestamp', 'support', 'processes', 'threads', 'density', 'schedule']) \
        .agg({'time': np.max}) \
        .reset_index()


def aggregate_step(dataset):
    return dataset\
        .groupby(['timestamp', 'support', 'processes', 'threads', 'schedule', 'density', 'msg']) \
        .agg({'time': np.max}) \
        .reset_index()


def load_config(dataset, config, excluded_param,
                excluded_steps=[], include_schedule=True):
    # mask = reduce(lambda x, y: x & (dataset[y.name] == y.default),
    #               filter(lambda x: x != excluded_param, DEFAULT_PARAMS_STAT),
    #               pd.Series((True for _ in range(dataset.shape[0]))))
    mask = pd.Series((True for _ in range(dataset.shape[0])))
    for param in filter(lambda x: x != excluded_param and
                        x.name in dataset.columns and
                        (x != config.schedule or include_schedule), config):
        mask &= dataset[param.name] == param.default
    for step in excluded_steps:
        mask &= dataset['msg'] != step
    return dataset[mask]


def get_plot_title(excluded_param, config, additional_param='', include_schedule=False):

    params = ', '.join(f"{param.name}={param.default}" for param in config
                       if param != excluded_param and (include_schedule or param != config.schedule))
    return f"""Time with different {excluded_param.long} {additional_param}
({params})"""


def plot_vary_param(dataset, param, out, config=DEFAULT_PARAMS_STAT):
    # print(load_config(dataset, config, excluded_param=param).sort_values(
    #     by=[param.name], key=lambda c: pd.to_numeric(c)))
    p = load_config(dataset, config, excluded_param=param, include_schedule=False) \
        .groupby([param.name, 'schedule']) \
        .agg({'time': np.mean}) \
        .unstack() \
        .sort_values(by=[param.name])

    p.plot(ylabel='seconds',
           title=get_plot_title(param, config, include_schedule=False))
    if param.name != 'density':
        plt.xticks(p.index)

    plt.legend(title=None)
    plt.tight_layout()
    plt.savefig(os.path.join(
        out, f'{config.nodes.default}_{config.cpus.default}_{param.name}.{EXT}'))
    plt.cla()
    plt.close()


def plot_count(dataset, out):
    p = dataset.groupby(['processes', 'threads', 'density', 'schedule'])\
        .agg({'timestamp': len}) \
        .sort_values(by=['processes', 'threads', 'density'])

    print(p.reset_index())
    p.unstack() \
        .plot(kind='bar',
              title="Number of runs per configuration")

    plt.legend(title=None)
    plt.tight_layout()
    plt.savefig(os.path.join(out, f'count.{EXT}'))
    plt.cla()
    plt.close()


def plot_vary_param_per_step(dataset, param, out, config=DEFAULT_PARAMS_STAT, fn=np.mean, sx='', excluded_steps=[]):
    p = load_config(dataset, config, excluded_param=param, excluded_steps=excluded_steps) \
        .groupby([param.name, 'msg']) \
        .agg({'time': fn}) \
        .unstack() \
        .sort_values(by=[param.name])

    p.plot(ylabel='seconds',
           title=get_plot_title(param, config, additional_param='per step'))
    if param.name != 'density':
        plt.xticks(p.index)
    plt.legend(title=None)
    plt.tight_layout()
    plt.savefig(os.path.join(
        out, f'{config.nodes.default}_{config.cpus.default}_{config.schedule.default}_{param.name}_step{sx}.{EXT}'))
    plt.cla()
    plt.close()


def plot_configurations_time(dataset, out, fix_param):
    config_params = ['processes', 'threads', 'schedule']
    params = ', '.join(f"{param.name}={param.default}" for param in DEFAULT_PARAMS_STAT
                       if param.name not in config_params)
    title = f"""Time with different number of processes and threads
({params})"""

    dataset_agg = dataset[dataset[fix_param.name] == fix_param.default] \
        .groupby(config_params) \
        .agg({'time': np.mean}) \
        .unstack() \
        .sort_values(by=['processes', 'threads'])

    ticks = [f"({p},{t})" for p, t in dataset_agg.index.tolist()]
    ax = dataset_agg.plot(ylabel='seconds', rot=90,
                          title=title)
    ax.set_xticks(range(len(ticks)))
    ax.set_xticklabels(ticks)

    plt.tight_layout()
    plt.legend(title=None)
    plt.savefig(os.path.join(out, f'config_time.{EXT}'))
    plt.cla()
    plt.close()


def plot_1_1(dataset_agg_all, dataset_agg_step, out):
    # plots with 1 proc 1 thread
    plot_vary_param(dataset_agg_all, DEFAULT_PARAMS_STAT.density, out=out_dir,
                    config=SINGLE_PARAMS)

    # plots with 1 proc 1 thread for each step
    plot_vary_param_per_step(dataset_agg_step, DEFAULT_PARAMS_STAT.density,
                             out, config=SINGLE_PARAMS)

    # plots with 1 proc 1 thread for each step without some steps
    plot_vary_param_per_step(
        dataset_agg_step, DEFAULT_PARAMS_STAT.density,  out, config=SINGLE_PARAMS,
        excluded_steps=['built local tree'], sx='_excluded_local_tree')

    plot_vary_param_per_step(
        dataset_agg_step, DEFAULT_PARAMS_STAT.density,   out=out_dir, config=SINGLE_PARAMS,
        excluded_steps=['read transactions', 'built local tree'], sx='_excluded_read_transactions')


def plot_16_8(dataset_agg_all, dataset_agg_step, default, out):
    default_varying = (default.processes,
                       default.threads, default.density)

    if default == DEFAULT_PARAMS_DYN:

        for param in default_varying:
            plot_vary_param(dataset_agg_all, param,
                            out, config=default)

    # plots with 16 proc 8 threads for each step
    for param in default_varying:
        plot_vary_param_per_step(
            dataset_agg_step, param, out, config=default)

    plot_vary_param_per_step(
        dataset_agg_step, default.density,   out, config=default,
        excluded_steps=['received global tree'], sx='_excluded_global_tree')
    plot_vary_param_per_step(
        dataset_agg_step, default.density,   out, config=default,
        excluded_steps=['received global tree', 'built local tree'], sx='_excluded_global_local_tree')


def plot_blt(dataset_agg_step, default, out):

    # print(dataset.shape)
    load_config(
        dataset_agg_step,
        default,
        excluded_param=default.threads,
        excluded_steps=['read transactions',
                        'received global map',
                        'sorted local items',
                        'received sorted global items'
                        'received global tree'],
        include_schedule=False) \
        .groupby(['processes', 'threads', 'schedule']) \
        .agg({'time': np.mean}) \
        .unstack() \
        .sort_values(by=['processes', 'threads']) \
        .plot(title=get_plot_title(default.threads, default, additional_param='(built local tree)'))

    plt.tight_layout()
    plt.legend(title=None)
    plt.savefig(os.path.join(out, f'built_local_tree.{EXT}'))
    plt.cla()
    plt.close()


if __name__ == '__main__':
    basepath = os.path.dirname(sys.argv[0])
    out_dir = os.path.join(basepath, OUTPUT_DIR)
    # aggregate data
    dataset = load_dataset(os.path.join(basepath, RESULTS_DIR))
    dataset_agg_all = aggregate_rank(dataset)
    dataset_agg_step = aggregate_step(dataset)
    # plots with 16 proc 8 threads
    plot_count(dataset_agg_all, out_dir)

    plot_configurations_time(dataset_agg_all, out=out_dir,
                             fix_param=DEFAULT_PARAMS_STAT.density)

    plot_1_1(dataset_agg_all, dataset_agg_step, out=out_dir)

    plot_16_8(dataset_agg_all, dataset_agg_step,
              DEFAULT_PARAMS_DYN, out=out_dir)

    plot_16_8(dataset_agg_all, dataset_agg_step,
              DEFAULT_PARAMS_STAT, out=out_dir)

    plot_16_8(dataset_agg_all, dataset_agg_step,
              DEFAULT_PARAMS_GUI, out=out_dir)
    plot_blt(dataset_agg_step, DEFAULT_PARAMS_STAT, out=out_dir)
