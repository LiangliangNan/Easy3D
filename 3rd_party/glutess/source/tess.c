/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 */
/*
** Author: Eric Veach, July 1994.
**
*/


#include <stddef.h>
#include <assert.h>
#include <setjmp.h>
#include "memalloc.h"
#include "tess.h"
#include "mesh.h"
#include "normal.h"
#include "sweep.h"
#include "tessmono.h"
#include "render.h"

#define TESS_DEFAULT_TOLERANCE 0.0
#define TESS_MESH		100112	/* void (*)(GLUmesh *mesh)	    */

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*ARGSUSED*/ static void noBegin( unsigned int type ) {}
/*ARGSUSED*/ static void noEdgeFlag( TESS_boolean boundaryEdge ) {}
/*ARGSUSED*/ static void noVertex( void *data ) {}
/*ARGSUSED*/ static void noEnd( void ) {}
/*ARGSUSED*/ static void noError( unsigned int errnum ) {}
/*ARGSUSED*/ static void noCombine( double coords[3], void *data[4],
				    double weight[4], void **dataOut ) {}
/*ARGSUSED*/ static void noMesh( GLUmesh *mesh ) {}


/*ARGSUSED*/ void __gl_noBeginData( unsigned int type,
					     void *polygonData ) {}
/*ARGSUSED*/ void __gl_noEdgeFlagData( TESS_boolean boundaryEdge,
				       void *polygonData ) {}
/*ARGSUSED*/ void __gl_noVertexData( void *data,
					      void *polygonData ) {}
/*ARGSUSED*/ void __gl_noEndData( void *polygonData ) {}
/*ARGSUSED*/ void __gl_noErrorData( unsigned int errnum,
					     void *polygonData ) {}
/*ARGSUSED*/ void __gl_noCombineData( double coords[3],
					       void *data[4],
					       double weight[4],
					       void **outData,
					       void *polygonData ) {}

/* Half-edges are allocated in pairs (see mesh.c) */
typedef struct { GLUhalfEdge e, eSym; } EdgePair;

#undef	MAX
#define MAX(a,b)	((a) > (b) ? (a) : (b))
#define MAX_FAST_ALLOC	(MAX(sizeof(EdgePair), \
                         MAX(sizeof(GLUvertex),sizeof(GLUface))))


GLUtesselator *
NewTess( void )
{
  GLUtesselator *tess;

  /* Only initialize fields which can be changed by the api.  Other fields
   * are initialized where they are used.
   */

  if (memInit( MAX_FAST_ALLOC ) == 0) {
     return 0;			/* out of memory */
  }
  tess = (GLUtesselator *)memAlloc( sizeof( GLUtesselator ));
  if (tess == NULL) {
     return 0;			/* out of memory */
  }

  tess->state = T_DORMANT;

  tess->normal[0] = 0;
  tess->normal[1] = 0;
  tess->normal[2] = 0;

  tess->relTolerance = TESS_DEFAULT_TOLERANCE;
  tess->windingRule = TESS_WINDING_ODD;
  tess->flagBoundary = FALSE;
  tess->boundaryOnly = FALSE;

  tess->callBegin = &noBegin;
  tess->callEdgeFlag = &noEdgeFlag;
  tess->callVertex = &noVertex;
  tess->callEnd = &noEnd;

  tess->callError = &noError;
  tess->callCombine = &noCombine;
  tess->callMesh = &noMesh;

  tess->callBeginData= &__gl_noBeginData;
  tess->callEdgeFlagData= &__gl_noEdgeFlagData;
  tess->callVertexData= &__gl_noVertexData;
  tess->callEndData= &__gl_noEndData;
  tess->callErrorData= &__gl_noErrorData;
  tess->callCombineData= &__gl_noCombineData;

  tess->polygonData= NULL;

  return tess;
}

static void MakeDormant( GLUtesselator *tess )
{
  /* Return the tessellator to its original dormant state. */

  if( tess->mesh != NULL ) {
    __gl_meshDeleteMesh( tess->mesh );
  }
  tess->state = T_DORMANT;
  tess->lastEdge = NULL;
  tess->mesh = NULL;
}

#define RequireState( tess, s )   if( tess->state != s ) GotoState(tess,s)

static void GotoState( GLUtesselator *tess, enum TessState newState )
{
  while( tess->state != newState ) {
    /* We change the current state one level at a time, to get to
     * the desired state.
     */
    if( tess->state < newState ) {
      switch( tess->state ) {
      case T_DORMANT:
	CALL_ERROR_OR_ERROR_DATA( TESS_MISSING_BEGIN_POLYGON );
	TessBeginPolygon( tess, NULL );
	break;
      case T_IN_POLYGON:
	CALL_ERROR_OR_ERROR_DATA( TESS_MISSING_BEGIN_CONTOUR );
	TessBeginContour( tess );
	break;
      default:
	 ;
      }
    } else {
      switch( tess->state ) {
      case T_IN_CONTOUR:
	CALL_ERROR_OR_ERROR_DATA( TESS_MISSING_END_CONTOUR );
	TessEndContour( tess );
	break;
      case T_IN_POLYGON:
	CALL_ERROR_OR_ERROR_DATA( TESS_MISSING_END_POLYGON );
	/* TessEndPolygon( tess ) is too much work! */
	MakeDormant( tess );
	break;
      default:
	 ;
      }
    }
  }
}


void
DeleteTess( GLUtesselator *tess )
{
  RequireState( tess, T_DORMANT );
  memFree( tess );
}


void
TessProperty( GLUtesselator *tess, unsigned int which, double value )
{
  unsigned int windingRule;

  switch( which ) {
  case TESS_TOLERANCE:
    if( value < 0.0 || value > 1.0 ) break;
    tess->relTolerance = value;
    return;

  case TESS_WINDING_RULE:
    windingRule = (unsigned int) value;
    if( windingRule != value ) break;	/* not an integer */

    switch( windingRule ) {
    case TESS_WINDING_ODD:
    case TESS_WINDING_NONZERO:
    case TESS_WINDING_POSITIVE:
    case TESS_WINDING_NEGATIVE:
    case TESS_WINDING_ABS_GEQ_TWO:
      tess->windingRule = windingRule;
      return;
    default:
      break;
    }

  case TESS_BOUNDARY_ONLY:
    tess->boundaryOnly = (value != 0);
    return;

  default:
    CALL_ERROR_OR_ERROR_DATA( INVALID_ENUM );
    return;
  }
  CALL_ERROR_OR_ERROR_DATA( INVALID_VALUE );
}

/* Returns tessellator property */
void
gluGetTessProperty( GLUtesselator *tess, unsigned int which, double *value )
{
   switch (which) {
   case TESS_TOLERANCE:
      /* tolerance should be in range [0..1] */
      assert(0.0 <= tess->relTolerance && tess->relTolerance <= 1.0);
      *value= tess->relTolerance;
      break;
   case TESS_WINDING_RULE:
      assert(tess->windingRule == TESS_WINDING_ODD ||
	     tess->windingRule == TESS_WINDING_NONZERO ||
	     tess->windingRule == TESS_WINDING_POSITIVE ||
	     tess->windingRule == TESS_WINDING_NEGATIVE ||
	     tess->windingRule == TESS_WINDING_ABS_GEQ_TWO);
      *value= tess->windingRule;
      break;
   case TESS_BOUNDARY_ONLY:
      assert(tess->boundaryOnly == TRUE || tess->boundaryOnly == FALSE);
      *value= tess->boundaryOnly;
      break;
   default:
      *value= 0.0;
      CALL_ERROR_OR_ERROR_DATA( INVALID_ENUM );
      break;
   }
} /* gluGetTessProperty() */

void
TessNormal( GLUtesselator *tess, double x, double y, double z )
{
  tess->normal[0] = x;
  tess->normal[1] = y;
  tess->normal[2] = z;
}

void
TessCallback( GLUtesselator *tess, unsigned int which, _GLUfuncptr fn)
{
  switch( which ) {
  case TESS_BEGIN:
    tess->callBegin = (fn == NULL) ? &noBegin : (void (*)(unsigned int)) fn;
    return;
  case TESS_BEGIN_DATA:
    tess->callBeginData = (fn == NULL) ?
	&__gl_noBeginData : (void (*)(unsigned int, void *)) fn;
    return;
  case TESS_EDGE_FLAG:
    tess->callEdgeFlag = (fn == NULL) ? &noEdgeFlag :
					(void (*)(TESS_boolean)) fn;
    /* If the client wants boundary edges to be flagged,
     * we render everything as separate triangles (no strips or fans).
     */
    tess->flagBoundary = (fn != NULL);
    return;
  case TESS_EDGE_FLAG_DATA:
    tess->callEdgeFlagData= (fn == NULL) ?
	&__gl_noEdgeFlagData : (void (*)(TESS_boolean, void *)) fn;
    /* If the client wants boundary edges to be flagged,
     * we render everything as separate triangles (no strips or fans).
     */
    tess->flagBoundary = (fn != NULL);
    return;
  case TESS_VERTEX:
    tess->callVertex = (fn == NULL) ? &noVertex :
				      (void (*)(void *)) fn;
    return;
  case TESS_VERTEX_DATA:
    tess->callVertexData = (fn == NULL) ?
	&__gl_noVertexData : (void (*)(void *, void *)) fn;
    return;
  case TESS_END:
    tess->callEnd = (fn == NULL) ? &noEnd : (void (*)(void)) fn;
    return;
  case TESS_END_DATA:
    tess->callEndData = (fn == NULL) ? &__gl_noEndData :
				       (void (*)(void *)) fn;
    return;
  case TESS_ERROR:
    tess->callError = (fn == NULL) ? &noError : (void (*)(unsigned int)) fn;
    return;
  case TESS_ERROR_DATA:
    tess->callErrorData = (fn == NULL) ?
	&__gl_noErrorData : (void (*)(unsigned int, void *)) fn;
    return;
  case TESS_COMBINE:
    tess->callCombine = (fn == NULL) ? &noCombine :
	(void (*)(double [3],void *[4], double[4], void ** )) fn;
    return;
  case TESS_COMBINE_DATA:
    tess->callCombineData = (fn == NULL) ? &__gl_noCombineData :
					   (void (*)(double [3],
						     void *[4],
						     double [4],
						     void **,
						     void *)) fn;
    return;
  case TESS_MESH:
    tess->callMesh = (fn == NULL) ? &noMesh : (void (*)(GLUmesh *)) fn;
    return;
  default:
    CALL_ERROR_OR_ERROR_DATA( INVALID_ENUM );
    return;
  }
}

static int AddVertex( GLUtesselator *tess, double coords[3], void *data )
{
  GLUhalfEdge *e;

  e = tess->lastEdge;
  if( e == NULL ) {
    /* Make a self-loop (one vertex, one edge). */

    e = __gl_meshMakeEdge( tess->mesh );
    if (e == NULL) return 0;
    if ( !__gl_meshSplice( e, e->Sym ) ) return 0;
  } else {
    /* Create a new vertex and edge which immediately follow e
     * in the ordering around the left face.
     */
    if (__gl_meshSplitEdge( e ) == NULL) return 0;
    e = e->Lnext;
  }

  /* The new vertex is now e->Org. */
  e->Org->data = data;
  e->Org->coords[0] = coords[0];
  e->Org->coords[1] = coords[1];
  e->Org->coords[2] = coords[2];

  /* The winding of an edge says how the winding number changes as we
   * cross from the edge''s right face to its left face.  We add the
   * vertices in such an order that a CCW contour will add +1 to
   * the winding number of the region inside the contour.
   */
  e->winding = 1;
  e->Sym->winding = -1;

  tess->lastEdge = e;

  return 1;
}


static void CacheVertex( GLUtesselator *tess, double coords[3], void *data )
{
  CachedVertex *v = &tess->cache[tess->cacheCount];

  v->data = data;
  v->coords[0] = coords[0];
  v->coords[1] = coords[1];
  v->coords[2] = coords[2];
  ++tess->cacheCount;
}


static int EmptyCache( GLUtesselator *tess )
{
  CachedVertex *v = tess->cache;
  CachedVertex *vLast;

  tess->mesh = __gl_meshNewMesh();
  if (tess->mesh == NULL) return 0;

  for( vLast = v + tess->cacheCount; v < vLast; ++v ) {
    if ( !AddVertex( tess, v->coords, v->data ) ) return 0;
  }
  tess->cacheCount = 0;
  tess->emptyCache = FALSE;

  return 1;
}


void
TessVertex( GLUtesselator *tess, double coords[3], void *data )
{
  int i, tooLarge = FALSE;
  double x, clamped[3];

  RequireState( tess, T_IN_CONTOUR );

  if( tess->emptyCache ) {
    if ( !EmptyCache( tess ) ) {
       CALL_ERROR_OR_ERROR_DATA( OUT_OF_MEMORY );
       return;
    }
    tess->lastEdge = NULL;
  }
  for( i = 0; i < 3; ++i ) {
    x = coords[i];
    if( x < - TESS_MAX_COORD ) {
      x = - TESS_MAX_COORD;
      tooLarge = TRUE;
    }
    if( x > TESS_MAX_COORD ) {
      x = TESS_MAX_COORD;
      tooLarge = TRUE;
    }
    clamped[i] = x;
  }
  if( tooLarge ) {
    CALL_ERROR_OR_ERROR_DATA( TESS_COORD_TOO_LARGE );
  }

  if( tess->mesh == NULL ) {
    if( tess->cacheCount < TESS_MAX_CACHE ) {
      CacheVertex( tess, clamped, data );
      return;
    }
    if ( !EmptyCache( tess ) ) {
       CALL_ERROR_OR_ERROR_DATA( OUT_OF_MEMORY );
       return;
    }
  }
  if ( !AddVertex( tess, clamped, data ) ) {
       CALL_ERROR_OR_ERROR_DATA( OUT_OF_MEMORY );
  }
}


void
TessBeginPolygon( GLUtesselator *tess, void *data )
{
  RequireState( tess, T_DORMANT );

  tess->state = T_IN_POLYGON;
  tess->cacheCount = 0;
  tess->emptyCache = FALSE;
  tess->mesh = NULL;

  tess->polygonData= data;
}


void
TessBeginContour( GLUtesselator *tess )
{
  RequireState( tess, T_IN_POLYGON );

  tess->state = T_IN_CONTOUR;
  tess->lastEdge = NULL;
  if( tess->cacheCount > 0 ) {
    /* Just set a flag so we don't get confused by empty contours
     * -- these can be generated accidentally with the obsolete
     * NextContour() interface.
     */
    tess->emptyCache = TRUE;
  }
}


void
TessEndContour( GLUtesselator *tess )
{
  RequireState( tess, T_IN_CONTOUR );
  tess->state = T_IN_POLYGON;
}

void
TessEndPolygon( GLUtesselator *tess )
{
  GLUmesh *mesh;

  if (setjmp(tess->env) != 0) { 
     /* come back here if out of memory */
     CALL_ERROR_OR_ERROR_DATA( OUT_OF_MEMORY );
     return;
  }

  RequireState( tess, T_IN_POLYGON );
  tess->state = T_DORMANT;

  if( tess->mesh == NULL ) {
    if( ! tess->flagBoundary && tess->callMesh == &noMesh ) {

      /* Try some special code to make the easy cases go quickly
       * (eg. convex polygons).  This code does NOT handle multiple contours,
       * intersections, edge flags, and of course it does not generate
       * an explicit mesh either.
       */
      if( __gl_renderCache( tess )) {
	tess->polygonData= NULL;
	return;
      }
    }
    if ( !EmptyCache( tess ) ) longjmp(tess->env,1); /* could've used a label*/
  }

  /* Determine the polygon normal and project vertices onto the plane
   * of the polygon.
   */
  __gl_projectPolygon( tess );

  /* __gl_computeInterior( tess ) computes the planar arrangement specified
   * by the given contours, and further subdivides this arrangement
   * into regions.  Each region is marked "inside" if it belongs
   * to the polygon, according to the rule given by tess->windingRule.
   * Each interior region is guaranteed be monotone.
   */
  if ( !__gl_computeInterior( tess ) ) {
     longjmp(tess->env,1);	/* could've used a label */
  }

  mesh = tess->mesh;
  if( ! tess->fatalError ) {
    int rc = 1;

    /* If the user wants only the boundary contours, we throw away all edges
     * except those which separate the interior from the exterior.
     * Otherwise we tessellate all the regions marked "inside".
     */
    if( tess->boundaryOnly ) {
      rc = __gl_meshSetWindingNumber( mesh, 1, TRUE );
    } else {
      rc = __gl_meshTessellateInterior( mesh );
    }
    if (rc == 0) longjmp(tess->env,1);	/* could've used a label */

    __gl_meshCheckMesh( mesh );

    if( tess->callBegin != &noBegin || tess->callEnd != &noEnd
       || tess->callVertex != &noVertex || tess->callEdgeFlag != &noEdgeFlag
       || tess->callBeginData != &__gl_noBeginData
       || tess->callEndData != &__gl_noEndData
       || tess->callVertexData != &__gl_noVertexData
       || tess->callEdgeFlagData != &__gl_noEdgeFlagData )
    {
      if( tess->boundaryOnly ) {
	__gl_renderBoundary( tess, mesh );  /* output boundary contours */
      } else {
	__gl_renderMesh( tess, mesh );	   /* output strips and fans */
      }
    }
    if( tess->callMesh != &noMesh ) {

      /* Throw away the exterior faces, so that all faces are interior.
       * This way the user doesn't have to check the "inside" flag,
       * and we don't need to even reveal its existence.  It also leaves
       * the freedom for an implementation to not generate the exterior
       * faces in the first place.
       */
      __gl_meshDiscardExterior( mesh );
      (*tess->callMesh)( mesh );		/* user wants the mesh itself */
      tess->mesh = NULL;
      tess->polygonData= NULL;
      return;
    }
  }
  __gl_meshDeleteMesh( mesh );
  tess->polygonData= NULL;
  tess->mesh = NULL;
}


/*XXXblythe unused function*/
#if 0
void
gluDeleteMesh( GLUmesh *mesh )
{
  __gl_meshDeleteMesh( mesh );
}
#endif



/*******************************************************/

/* Obsolete calls -- for backward compatibility */

void
gluBeginPolygon( GLUtesselator *tess )
{
  TessBeginPolygon( tess, NULL );
  TessBeginContour( tess );
}


/*ARGSUSED*/
void
gluNextContour( GLUtesselator *tess, unsigned int type )
{
  TessEndContour( tess );
  TessBeginContour( tess );
}


void
gluEndPolygon( GLUtesselator *tess )
{
  TessEndContour( tess );
  TessEndPolygon( tess );
}
