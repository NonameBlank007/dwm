#ifndef OVERVIEW_H
#define OVERVIEW_H

typedef struct {
	int tag;          /* 0-indexed tag (0 to 8) */
	int y;            /* Y coordinate on screen */
	int h;            /* Height of this layer */
	int nwin;         /* Number of windows / columns */
	Client *first_win;/* First window in this layer */
	int scroll_x;     /* Horizontal ribbon scroll offset */
	int max_scroll_x; /* Maximum horizontal scroll offset */
} OvLayer;

static void overview(Monitor *m);
static void setov(int enable);
static void toggleov(const Arg *arg);
static void focusovlayer(const Arg *arg);
static void focusovwin(const Arg *arg);
static void zoomov(const Arg *arg);
static void ovjumptag(const Arg *arg);
static void ovdrop(Client *c, int drop_x, int drop_y);
static int getovlayertag(int px, int py);
static void ovscrollribbon(const Arg *arg);
static void ovscrollvert(const Arg *arg);
static void ovscrollribbon_px(int tag, int delta_px);
static void ovscrollvert_px(int delta_px);
static void ovresizemouse(const Arg *arg);
static void ovclick(Client *c, int px, int py, int button);
static void ovmovewin(const Arg *arg);
static void ovmovewinlayer(const Arg *arg);
static void toggleovloopnav(const Arg *arg);
static Client *getovwin(int px, int py);

#endif /* OVERVIEW_H */
