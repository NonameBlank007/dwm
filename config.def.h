/* See LICENSE file for copyright and license details. */

/* appearance */
static const unsigned int borderpx  = 2;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int gappih    = 20;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 10;       /* vert inner gap between windows */
static const unsigned int gappoh    = 10;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 30;       /* vert outer gap between windows and screen edge */
static       int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */
static const int swallowfloating    = 0;        /* 1 means swallow floating windows by default */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = {"FiraCodeNerdFont:style=Regular:size=12:antialias=true:autohint=true:hintstyle=hintfull", "JetBrainsMono Nerd Font:style=Regular:size=12:antialias=true:autohint=true:hintstyle=hintfull", "NotoColorEmoji:pixelsize=14:antialias=true:autohint=true"};
static const char dmenufont[]       = "JetBrainsMono Nerd Font:size=12";
static unsigned int baralpha        = 0xd0;
static unsigned int borderalpha     = OPAQUE;
static char normbgcolor[]           = "#222222";
static char normbordercolor[]       = "#444444";
static char normfgcolor[]           = "#bbbbbb";
static char selfgcolor[]            = "#eeeeee";
static char selbordercolor[]        = "#005577";
static char selbgcolor[]            = "#005577";
static char *colors[][3] = {
    /*               fg           bg           border   */
    [SchemeNorm] = { normfgcolor, normbgcolor, normbordercolor },
    [SchemeSel]  = { selfgcolor,  selbgcolor,  selbordercolor  },
};

static const char *const autostart[] = {
	/* reload prev color scheme */
    "wal", "-R", NULL,
    
    /* start in background without animation */
    "picom", "-b", NULL,
    
    /* notification daemon */
    "dunst", "&", NULL,

    /* bluetooth manager */
    "blueman-applet", NULL,

    /* network manager */
    "nm-applet", "&", NULL,

    /* start sxhkd */
    "sxhkd", NULL,

    /* lockscreen */
    "xss-lock", "--", "slock", NULL,

    /* unclutter-xfixes*/
    "unclutter --timeout 2 --jitter 5 --ignore-scrolling --exclude-root --fork", NULL,
    
    /* auth handler */ 
    "/usr/lib/polkit-mate/polkit-mate-authentication-agent-1", "&", NULL,

    /* Input Gesture */
    "libinput-gestures-setup", "autostart",  "start", NULL, 

	/* xrdb key press  */
    "sh", "-c", "sleep 2 && xdotool key alt+F5", NULL,

	NULL /* terminate */
};

/* tagging */
static const char *tags[] = { " ", "󰻀", "", "", "", "󰘻", "󱍙", "", "" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal isfullscreen noswallow  monitor */
	{ "Gimp",    NULL,     NULL,            0,         1,          0,         0,           0,        -1 },
	{ "Firefox", NULL,     NULL,            1 << 8,    0,          0,         0,          -1,        -1 },
    { "kitty",   NULL,     NULL,            0,         0,          1,         0,           0,        -1 },
	{ "St",      NULL,     NULL,            0,         0,          1,         0,           0,        -1 },
    { NULL,      NULL,     "KDE Connect Daemon",0,     1,          0,         1,          -1,        -1 },
    { NULL,      NULL,     "Wall-qs",       0,         1,          0,         1,          -1,        -1 },
	{ NULL,      NULL,     "Event Tester",  0,         0,          0,         0,           1,        -1 }, /* xev */
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 120;  /* refresh rate (per second) for client move/resize */
static const int repeat_rate = 25;   /* keyboard key repeat rate (per second) */
static const int repeat_delay = 200; /* keyboard key repeat delay (per milisecond) */

#define FORCE_VSPLIT 1  /* nrowgrid layout: force two clients to always split vertically */
#include "vanitygaps.c"

/* scroller layout settings */
static int centermode                = CenterOnFlow; /* CenterOnFlow, CenterAlways, CenterNever */
static const float winsizes[]        = { 0.33333f, 0.50f, 0.66667f, 1.0f }; /* window width presets [0.1..1.0] */
static const float defwinw           = 0.50f; /* default width factor for multi-window ribbons [0.1..1.0] */
static const float swinw             = 0.50f; /* default width factor for single window or stack [0.1..1.0] */
static const int centerswin          = 0;     /* 1 means always center single window */
static const int scrollstep          = 45;    /* trackpad and mouse wheel scroll step in pixels */
static const int strutleft           = 55;    /* pixels reserved on left edge for focus jump (0 to disable) */
static const int strutright          = 55;    /* pixels reserved on right edge for focus jump (0 to disable) */
static int loopnav                   = 1;     /* 1 means loop navigation */

/* overview layout settings */
static const int ovgappov            = 16;    /* vert outer padding between overview area and screen edge */
static const int ovgappoh            = 24;    /* horiz outer padding between overview ribbons and screen edge */
static const int ovlayergap          = 14;    /* vert gap between workspace layers */
static const int ovinpady            = 6;     /* vert inner padding inside workspace layer */
static const int ovgappih            = 8;     /* horiz gap between miniature windows */
static const int ovgappiv            = 4;     /* vert gap between stacked window panes */
static const float ovsingleh         = 0.45f; /* single layer window height factor */
static const int ovuniformlayer      = 1;     /* 0 means scale to fit ovmaxfit */
static const int ovmaxfit            = 4;     /* max visible workspace layers before vertical scrolling */
static const int ovedgezone          = 80;    /* edge distance in pixels triggering auto-scroll during drag */
static const int ovedgestep          = 25;    /* auto-scroll step in pixels per motion event during drag */
static int ovloopnav                 = 1;     /* 1 means loop navigation */

#include "scrollview.c"

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "󰣇 ",       tile },    /* first entry is default */
	{ "[|||]",    scroller },
	{ "[M]",      monocle },
	{ "[@]",      spiral },
	{ "[\\]",     dwindle },
	{ "H[]",      deck },
	{ "TTT",      bstack },
	{ "===",      bstackhoriz },
	{ "HHH",      grid },
	{ "###",      nrowgrid },
	{ "---",      horizgrid },
	{ ":::",      gaplessgrid },
	{ "|M|",      centeredmaster },
	{ ">M>",      centeredfloatingmaster },
	{ "[⊞]",      overview },
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ NULL,       NULL },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ Normal,   MODKEY,                       KEY,      view,             {.ui = 1 << TAG} }, \
	{ Normal,   MODKEY|ControlMask,           KEY,      toggleview,       {.ui = 1 << TAG} }, \
	{ Normal,   MODKEY|ShiftMask,             KEY,      tag,              {.ui = 1 << TAG} }, \
	{ Normal,   MODKEY|ControlMask|ShiftMask, KEY,      toggletag,        {.ui = 1 << TAG} }, \
	{ Overview, 0,                            KEY,      ovjumptag,  {.ui = TAG} }, \
	{ Overview, MODKEY,                       KEY,      ovjumptag,  {.ui = TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbordercolor, "-sf", selfgcolor, NULL };

/* system, app control */ 
static const char *lock[]     = { "slock", NULL};
static const char *termcmd[]  = { "kitty", NULL };
static const char *browser[]  = { "brave", NULL};
static const char *manager[]  = { "dolphin", NULL};

/* night light control */
static const char *night_on[] = {"sh", "-c", "redshift -O 4500", NULL};
static const char *night_off[] = {"sh", "-c", "redshift -x", NULL};

/* Volume Control */
static const char *volup[]     = { "wpctl",   "set-volume", "@DEFAULT_AUDIO_SINK@",      "5%+",      NULL };
static const char *voldown[]   = { "wpctl",   "set-volume", "@DEFAULT_AUDIO_SINK@",      "5%-",      NULL };
static const char *mutevol[]   = { "wpctl",   "set-mute",   "@DEFAULT_AUDIO_SINK@",      "toggle",   NULL };

/* Brightness Control */
static const char *bright[]    = { "brightnessctl", "set", "+10%", NULL };
static const char *dim[]       = { "brightnessctl", "set", "10%-", NULL }; 

/* custom */
static const char *rofi[] = {"sh", "-c", ".config/rofi/launchers/launcher.sh", NULL};
static const char *powermenu[] = {"sh", "-c", ".config/rofi/powermenu/powermenu.sh", NULL};

#include "movestack.c"

#include "overview.c"

static const Key keys[] = {
	/* mode     modifier                      key                        function        argument */
	/* application launcher */
	{ Normal,   ALTKEY,                       XK_e,                      spawn,          {.v = manager } },
	{ Normal,   ALTKEY,                       XK_f,                      spawn,          {.v = browser } },
	{ Normal,   MODKEY|ShiftMask,             XK_Return,                 spawn,          {.v = termcmd } },
	{ Normal,   ALTKEY|ShiftMask,             XK_a,                      spawn,          {.v = rofi } },

	/* system general control */
	{ All,      0,                            XF86XK_AudioRaiseVolume,   spawn,          {.v = volup } },
	{ All,      0,                            XF86XK_AudioLowerVolume,   spawn,          {.v = voldown } },
	{ All,      0,                            XF86XK_AudioMute,          spawn,          {.v = mutevol } },
	{ All,      0,                            XF86XK_MonBrightnessUp,    spawn,          {.v = bright } },
	{ All,      0,                            XF86XK_MonBrightnessDown,  spawn,          {.v = dim } },
	{ All,      0,                            XK_F12,                    spawn,          {.v = lock} }, /* lock screen */
	{ All,      ControlMask,                  XK_l,                      spawn,          {.v = lock} }, /* lock screen */
	{ Normal,   ALTKEY|ControlMask,           XK_z,                      spawn,          {.v = night_on } },
	{ Normal,   ALTKEY|ControlMask,           XK_x,                      spawn,          {.v = night_off } },

	/* wm management */
	{ All,      MODKEY,                       XK_Tab,                    toggleov,       {0} },
	{ All,      MODKEY,                       XK_o,                      toggleov,       {0} },
	{ Normal,   MODKEY,                       XK_grave,                  view,           {0} },
	{ Normal,   MODKEY,                       XK_space,                  togglefloating, {0} },
	{ Normal,   MODKEY,                       XK_b,                      togglebar,      {0} },
	{ Normal,   MODKEY|ShiftMask,             XK_space,                  setlayout,      {0} },
	{ Normal,   MODKEY|ShiftMask,             XK_f,                      togglefullscr,  {0} },

	/* layout control */
	{ Normal,   MODKEY,                       XK_t,                      setlayout,      {.v = &layouts[0]} }, /* tile (default) */
	{ Normal,   MODKEY,                       XK_s,                      setlayout,      {.v = &layouts[1]} }, /* scroller */
	{ Normal,   MODKEY,                       XK_m,                      setlayout,      {.v = &layouts[2]} }, /* monocle */
	{ Normal,   MODKEY,                       XK_g,                      setlayout,      {.v = &layouts[8]} }, /* grid */
	{ Normal,   MODKEY,                       XK_f,                      setlayout,      {.v = &layouts[15]} }, /* floating */
	{ Scroll,   MODKEY,                       XK_Left,                   scrollwin,      {.i = -1} },
	{ Scroll,   MODKEY,                       XK_Right,                  scrollwin,      {.i = +1} },
	{ Scroll,   MODKEY,                       XK_Up,                     focuswinstack,  {.i = -1} },
	{ Scroll,   MODKEY,                       XK_Down,                   focuswinstack,  {.i = +1} },
	{ Scroll,   MODKEY,                       XK_w,                      togglestack,    {0} },
	{ Scroll,   MODKEY,                       XK_r,                      cyclewinwidth,  {0} },
	{ Scroll,   MODKEY,                       XK_c,                      centerfocused,  {0} },
	{ Scroll,   MODKEY|ShiftMask,             XK_c,                      togglecentering,{0} },
	{ Scroll,   MODKEY|ShiftMask,             XK_Left,                   movewin,        {.i = -1} },
	{ Scroll,   MODKEY|ShiftMask,             XK_Right,                  movewin,        {.i = +1} },
	{ Scroll,   MODKEY|ShiftMask,             XK_Up,                     movewinstack,   {.i = -1} },
	{ Scroll,   MODKEY|ShiftMask,             XK_Down,                   movewinstack,   {.i = +1} },
	{ Scroll,   MODKEY|ControlMask,           XK_n,                      toggleloopnav,  {0} },
	{ Normal,   MODKEY,                       XK_i,                      incnmaster,     {.i = +1 } },
	{ Normal,   MODKEY,                       XK_u,                      incnmaster,     {.i = -1 } },
	{ Normal,   ALTKEY|ControlMask,           XK_comma,                  cyclelayout,    {.i = -1 } },
	{ Normal,   ALTKEY|ControlMask,           XK_period,                 cyclelayout,    {.i = +1 } },

	/* window focus */
	{ Normal,   MODKEY,                       XK_Return,                 zoom,           {0} },
	{ Normal,   MODKEY,                       XK_h,                      focusstack,     {.i = -1 } },
	{ Normal,   MODKEY,                       XK_l,                      focusstack,     {.i = +1 } },
	{ Normal,   MODKEY,                       XK_j,                      focusmon,       {.i = -1 } },
	{ Normal,   MODKEY,                       XK_k,                      focusmon,       {.i = +1 } },
    
	/* monitor tagging */
	{ Normal,   MODKEY|ControlMask,           XK_j,                      tagmon,         {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_k,                      tagmon,         {.i = +1 } },

	/* move windows */
	{ Normal,   MODKEY|ShiftMask,             XK_j,                      movestack,      {.i = +1 } },
	{ Normal,   MODKEY|ShiftMask,             XK_k,                      movestack,      {.i = -1 } },

	/* Window sizing */
	{ Normal,   MODKEY|ControlMask,           XK_h,                      setmfact,       {.f = -0.05} },
	{ Normal,   MODKEY|ControlMask,           XK_l,                      setmfact,       {.f = +0.05} },
	{ Normal,   MODKEY|ShiftMask,             XK_h,                      setcfact,       {.f = +0.25} },
	{ Normal,   MODKEY|ShiftMask,             XK_l,                      setcfact,       {.f = -0.25} },
	{ Normal,   MODKEY|ShiftMask,             XK_o,                      setcfact,       {.f =  0.00} },

	/* gaps control */ 
	{ Normal,   MODKEY|ControlMask,           XK_u,                      incrgaps,       {.i = +1 } },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_u,                      incrgaps,       {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_i,                      incrigaps,      {.i = +1 } },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_i,                      incrigaps,      {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_o,                      incrogaps,      {.i = +1 } },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_o,                      incrogaps,      {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_6,                      incrihgaps,     {.i = +1 } },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_6,                      incrihgaps,     {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_7,                      incrivgaps,     {.i = +1 } },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_7,                      incrivgaps,     {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_8,                      incrohgaps,     {.i = +1 } },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_8,                      incrohgaps,     {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_9,                      incrovgaps,     {.i = +1 } },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_9,                      incrovgaps,     {.i = -1 } },
	{ Normal,   MODKEY|ControlMask,           XK_0,                      togglegaps,     {0} },
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_0,                      defaultgaps,    {0} },

	/* tags */
	{ Normal,   MODKEY,                       XK_0,                      view,           {.ui = ~0 } },
	{ Normal,   MODKEY|ShiftMask,             XK_0,                      tag,            {.ui = ~0 } },
	TAGKEYS(                                  XK_1,                                      0)
	TAGKEYS(                                  XK_2,                                      1)
	TAGKEYS(                                  XK_3,                                      2)
	TAGKEYS(                                  XK_4,                                      3)
	TAGKEYS(                                  XK_5,                                      4)
	TAGKEYS(                                  XK_6,                                      5)
	TAGKEYS(                                  XK_7,                                      6)
	TAGKEYS(                                  XK_8,                                      7)
	TAGKEYS(                                  XK_9,                                      8)

	/* system */
	{ Normal,   MODKEY,                       XK_q,                      killclient,     {0} },
	{ Normal,   MODKEY|ShiftMask,             XK_q,                      quit,           {0} }, /* quit dwm */
	{ Normal,   MODKEY|ControlMask|ShiftMask, XK_q,                      quit,           {1} }, /* refresh (restartsig) */
	{ Normal,   ALTKEY,                       XK_q,                      spawn,          {.v = powermenu} },
	{ Normal,   ALTKEY,                       XK_F5,                     xrdb,           {.v = NULL } }, 

	/* custom */
	{ All,      0,                            XK_Print,                  spawn,          SHCMD("flameshot gui") },
	{ Normal,   MODKEY|ShiftMask,             XK_s,                      spawn,          SHCMD("flameshot gui") },
	{ Normal,   ALTKEY,                       XK_w,                      spawn,          SHCMD("~/.scripts/wallshuf.sh") },
	{ Normal,   ALTKEY|ShiftMask,             XK_w,                      spawn,          SHCMD("qs -p .config/quickshell/launcher/wallpaper/shell.qml") },

	/* overview */
	{ Overview, 0,                            XK_Escape,                 toggleov,       {0} },
	{ Overview, 0,                            XK_q,                      toggleov,       {0} },
	{ Overview, MODKEY,                       XK_q,                      toggleov,       {0} },
	{ Overview, 0,                            XK_Return,                 zoomov,         {0} },
	{ Overview, MODKEY,                       XK_Return,                 zoomov,         {0} },
	{ Overview, 0,                            XK_space,                  zoomov,         {0} },
	{ Overview, 0,                            XK_Up,                     focusovlayer,   {.i = -1} },
	{ Overview, 0,                            XK_k,                      focusovlayer,   {.i = -1} },
	{ Overview, 0,                            XK_Down,                   focusovlayer,   {.i = +1} },
	{ Overview, 0,                            XK_j,                      focusovlayer,   {.i = +1} },
	{ Overview, 0,                            XK_Left,                   focusovwin,     {.i = -1} },
	{ Overview, 0,                            XK_h,                      focusovwin,     {.i = -1} },
	{ Overview, 0,                            XK_Right,                  focusovwin,     {.i = +1} },
	{ Overview, 0,                            XK_l,                      focusovwin,     {.i = +1} },
	{ Overview, MODKEY|ShiftMask,             XK_Left,                   ovmovewin,      {.i = -1} },
	{ Overview, MODKEY|ShiftMask,             XK_h,                      ovmovewin,      {.i = -1} },
	{ Overview, MODKEY|ShiftMask,             XK_Right,                  ovmovewin,      {.i = +1} },
	{ Overview, MODKEY|ShiftMask,             XK_l,                      ovmovewin,      {.i = +1} },
	{ Overview, MODKEY|ShiftMask,             XK_Up,                     ovmovewinlayer, {.i = -1} },
	{ Overview, MODKEY|ShiftMask,             XK_k,                      ovmovewinlayer, {.i = -1} },
	{ Overview, MODKEY|ShiftMask,             XK_Down,                   ovmovewinlayer, {.i = +1} },
	{ Overview, MODKEY|ShiftMask,             XK_j,                      ovmovewinlayer, {.i = +1} },
	{ Overview, 0,                            XK_n,                      toggleovloopnav,{0} },
	{ Overview, MODKEY,                       XK_n,                      toggleovloopnav,{0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* mode         click           event mask      button          function        argument */
	{ Normal,     ClkLtSymbol,    0,              Button1,        setlayout,      {0} },
	{ Normal,     ClkLtSymbol,    0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ Normal,     ClkWinTitle,    0,              Button2,        zoom,           {0} },
	{ Normal,     ClkStatusText,  0,              Button1,        sigstatusbar,   {.i = 1} },
	{ Normal,     ClkStatusText,  0,              Button2,        sigstatusbar,   {.i = 2} },
	{ Normal,     ClkStatusText,  0,              Button3,        sigstatusbar,   {.i = 3} },
	{ Normal,     ClkStatusText,  0,              Button4,        sigstatusbar,   {.i = 4} },
	{ Normal,     ClkStatusText,  0,              Button5,        sigstatusbar,   {.i = 5} },
	{ All,        ClkClientWin,   MODKEY,         Button1,        dragdrop,       {0} },
	{ Normal,     ClkClientWin,   MODKEY,         Button2,        togglefloating, {0} },
	{ All,        ClkClientWin,   MODKEY,         Button3,        resizemouse,    {0} },
	{ Scroll,     ClkClientWin,   MODKEY,         Button4,        scrollwin,      {.i = -1} },
	{ Scroll,     ClkClientWin,   MODKEY,         Button5,        scrollwin,      {.i = +1} },
	{ Scroll,     ClkRootWin,     MODKEY,         Button4,        scrollwin,      {.i = -1} },
	{ Scroll,     ClkRootWin,     MODKEY,         Button5,        scrollwin,      {.i = +1} },
	{ Scroll,     ClkClientWin,   MODKEY,         6,              trackscroll,    {.i = -1} },
	{ Scroll,     ClkClientWin,   MODKEY,         7,              trackscroll,    {.i = +1} },
	{ Scroll,     ClkRootWin,     MODKEY,         6,              trackscroll,    {.i = -1} },
	{ Scroll,     ClkRootWin,     MODKEY,         7,              trackscroll,    {.i = +1} },
	{ Normal,     ClkTagBar,      0,              Button1,        view,           {0} },
	{ Normal,     ClkTagBar,      0,              Button3,        toggleview,     {0} },
	{ Normal,     ClkTagBar,      MODKEY,         Button1,        tag,            {0} },
	{ Normal,     ClkTagBar,      MODKEY,         Button3,        toggletag,      {0} },
	{ Overview,   ClkClientWin,   0,              Button4,        ovscrollvert,   {.i = -1} },
	{ Overview,   ClkClientWin,   0,              Button5,        ovscrollvert,   {.i = +1} },
	{ Overview,   ClkRootWin,     0,              Button4,        ovscrollvert,   {.i = -1} },
	{ Overview,   ClkRootWin,     0,              Button5,        ovscrollvert,   {.i = +1} },
	{ Overview,   ClkClientWin,   ShiftMask,      Button4,        ovscrollribbon, {.i = -1} },
	{ Overview,   ClkClientWin,   ShiftMask,      Button5,        ovscrollribbon, {.i = +1} },
	{ Overview,   ClkRootWin,     ShiftMask,      Button4,        ovscrollribbon, {.i = -1} },
	{ Overview,   ClkRootWin,     ShiftMask,      Button5,        ovscrollribbon, {.i = +1} },
	{ Overview,   ClkClientWin,   0,              6,              ovscrollribbon, {.i = -1} },
	{ Overview,   ClkClientWin,   0,              7,              ovscrollribbon, {.i = +1} },
	{ Overview,   ClkRootWin,     0,              6,              ovscrollribbon, {.i = -1} },
	{ Overview,   ClkRootWin,     0,              7,              ovscrollribbon, {.i = +1} },
};

