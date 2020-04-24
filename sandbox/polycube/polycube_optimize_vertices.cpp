#include "polycube.h"
#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/stop_watch.h>
#include <easy3d/util/logging.h>
#include "../liblbfgs/include/lbfgs.h"


#define ENCODE_EDGE_LENGTH
//#define ENCODE_LAPLACIAN

//#define ERROR_RELATIVE_TO_ORIGINAL_VERTICES



using namespace easy3d;
