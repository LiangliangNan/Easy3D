"""Build the Python site in isolation, without writing generated files to source."""

from pathlib import Path
import shutil
import subprocess
import sys
import tempfile


root = Path(__file__).resolve().parents[2]
build = root / "build"
output = build / "doc" / "html" / "python"
build.mkdir(exist_ok=True)

# Preserve relative paths used by conf.py, index.rst, and Sphinx-Gallery.
# Build into a fresh output tree too, so removed tutorials cannot be published.
with tempfile.TemporaryDirectory(prefix="python-docs-", dir=build) as temporary:
    python = Path(temporary) / "python"
    source = python / "docs" / "source"
    shutil.copytree(root / "python" / "docs" / "source", source)
    shutil.copytree(root / "python" / "tutorials", python / "tutorials")
    shutil.copy2(root / "python" / "README.md", python / "README.md")
    # conf.py reads the project version from the root CMake configuration.
    shutil.copy2(root / "CMakeLists.txt", Path(temporary) / "CMakeLists.txt")
    html = Path(temporary) / "html"
    subprocess.run(
        [sys.executable, "-m", "sphinx", "-b", "html", "-W", "--keep-going",
         str(source), str(html)],
        cwd=source.parent,
        check=True,
    )
    if output.exists():
        shutil.rmtree(output)
    shutil.copytree(html, output)

for page in ["index.html", "tutorials/index.html"]:
    if not (output / page).is_file():
        raise RuntimeError(f"Missing generated page: {output / page}")
print(f"Python documentation: {output / 'index.html'}")
