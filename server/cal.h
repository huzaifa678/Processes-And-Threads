#ifndef CAL_H
#define CAL_H

#include <pthread.h>

#define MAX_TASKS 64
#define MAX_EXPR 128

typedef enum { DERIVATIVE, INTEGRAL } task_type_t;

typedef struct {
    task_type_t type;
    char expr[MAX_EXPR];
    double a, b; // integral bounds
} calc_task_t;

typedef struct {
    calc_task_t tasks[MAX_TASKS];
    int front, rear;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} task_queue_t;

typedef struct {
    const char *expr;
} expr_context_t;

// global queue
extern task_queue_t g_queue;

// queue functions
void task_queue_init(task_queue_t *q);
void enqueue_task(task_queue_t *q, calc_task_t *task);
int dequeue_task(task_queue_t *q, calc_task_t *task);

// math functions
double parse_expr(const char *expr, double x);
double derivative(double (*f)(double, void *), void *ctx, double x);
double integrate(double (*f)(double, void *), void *ctx, double a, double b, int steps);
double expr_func(double x, void *ctx);

// worker thread
void *worker_thread(void *arg);

#endif


