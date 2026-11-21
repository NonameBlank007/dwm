#ifndef OVERVIEW_C
#define OVERVIEW_C

#include "overview.h"
#include "scrollview.h"

static OvLayer g_layers[16];
static int g_nlayers = 0;
static int g_sel_layer = 0;
static int g_ovscroll_y = 0;
static int g_ovmaxscroll_y = 0;
static int g_need_center_sel = 1;
static unsigned int saved_ovtags = 0;
static const Layout *saved_ovlayout = NULL;

static inline void
restore_ovlayout(void)
{
	const Layout *lt = (saved_ovlayout && saved_ovlayout->arrange != overview)
	                   ? saved_ovlayout : &layouts[0];
	saved_ovlayout = NULL;
	if (selmon->lt[selmon->sellt]->arrange == overview)
		setlayout(&(Arg){.v = (void *)lt});
}

static inline int
layer_by_tag(unsigned int tagmask)
{
	for (int i = 0; i < g_nlayers; i++)
		if (tagmask & (1 << g_layers[i].tag))
			return i;
	return -1;
}

static inline int
sync_sel_layer(void)
{
	int l = selmon->sel ? layer_by_tag(selmon->sel->tags) : -1;
	if (l < 0)
		l = (g_sel_layer >= 0 && g_sel_layer < g_nlayers) ? g_sel_layer : 0;
	return g_sel_layer = l;
}

static inline void
ovfocuswin(Client *c)
{
	if (!c) return;
	focus(c);
	restack(selmon);
	g_need_center_sel = 1;
	arrangemon(selmon);
}

static void
setov(int enable)
{
	if (isov == enable)
		return;
	isov = enable;
	if (enable) {
		g_ovscroll_y = 0;
		g_ovmaxscroll_y = 0;
		g_need_center_sel = 1;
		ovacquireseat();
		for (Monitor *m = mons; m; m = m->next)
			for (Client *c = m->clients; c; c = c->next)
				grabbuttons(c, 0);
	} else {
		ovreleaseseat();
		grabkeys();
		restore_ovlayout();
		for (Monitor *m = mons; m; m = m->next)
			for (Client *c = m->clients; c; c = c->next)
				grabbuttons(c, c == m->sel);
		if (selmon->sel)
			focus(selmon->sel);
	}
}

static void
toggleov(const Arg *arg)
{
	if (isov) {
		setov(0);
		if (saved_ovtags)
			selmon->tagset[selmon->seltags] = saved_ovtags;
		focus(NULL);
		arrange(selmon);
	} else {
		saved_ovtags = selmon->tagset[selmon->seltags];
		saved_ovlayout = selmon->lt[selmon->sellt];
		g_sel_layer = MAX(0, layer_by_tag(saved_ovtags));
		for (int i = 0; i < LENGTH(layouts); i++) {
			if (layouts[i].arrange == overview) {
				selmon->lt[selmon->sellt] = (Layout *)&layouts[i];
				break;
			}
		}
		g_ovscroll_y = 0;
		g_ovmaxscroll_y = 0;
		g_need_center_sel = 1;
		setov(1);
		arrange(selmon);
	}
}

static int
getlayercols(Monitor *m, unsigned int tagmask, Client *skip, Column cols[MAX_COLS])
{
	int ncol = 0;
	if (!m)
		return 0;

	for (Client *c = m->clients; c; c = c->next) {
		if (c == skip || c->isfloating || !(c->tags & tagmask))
			continue;
		if (ncol == 0 || !c->is_stacked) {
			if (ncol < MAX_COLS) {
				cols[ncol].clients[0] = c;
				cols[ncol].count = 1;
				ncol++;
			}
		} else {
			if (cols[ncol - 1].count < MAX_COL_CLIENTS)
				cols[ncol - 1].clients[cols[ncol - 1].count++] = c;
		}
	}
	return ncol;
}

static void
focusovwindir(int dir)
{
	if (g_nlayers == 0)
		return;

	int cur_layer = sync_sel_layer();
	Column cols[MAX_COLS];
	int ncol = getlayercols(selmon, 1 << g_layers[cur_layer].tag, NULL, cols);
	if (ncol == 0)
		return;

	int cur_row = 0;
	int cur_col = find_col_row(cols, ncol, selmon->sel, &cur_row);

	int target_col;
	if (cur_col == -1) {
		target_col = (dir > 0) ? 0 : ncol - 1;
	} else {
		target_col = NAV_WRAP(cur_col, dir, ncol, ovloopnav);
		if (target_col == cur_col)
			return;
	}
	int target_row = (cur_col == -1) ? 0 : MIN(cur_row, cols[target_col].count - 1);

	ovfocuswin(cols[target_col].clients[target_row]);
}

static void
focusovlayerdir(int dir)
{
	if (g_nlayers == 0)
		return;

	int cur_layer = sync_sel_layer();
	Column cols[MAX_COLS];
	int ncol = getlayercols(selmon, 1 << g_layers[cur_layer].tag, NULL, cols);

	int cur_row = 0;
	int cur_col = find_col_row(cols, ncol, selmon->sel, &cur_row);

	if (cur_col != -1 && cols[cur_col].count > 1) {
		if ((dir < 0 && cur_row > 0) || (dir > 0 && cur_row < cols[cur_col].count - 1)) {
			ovfocuswin(cols[cur_col].clients[cur_row + dir]);
			return;
		}
	}

	if (g_nlayers > 1) {
		int target_layer = NAV_WRAP(cur_layer, dir, g_nlayers, ovloopnav);
		if (target_layer == cur_layer)
			return;
		Column target_cols[MAX_COLS];
		int target_ncol = getlayercols(selmon, 1 << g_layers[target_layer].tag, NULL, target_cols);

		if (target_ncol > 0) {
			int tcol = CLAMP(cur_col < 0 ? 0 : cur_col, 0, target_ncol - 1);
			int trow = (dir > 0) ? 0 : (target_cols[tcol].count - 1);
			Client *target_win = target_cols[tcol].clients[MIN(trow, target_cols[tcol].count - 1)];
			if (target_win) {
				focus(target_win);
				restack(selmon);
			}
		}
		g_sel_layer = target_layer;
		g_need_center_sel = 1;
		arrangemon(selmon);
		return;
	}

	if (cur_col != -1 && cols[cur_col].count > 1) {
		if (!ovloopnav)
			return;
		int trow = (dir < 0) ? (cols[cur_col].count - 1) : 0;
		ovfocuswin(cols[cur_col].clients[trow]);
	} else {
		focusovwindir(dir);
	}
}

static void
ovjumptagidx(int tag_idx)
{
	int l = layer_by_tag(1 << tag_idx);
	if (l >= 0) {
		g_sel_layer = l;
		if (g_layers[l].first_win)
			focus(g_layers[l].first_win);
		g_need_center_sel = 1;
		arrangemon(selmon);
		return;
	}
	view(&(Arg){.ui = 1 << tag_idx});
}

static void
zoomovsel(void)
{
	if (g_nlayers == 0) {
		toggleov(NULL);
		return;
	}

	int tag = g_layers[g_sel_layer].tag;
	Client *c = selmon->sel;
	view(&(Arg){.ui = 1 << tag});
	restore_ovlayout();
	focus((c && (c->tags & (1 << tag))) ? c : g_layers[g_sel_layer].first_win);
	restack(selmon);
}

static void
zoomovwin(Client *c)
{
	if (!c)
		return;
	view(&(Arg){.ui = c->tags});
	restore_ovlayout();
	focus(c);
	restack(selmon);
}

static int
getovexactlayertag(int px, int py)
{
	for (int i = 0; i < g_nlayers; i++) {
		if (py >= g_layers[i].y && py < g_layers[i].y + g_layers[i].h)
			return g_layers[i].tag;
	}
	return -1;
}

static Client *
getovwin(int px, int py)
{
	int tag = getovexactlayertag(px, py);
	if (tag < 0)
		return NULL;
	unsigned int tagmask = 1 << tag;
	for (Client *c = selmon->clients; c; c = c->next) {
		if (!c->isfloating && (c->tags & tagmask) &&
		    px >= c->x && px < c->x + c->w + 2 * c->bw &&
		    py >= c->y && py < c->y + c->h + 2 * c->bw)
			return c;
	}
	return NULL;
}

static int
getovlayertag(int px, int py)
{
	int best_tag = -1;
	long min_dy = -1;

	for (int i = 0; i < g_nlayers; i++) {
		if (py >= g_layers[i].y && py < g_layers[i].y + g_layers[i].h)
			return g_layers[i].tag;

		int cy = g_layers[i].y + g_layers[i].h / 2;
		long dy = abs(py - cy);
		if (min_dy < 0 || dy < min_dy) {
			min_dy = dy;
			best_tag = g_layers[i].tag;
		}
	}
	return best_tag;
}

static void
ovdrop(Client *c, int drop_x, int drop_y)
{
	if (!c)
		return;

	if (drop_y <= selmon->by + bh) {
		int i = 0, tx = 0;
		do {
			tx += TEXTW(tags[i]);
		} while (drop_x >= tx && ++i < LENGTH(tags));
		if (i < LENGTH(tags)) {
			c->tags = 1 << i;
			c->is_stacked = 0;
			c->cfact = (c->unstacked_cfact > 0.05f) ? c->unstacked_cfact : defwinw;
			detach(c);
			attach(c);
		}
	} else {
		int target_tag = getovlayertag(drop_x, drop_y);
		if (target_tag >= 0) {
			unsigned int new_tags = 1 << target_tag;
			Column tag_cols[MAX_COLS];
			int tag_ncol = getlayercols(selmon, new_tags, c, tag_cols);

			c->tags = new_tags;

			if (tag_ncol == 0) {
				c->is_stacked = 0;
				c->cfact = (c->unstacked_cfact > 0.05f) ? c->unstacked_cfact : defwinw;
				detach(c);
				attach(c);
			} else {
				Client *first_win = tag_cols[0].clients[0];
				Client *last_win = tag_cols[tag_ncol - 1].clients[tag_cols[tag_ncol - 1].count - 1];
				int left_bound = first_win->x;
				int right_bound = last_win->x + last_win->w + 2 * last_win->bw;

				if (drop_x <= left_bound + 10) {
					c->is_stacked = 0;
					unstackreorder(c, first_win, InsBefore);
				} else if (drop_x >= right_bound - 10) {
					c->is_stacked = 0;
					unstackreorder(c, last_win, InsAfter);
				} else {
					int placed = 0;
					for (int k = 0; k < tag_ncol; k++) {
						Client *win_lead = tag_cols[k].clients[0];
						int col_x = win_lead->x;
						int col_w = win_lead->w + 2 * win_lead->bw;
						int next_col_x = (k + 1 < tag_ncol) ? tag_cols[k + 1].clients[0]->x : right_bound;

						if (drop_x >= col_x && drop_x < col_x + col_w) {
							int edge_zone = MAX(20, (int)(col_w * 0.22));

							if (drop_x < col_x + edge_zone) {
								c->is_stacked = 0;
								unstackreorder(c, win_lead, InsBefore);
								placed = 1;
								break;
							} else if (drop_x > col_x + col_w - edge_zone) {
								c->is_stacked = 0;
								unstackreorder(c, tag_cols[k].clients[tag_cols[k].count - 1], InsAfter);
								placed = 1;
								break;
							} else {
								Client *target_win = win_lead;
								for (int j = 0; j < tag_cols[k].count; j++) {
									Client *win = tag_cols[k].clients[j];
									if (drop_y >= win->y && drop_y <= win->y + win->h + 2 * win->bw) {
										target_win = win;
										break;
									}
								}

								if (drop_y < target_win->y + target_win->h / 3) {
									c->is_stacked = 1;
									reorderclient(c, target_win, StkBefore);
									c->cfact = target_win->cfact;
								} else if (drop_y > target_win->y + (target_win->h * 2) / 3) {
									c->is_stacked = 1;
									reorderclient(c, target_win, StkAfter);
									c->cfact = target_win->cfact;
								} else if (drop_x < col_x + col_w / 2) {
									c->is_stacked = 0;
									unstackreorder(c, win_lead, InsBefore);
								} else {
									c->is_stacked = 0;
									unstackreorder(c, tag_cols[k].clients[tag_cols[k].count - 1], InsAfter);
								}
								placed = 1;
								break;
							}
						} else if (drop_x >= col_x + col_w && drop_x < next_col_x) {
							c->is_stacked = 0;
							unstackreorder(c, tag_cols[k].clients[tag_cols[k].count - 1], InsAfter);
							placed = 1;
							break;
						}
					}

					if (!placed) {
						c->is_stacked = 0;
						unstackreorder(c, last_win, InsAfter);
					}
				}
			}
			int l = layer_by_tag(new_tags);
			if (l >= 0) g_sel_layer = l;
		}
	}
	c->x = -9999;
	c->y = -9999;
	selmon->sel = c;
	g_need_center_sel = 1;
	arrangemon(selmon);
	focus(c);
	XSync(dpy, False);
}

static void
focusovlayer(const Arg *arg)
{
	focusovlayerdir(arg->i);
}

static void
focusovwin(const Arg *arg)
{
	focusovwindir(arg->i);
}

static void
ovmovewindir(int dir)
{
	if (!selmon || !selmon->sel || selmon->sel->isfloating || g_nlayers == 0)
		return;

	Client *c = selmon->sel;
	int cur_layer = sync_sel_layer();
	if (cur_layer < 0 || cur_layer >= g_nlayers)
		return;

	Column cols[MAX_COLS];
	int ncol = getlayercols(selmon, 1 << g_layers[cur_layer].tag, NULL, cols);
	if (ncol == 0)
		return;

	int cur_col = find_col_row(cols, ncol, c, NULL);
	if (cur_col == -1 || (ncol <= 1 && cols[cur_col].count <= 1))
		return;

	if (!reorder_col_dir(c, cols, ncol, cur_col, dir, ovloopnav))
		return;

	c->x = -9999;
	c->y = -9999;
	selmon->sel = c;
	g_need_center_sel = 1;
	arrangemon(selmon);
	focus(c);
	keep_cursor_inside(c);
}

static void
ovmovewinlayerdir(int dir)
{
	if (!selmon || !selmon->sel || selmon->sel->isfloating || g_nlayers <= 1)
		return;

	Client *c = selmon->sel;
	int cur_layer = sync_sel_layer();
	if (cur_layer < 0 || cur_layer >= g_nlayers)
		return;

	int target_layer = NAV_WRAP(cur_layer, dir, g_nlayers, ovloopnav);
	if (target_layer == cur_layer)
		return;

	unsigned int target_tag = g_layers[target_layer].tag;
	unsigned int new_tags = 1 << target_tag;

	Column cur_cols[MAX_COLS];
	int cur_ncol = getlayercols(selmon, 1 << g_layers[cur_layer].tag, NULL, cur_cols);
	int cur_col = find_col_row(cur_cols, cur_ncol, c, NULL);

	Column target_cols[MAX_COLS];
	int target_ncol = getlayercols(selmon, new_tags, c, target_cols);

	detach_unstack(c);

	c->is_stacked = 0;
	ENSURE_CFACT(c);
	c->is_custom_cfact = IS_CUSTOM_CFACT(c->cfact);
	c->tags = new_tags;

	if (target_ncol == 0) {
		c->next = selmon->clients;
		selmon->clients = c;
	} else {
		int col_pos = (cur_col < 0) ? target_ncol - 1 : cur_col;
		Client **tc;
		if (col_pos == 0) {
			Client *first_win = target_cols[0].clients[0];
			for (tc = &selmon->clients; *tc && *tc != first_win; tc = &(*tc)->next);
			c->next = first_win;
			*tc = c;
		} else {
			int t_col = MIN(col_pos, target_ncol) - 1;
			Client *prev_win = target_cols[t_col].clients[target_cols[t_col].count - 1];
			c->next = prev_win->next;
			prev_win->next = c;
		}
	}

	c->x = -9999;
	c->y = -9999;
	selmon->sel = c;
	g_sel_layer = target_layer;
	g_need_center_sel = 1;
	arrangemon(selmon);
	focus(c);
	keep_cursor_inside(c);
}

static void
ovmovewin(const Arg *arg)
{
	ovmovewindir(arg->i);
}

static void
ovmovewinlayer(const Arg *arg)
{
	ovmovewinlayerdir(arg->i);
}

static void
toggleovloopnav(const Arg *arg)
{
	ovloopnav = !ovloopnav;
}

static void
zoomov(const Arg *arg)
{
	zoomovsel();
}

static void
ovclick(Client *c, int px, int py, int button)
{
	if (button == Button1) {
		if (c) {
			zoomovwin(c);
		} else {
			int tag = getovexactlayertag(px, py);
			if (tag >= 0) {
				Arg a = {.ui = 1 << tag};
				view(&a);
			} else {
				toggleov(NULL);
			}
		}
	} else if (button == Button3) {
		toggleov(NULL);
	}
}

static void
ovjumptag(const Arg *arg)
{
	ovjumptagidx(arg->ui);
}

static int
drain_scroll_events(int is_horiz, int delta)
{
	XEvent next_ev;
	while (XCheckMaskEvent(dpy, ButtonPressMask, &next_ev)) {
		int b = next_ev.xbutton.button;
		unsigned int state = CLEANMASK(next_ev.xbutton.state);
		int match_neg = is_horiz ? ((b == 6 && !state) || (b == Button4 && state == ShiftMask))
		                         : (b == Button4 && !state);
		int match_pos = is_horiz ? ((b == 7 && !state) || (b == Button5 && state == ShiftMask))
		                         : (b == Button5 && !state);
		if (match_neg)
			delta -= scrollstep;
		else if (match_pos)
			delta += scrollstep;
		else {
			XPutBackEvent(dpy, &next_ev);
			break;
		}
	}
	return delta;
}

static void
ovscrollribbon_px(int tag, int delta_px)
{
	int l = layer_by_tag(1 << tag);
	if (l >= 0) {
		g_need_center_sel = 0;
		g_layers[l].scroll_x = CLAMP(g_layers[l].scroll_x + delta_px, 0, MAX(0, g_layers[l].max_scroll_x));
	}
}

static void
ovscrollvert_px(int delta_px)
{
	g_need_center_sel = 0;
	g_ovscroll_y = CLAMP(g_ovscroll_y + delta_px, 0, g_ovmaxscroll_y);
}

static void
ovscrollribbon(const Arg *arg)
{
	if (!isov)
		return;
	int dir = (arg && arg->i < 0) ? -1 : +1;
	int init_d = dir * scrollstep;
	int x, y;
	getrootptr(&x, &y);
	int layer_tag = getovlayertag(x, y);
	int delta = drain_scroll_events(1, init_d);
	ovscrollribbon_px(layer_tag, delta);
	arrangemon(selmon);
}

static void
ovscrollvert(const Arg *arg)
{
	if (!isov)
		return;
	int dir = (arg && arg->i < 0) ? -1 : +1;
	int init_d = dir * scrollstep;
	if (g_ovmaxscroll_y > 0) {
		int delta = drain_scroll_events(0, init_d);
		ovscrollvert_px(delta);
		arrangemon(selmon);
	} else {
		static Time last_layer_move = 0;
		if ((last_button_time - last_layer_move) > 120) {
			last_layer_move = last_button_time;
			focusovlayerdir(dir);
		}
	}
}

static void
ovresizemouse(const Arg *arg)
{
	Client *c = selmon->sel;
	int x, y;
	XEvent ev;
	Time lasttime = 0;

	if (!isov || !c || c->isfloating)
		return;

	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurResize]->cursor, CurrentTime) != GrabSuccess)
		return;

	if (!getrootptr(&x, &y)) {
		XUngrabPointer(dpy, CurrentTime);
		return;
	}

	float start_cfact = (c->cfact > 0.05f) ? c->cfact : defwinw;
	int pad_x = ovgappoh;
	int usable_w = selmon->ww - 2 * pad_x;
	if (usable_w < 100) usable_w = selmon->ww;

	do {
		XMaskEvent(dpy, MOUSEMASK|ExposureMask|SubstructureRedirectMask, &ev);
		switch (ev.type) {
		case ConfigureRequest:
		case Expose:
		case MapRequest:
			handler[ev.type](&ev);
			break;
		case MotionNotify:
			if ((ev.xmotion.time - lasttime) <= (1000 / refreshrate))
				continue;
			lasttime = ev.xmotion.time;

			float delta = (float)(ev.xmotion.x_root - x) / (float)usable_w;
			float new_cfact = CLAMP(start_cfact + delta, 0.10f, 1.0f);

			c->cfact = new_cfact;
			c->unstacked_cfact = new_cfact;
			c->is_custom_cfact = 1;
			syncwincfact(c, new_cfact);
			arrangemon(selmon);
			break;
		}
	} while (ev.type != ButtonRelease);

	XUngrabPointer(dpy, CurrentTime);
	arrangemon(selmon);
	focus(c);
	XSync(dpy, False);
}

static void
overview(Monitor *m)
{
	unsigned int in_use = 0;
	for (Client *c = m->clients; c; c = c->next)
		if (!c->isfloating)
			in_use |= (c->tags & TAGMASK);

	if (!in_use)
		in_use = m->tagset[m->seltags] & TAGMASK;

	g_nlayers = 0;
	for (int i = 0; i < LENGTH(tags); i++) {
		if (in_use & (1 << i)) {
			g_layers[g_nlayers].tag = i;
			g_layers[g_nlayers].nwin = 0;
			g_layers[g_nlayers].first_win = NULL;
			g_nlayers++;
		}
	}

	if (g_nlayers == 0) {
		g_layers[0].tag = 0;
		g_layers[0].nwin = 0;
		g_layers[0].first_win = NULL;
		g_nlayers = 1;
	}

	int sel_l = m->sel ? layer_by_tag(m->sel->tags) : (saved_ovtags ? layer_by_tag(saved_ovtags) : -1);
	if (sel_l >= 0)
		g_sel_layer = sel_l;
	g_sel_layer = CLAMP(g_sel_layer, 0, g_nlayers - 1);

	int mx = m->wx, my = m->wy, mw = m->ww, mh = m->wh;
	int pad_y = ovgappov;
	int gap_y = ovlayergap;
	int pad_x = ovgappoh;
	int uw = mw - 2 * pad_x;
	int avail_h = mh - 2 * pad_y;
	if (avail_h < 100) avail_h = mh;

	int total_gaps = (g_nlayers - 1) * gap_y;
	int layer_h;
	if (ovuniformlayer || g_nlayers == 1)
		layer_h = (int)(avail_h * ovsingleh);
	else if (g_nlayers <= ovmaxfit)
		layer_h = (avail_h - total_gaps) / g_nlayers;
	else
		layer_h = (avail_h - (ovmaxfit - 1) * gap_y) / ovmaxfit;

	if (layer_h < 120) layer_h = 120;

	int total_h = g_nlayers * layer_h + total_gaps;
	if (total_h <= avail_h) {
		int start_y = my + pad_y + (avail_h - total_h) / 2;
		for (int i = 0; i < g_nlayers; i++) {
			g_layers[i].y = start_y + i * (layer_h + gap_y);
			g_layers[i].h = layer_h;
		}
		if (m->sel) {
			int l = layer_by_tag(m->sel->tags);
			if (l >= 0) g_sel_layer = l;
		}
		g_ovscroll_y = 0;
		g_ovmaxscroll_y = 0;
	} else {
		int stride = layer_h + gap_y;
		int total_h = g_nlayers * layer_h + total_gaps;
		int max_scroll = MAX(0, total_h - avail_h);
		g_ovmaxscroll_y = max_scroll;

		if (g_need_center_sel && !is_dragging && !is_scrolling) {
			int sel_center = g_sel_layer * stride + layer_h / 2;
			g_ovscroll_y = CLAMP(sel_center - (avail_h / 2), 0, max_scroll);
		}
		g_ovscroll_y = CLAMP(g_ovscroll_y, 0, max_scroll);

		int start_y = my + pad_y - g_ovscroll_y;
		for (int i = 0; i < g_nlayers; i++) {
			g_layers[i].y = start_y + i * stride;
			g_layers[i].h = layer_h;
		}

		if (!m->sel && (is_scrolling || is_dragging)) {
			int screen_center_y = my + pad_y + avail_h / 2;
			int best_diff = -1;
			for (int i = 0; i < g_nlayers; i++) {
				int layer_center_y = g_layers[i].y + layer_h / 2;
				int diff = abs(layer_center_y - screen_center_y);
				if (best_diff < 0 || diff < best_diff) {
					best_diff = diff;
					g_sel_layer = i;
				}
			}
		} else if (m->sel) {
			int l = layer_by_tag(m->sel->tags);
			if (l >= 0) g_sel_layer = l;
		}
	}

	int in_pad_y = ovinpady;
	int uh = layer_h - 2 * in_pad_y;
	if (uh < 60) uh = layer_h;
	int igap_x = ovgappih;
	int igap_y = ovgappiv;
	float S = (float)uh / (float)(mh > 0 ? mh : 1);

	for (int i = 0; i < g_nlayers; i++) {
		int tag_idx = g_layers[i].tag;
		int layer_y = g_layers[i].y;

		Column tag_cols[MAX_COLS];
		int tag_ncol = getlayercols(m, 1 << tag_idx, NULL, tag_cols);
		g_layers[i].first_win = (tag_ncol > 0) ? tag_cols[0].clients[0] : NULL;
		g_layers[i].nwin = tag_ncol;
		if (tag_ncol == 0)
			continue;

		int total_ribbon_w = 0;
		for (int k = 0; k < tag_ncol; k++) {
			Client *top = tag_cols[k].clients[0];
			float factor = CLAMP((top->cfact > 0.05f) ? top->cfact : defwinw, 0.1f, 1.0f);
			int w = (tag_ncol == 1) ? MIN((int)(uh * 1.6f), (int)(uw * 0.70f))
			                        : (int)(uw * factor * S * 1.3f);
			tag_cols[k].w = CLAMP(w, 100, uw);
			total_ribbon_w += tag_cols[k].w + (k > 0 ? igap_x : 0);
		}

		int start_x;
		if (total_ribbon_w <= uw) {
			start_x = mx + pad_x + (uw - total_ribbon_w) / 2;
			g_layers[i].scroll_x = 0;
			g_layers[i].max_scroll_x = 0;
		} else {
			int max_scroll = total_ribbon_w - uw;
			g_layers[i].max_scroll_x = max_scroll;

			if (g_need_center_sel && !is_dragging && !is_scrolling) {
				int sel_col = find_col_row(tag_cols, tag_ncol, m->sel, NULL);
				if (sel_col >= 0) {
					int cur_col_x = 0;
					for (int k = 0; k < sel_col; k++)
						cur_col_x += tag_cols[k].w + igap_x;
					int target_scroll = (cur_col_x + tag_cols[sel_col].w / 2) - (uw / 2);
					g_layers[i].scroll_x = CLAMP(target_scroll, 0, max_scroll);
				}
			}
			g_layers[i].scroll_x = CLAMP(g_layers[i].scroll_x, 0, max_scroll);
			start_x = mx + pad_x - g_layers[i].scroll_x;
		}

		int cur_x = start_x;
		for (int k = 0; k < tag_ncol; k++) {
			int col_w = tag_cols[k].w;
			int count = tag_cols[k].count;
			int total_gap = (count - 1) * igap_y;
			int win_h = (uh - total_gap) / count;
			int cur_y = layer_y + in_pad_y;

			for (int j = 0; j < count; j++) {
				Client *c = tag_cols[k].clients[j];
				int h = (j == count - 1) ? (uh - (cur_y - (layer_y + in_pad_y))) : win_h;
				if (!is_dragging || c != m->sel)
					scrollermoveresize(c, cur_x, cur_y, MAX(1, col_w - 2 * c->bw), MAX(1, h - 2 * c->bw));
				cur_y += win_h + igap_y;
			}
			cur_x += col_w + igap_x;
		}
	}
	if (!m->sel && g_layers[g_sel_layer].first_win)
		focus(g_layers[g_sel_layer].first_win);
	g_need_center_sel = 0;
	XSync(dpy, False);
}

#endif /* OVERVIEW_C */
