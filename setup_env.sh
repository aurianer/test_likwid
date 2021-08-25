#!/bin/bash

module load gcc/11
spack load likwid@5.2.0%gcc@10.2.0 arch=linux-centos8-skylake_avx512
