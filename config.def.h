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
static const int usealtbar          = 1;        /* 1 means use non-dwm status bar */
static const char *altbarclass      = "Polybar"; /* Alternate bar class name */
static const char *alttrayname      = "tray";    /* Polybar tray instance name */
static const char *altbarcmd        = "$HOME/.scripts/bar.sh"; /* Alternate bar launch command */
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

    /* network manager */
    "nm-applet", "&", NULL,

    /* start sxhkd */
    "sxhkd", NULL,
    
    /* auth handler */ 
    "/usr/lib/polkit-gnome/polkit-gnome-authentication-agent-1", "&", NULL,

    /* Input Gesture */
    "libinput-gestures-setup", "autostart",  "start", NULL, 

	/* xrdb key press and local session launch */
    "sh", "-c", "sleep 2 && xdotool key alt+F5 && runsvdir ~/.runit/runsvdir", "&",  NULL,

	NULL /* terminate */
};

/* tagging */
static const char *tags[] = { " ", "󰻀", "󰈹", "", "", "󰘻", "󱍙", "", "" };

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class     instance  title           tags mask  isfloating  isterminal  noswallow  monitor */
	{ "Gimp",    NULL,     NULL,           0,         1,          0,           0,        -1 },
	{ "Firefox", NULL,     NULL,           0,         0,          0,          -1,        -1 },
    { "kitty",   NULL,     NULL,           0,         0,          1,           0,        -1 },
	{ "St",      NULL,     NULL,           0,         0,          1,           0,        -1 },
	{ NULL,      NULL,     "Event Tester", 0,         0,          0,           1,        -1 }, /* xev */
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

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "󰣇 ",       tile },    /* first entry is default */
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
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ NULL,       NULL },
};

/* key definitions */
#define MODKEY Mod4Mask
#define ALTKEY Mod1Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-fn", dmenufont, "-nb", normbgcolor, "-nf", normfgcolor, "-sb", selbordercolor, "-sf", selfgcolor, NULL };

/* system, app control */ 
static const char *lock[]     = { "slock", NULL};
static const char *termcmd[]  = { "kitty", NULL };
static const char *browser[]  = { "firefox", NULL};
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
static const char *screenshot[] = {"sh", "-c", ".config/rofi/applets/bin/screenshot.sh", NULL};
static const char *mpd[] = {"sh", "-c", ".config/rofi/applets/bin/mpd.sh", NULL};
static const char *rofi[] = {"sh", "-c", ".config/rofi/launchers/type-7/launcher.sh", NULL};
static const char *powermenu[] = {"sh", "-c", ".config/rofi/powermenu/type-6/powermenu.sh", NULL};

#include "movestack.c"
static const Key keys[] = {
	/* modifier                     key        function        argument */
	/* application launcher */
    { ALTKEY,                       XK_e,      spawn,          {.v = manager } },
    { ALTKEY,                       XK_f,      spawn,          {.v = browser } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
    { ALTKEY|ShiftMask,             XK_a,      spawn,          {.v = rofi } },
    { ALTKEY|ShiftMask,             XK_m,      spawn,          {.v = mpd } },

	/* system general control */
    { 0,                            XF86XK_AudioRaiseVolume,  spawn, {.v = volup } },
    { 0,                            XF86XK_AudioLowerVolume,  spawn, {.v = voldown } },
    { 0,                            XF86XK_AudioMute,         spawn, {.v = mutevol } },
    { 0,                            XF86XK_MonBrightnessUp,   spawn, {.v = bright } },
    { 0,                            XF86XK_MonBrightnessDown, spawn, {.v = dim } },
	{ 0,                  			XK_F12,      			  spawn, {.v = lock}}, /* lock screen */
	{ ControlMask,                  XK_l,      			  	  spawn, {.v = lock}}, /* lock screen */
    { ALTKEY|ControlMask,           XK_z,                     spawn, {.v = night_on } },
    { ALTKEY|ControlMask,           XK_x,                     spawn, {.v = night_off } },

	/* wm management */
    { MODKEY,                       XK_Tab,    view,           {0} },
    { MODKEY,                       XK_space,  togglefloating, {0} },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
    { MODKEY|ShiftMask,             XK_space,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },

	/* layout control */
    { MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} }, /* default */
    { MODKEY,                       XK_m,      setlayout,      {.v = &layouts[1]} }, /* monocle */
    { MODKEY,                       XK_g,      setlayout,      {.v = &layouts[7]} }, /* grid */
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[13]} }, /* floating */
    { MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
    { MODKEY,                       XK_u,      incnmaster,     {.i = -1 } },
	{ ALTKEY|ControlMask,           XK_comma,  cyclelayout,    {.i = -1 } },
    { ALTKEY|ControlMask,           XK_period, cyclelayout,    {.i = +1 } },

	/* window focus */
	{ MODKEY,                       XK_Return, zoom,           {0} },
    { MODKEY,                       XK_h,      focusstack,     {.i = -1 } },
    { MODKEY,                       XK_l,      focusstack,     {.i = +1 } },
    { MODKEY,                       XK_j,      focusmon,       {.i = -1 } },
    { MODKEY,                       XK_k,      focusmon,       {.i = +1 } },
    
    /* monitor tagging */
    { MODKEY|ControlMask,           XK_j,      tagmon,         {.i = -1 } },
    { MODKEY|ControlMask,           XK_k,      tagmon,         {.i = +1 } },

    /* move windows */
    { MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },

	/* Window sizing */
	{ MODKEY|ControlMask,           XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY|ControlMask,           XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,             XK_h,      setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_l,      setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },

	/* gaps control */ 
    { MODKEY|ControlMask,           XK_u,      incrgaps,       {.i = +1 } },
    { MODKEY|ControlMask|ShiftMask, XK_u,      incrgaps,       {.i = -1 } },
    { MODKEY|ControlMask,           XK_i,      incrigaps,      {.i = +1 } },
    { MODKEY|ControlMask|ShiftMask, XK_i,      incrigaps,      {.i = -1 } },
    { MODKEY|ControlMask,           XK_o,      incrogaps,      {.i = +1 } },
    { MODKEY|ControlMask|ShiftMask, XK_o,      incrogaps,      {.i = -1 } },
    { MODKEY|ControlMask,           XK_6,      incrihgaps,     {.i = +1 } },
    { MODKEY|ControlMask|ShiftMask, XK_6,      incrihgaps,     {.i = -1 } },
    { MODKEY|ControlMask,           XK_7,      incrivgaps,     {.i = +1 } },
    { MODKEY|ControlMask|ShiftMask, XK_7,      incrivgaps,     {.i = -1 } },
    { MODKEY|ControlMask,           XK_8,      incrohgaps,     {.i = +1 } },
    { MODKEY|ControlMask|ShiftMask, XK_8,      incrohgaps,     {.i = -1 } },
    { MODKEY|ControlMask,           XK_9,      incrovgaps,     {.i = +1 } },
    { MODKEY|ControlMask|ShiftMask, XK_9,      incrovgaps,     {.i = -1 } },
    { MODKEY|ControlMask,           XK_0,      togglegaps,     {0} },
    { MODKEY|ControlMask|ShiftMask, XK_0,      defaultgaps,    {0} },

	/* tags */
    { MODKEY,                       XK_0,      view,           {.ui = ~0 } },
    { MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
    TAGKEYS(                        XK_1,                      0)
    TAGKEYS(                        XK_2,                      1)
    TAGKEYS(                        XK_3,                      2)
    TAGKEYS(                        XK_4,                      3)
    TAGKEYS(                        XK_5,                      4)
    TAGKEYS(                        XK_6,                      5)
    TAGKEYS(                        XK_7,                      6)
    TAGKEYS(                        XK_8,                      7)
    TAGKEYS(                        XK_9,                      8)

	/* system */
	{ MODKEY,                       XK_q,      killclient,     {0} },
    { MODKEY|ShiftMask,             XK_q,      quit,           {0} }, /* quit dwm */
    { MODKEY|ControlMask|ShiftMask, XK_q,      quit,           {1} }, /* refresh (restartsig) */
    { ALTKEY,                       XK_q,      spawn,          {.v = powermenu}},
	{ ALTKEY,                       XK_F5,     xrdb,           {.v = NULL } }, 

	/* custom */
    { 0,                            XK_Print,  spawn,          {.v = screenshot}},
	{ MODKEY|ShiftMask,             XK_s,      spawn,          {.v = screenshot}},
    { ALTKEY,                       XK_w,      spawn,          SHCMD("~/.scripts/wallshuf.sh") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

static const char *ipcsockpath = "/tmp/dwm.sock";
static IPCCommand ipccommands[] = {
  IPCCOMMAND(  view,                1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggleview,          1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tag,                 1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  toggletag,           1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  tagmon,              1,      {ARG_TYPE_UINT}   ),
  IPCCOMMAND(  focusmon,            1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  focusstack,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  zoom,                1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  incnmaster,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  killclient,          1,      {ARG_TYPE_SINT}   ),
  IPCCOMMAND(  togglefloating,      1,      {ARG_TYPE_NONE}   ),
  IPCCOMMAND(  setmfact,            1,      {ARG_TYPE_FLOAT}  ),
  IPCCOMMAND(  setlayoutsafe,       1,      {ARG_TYPE_PTR}    ),
  IPCCOMMAND(  quit,                1,      {ARG_TYPE_NONE}   )
};

