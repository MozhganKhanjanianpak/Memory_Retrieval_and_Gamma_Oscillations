# Fixed-Target Network Simulation and Analysis

This folder contains the simulation and analysis code for the neuronal network model with a **fixed target module**.

In this setup, the first network module is selected as the target module, and the external input is distributed between the target module and the remaining modules according to the input concentration parameter \(F\). The target module remains fixed throughout a simulation.

The folder provides the complete workflow from a single C++ simulation run to Python-based analysis and visualization.

---

## Contents

- `simulation.cpp`  
  C++ source code for running a single simulation of the neuronal network.

- `analysis.ipynb`  
  Jupyter notebook for analyzing and visualizing the simulation output.

- `ModularAdjList.txt`  
  Adjacency-list representation of the modular network used as an example input.

- `NonModularAdjList.txt`  
  Adjacency-list representation of the corresponding non-modular network.

- `ActivityInTime.txt`  
  Example simulation output containing the activity of the eight network modules as a function of time.

- `NumberOfSynExtInTime.txt`  
  Example simulation output containing synaptic and external activity for the target and non-target modules.

---

## Model and Simulation

The network consists of \(N=2000\) neurons organized into eight modules. The first module is designated as the target module.

The external activation probability is distributed according to the input concentration parameter \(F\). For a given value of \(F\), the target module receives a fraction of the external input proportional to \(F\), while the remaining input is distributed among the other modules.

The accompanying C++ code performs a single simulation realization and records:

1. the activity of each network module over time;
2. the number of externally activated neurons in the target and non-target modules;
3. the number of synaptically activated neurons in the target and non-target modules.

The network structure can be changed by selecting either the modular or non-modular adjacency-list input in `simulation.cpp`.

---

## Running the Simulation

### 1. Select the Network Structure

In `simulation.cpp`, select the desired network input file:

- `ModularAdjList.txt` for the modular network;
- `NonModularAdjList.txt` for the non-modular network.

The corresponding network structure is loaded from the selected adjacency-list file.

### 2. Set the Simulation Parameters

The main simulation parameters can be modified directly in `simulation.cpp`, including:

- network size;
- synaptic weights;
- synaptic lifetimes;
- simulation duration;
- input concentration \(F\);
- external activation probability \(\eta\).

The default parameters in the provided code correspond to the example simulation.

### 3. Compile and Run

Using a C++ compiler such as `g++`:

```bash
g++ simulation.cpp -o simulation
