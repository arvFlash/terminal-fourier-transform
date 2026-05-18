#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#define PI 3.14159265358979323846

typedef struct {
    int start;
    int end;
    int sizex;
    int sizey;
    double scale;
    bool **array;
    double *g_table;
    int nsteps;
    double step;
} Task;

double g(double a)
{
    return cos(a * PI * 8) + cos(a * PI * 7) + sin(a * PI * 4) + cos(a * PI * 0.1) + cos(a * PI * 7.5) + cos(a * PI * 32);
    // return cos(a * PI * 2 * 6) + cos(a * PI * 2 * 7) + cos(a * PI * 2 * 4) + cos(a * PI * 2 * 9);
    // return cos(a * PI * 8);
    // return cos(a * PI * 8) + sin(a * PI * 16);
    // return sin(a * PI * 8) + cos(a * PI * 8);
    // return 0;
}
double eval(double xo, double scale, double *g_table, int nsteps, double step)
{
    double x = xo / scale / 2;
    double real = 0;
    double imag = 0;
    double t2 = nsteps * step;
    for(double t = 0; t < t2; t+= step) {
        int index = (int)(t / step);
        double value = g_table[index];
        real += value * cos(-2 * PI * x * t) * step;
        imag += -value * sin(-2 * PI * x * t) * step;
    }
    return sqrt(real * real + imag * imag) / 1;
    // return real + imag;
    // return atan2(imag, real)+10;
    // return x;
    // return cos(x);
    // return g(x);
}

void* worker(void* arg)
{
    Task* task = (Task*)arg;
    double ratio = 1.1;
    double sub = 10;
    for(int x = task->start; x < task->end; x++) {
        double res = 0;
        for(double i = 0; i < sub; i++) {
            // double res2 = eval(x + ((i + 0.5) / sub), task->scale, task->g_table, task->nsteps, task->step);
            res += eval(x + ((i * 0.5) / sub), task->scale, task->g_table, task->nsteps, task->step) / sub;
            // double res2 = eval(x + i / sub, task->scale, task->g_table, task->nsteps, task->step);
            // double res2 = x + (1 / sub);
            // printf("%lf\n", res2);
            // if(res2 > res){res = res2;}
        }
        for(int y = 0; y < task->sizey / ratio; y++) {
            if(((double)task->sizey / ratio) - y < res * task->scale) {
                task->array[x][y] = true;
            }
        }
        if(floor(x*0.5 / task->scale) == x*0.5 / task->scale) {
            for(int y = task->sizey / ratio; y < task->sizey; y++) {
                task->array[x][y] = true;
            }
        }
    }


    return NULL;
}

void draw(bool **array, int sizex, int sizey, double scale, int nthreads)
{

    double step = 0.001;
    double t2 = 1000;
    int nsteps = (int)(t2 / step);

    double *g_table = malloc(nsteps * sizeof(double));

    for(int i = 0; i < nsteps; i++) {
        double t = i * step;
        g_table[i] = g(t);
    }
    
    pthread_t threads[nthreads];
    Task tasks[nthreads];

    int chunk = sizex / nthreads;

    for(int i = 0; i < nthreads; i++) {
        tasks[i].start = i * chunk;
        tasks[i].end = (i == nthreads - 1) ? sizex : (i + 1) * chunk;

        tasks[i].sizex = sizex;
        tasks[i].sizey = sizey;
        tasks[i].scale = scale;
        tasks[i].array = array;
        tasks[i].g_table = g_table;
        tasks[i].nsteps = nsteps;
        tasks[i].step = step;

        pthread_create(&threads[i], NULL, worker, &tasks[i]);
    }

    for(int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    for(int y = 0; y < sizey; y++) {
        for(int x = 0; x < sizex; x++) {
            if(array[x][y]) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("\n");
    }
}


int main()
{
    int threads = 32;
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    double scale = 12;
    int sizex = w.ws_col;
    int sizey = w.ws_row;
    bool **array = malloc(sizeof(bool*) * sizex);
    for(int i = 0; i < sizex; i++) {
        array[i] = malloc(sizeof(bool) * sizey);
    }
    for(int x = 0; x < sizex; x++) {
        for(int y = 0; y < sizey; y++) {
            array[x][y] = false;
        }
    }
    draw(array, sizex, sizey, scale, threads);

    return 0;
}
