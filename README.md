# Structured Inputs Trigger Gamma Bursts and Rapid Retrieval in Modular Neural Networks

This repository contains the simulation and analysis codes accompanying the manuscript **“Structured inputs trigger gamma bursts and rapid retrieval in modular neural networks,”** which is currently **under review**.

The repository provides the computational materials required to reproduce the main simulation and analysis procedures associated with the study of stimulus-driven activity and retrieval in modular neuronal networks.

## Repository Structure

The repository is organized into two main directories:

### `A Fixed target module`

This directory contains the simulation and analysis files for the **fixed-target condition**, in which the first network module remains the target throughout the simulation.

It includes:

* the C++ simulation code;
* the Python/Jupyter analysis notebook;
* network adjacency-list and module-size input files; and
* example simulation outputs for testing the analysis pipeline.

This condition is used to examine network activity and target-module saliency when the identity of the target module is fixed.

### `B Dynamic target module`

This directory contains the corresponding simulation and analysis files for the **dynamic-target condition**, in which the target module is randomly selected for each successive stimulation window.

It includes:

* the C++ simulation code;
* the Python/Jupyter analysis notebook;
* the modular network input files; and
* example simulation outputs.

This condition is used to examine stimulus-dependent burst detection, module identification, and post-stimulus persistence when the target changes over time.

## Reproducibility

The provided C++ codes implement the network simulations, while the accompanying Jupyter notebooks perform the subsequent analysis and visualization.

Example output files are included in each directory so that the analysis workflow can be inspected and reproduced without first running the simulations. The supplied network files define the network topologies used by the corresponding simulations.

The repository is intended as a **compact computational supplement to the manuscript**, providing representative code, input files, and example outputs rather than the complete ensemble of simulation data used to generate all manuscript results.

For details of the model, simulation protocol, and scientific results, please refer to the accompanying manuscript.
