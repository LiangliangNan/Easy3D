# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

import os
import sys
from sphinx_gallery.sorting import FileNameSortKey

# Add the root directory of the project to sys.path
sys.path.insert(0, os.path.abspath("../"))
sys.path.insert(0, os.path.abspath('../my_env/lib/python3.13/site-packages'))

project = 'Easy3D Python Tutorials'
copyright = '2025, Liangliang Nan'
author = 'Liangliang Nan'
release = '2.6.1'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration


# Sphinx extensions
extensions = [
    "sphinx.ext.autodoc",
    "sphinx.ext.napoleon",
    "sphinx.ext.viewcode",
    "sphinx_gallery.gen_gallery",
]

# Paths for templates and static files
templates_path = ["_templates"]


# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'alabaster'
# html_theme = 'classic'  # Or your preferred theme

# Sphinx-Gallery configuration
sphinx_gallery_conf = {
    "examples_dirs": ["../../tutorials"],  # Path to your tutorials directory
    "gallery_dirs": ["auto_tutorials"],  # Output directory for generated tutorials
    "within_subsection_order": FileNameSortKey,
    "download_all_examples": False,  # Option to provide a download link for all tutorials
    "filename_pattern": r".*\.py",  # Match all .py files
    "backreferences_dir": "generated/backreferences",  # Optional: Cross-link generated docs
}

# Autodoc settings (optional)
autodoc_default_options = {
    "members": True,
    "undoc-members": True,
    "private-members": False,
    "show-inheritance": True,
}
