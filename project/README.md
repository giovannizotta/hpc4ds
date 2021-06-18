# Frequent Pattern Growth Algorithm using MPI and OpenMP

First part of the Project for the High Performance Computing for Data Science course.
Parallelization of the Frequent Pattern Growth algorithm using MPI and OpenMP.

##### Team 8

|   Name   |  Surname  |     Username     |
| :------: | :-------: | :--------------: |
| Gabriele |  Masina   | `masinag`        |
| Giovanni |   Zotta   | `GiovanniZotta`  |

## Acknowledgements

* [Dataset source](https://www.kaggle.com/jeffheaton/characteristics-that-favor-freqitemset-algorithms)
* [Hash-table implementation credits](https://github.com/zigalenarcic/c_hashmap)
* [Vector implementation credits](https://github.com/eteran/c-vector)


## Folder structure

* `bin/` compiled binaries
* `doc/` documentation
  * `doxygen/latex/refman.pdf` documentation of the code
  * `HPC_report.pdf` final report
* `src/` source code
* `sub_results/` benchmark results
* `sub_scripts/` examples of scripts to run on cluster with PBS


## How to run

* `make build` build the code
* `make run_local N_PROC=<n_proc> FILENAME=<filename> N_THREAD=<n_thread> MIN_SUPPORT=<min_support> DEBUG=<1/0>` run the code locally 
* see `sub_scripts/` for examples on how to deploy on a cluster using PBS