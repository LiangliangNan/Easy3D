#include "glutess.h"
#include "tess.h"
#include <stdio.h>
#include <stdlib.h>

/******************************************************************************/

struct Triangle_s {
    int v[3];
    struct Triangle_s *prev;
};

struct Vertex_s {
    double pt[3];
    int index;
    struct Vertex_s *prev;
};

struct TessContext_s {
    struct Triangle_s *latest_t;
    int n_tris;
    
    struct Vertex_s *v_prev;
    struct Vertex_s *v_prevprev;
    struct Vertex_s *latest_v;
    GLenum current_mode;
    int odd_even_strip;

    void (*vertex_cb)(struct Vertex_s *, struct TessContext_s *);
};

void skip_vertex(struct Vertex_s *v, struct TessContext_s *ctx);

/******************************************************************************/

struct TessContext_s *new_tess_context()
{
    struct TessContext_s *result = (struct TessContext_s *)malloc(sizeof (struct TessContext_s));
    result->latest_t = NULL;
    result->latest_v = NULL;
    result->n_tris = 0;
    result->v_prev = NULL;
    result->v_prevprev = NULL;
    result->v_prev = NULL;
    result->v_prev = NULL;
    result->vertex_cb = &skip_vertex;
    result->odd_even_strip = 0;
    return result;
}


void destroy_tess_context(struct TessContext_s *ctx)
{
    free(ctx);
}

struct Vertex_s *new_vertex(struct TessContext_s *ctx, double x, double y)
{
    struct Vertex_s *result = (struct Vertex_s *)malloc(sizeof(struct Vertex_s));
    result->prev = ctx->latest_v;
    result->pt[0] = x;
    result->pt[1] = y;
    result->pt[2] = 0;

    if (ctx->latest_v == NULL) {
        result->index = 0;
    } else {
        result->index = ctx->latest_v->index+1;
    }
    return ctx->latest_v = result;
}


struct Triangle_s *new_triangle(struct TessContext_s *ctx, int v1, int v2, int v3)
{
    struct Triangle_s *result = (struct Triangle_s *)malloc(sizeof(struct Triangle_s));
    result->prev = ctx->latest_t;
    result->v[0] = v1;
    result->v[1] = v2;
    result->v[2] = v3;
    ctx->n_tris++;
    return ctx->latest_t = result;
}

/******************************************************************************/

void skip_vertex(struct Vertex_s *v, struct TessContext_s *ctx) {};

void fan_vertex(struct Vertex_s *v, struct TessContext_s *ctx) {
    if (ctx->v_prevprev == NULL) {
        ctx->v_prevprev = v;
        return;
    }
    if (ctx->v_prev == NULL) {
        ctx->v_prev = v;
        return;
    }
    new_triangle(ctx, ctx->v_prevprev->index, ctx->v_prev->index, v->index);
    ctx->v_prev = v;
}

void strip_vertex(struct Vertex_s *v, struct TessContext_s *ctx)
{
    if (ctx->v_prev == NULL) {
        ctx->v_prev = v;
        return;
    }
    if (ctx->v_prevprev == NULL) {
        ctx->v_prevprev = v;
        return;
    }
    if (ctx->odd_even_strip) {
        new_triangle(ctx, ctx->v_prevprev->index, ctx->v_prev->index, v->index);
    } else {
        new_triangle(ctx, ctx->v_prev->index, ctx->v_prevprev->index, v->index);
    }
    ctx->odd_even_strip = !ctx->odd_even_strip;

    ctx->v_prev = ctx->v_prevprev;
    ctx->v_prevprev = v;
}

void triangle_vertex(struct Vertex_s *v, struct TessContext_s *ctx) {
    if (ctx->v_prevprev == NULL) {
        ctx->v_prevprev = v;
        return;
    }
    if (ctx->v_prev == NULL) {
        ctx->v_prev = v;
        return;
    }
    new_triangle(ctx, ctx->v_prevprev->index, ctx->v_prev->index, v->index);
    ctx->v_prev = ctx->v_prevprev = NULL;
}

void vertex(void *vertex_data, void *poly_data)
{
    struct Vertex_s *ptr = (struct Vertex_s *)vertex_data;
    struct TessContext_s *ctx = (struct TessContext_s *)poly_data;
    ctx->vertex_cb(ptr, ctx);
}

void begin(GLenum which, void *poly_data)
{
    struct TessContext_s *ctx = (struct TessContext_s *)poly_data;
    ctx->v_prev = ctx->v_prevprev = NULL;
    ctx->odd_even_strip = 0;
    switch (which) {
    case GL_TRIANGLES: ctx->vertex_cb = &triangle_vertex; break;
    case GL_TRIANGLE_STRIP: ctx->vertex_cb = &strip_vertex; break;
    case GL_TRIANGLE_FAN: ctx->vertex_cb = &fan_vertex; break;
    default:
        fprintf(stderr, "ERROR, can't handle %d\n", (int)which);
        ctx->vertex_cb = &skip_vertex;
    }
}

void combine(const GLdouble newVertex[3],
             const void *neighborVertex_s[4],
             const GLfloat neighborWeight[4], void **outData, void *polyData)
{
    struct TessContext_s *ctx = (struct TessContext_s *)polyData;
    struct Vertex_s *result = new_vertex(ctx, newVertex[0], newVertex[1]);
    *outData = result;
}


void write_output(struct TessContext_s *ctx, double **coordinates_out, int **tris_out, int *vc, int *tc)
{
    struct Vertex_s *prev_v;
    struct Triangle_s *prev_t;
    int n_verts = 1 + ctx->latest_v->index;
    int n_tris_copy = ctx->n_tris;

    *vc = n_verts; 

    *tc = ctx->n_tris;
    *coordinates_out = malloc(n_verts * sizeof(double) * 2);
    *tris_out = (ctx->n_tris ? malloc(ctx->n_tris * sizeof(int) * 3) : NULL);

    while (ctx->latest_v) {
        (*coordinates_out)[2*ctx->latest_v->index]   = ctx->latest_v->pt[0];
        (*coordinates_out)[2*ctx->latest_v->index+1] = ctx->latest_v->pt[1];
        prev_v = ctx->latest_v->prev;
        free(ctx->latest_v);
        ctx->latest_v = prev_v;
    }

    while (ctx->latest_t) {
        (*tris_out)[3*(n_tris_copy-1)]   = ctx->latest_t->v[0];
        (*tris_out)[3*(n_tris_copy-1)+1] = ctx->latest_t->v[1];
        (*tris_out)[3*(n_tris_copy-1)+2] = ctx->latest_t->v[2];
        prev_t = ctx->latest_t->prev;
        free(ctx->latest_t);
        ctx->latest_t = prev_t;
        n_tris_copy--;
    }
}


void tessellate
    (double **verts,
     int *nverts,
     int **tris,
     int *ntris,
     const double **contoursbegin, 
     const double **contoursend)
{
    const double *contourbegin, *contourend;
    struct Vertex_s *current_vertex;
    GLUtesselator *tess;
    struct TessContext_s *ctx;

    tess = NewTess();
    ctx = new_tess_context();
    TessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
    TessCallback(tess, GLU_TESS_VERTEX_DATA,  (GLvoid (*) ()) &vertex);
    TessCallback(tess, GLU_TESS_BEGIN_DATA,   (GLvoid (*) ()) &begin);
    TessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid (*) ()) &combine);

    TessBeginPolygon(tess, ctx);
    do {
        contourbegin = *contoursbegin++;
        contourend = *contoursbegin;
        TessBeginContour(tess);
        while (contourbegin != contourend) {
            current_vertex = new_vertex(ctx, contourbegin[0], contourbegin[1]);
            contourbegin += 2;
            TessVertex(tess, current_vertex->pt, current_vertex);
        }
        TessEndContour(tess);
    } while (contoursbegin != (contoursend - 1));
    TessEndPolygon(tess);

    write_output(ctx, verts, tris, nverts, ntris);
    destroy_tess_context(ctx);
    DeleteTess(tess);
}
