# Publishing the documentation

The [Documentation workflow](../.github/workflows/documentation.yml) builds the
existing `Doxyfile.in` and the Sphinx site in `python/docs`, publishing:

- C++ documentation: <https://liangliangnan.github.io/Easy3D/>
- Python documentation: <https://liangliangnan.github.io/Easy3D/python>
- Python tutorials: <https://liangliangnan.github.io/Easy3D/python/tutorials>

## One-time setup

1. In **LiangliangNan/Easy3D → Settings → Pages → Build and deployment**, select
   **GitHub Actions** as the source.
2. Commit and push these files to `main`. Every push to `main` builds and deploys
   the documentation. You can also run **Actions → Documentation → Run workflow**
   on `main`.
3. Check the workflow's `github-pages` deployment URL after it succeeds.

Pull requests targeting `main` build the documentation but do not deploy it.
The workflow uses the repository's built-in `GITHUB_TOKEN`; no personal access
token, cross-repository access, or `gh-pages` branch is needed. The default Pages
URL above assumes the account's Pages site uses `liangliangnan.github.io` without
a custom domain, and that the personal-page repository does not separately
publish content under `Easy3D/`.

GitHub discovers workflow files only under `.github/workflows`. The C++ and Python
sites are uploaded together as one Pages artifact, so deploying either does not
replace the other. Generated HTML is never committed to the repository.

## Build locally

Install CMake (3.18 or later) and Doxygen, then run from the repository root:

```sh
cmake -P docs/build.cmake
python3 -m http.server 8000 --directory build/doc/html
```

Open `http://localhost:8000/` to view the result. No C++ compilation, OpenGL, Qt,
or other Easy3D development dependencies are required. The script reads the
version from the root `CMakeLists.txt` and configures `Doxyfile.in` directly,
without running the root CMake project or modifying its version header.

To choose a different output location, pass
`-DPROJECT_BINARY_DIR=/absolute/path/to/output` before `-P`.

## Build the Python documentation and tutorials

With Python 3.12, run from the repository root:

```sh
python -m pip install -r python/docs/requirements.txt
python python/docs/build.py
python -m http.server 8000 --directory build/doc/html
```

View `/python/` for the Python documentation and `/python/tutorials/` for the
gallery. The builder reuses the Read the Docs theme, custom templates, tutorial
text and code, and checked-in thumbnails in `python/docs/source/_static`.
It builds in a temporary copy of the sources, checks Sphinx warnings, and places
the result under `build/doc/html/python` without changing the source tree.
Both documentation sites read their version from the root `CMakeLists.txt`.

Sphinx-Gallery publishes all tutorial scripts, including downloadable Python files
and notebooks, without executing them (`plot_gallery=False`). Several tutorials
open windows and wait for user input, so executing them would block CI. No compiled
Python bindings, display server, or GPU is required for this documentation build.
Runtime output and newly rendered screenshots are not captured; the existing
thumbnail images are used. This builds documentation, not a test of the bindings.
