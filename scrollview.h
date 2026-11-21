#ifndef SCROLLVIEW_H
#define SCROLLVIEW_H

#define MAX_COLS 128
#define MAX_COL_CLIENTS 64

#define CURRENT_MODE() (isov ? Overview : (Normal | (selmon && \
	selmon->lt[selmon->sellt]->arrange == scroller ? Scroll : 0)))
#define CLAMP(x, lo, hi)   ((x) < (lo) ? (lo) : ((x) > (hi) ? (hi) : (x)))
#define DIFF_ABS(a, b)     (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define IS_CUSTOM_CFACT(f) (DIFF_ABS((f), defwinw) > 0.01f)
#define ENSURE_CFACT(c)    do { if ((c)->unstacked_cfact <= 0.05f) \
	(c)->unstacked_cfact = ((c)->cfact > 0.05f) ? (c)->cfact : defwinw; } while(0)
#define NAV_WRAP(cur, delta, count, loop) \
	((loop) ? (((cur) + (delta) % (count) + (count)) % (count)) : CLAMP((cur) + (delta), 0, (count) - 1))

enum { CenterOnFlow, CenterAlways, CenterNever };
enum { InsBefore, InsAfter, StkBefore, StkAfter };

typedef struct {
	Client *clients[MAX_COL_CLIENTS];
	int count;
	int w;
	int x;
} Column;

static void scroller(Monitor *m);
static void scrollwin(const Arg *arg);
static void trackscroll(const Arg *arg);
static void focuswinstack(const Arg *arg);
static void stackconsume(const Arg *arg);
static void stackexpel(const Arg *arg);
static void togglestack(const Arg *arg);
static void movewin(const Arg *arg);
static void movewinstack(const Arg *arg);
static void cyclewinwidth(const Arg *arg);
void centerfocused(const Arg *arg);
static void togglecentering(const Arg *arg);
static void toggleloopnav(const Arg *arg);
void syncwincfact(Client *c, float f);
static void dragdrop(const Arg *arg);

/* Shared functions */
static int find_col_row(const Column *cols, int ncol, const Client *c, int *out_row);
static void scrollermoveresize(Client *c, int x, int y, int w, int h);
static void reorderclient(Client *c, Client *target, int pos);
static void unstackreorder(Client *c, Client *target, int pos);
static void detach_unstack(Client *c);
static int reorder_col_dir(Client *c, const Column *cols, int ncol, int col_idx, int dir, int loop);
static void keep_cursor_inside(Client *c);

#endif /* SCROLLVIEW_H */
