#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NPTS 10000

typedef struct {
    double x;
    double y;
} point;

int main() {
    int i;
    point pts[NPTS];
    srand(time(NULL));
    for (i = 0; i < NPTS; i++) {
        pts[i].x = (double) rand() / RAND_MAX * 100;
        pts[i].y = (double) rand() / RAND_MAX * 100;
    }
    FILE *fp = fopen("data.txt", "w");
    for (i = 0; i < NPTS; i++) {
        fprintf(fp, "%lf %lf\n", pts[i].x, pts[i].y);
    }
    fclose(fp);
    return 0;
}

