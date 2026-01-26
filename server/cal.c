#include "cal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// define the queue globally
task_queue_t g_queue;


// initialize the task queue
void task_queue_init(task_queue_t *q) {
    q->front = q->rear = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

// enqueue when the thread wants to add a task
void enqueue_task(task_queue_t *q, calc_task_t *task) {
    // lock the mutex before accessing the queue
    pthread_mutex_lock(&q->mutex);
    q->tasks[q->rear] = *task;   // add the task to the rear of the queue
    q->rear = (q->rear + 1) % MAX_TASKS;  // advance the rear index circularly
    pthread_cond_signal(&q->cond);     // signal one waiting thread that a new task is available
    pthread_mutex_unlock(&q->mutex); // finally unlock the mutex
}

// dequeue the task when the worker thread wants to get a task
int dequeue_task(task_queue_t *q, calc_task_t *task) {
    pthread_mutex_lock(&q->mutex); // lock the mutex before accessing the queue
    while (q->front == q->rear) {   // if the queue is empty, wait for a task to be available
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    *task = q->tasks[q->front]; // get the task from the front of the queue
    q->front = (q->front + 1) % MAX_TASKS;
    pthread_mutex_unlock(&q->mutex); // finally unlock the mutex so other threads can access the queue
    return 1;
}

// -----application logic functions-----

double derivative(double (*f)(double, void *), void *ctx, double x) {
    double h = 1e-5;
    return (f(x+h, ctx) - f(x-h, ctx)) / (2*h);
}

double integrate(double (*f)(double, void *), void *ctx, double a, double b, int steps) {
    double sum = 0;
    double dx = (b - a) / steps;
    for (int i = 0; i < steps; i++)
        sum += f(a + i*dx, ctx) * dx;
    return sum;
}

double expr_func(double x, void *ctx) {
    expr_context_t *e = (expr_context_t *)ctx;
    return parse_expr(e->expr, x);
}

double parse_expr(const char *expr, double x) {
    if (strncmp(expr, "x^", 2) == 0) {
        int n = atoi(expr+2);
        return pow(x, n);
    }
    return 0;
}

void *worker_thread(void *arg) {
    (void)arg;
    calc_task_t task;
    while (1) {
        dequeue_task(&g_queue, &task); // get a task from the global queue

        expr_context_t ctx = { .expr = task.expr }; // set up the expression context
        double result = 0; // initalize the result variable

        // performing the application logic based on the task type
        if (task.type == DERIVATIVE) {
            result = derivative(expr_func, &ctx, 1.0);
            printf("DERIVATIVE %s at x=1 = %.5f\n", task.expr, result);
        } else if (task.type == INTEGRAL) {
            result = integrate(expr_func, &ctx, task.a, task.b, 1000);
            printf("INTEGRAL %s from %.2f to %.2f = %.5f\n",
                   task.expr, task.a, task.b, result);
        }
    }
    return NULL;
}
