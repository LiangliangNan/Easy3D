---
title: 'Easy3D: a lightweight, easy-to-use, and efficient C++ library for processing and rendering 3D data'
tags:
  - C++
  - data structure
  - geometry processing
  - rendering
authors:
  - name: Liangliang Nan
    orcid: 0000-0002-5629-9975
    affiliation: 1
affiliations:
 - name: 3D Geoinformation Research Group, Delft University of Technology
   index: 1
date: 2 December 2020
bibliography: paper.bib
---

# Summary

The amount of 3D data has been explosively increasing in the past decades, driven by the fast development of data acquisition techniques (e.g., laser scanning, photogrammetry, and 3D computer vision) and real-world applications (e.g., urban planning, autonomous driving, surveying, and entertainment). The processing and application of such data require specialized algorithms and software systems, for which great tools and algorithm libraries are available. For example, MeshLab [@meshlab:2008] and CloudCompare [@cloudcompare:2016]) provide easy-to-use GUIs (Graphical User Interface) for processing meshes and point clouds, respectively. Callable libraries, such as libigl [@libigl:2016], PMP [@pmp:2020], and CGAL [@cgal:2009], can be integrated into existing software systems. However, developing end-to-end 3D data processing software typically requires integrating different functionalities such as data structures (for representing 3D data) and rendering (for visualizing the data and the derived information). Thus, the development process has a high demand for the expertise of a developer in all the related aspects for 3D data processing, which hinders the fast development of advanced data processing algorithms. Figure \autoref{fig:overview} shows an overview of the rendering functionalities provided by *Easy3D*. 

![An overview of the rendering functionalities of Easy3D. \label{fig:overview}](paper.jpg)

*Easy3D* is an open-source library for 3D modeling, geometry processing, and rendering. It is implemented in C++ and designed with an emphasis on simplicity and efficiency. The core features of *Easy3D* include:

- Efficient data structures for representing and managing 3D models (i.e., point clouds, meshes, and graphs). Easy to add/access arbitrary types of per-element properties. Non-manifoldness is automatically resolved when loading models from files.

- A set of widely used algorithms, e.g., point cloud normal estimation/re-orientation, Poisson Surface Reconstruction, RANSAC, mesh simplification, subdivision, smoothing, parameterization, and remeshing.

- A bunch of rendering techniques, e.g., point/line imposters, ambient occlusion (SSAO), hard shadow (shadow maps), soft shadow (PCSS), eye-dome lighting (for rendering point clouds without normal information), and transparency (average color blending, dual depth peeling).

- High-level encapsulation of OpenGL and GLSL for convenient and efficient rendering (based on modern and faster programmable-shader-style rendering, i.e., no fixed function calls). Client code does not need to touch the low-level APIs of OpenGL.

- Step-by-step tutorials to get acquainted with the data structures, rendering techniques, and algorithms for 3D modeling and geometry processing.

- A viewer that can be used directly to visualize 3D scenes in various formats, which can also be easily extended.

- A handy tool *Mapple* created out of the *Easy3D* library for rendering and processing 3D data.

*Easy3D* was designed with an intention for research and educational purposes, but it can also a good starting point for developing sophisticated 3D applications. *Easy3D* has been used by graduate students in courses and thesis projects and has enabled several scientific publications [@adtree:2019; @adqsm:2020].

# References
