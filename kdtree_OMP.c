#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define DIM 2
#define MAX_POINTS 1000

typedef struct point {
    double x[DIM];
} point;

typedef struct kd_node {
    point p;
    struct kd_node *left, *right;
} kd_node;

double distance(point a, point b) {
    double sum = 0;
    for (int i = 0; i < DIM; i++) {
        double diff = a.x[i] - b.x[i];
        sum += diff * diff;
    }
    return sqrt(sum);
}

kd_node *build_kd_tree(point *points, int n, int depth) {
    if (n == 0) {
        return NULL;
    }
    if (n == 1) {
        kd_node *node = (kd_node*) malloc(sizeof(kd_node));
        node->p = points[0];
        node->left = NULL;
        node->right = NULL;
        return node;
    }
    int mid = n / 2;
    kd_node *node = (kd_node*) malloc(sizeof(kd_node));
    node->p = points[mid];

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            node->left = build_kd_tree(points, mid, depth+1);
        }
        #pragma omp section
        {
            node->right = build_kd_tree(points+mid+1, n-mid-1, depth+1);
        }
    }

    return node;
}


kd_node *find_nearest_neighbor(kd_node *node, point p, kd_node *best) {
    if (node == NULL) {
        return best;
    }
    double dist = distance(node->p, p);
    if (dist < distance(best->p, p)) {
        best = node;
    }
    int axis = 0;
    if (p.x[axis] < node->p.x[axis]) {
        #pragma omp task shared(best)
        {
            kd_node *left_best = find_nearest_neighbor(node->left, p, best);
            if (distance(left_best->p, p) < distance(best->p, p)) {
                #pragma omp critical
                {
                    best = left_best;
                }
            }
        }
        if (p.x[axis] + distance(best->p, p) > node->p.x[axis]) {
            #pragma omp task shared(best)
            {
                kd_node *right_best = find_nearest_neighbor(node->right, p, best);
                if (distance(right_best->p, p) < distance(best->p, p)) {
                    #pragma omp critical
                    {
                        best = right_best;
                    }
                }
            }
        }
    } else {
        #pragma omp task shared(best)
        {
            kd_node *right_best = find_nearest_neighbor(node->right, p, best);
            if (distance(right_best->p, p) < distance(best->p, p)) {
                #pragma omp critical
                {
                    best = right_best;
                }
            }
        }
        if (p.x[axis] - distance(best->p, p) < node->p.x[axis]) {
            #pragma omp task shared(best)
            {
                kd_node *left_best = find_nearest_neighbor(node->left, p, best);
                if (distance(left_best->p, p) < distance(best->p, p)) {
                    #pragma omp critical
                    {
                        best = left_best;
                    }
                }
            }
        }
    }
    #pragma omp taskwait
    return best;
}


int main() {
    point points[MAX_POINTS];
    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    int i = 0;
    double x, y;
    while (fscanf(file, "%lf %lf", &x, &y) == 2 && i < MAX_POINTS) {
        points[i].x[0] = x;
        points[i].x[1] = y;
        i++;
    }
    fclose(file);

    kd_node *root = build_kd_tree(points, i, 0);
    point query = {{95, 95}};
    kd_node *nearest = find_nearest_neighbor(root, query, root);
    printf("Nearest neighbor to (%f, %f) is (%f, %f)\n", query.x[0], query.x[1], nearest->p.x[0], nearest->p.x[1]);

    free(root->left);
    free(root->right);
    free(root);
    return 0;
}







