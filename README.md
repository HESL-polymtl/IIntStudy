# IIntStudy
Interrupt-Related Interference Study | Benchmarks, Measurements and Scripts Repository

This repository is organized as follows:

## Measurement
This folder contains the measurements used in our publication. Each CSV is extracted from raw binaries using the extraction scripts in the **Scripts** folder.

Each measurement should be organized with the following hierarchy:

```
Measurements
├── MITIGATION-NAME_mitig
| ├── [SC/MC]_[INT-TYPE]_[INT_RATE]
| | ├── [INT-TYPE / PART]_outpout.csv
| ├── Baseline
```

*MITIGATION-NAME* refers to the memory configuration name used to mitigate interrupt interferences.

*[SC/MC]* should be set to tell if the measurements were done on a single core (SC) or multicore (MC) environment.

*[INT-TYPE/PART]* defines the interrupt type that was measured. Any values are possible, however, in our study we use the following: *INT, EXT, IPI, SC*. PART is used to define the measurements done on the partitions.

*Baseline* are the partition measurements done without any interrupt during the execution.

## Results
This folder contains the Excel sheet used to compile and study the measurements. This file also contains the macro (script) used to classify and rank the different memory configurations.

## RTOS_Benchmark
This folder contains the benchmark framework as well as examples for an applicative partition and a system call generator partition.

* Example_applicativePartition.c contains the code used for an ARINC-653 partition executing an empty benchmark routine.
* Example_SysCallPartition.c provides the code for a system call generator  that raises a system call every 0.5ms on the T2080 NXP platform.
* InterruptBench.h contains the API provided by the benchmark framework.
* OSAbstraction.h contains the RTOS abstraction used to generate and handle interrupts. This file must be updated for the platform the user targets.
* PMCDriver.h is a Performance Monitoring Counters driver used by the benchmarks suite. The file provides the required API and should be modified to adapt to the targeted platform.

## Scripts
This folder gathers the scripts we used to extract the raw data to CSV files and to display the extracted data.


## Additional Information
This study was conduced by the HEL laboratory at Polytechnique Montréal, QC, Canada in partnership with MANNARINO System & Software, QC, Canada.
