#include	<stdlib.h>
#include	"shellcomp.h"
#include	"buffer.h"

static struct {
  t_buff	*l;
  t_buff	*r;
}		g_buff;

#include	<stdio.h>
#define	SEP "%ld ===================================\n"
void
buff_dumper() {
  t_buff *a = g_buff.l;
  while (a) {
    fprintf(stdout, SEP, a->count);
    fflush(stdout);
    write(1, a->buff, a->count);
    write(1, "\n", 1);
    a = a->next;
  }
}

static int
init_buff(t_buff *obj) {
  obj->count = 0;
  if (!(obj->buff = malloc(BUFF_SIZE * sizeof(*(obj->buff)))))
    return (EXIT_FAILURE);
  obj->next = NULL;
  return (EXIT_SUCCESS);
}


static int
send_lines(t_buff *b, size_t count, int (*cb)(char *, size_t)) {
  if (!b)
    return (EXIT_SUCCESS);
  //write(1, b->buff + count, b->count - count);
  if (cb(b->buff + count, b->count - count) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  while ((b = b->next) != NULL) {
    if (cb(b->buff, b->count) == EXIT_FAILURE)
      return (EXIT_FAILURE);
  }
  return (EXIT_SUCCESS);
}

static int
buff_lines_cb(t_buff *b, size_t i, int (*cb)()) {
  //logger(b == NULL ? "YES" : "NO");
  if (send_lines(b, i, cb) == EXIT_FAILURE) {
  }
  return (EXIT_SUCCESS);
}

static size_t
set_curr(t_buff *b, size_t i) {
  if (!b) {
    logger("Y'A UN SOUCIS..\n");
    return (0);
  }
  if (++i >= b->count) {
    return (0);
  }
  return (i);
}

static int
buff_lines_each(t_buff *b, size_t i, int (*cb)(), struct winsize *ws) {
  size_t	current, nb, gn;
  t_buff	*s = b;

  nb = 0;
  current = i;
  while (s != NULL) {
    if (nb >= ws->ws_col || s->buff[current] == '\n') {
      gn = set_curr(s, current);
      if (!(nb = 1 + buff_lines_each((!gn) ? s->next : s, gn, cb, ws))) {
	return (-1);
      }
      if (nb >= ws->ws_row) {
	buff_lines_cb(b, i, cb);
	return (-1);
      }
      return (nb);
    }
    if (!(current = set_curr(s, current)))
      s = s->next;
    ++nb;
  }
  return (1);
}

static int
buff_lines(
    t_buff *b,
    int (*cb)(char *, size_t),
    struct winsize *ws
    ) {
  buff_lines_each(b, 0, cb, ws);
  return (EXIT_SUCCESS);
}

int
buff_lines_r(int (*cb)(char *, size_t), struct winsize *ws) {
  return (buff_lines(g_buff.r, cb, ws));
}

int
buff_lines_l(int (*cb)(char *, size_t), struct winsize *ws) {
  return (buff_lines(g_buff.l, cb, ws));
}

int
buff_init(t_opts *opt) {
  (void)opt;
  if (!(g_buff.l = malloc(sizeof(*(g_buff.l)))))
    return (EXIT_FAILURE);
  if (!(g_buff.r = malloc(sizeof(*(g_buff.r))))) {
    free(g_buff.r);
    return (EXIT_FAILURE);
  }
  if (init_buff(g_buff.l) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  if (init_buff(g_buff.r) == EXIT_FAILURE)
    return (EXIT_FAILURE);
  return (EXIT_SUCCESS);
}

static t_buff *
add_buff(t_buff *obj) {
  t_buff	*new;

  if (!(new = malloc(sizeof(*new))))
    return (NULL);
  if (init_buff(new) == EXIT_FAILURE)
    return (NULL);
  obj->next = new;
  return (new);
}

int
push_buff(t_buff *buff, char *str, size_t count) {
  size_t	i = 0, d;

  while (buff->next)
    buff = buff->next;
  while (i < count) {
    if (buff->count == BUFF_SIZE) {
      if ((buff = add_buff(buff)) == NULL)
	return (EXIT_FAILURE);
    }
    d = (((count - i) < (BUFF_SIZE - buff->count)) ?
	count - i :
	BUFF_SIZE - buff->count
	);
    memcpy(buff->buff + buff->count, str + i, d);
    i += d;
    buff->count += d;
  }
  return (EXIT_SUCCESS);
}

int
push_buff_left(char *str, size_t count) {
  return (push_buff(g_buff.l, str, count));
}

int
push_buff_right(char *str, size_t count) {
  return (push_buff(g_buff.r, str, count));
}
