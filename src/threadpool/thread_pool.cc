#include "thread_pool.h"

static void thread_pool_exit_handler(void *data);
static void *thread_pool_cycle(void *data);
static int_t thread_pool_init_default(thread_pool_t *tpp, char *name);

static uint_t thread_pool_task_id;

static int debug = 0;

thread_pool_t *thread_pool_init() {
  int err;
  pthread_t tid;
  uint_t n;
  pthread_attr_t attr;
  thread_pool_t *tp = NULL;

  // TODO: static_cast
  tp = (thread_pool_t *)calloc(1, sizeof(thread_pool_t));

  if (tp == NULL) {
    fprintf(stderr, "thread_pool_init: calloc failed!\n");
  }

  thread_pool_init_default(tp, NULL);

  thread_pool_queue_init(&tp->queue);

  if (thread_mutex_create(&tp->mtx) != T_OK) {
    free(tp);
    return NULL;
  }

  if (thread_cond_create(&tp->cond) != T_OK) {
    (void)thread_mutex_destroy(&tp->mtx);
    free(tp);
    return NULL;
  }

  err = pthread_attr_init(&attr);
  if (err) {
    fprintf(stderr, "pthread_attr_init() failed, reason: %s\n",
            strerror(errno));
    free(tp);
    return NULL;
  }

  err = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  if (err) {
    fprintf(stderr, "pthread_attr_setdetachstate() failed, reason: %s\n",
            strerror(errno));
    free(tp);
    return NULL;
  }

  for (n = 0; n < tp->threads; n++) {
    err = pthread_create(&tid, &attr, thread_pool_cycle, tp);
    if (err) {
      fprintf(stderr, "pthread_create() failed, reason: %s\n", strerror(errno));
      free(tp);
      return NULL;
    }
  }

  (void)pthread_attr_destroy(&attr);

  return tp;
}

void thread_pool_destroy(thread_pool_t *tp) {
  uint_t n;
  thread_task_t task;
  volatile uint_t lock;

  memset(&task, '\0', sizeof(thread_task_t));

  task.handler = thread_pool_exit_handler;
  task.ctx = (void *)&lock;

  for (n = 0; n < tp->threads; n++) {
    lock = 1;

    if (thread_task_post(tp, &task) != T_OK) {
      return;
    }

    while (lock) {
      sched_yield();
    }

    // task.event.active = 0;
  }

  (void)thread_cond_destroy(&tp->cond);
  (void)thread_mutex_destroy(&tp->mtx);

  free(tp);
}

static void thread_pool_exit_handler(void *data) {
  // TODO: static_cast
  uint_t *lock = (uint_t *)data;

  *lock = 0;

  pthread_exit(0);
}

thread_task_t *thread_task_alloc(size_t size) {
  thread_task_t *task;

  // TODO: static_cast
  task = (thread_task_t *)calloc(1, sizeof(thread_task_t) + size);
  if (task == NULL) {
    return NULL;
  }

  task->ctx = task + 1;

  return task;
}

void thread_task_free(thread_task_t *task) {
  if (task) {
    free(task);
    task = NULL;
  }
}

int_t thread_task_post(thread_pool_t *tp, thread_task_t *task) {
  if (thread_mutex_lock(&tp->mtx) != T_OK) {
    return T_ERROR;
  }

  if (tp->waiting >= tp->max_queue) {
    (void)thread_mutex_unlock(&tp->mtx);

    fprintf(stderr, "thread pool \"%s\" queue overflow: %ld tasks waiting\n",
            tp->name, tp->waiting);
    return T_ERROR;
  }

  // task->event.active = 1;

  task->id = thread_pool_task_id++;
  task->next = NULL;

  if (thread_cond_signal(&tp->cond) != T_OK) {
    (void)thread_mutex_unlock(&tp->mtx);
    return T_ERROR;
  }

  *tp->queue.last = task;
  tp->queue.last = &task->next;

  tp->waiting++;

  (void)thread_mutex_unlock(&tp->mtx);

  if (debug)
    fprintf(stderr, "task #%lu added to thread pool \"%s\"\n", task->id,
            tp->name);

  return T_OK;
}

static void *thread_pool_cycle(void *data) {
  // TODO: static_cast
  thread_pool_t *tp = (thread_pool_t *)data;

  // int err;
  thread_task_t *task;

  if (debug) fprintf(stderr, "thread in pool \"%s\" started\n", tp->name);

  for (;;) {
    if (thread_mutex_lock(&tp->mtx) != T_OK) {
      return NULL;
    }

    tp->waiting--;

    while (tp->queue.first == NULL) {
      if (thread_cond_wait(&tp->cond, &tp->mtx) != T_OK) {
        (void)thread_mutex_unlock(&tp->mtx);
        return NULL;
      }
    }

    task = tp->queue.first;
    tp->queue.first = task->next;

    if (tp->queue.first == NULL) {
      tp->queue.last = &tp->queue.first;
    }

    if (thread_mutex_unlock(&tp->mtx) != T_OK) {
      return NULL;
    }

    if (debug)
      fprintf(stderr, "run task #%lu in thread pool \"%s\"\n", task->id,
              tp->name);

    task->handler(task->ctx);

    if (debug)
      fprintf(stderr, "complete task #%lu in thread pool \"%s\"\n", task->id,
              tp->name);

    task->next = NULL;
    thread_task_free(task);

    // notify
  }
}

static int_t thread_pool_init_default(thread_pool_t *tpp, char *name) {
  if (tpp) {
    tpp->threads = DEFAULT_THREADS_NUM;
    tpp->max_queue = DEFAULT_QUEUE_NUM;

    tpp->name = strdup(name ? name : "default");
    if (debug)
      fprintf(stderr,
              "thread_pool_init, name: %s ,threads: %lu max_queue: %ld\n",
              tpp->name, tpp->threads, tpp->max_queue);

    return T_OK;
  }

  return T_ERROR;
}
