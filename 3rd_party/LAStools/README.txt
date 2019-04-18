LAStools: award-winning software for efficient LiDAR processing (with LASzip)

You find all individual LAStools in the .\LAStools\bin directory. Start
them by double-clicking or run them in the DOS command line. The ArcGIS
toolbox can be found in .\LAStools\ArcGIS_toolbox and the QGIS toolbox
in .\LAStools\QGIS_toolbox. A few example DOS batch scripts can be found
in the .\LAStools\example_batch_scripts directory.

user group:      http://groups.google.com/group/lastools
company page:    http://rapidlasso.com
latest updates:  http://lastools.org
twitter feed:    http://twitter.com/LAStools
facebook page:   http://facebook.com/LAStools
linkedin group:  http://linkedin.com/groups?gid=4408378

open source tools:

* laszip.exe compresses the LAS files in a completely lossless manner
* lasinfo.exe prints out a quick overview of the contents of a LAS file
* lasindex.exe creates a spatial index LAX file for fast spatial queries
* las2las.exe extracts last returns, clips, subsamples, translates, etc ...
* lasmerge.exe merges several LAS or LAZ files into a single LAS or LAZ file
* txt2las.exe converts LIDAR data from ASCII text to binary LAS format
* las2txt.exe turns LAS into human-readable and easy-to-parse ASCII
* lasprecision.exe analyses the actual precision of the LIDAR points

closed source tools:

* lastool.exe is an old GUI for multiple LAStools (now each tool has its own GUI)

* lasground.exe extracts the bare-earth by classifying all ground points
* lasground_new.exe an improved version of lasground.exe for complex terrains
* lasoverlap.exe checks overlap & vertical/horizontal alignment of flight lines
* lascontrol.exe quality checks elevations for a list of control points 
* lasclassify.exe finds buildings and the vegetation above the ground
* lascolor.exe colors the LAS points based on ortho imagery in TIF format 
* lasgrid.exe grids onto min/max/avg/std elevation, intensity, or counter rasters
* lascanopy.exe computes many raster and plot metrics for forestry applications
* lasboundary.exe extracts a boundary polygon that encloses the points
* lasheight.exe computes for each point its height above the ground
* lastrack.exe classifies LiDAR point based on distance from a trajectory
* lasplanes.exe finds planar patches in terrestrial, mobile, (airborne?) scans 
* lasclip.exe clips points against building footprints / swath boundaries
* lastile.exe tiles huge amounts of LAS points into square tiles
* lassplit.exe splits points of LAS file(s) into flightlines or other criteria
* lassort.exe sorts points by gps_time, point_source, or into spatial proximity
* lasduplicate.exe removes duplicate points (with identical x and y, z optional) 
* lasthin.exe thins lowest / highest / random LAS points via a grid
* las2tin.exe triangulates the points of a LAS file into a TIN
* las2dem.exe rasters (via a TIN) into elevation/slope/intensity/rgb DEMs
* las2iso.exe extracts, optionally simplified, elevation contours
* lasview.exe visualizes a LAS file with a simple OpenGL viewer
* las2shp.exe turns binary LAS into ESRI's Shapefile format
* shp2las.exe turns an ESRI's Shapefile into binary LAS

BLAST extension

* blast2dem.exe rasters like las2dem but with streaming TINs for billions of points. 
* blast2iso.exe contours like las2iso but with streaming TINs for billions of points. 

For Windows all binaries are included. All open source tools can be compiled
from the source code. For MSVC6.0 there is a project file. For Linux and MacOS
the makefiles are included. Simply go into the root directory and run 'make':

unzip LAStools.zip
cd LAStools/
make

The compiled binary executables are or will be in the ./LAStools/bin directory.

---

Please read the "LICENSE.txt" file for information on the legal use and licensing
of LAStools. I would also really like it if you would send me an email and tell me
what you use LAStools for and what features and improvements you could need. 

(c) 2007-2015 martin.isenburg@rapidlasso.com - http://rapidlasso.com
