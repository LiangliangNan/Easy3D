# Publishing the C++ documentation

The [Documentation workflow](../.github/workflows/documentation.yml) builds the
existing `Doxyfile.in` and publishes its HTML to
<https://liangliangnan.github.io/Easy3D/>. This is the Easy3D repository's project
site.

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

GitHub discovers workflow files only under `.github/workflows`, so that small
file is the only addition outside `docs`. Generated HTML is uploaded as a Pages
artifact and is never committed to the repository.

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
