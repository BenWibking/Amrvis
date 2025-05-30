# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## About Amrvis

Amrvis is a visualization tool for AMReX (Adaptive Mesh Refinement) data, part of the AMReX ecosystem. It's a C++ application using X11/Motif for GUI that can visualize 1D, 2D, and 3D AMR datasets with features like volume rendering, profiling data analysis, and interactive plotting.

## Build System

The project uses GNU Make with AMReX's build system. Key configuration is in `GNUmakefile`:

**Build Commands:**
- `make` - Build 2D version (default)
- `make DIM=1` - Build 1D version  
- `make DIM=3` - Build 3D version
- `make DEBUG=TRUE` - Debug build
- `make clean` - Clean build artifacts

**Key Build Variables in GNUmakefile:**
- `DIM` - Spatial dimension (1, 2, or 3)
- `PRECISION` - FLOAT or DOUBLE
- `USE_VOLRENDER` - Enable volume rendering (3D only)
- `USE_PARALLELVOLRENDER` - Enable parallel volume rendering
- `USE_PROFPARSER` - Enable profiling data analysis
- `USE_ARRAYVIEW` - Enable ArrayView functionality
- `USE_MPI` - Enable MPI support
- `COMP` - Compiler (gnu, intel)

## Dependencies

- **AMReX**: Set `AMREX_HOME` (default: `../amrex`)
- **X11/Motif**: Required for GUI (OpenMotif on macOS via Homebrew)
- **Volpack**: Optional for 3D volume rendering (set `VOLPACKDIR`, default: `../volpack`)

## Architecture

**Main Application Classes:**
- `AmrVisTool.cpp` - Main entry point and X11/Motif application setup
- `PltApp` - Core application for plotting AMR data  
- `ProfApp` - Application for profiling data (when `USE_PROFPARSER=TRUE`)
- `AmrPicture` - Handles AMR data visualization
- `Dataset` - Data management and I/O
- `Palette` - Color mapping functionality
- `ExpressionParser/ExpressionDialog` - Derived variable expressions

**Key Components:**
- GUI uses X11/Motif widgets for cross-platform compatibility
- Supports batch mode operations for automated processing
- Modular design allows 1D/2D/3D builds with different feature sets
- Volume rendering integration with Volpack library for 3D visualization

**Configuration:**
- Runtime config via `amrvis.defaults` file in home directory
- Default palette via `Palette` file (can be hidden as `.Palette`)
- Expression parsing for derived variables on datasets

The codebase is designed to handle large-scale AMR simulation data with interactive visualization capabilities across different dimensionalities.