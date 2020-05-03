#include "tessellate.h"
#include <stdio.h>
#include <stdlib.h>

void run_example(const double vertices_array[],
                 const double *contours_array[],
                 int contours_size)
{
    double *coordinates_out;
    int *tris_out;
    int nverts, ntris, i;

    const double *p = vertices_array;
    /* const double **contours = contours_array; */

    tessellate(&coordinates_out, &nverts,
               &tris_out, &ntris,
               contours_array, contours_array + contours_size);

    for (i=0; i<2 * nverts; ++i) {
        fprintf(stdout, "%g ", coordinates_out[i]);
    }
    fprintf(stdout, "\n");
    for (i=0; i<3 * ntris; ++i) {
        fprintf(stdout, "%d ", tris_out[i]);
    }
    fprintf(stdout, "\n");
    free(coordinates_out);
    if (tris_out)
        free(tris_out);
}

int main()
{
    double a1[] = { 0, 0, 1, 5, 2, 0, -1, 3, 3, 3 };
    const double *c1[] = {a1, a1+10};
    int s1 = 2;
    run_example(a1, c1, 2);

    double a2[] = { 0, 0, 3, 0, 3, 3, 0, 3, 
                    1, 1, 2, 1, 2, 2, 1, 2 };
    const double *c2[] = {a2, a2+8, a2+16};
    int s2 = 3;
    run_example(a2, c2, s2);

    double a3[] = { 441, 0, 326, 0, 326, 889, 12, 889, 12, 992, 755, 992, 755, 889, 441, 889 };
    const double *c3[] = { a3, a3+16 };
    int s3 = 2;
    run_example(a3, c3, s3);

    return 0;
}
