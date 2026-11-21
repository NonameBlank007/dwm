#include "scrollview.h"

static int is_scrolling = 0;
static int is_dragging = 0;

static Column g_cols[MAX_COLS];
static int g_ncol = 0;
static Client *prev_sel_client = NULL;
static unsigned int prev_tagset = 0;

static void
scrollermoveresize(Client *c, int x, int y, int w, int h)
{
	if (c->x == x && c->y == y && c->w == w && c->h == h)
		return;

	if (c->w == w && c->h == h) {
		c->oldx = c->x; c->x = x;
		c->oldy = c->y; c->y = y;
		XMoveWindow(dpy, c->win, x, y);
	} else {
		resize(c, x, y, w, h, 0);
	}
}

static int
find_col_row(const Column *cols, int ncol, const Client *c, int *out_row)
{
	if (out_row) *out_row = 0;
	if (!c || !cols) return -1;
	for (int i = 0; i < ncol; i++)
		for (int j = 0; j < cols[i].count; j++)
			if (cols[i].clients[j] == c) {
				if (out_row) *out_row = j;
				return i;
			}
	return -1;
}

static inline int
get_view_geom(Monitor *m, int ncol, int *out_base_x)
{
	int uw = (m->ww - 2 * m->gappoh >= 100) ? m->ww - 2 * m->gappoh : m->ww;
	int sl = (ncol > 1 && strutleft > 0) ? strutleft : 0;
	int sr = (ncol > 1 && strutright > 0) ? strutright : 0;
	int vw = (uw - sl - sr >= 100) ? uw - sl - sr : uw;
	if (out_base_x)
		*out_base_x = m->wx + m->gappoh + sl;
	return vw;
}

static inline void
unstackreorder(Client *c, Client *target, int pos)
{
	reorderclient(c, target, pos);
	c->cfact = (c->unstacked_cfact > 0.05f) ? c->unstacked_cfact : defwinw;
	c->is_custom_cfact = IS_CUSTOM_CFACT(c->cfact);
}

static inline void
unstack_to_edge(Client *c, const Column *cols, int c_col, int target_col, int pos)
{
	int is_before = (pos == InsBefore);
	Client *target_win;
	if (c_col != target_col) {
		target_win = is_before ? cols[target_col].clients[0]
		                       : cols[target_col].clients[cols[target_col].count - 1];
	} else if (cols[c_col].count > 1) {
		int count = cols[c_col].count;
		if (is_before)
			target_win = (cols[c_col].clients[0] == c) ? cols[c_col].clients[1] : cols[c_col].clients[0];
		else
			target_win = (cols[c_col].clients[count - 1] == c)
			             ? cols[c_col].clients[count - 2]
			             : cols[c_col].clients[count - 1];
	} else {
		return;
	}
	unstackreorder(c, target_win, pos);
}

static inline void
unstack_client(Client *c, int col_idx)
{
	unstack_to_edge(c, g_cols, col_idx, col_idx, (c == g_cols[col_idx].clients[0]) ? InsBefore : InsAfter);
}

static inline void
stack_client_target(Client *c, Column *col, Client *target_win, int pos)
{
	ENSURE_CFACT(c);
	for (int j = 0; j < col->count; j++)
		ENSURE_CFACT(col->clients[j]);

	float target_cfact = col->clients[0]->cfact;
	int target_custom = col->clients[0]->is_custom_cfact;
	reorderclient(c, target_win, pos);
	c->cfact = target_cfact;
	c->is_custom_cfact = target_custom;
}

static int
reorder_col_dir(Client *c, const Column *cols, int ncol, int col_idx, int dir, int loop)
{
	if (cols[col_idx].count > 1) {
		unstack_to_edge(c, cols, col_idx, col_idx, (dir > 0) ? InsAfter : InsBefore);
		return 1;
	}

	if (dir > 0) {
		if (col_idx < ncol - 1)
			unstackreorder(c, cols[col_idx + 1].clients[cols[col_idx + 1].count - 1], InsAfter);
		else if (loop && ncol > 1)
			unstackreorder(c, cols[0].clients[0], InsBefore);
		else
			return 0;
	} else if (dir < 0) {
		if (col_idx > 0)
			unstackreorder(c, cols[col_idx - 1].clients[0], InsBefore);
		else if (loop && ncol > 1)
			unstackreorder(c, cols[ncol - 1].clients[cols[ncol - 1].count - 1], InsAfter);
		else
			return 0;
	} else {
		return 0;
	}
	return 1;
}

static void
keep_cursor_inside(Client *c)
{
	int px, py;
	XEvent ev;

	if (!c)
		return;

	if (getrootptr(&px, &py)) {
		if (px < c->x || px >= c->x + c->w || py < c->y || py >= c->y + c->h)
			XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
	} else {
		XWarpPointer(dpy, None, c->win, 0, 0, 0, 0, c->w / 2, c->h / 2);
	}

	focus(c);
	if (c->mon)
		restack(c->mon);
	XSync(dpy, False);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));
}

static inline void
finish_drag_client(Client *c)
{
	c->x = c->y = -9999;
	selmon->sel = c;
	arrangemon(selmon);
	focus(c);
	keep_cursor_inside(c);
	XSync(dpy, False);
}

static inline int
get_ribbon_offset(int total_w, int view_w)
{
	if (total_w < view_w && centermode == CenterAlways)
		return (view_w - total_w) / 2;
	return 0;
}

static void
scroller_arrange_col(Monitor *m, Column *col, int col_x, int col_w)
{
	int count = col->count;
	int total_gaps_v = (count - 1) * m->gappiv;
	int avail_h = m->wh - 2 * m->gappov - total_gaps_v;
	if (avail_h < count) avail_h = count;
	int h_each = avail_h / count;
	int cur_y = m->wy + m->gappov;

	for (int j = 0; j < count; j++) {
		Client *c = col->clients[j];
		int win_h = (j == count - 1) ? (avail_h - (count - 1) * h_each) : h_each;
		int bw = c->bw;
		if (is_dragging && c == m->sel) {
			cur_y += win_h + m->gappiv;
			continue;
		}
		scrollermoveresize(c, col_x, cur_y, MAX(1, col_w - 2 * bw), MAX(1, win_h - 2 * bw));
		cur_y += win_h + m->gappiv;
	}
}

static int
build_columns(Monitor *m)
{
	Client *c;
	g_ncol = 0;

	for (c = nexttiled(m->clients); c && g_ncol < MAX_COLS; c = nexttiled(c->next)) {
		if (g_ncol == 0 || !c->is_stacked) {
			g_cols[g_ncol].clients[0] = c;
			g_cols[g_ncol].count = 1;
			g_ncol++;
		} else {
			if (g_cols[g_ncol - 1].count < MAX_COL_CLIENTS)
				g_cols[g_ncol - 1].clients[g_cols[g_ncol - 1].count++] = c;
		}
	}

	int view_w = get_view_geom(m, g_ncol, NULL);
	int cur_x = 0;
	for (int i = 0; i < g_ncol; i++) {
		Client *top = g_cols[i].clients[0];

		if (g_ncol > 1 && g_cols[i].count == 1) {
			if (top->unstacked_cfact > 0.05f)
				top->cfact = top->unstacked_cfact;
			top->is_custom_cfact = IS_CUSTOM_CFACT(top->cfact);
		}

		float factor = top->is_custom_cfact ? ((top->cfact > 0.05f) ? top->cfact : defwinw)
		                                    : ((g_ncol > 1) ? defwinw : swinw);
		factor = CLAMP(factor, 0.1f, 1.0f);

		if (!top->is_custom_cfact) {
			for (int j = 0; j < g_cols[i].count; j++)
				g_cols[i].clients[j]->cfact = factor;
		}

		int avail_w = view_w - ((factor <= 0.35f) ? 2 * m->gappih : ((factor < 0.99f) ? m->gappih : 0));
		int w = CLAMP((int)(avail_w * factor), 80, view_w);

		g_cols[i].w = w;
		g_cols[i].x = cur_x;
		cur_x += w + ((i < g_ncol - 1) ? m->gappih : 0);
	}

	return g_ncol;
}

void
trackscroll(const Arg *arg)
{
	int ncol;

	if (!selmon || selmon->lt[selmon->sellt]->arrange != scroller)
		return;

	ncol = build_columns(selmon);
	if (ncol <= 1)
		return;

	int view_w = get_view_geom(selmon, ncol, NULL);
	int total_w = g_cols[ncol - 1].x + g_cols[ncol - 1].w;
	int max_scroll = (total_w > view_w) ? (total_w - view_w) : 0;

	selmon->scroll_x = CLAMP(selmon->scroll_x + arg->i * scrollstep, 0, max_scroll);
	is_scrolling = 1;
	arrangemon(selmon);

	int screen_mid = selmon->scroll_x + view_w / 2;
	int best_col = 0;
	int min_dist = 999999;
	for (int i = 0; i < ncol; i++) {
		int mid = g_cols[i].x + g_cols[i].w / 2;
		int dist = abs(mid - screen_mid);
		if (dist < min_dist) {
			min_dist = dist;
			best_col = i;
		}
	}

	Client *target_win = (g_cols[best_col].count > 0) ? g_cols[best_col].clients[0] : selmon->sel;
	if (target_win) {
		if (target_win != selmon->sel)
			focus(target_win);
		keep_cursor_inside(target_win);
	}
	is_scrolling = 0;
}

void
scrollwin(const Arg *arg)
{
	int ncol, cur_col, cur_row = 0;

	if (!selmon)
		return;

	if (selmon->lt[selmon->sellt]->arrange != scroller) {
		focusstack(arg);
		return;
	}

	ncol = build_columns(selmon);
	if (ncol <= 1)
		return;

	cur_col = find_col_row(g_cols, ncol, selmon->sel, &cur_row);

	if (cur_col == -1) {
		int base_x;
		int view_w = get_view_geom(selmon, ncol, &base_x);
		int px, py;
		if (getrootptr(&px, &py)) {
			int total_w = g_cols[ncol - 1].x + g_cols[ncol - 1].w;
			int offset_x = get_ribbon_offset(total_w, view_w);
			int ribbon_x = px - base_x - offset_x + selmon->scroll_x;
			for (int i = 0; i < ncol; i++) {
				if (ribbon_x >= g_cols[i].x && ribbon_x <= g_cols[i].x + g_cols[i].w + selmon->gappih) {
					cur_col = i;
					break;
				}
			}
		}
	}
	if (cur_col == -1) cur_col = 0;

	int target_col = NAV_WRAP(cur_col, arg->i, ncol, loopnav);
	if (target_col == cur_col)
		return;

	if (g_cols[target_col].count > 0) {
		int target_row = (cur_row < g_cols[target_col].count) ? cur_row : (g_cols[target_col].count - 1);
		Client *target_win = g_cols[target_col].clients[target_row];
		focus(target_win);
		restack(selmon);
		keep_cursor_inside(target_win);
	}
}

void
focuswinstack(const Arg *arg)
{
	int ncol, cur_col, cur_row;

	if (!selmon || !selmon->sel)
		return;

	if (selmon->lt[selmon->sellt]->arrange != scroller) {
		focusstack(arg);
		return;
	}

	ncol = build_columns(selmon);
	cur_col = find_col_row(g_cols, ncol, selmon->sel, &cur_row);

	if (cur_col != -1 && g_cols[cur_col].count > 1) {
		int target_row = NAV_WRAP(cur_row, arg->i, g_cols[cur_col].count, loopnav);
		if (target_row == cur_row)
			return;
		Client *target_win = g_cols[cur_col].clients[target_row];
		focus(target_win);
		keep_cursor_inside(target_win);
	} else if (loopnav) {
		focusstack(arg);
		if (selmon->sel)
			keep_cursor_inside(selmon->sel);
	}
}

void
stackconsume(const Arg *arg)
{
	Client *c, *target_win = NULL;

	if (!selmon || selmon->lt[selmon->sellt]->arrange != scroller || !selmon->sel)
		return;

	c = selmon->sel;
	if (arg->i >= 0) {
		target_win = nexttiled(c->next);
	} else {
		Client *p;
		for (p = selmon->clients; p && p != c; p = p->next)
			if (!p->isfloating && ISVISIBLE(p))
				target_win = p;
	}

	if (!target_win || target_win == c)
		return;

	ENSURE_CFACT(c);
	ENSURE_CFACT(target_win);

	target_win->is_stacked = 1;
	target_win->cfact = c->cfact;
	target_win->is_custom_cfact = c->is_custom_cfact;
	arrangemon(selmon);
	keep_cursor_inside(selmon->sel);
}

void
stackexpel(const Arg *arg)
{
	if (!selmon || selmon->lt[selmon->sellt]->arrange != scroller || !selmon->sel)
		return;

	Client *c = selmon->sel;
	int ncol = build_columns(selmon);
	int col_idx = find_col_row(g_cols, ncol, c, NULL);
	if (col_idx != -1 && g_cols[col_idx].count > 1) {
		unstack_client(c, col_idx);
	} else {
		c->is_stacked = 0;
		c->cfact = (c->unstacked_cfact > 0.05f) ? c->unstacked_cfact : defwinw;
		c->is_custom_cfact = IS_CUSTOM_CFACT(c->cfact);
	}
	arrangemon(selmon);
	keep_cursor_inside(selmon->sel);
}

void
togglestack(const Arg *arg)
{
	if (!selmon || selmon->lt[selmon->sellt]->arrange != scroller || !selmon->sel)
		return;

	int ncol = build_columns(selmon);
	int col_idx = find_col_row(g_cols, ncol, selmon->sel, NULL);
	if (col_idx != -1 && g_cols[col_idx].count > 1)
		stackexpel(arg);
	else
		stackconsume(arg);
}

void
movewin(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c || c->isfloating || selmon->lt[selmon->sellt]->arrange != scroller)
		return;

	int ncol = build_columns(selmon);
	if (ncol == 0)
		return;

	int col_idx = find_col_row(g_cols, ncol, c, NULL);
	if (col_idx == -1 || (ncol <= 1 && g_cols[col_idx].count <= 1))
		return;

	if (!reorder_col_dir(c, g_cols, ncol, col_idx, arg->i, loopnav))
		return;

	centerfocused(NULL);
}

void
movewinstack(const Arg *arg)
{
	Client *c = selmon->sel;
	if (!c || c->isfloating || selmon->lt[selmon->sellt]->arrange != scroller || !arg->i)
		return;

	int ncol = build_columns(selmon);
	if (ncol == 0)
		return;

	int cur_row = -1;
	int col_idx = find_col_row(g_cols, ncol, c, &cur_row);
	if (col_idx == -1 || cur_row == -1)
		return;

	int count = g_cols[col_idx].count;
	int is_down = (arg->i > 0);

	if (count <= 1) {
		if (ncol <= 1)
			return;
		int t_col = (col_idx < ncol - 1) ? (col_idx + 1) : (col_idx - 1);
		int pos = is_down ? StkAfter : StkBefore;
		Client *target_win = is_down ? g_cols[t_col].clients[g_cols[t_col].count - 1]
		                             : g_cols[t_col].clients[0];
		stack_client_target(c, &g_cols[t_col], target_win, pos);
	} else {
		int at_edge = is_down ? (cur_row == count - 1) : (cur_row == 0);
		if (at_edge && !loopnav)
			return;
		int target_idx = at_edge ? (is_down ? 0 : count - 1) : (cur_row + (is_down ? 1 : -1));
		int pos = (at_edge == is_down) ? StkBefore : StkAfter;
		Client *target_win = g_cols[col_idx].clients[target_idx];
		reorderclient(c, target_win, pos);
	}

	arrangemon(selmon);
	keep_cursor_inside(c);
}

void
syncwincfact(Client *c, float f)
{
	int ncol = build_columns(selmon);
	int col_idx = find_col_row(g_cols, ncol, c, NULL);
	if (col_idx != -1) {
		for (int j = 0; j < g_cols[col_idx].count; j++) {
			g_cols[col_idx].clients[j]->cfact = f;
			g_cols[col_idx].clients[j]->is_custom_cfact = 1;
		}
	}
}

void
cyclewinwidth(const Arg *arg)
{
	int ncol, col_idx;

	if (!selmon || selmon->lt[selmon->sellt]->arrange != scroller || !selmon->sel)
		return;

	Client *c = selmon->sel;
	ncol = build_columns(selmon);
	col_idx = find_col_row(g_cols, ncol, c, NULL);

	float cur_factor = (ncol == 1 && !c->is_custom_cfact) ? 1.0f : c->cfact;
	int cur_idx = -1;
	for (int i = 0; i < (int)LENGTH(winsizes); i++) {
		if (cur_factor >= winsizes[i] - 0.04f && cur_factor <= winsizes[i] + 0.04f) {
			cur_idx = i;
			break;
		}
	}
	float next_factor = winsizes[(cur_idx + 1) % LENGTH(winsizes)];

	c->cfact = next_factor;
	c->is_custom_cfact = 1;
	if (col_idx != -1) {
		if (g_cols[col_idx].count == 1)
			c->unstacked_cfact = next_factor;
		for (int j = 0; j < g_cols[col_idx].count; j++) {
			g_cols[col_idx].clients[j]->cfact = next_factor;
			g_cols[col_idx].clients[j]->is_custom_cfact = 1;
		}
	}

	centerfocused(NULL);
}

void
centerfocused(const Arg *arg)
{
	int ncol, sel_col;

	if (!selmon || selmon->lt[selmon->sellt]->arrange != scroller || !selmon->sel)
		return;

	ncol = build_columns(selmon);
	sel_col = find_col_row(g_cols, ncol, selmon->sel, NULL);
	if (sel_col == -1) return;

	int view_w = get_view_geom(selmon, ncol, NULL);
	int center_col = g_cols[sel_col].x + g_cols[sel_col].w / 2;
	int total_w = g_cols[ncol - 1].x + g_cols[ncol - 1].w;
	int max_scroll = (total_w > view_w) ? (total_w - view_w) : 0;

	selmon->scroll_x = CLAMP(center_col - view_w / 2, 0, max_scroll);
	arrangemon(selmon);
	keep_cursor_inside(selmon->sel);
}

void
togglecentering(const Arg *arg)
{
	centermode = (centermode == CenterOnFlow) ? CenterAlways :
	             (centermode == CenterAlways) ? CenterNever : CenterOnFlow;

	if (selmon && selmon->lt[selmon->sellt]->arrange == scroller) {
		arrangemon(selmon);
		keep_cursor_inside(selmon->sel);
	}
}

void
toggleloopnav(const Arg *arg)
{
	loopnav = !loopnav;
}

void
scroller(Monitor *m)
{
	static int in_scroller = 0;
	if (in_scroller)
		return;
	in_scroller = 1;

	int ncol = build_columns(m);
	if (ncol == 0) {
		m->prev_ncol = 0;
		m->prev_sel_col = -1;
		prev_sel_client = NULL;
		prev_tagset = m->tagset[m->seltags];
		in_scroller = 0;
		return;
	}

	unsigned int cur_tagset = m->tagset[m->seltags];

	if (cur_tagset == prev_tagset && prev_sel_client &&
	    find_col_row(g_cols, ncol, prev_sel_client, NULL) == -1) {
		int target_col = CLAMP(m->prev_sel_col >= 0 ? m->prev_sel_col : 0, 0, ncol - 1);
		if (g_cols[target_col].count > 0) {
			Client *target_win = g_cols[target_col].clients[0];
			if (m->sel != target_win) {
				focus(target_win);
				restack(m);
				keep_cursor_inside(target_win);
			}
			if (target_col == 0)
				m->scroll_x = 0;
			m->prev_sel_col = target_col;
		}
	}
	prev_tagset = cur_tagset;

	if (m->prev_ncol > 1 && ncol == 1) {
		for (int j = 0; j < g_cols[0].count; j++) {
			g_cols[0].clients[j]->is_custom_cfact = 0;
			g_cols[0].clients[j]->cfact = swinw;
		}
	}
	m->prev_ncol = ncol;

	int base_x;
	int view_w = get_view_geom(m, ncol, &base_x);

	if (ncol == 1) {
		Client *top = g_cols[0].clients[0];
		int col_w = top->is_custom_cfact ? g_cols[0].w : (int)(view_w * swinw);
		int col_x = m->wx + m->gappoh + ((centerswin || centermode == CenterAlways) ? (view_w - col_w) / 2 : 0);

		scroller_arrange_col(m, &g_cols[0], col_x, col_w);
		m->scroll_x = 0;
		m->prev_sel_col = 0;
		prev_sel_client = m->sel;
		XSync(dpy, False);
		in_scroller = 0;
		return;
	}

	int total_w = g_cols[ncol - 1].x + g_cols[ncol - 1].w;
	int max_scroll = MAX(0, total_w - view_w);
	int sel_col = find_col_row(g_cols, ncol, m->sel, NULL);

	if (!is_scrolling && sel_col != -1) {
		int col_screen_x = g_cols[sel_col].x - m->scroll_x;
		int col_w = g_cols[sel_col].w;

		if (centermode == CenterAlways) {
			m->scroll_x = (g_cols[sel_col].x + col_w / 2) - (view_w / 2);
		} else if (centermode == CenterOnFlow) {
			if (m->prev_sel_col >= 0 && m->prev_sel_col < ncol && m->prev_sel_col != sel_col) {
				int prev_x = g_cols[m->prev_sel_col].x;
				int prev_w = g_cols[m->prev_sel_col].w;
				int pair_span = MAX(prev_x + prev_w, g_cols[sel_col].x + col_w) - MIN(prev_x, g_cols[sel_col].x);

				if (pair_span > view_w + 1)
					m->scroll_x = (g_cols[sel_col].x + col_w / 2) - (view_w / 2);
				else if (col_screen_x < 0)
					m->scroll_x = g_cols[sel_col].x;
				else if (col_screen_x + col_w > view_w)
					m->scroll_x = g_cols[sel_col].x + col_w - view_w;
			} else {
				if (col_screen_x < 0)
					m->scroll_x = g_cols[sel_col].x;
				else if (col_screen_x + col_w > view_w)
					m->scroll_x = g_cols[sel_col].x + col_w - view_w;
			}
		} else if (centermode == CenterNever) {
			if (col_screen_x < 0)
				m->scroll_x = g_cols[sel_col].x;
			else if (col_screen_x + col_w > view_w)
				m->scroll_x = g_cols[sel_col].x + col_w - view_w;
		}
		m->prev_sel_col = sel_col;
	}

	m->scroll_x = CLAMP(m->scroll_x, 0, max_scroll);
	int offset_x = get_ribbon_offset(total_w, view_w);

	for (int i = 0; i < ncol; i++) {
		int col_x = base_x + offset_x + g_cols[i].x - m->scroll_x;
		scroller_arrange_col(m, &g_cols[i], col_x, g_cols[i].w);
	}
	prev_sel_client = m->sel;
	XSync(dpy, False);
	in_scroller = 0;
}

static void
detach_unstack(Client *c)
{
	Client **tc, *nxt;

	if (!c)
		return;

	for (nxt = c->next; nxt; nxt = nxt->next) {
		if (!nxt->isfloating && (nxt->tags & c->tags)) {
			if (!c->is_stacked && nxt->is_stacked)
				nxt->is_stacked = 0;
			break;
		}
	}

	for (tc = &selmon->clients; *tc && *tc != c; tc = &(*tc)->next);
	if (*tc)
		*tc = c->next;
}

static void
reorderclient(Client *c, Client *target, int pos)
{
	Client **tc;

	if (!c || !target || c == target)
		return;

	detach_unstack(c);

	if (pos == InsBefore || pos == StkBefore) {
		for (tc = &selmon->clients; *tc && *tc != target; tc = &(*tc)->next);
		c->next = target;
		*tc = c;
		c->is_stacked = (pos == StkBefore) ? target->is_stacked : 0;
		if (pos == StkBefore)
			target->is_stacked = 1;
	} else {
		c->next = target->next;
		target->next = c;
		c->is_stacked = (pos == StkAfter);
	}
}

static void
stack_into_column(Client *c, Column *col, int drop_y)
{
	if (!c || !col || col->count == 0)
		return;

	Client *stack_target = NULL;
	int pos = StkAfter;

	for (int j = 0; j < col->count; j++) {
		Client *tj = col->clients[j];
		int win_mid_y = tj->y + tj->h / 2;
		if (drop_y < win_mid_y) {
			stack_target = tj;
			pos = StkBefore;
			break;
		}
		stack_target = tj;
		pos = StkAfter;
	}

	if (stack_target && stack_target != c)
		stack_client_target(c, col, stack_target, pos);
}

void
dragdrop(const Arg *arg)
{
	int x, y, ocx, ocy, nx, ny;
	Client *c;
	XEvent ev;
	Time lasttime = 0;

	if (!(c = selmon->sel))
		return;

	if ((!isov && selmon->lt[selmon->sellt]->arrange != scroller) || c->isfloating || c->isfullscreen) {
		movemouse(arg);
		return;
	}

	ocx = c->x;
	ocy = c->y;
	if (XGrabPointer(dpy, root, False, MOUSEMASK, GrabModeAsync, GrabModeAsync,
		None, cursor[CurMove]->cursor, CurrentTime) != GrabSuccess)
		return;

	if (!getrootptr(&x, &y))
		return;

	XRaiseWindow(dpy, c->win);
	is_scrolling = 1;
	is_dragging = 1;

	int ncol = build_columns(selmon);
	int view_w = get_view_geom(selmon, ncol, NULL);
	int total_w = (ncol > 0) ? (g_cols[ncol - 1].x + g_cols[ncol - 1].w) : 0;
	int max_scroll = MAX(0, total_w - view_w);

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

			nx = ocx + (ev.xmotion.x_root - x);
			ny = ocy + (ev.xmotion.y_root - y);
			XMoveWindow(dpy, c->win, nx, ny);

			if (isov) {
				int scrolled = 0;

				if (ev.xmotion.y_root < selmon->wy + ovedgezone) {
					ovscrollvert_px(-ovedgestep);
					scrolled = 1;
				} else if (ev.xmotion.y_root > selmon->wy + selmon->wh - ovedgezone) {
					ovscrollvert_px(+ovedgestep);
					scrolled = 1;
				}

				int layer_tag = getovlayertag(ev.xmotion.x_root, ev.xmotion.y_root);
				if (ev.xmotion.x_root < selmon->wx + ovedgezone) {
					ovscrollribbon_px(layer_tag, -ovedgestep);
					scrolled = 1;
				} else if (ev.xmotion.x_root > selmon->wx + selmon->ww - ovedgezone) {
					ovscrollribbon_px(layer_tag, +ovedgestep);
					scrolled = 1;
				}

				if (scrolled) {
					arrangemon(selmon);
					XMoveWindow(dpy, c->win, nx, ny);
				}
			} else if (ncol > 1) {
				int prev_sx = selmon->scroll_x;
				if (ev.xmotion.x_root < selmon->wx + 80)
					selmon->scroll_x = MAX(0, selmon->scroll_x - 20);
				else if (ev.xmotion.x_root > selmon->wx + selmon->ww - 80)
					selmon->scroll_x = MIN(max_scroll, selmon->scroll_x + 20);
				if (selmon->scroll_x != prev_sx) {
					arrangemon(selmon);
					XMoveWindow(dpy, c->win, nx, ny);
				}
			}
			break;
		}
	} while (ev.type != ButtonRelease);

	is_scrolling = 0;
	is_dragging = 0;
	XUngrabPointer(dpy, CurrentTime);
	while (XCheckMaskEvent(dpy, EnterWindowMask, &ev));

	int drop_x = ev.xbutton.x_root;
	int drop_y = ev.xbutton.y_root;

	if (abs(drop_x - x) < (int)snap && abs(drop_y - y) < (int)snap) {
		if (isov) {
			Arg a = {.ui = c->tags};
			view(&a);
			focus(c);
			restack(selmon);
			return;
		}
		finish_drag_client(c);
		return;
	}

	if (isov) {
		ovdrop(c, drop_x, drop_y);
		return;
	}

	ncol = build_columns(selmon);
	if (ncol > 0) {
		int c_col = find_col_row(g_cols, ncol, c, NULL);
		int base_x;
		int vw = get_view_geom(selmon, ncol, &base_x);
		int tot_w = g_cols[ncol - 1].x + g_cols[ncol - 1].w;
		int ribbon_drop_x = drop_x - base_x - get_ribbon_offset(tot_w, vw) + selmon->scroll_x;

		if (ribbon_drop_x < g_cols[0].x) {
			unstack_to_edge(c, g_cols, c_col, 0, InsBefore);
		} else if (ribbon_drop_x > g_cols[ncol - 1].x + g_cols[ncol - 1].w) {
			unstack_to_edge(c, g_cols, c_col, ncol - 1, InsAfter);
		} else {
			for (int i = 0; i < ncol; i++) {
				int col_left = g_cols[i].x;
				int col_right = g_cols[i].x + g_cols[i].w;
				if (ribbon_drop_x >= col_left && ribbon_drop_x <= col_right + selmon->gappih) {
					int edge_zone = CLAMP((int)(g_cols[i].w * 0.18), 25, 90);
					if (ribbon_drop_x < col_left + edge_zone)
						unstack_to_edge(c, g_cols, c_col, i, InsBefore);
					else if (ribbon_drop_x > col_right - edge_zone)
						unstack_to_edge(c, g_cols, c_col, i, InsAfter);
					else
						stack_into_column(c, &g_cols[i], drop_y);
					break;
				}
			}
		}
	}

	finish_drag_client(c);
}
