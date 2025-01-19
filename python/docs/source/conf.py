# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

# -- Path setup --------------------------------------------------------------

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.

import os
import sys
from sphinx_gallery.sorting import FileNameSortKey

sys.path.insert(0, os.path.abspath("../"))
sys.path.insert(0, os.path.abspath('../my_env/lib/python3.13/site-packages'))

# -- Project information -----------------------------------------------------

project = 'Easy3D Python Bindings'
copyright = '2025, Liangliang Nan'
author = 'Liangliang Nan'
release = '2.6.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

# Sphinx extensions
# Add any Sphinx extension module names here, as strings. They can be extensions coming
# with Sphinx (named 'sphinx.ext.*') or your custom ones.
extensions = [
    'sphinx.ext.duration',
    'sphinx.ext.doctest',
    "sphinx.ext.autodoc",
    'sphinx.ext.autosummary',
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx_gallery.gen_gallery",
    'myst_parser',  # Markdown support
]

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# html_theme = 'alabaster'          # Or your preferred theme
# html_theme = 'classic'
html_theme = 'sphinx_rtd_theme'

# Last updated format
html_last_updated_fmt = '%Y-%m-%d %H:%M:%S'

# Path to templates
templates_path = ['_templates']

# Sphinx-Gallery configuration
sphinx_gallery_conf = {
    "examples_dirs": ["../../tutorials"],       # Path to the tutorials directory
    "gallery_dirs": ["auto_tutorials"],         # Output directory for generated tutorials
    "within_subsection_order": FileNameSortKey, # Sort order for tutorials
    "download_all_examples": False,             # Option to provide a download link for all tutorials
    "filename_pattern": r".*\.py",              # Match all .py files
    "write_computation_times": False,           # Don't write computation times
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['_static']
