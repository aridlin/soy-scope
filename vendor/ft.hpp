// ft.hpp - combined FT family facade.
//
// Usage:
//   #define FT_IMPLEMENTATION
//   #include "ft.hpp"
//
// Build:
//   Linux GUI support needs the FTUI dependencies:
//     c++ app.cpp -std=c++17 $(pkg-config --cflags --libs cairo x11) -pthread
//
// Runtime mode flags:
//   --ft-auto / --ft-gui / --ft-tui / --ft-web
//   --gui     / --tui     / --web
//
// The ft:: API follows FTUI syntax. It chooses the native GUI backend when a
// display is available, the terminal backend on a real TTY, and the embedded
// web backend as the final fallback. Forced modes do not fallback.

#pragma once

#ifdef FT_IMPLEMENTATION
#ifndef FTUI_IMPLEMENTATION
#define FTUI_IMPLEMENTATION
#endif
#ifndef FTTI_IMPLEMENTATION
#define FTTI_IMPLEMENTATION
#endif
#ifndef FTHT_IMPLEMENTATION
#define FTHT_IMPLEMENTATION
#endif
#endif

// ============================================================
// Embedded ftui.hpp
// ============================================================

// ftui.hpp — Tiny immediate-mode GUI for Windows and Linux
// Single-header, no external dependencies beyond platform SDKs.
//
// Usage: #define FTUI_IMPLEMENTATION in exactly one .cpp before including.
//
// Windows: cl main.cpp /link d2d1.lib dwrite.lib gdi32.lib ole32.lib uuid.lib user32.lib
//          (MSVC auto-links via #pragma comment)
// Linux:   g++ main.cpp -o app -DFTUI_IMPLEMENTATION \
//            $(pkg-config --cflags --libs cairo x11) -std=c++17
//
// Optional defines before FTUI_IMPLEMENTATION:
//   FTUI_KEEP_CONSOLE    — keep console window on Windows
//   FTUI_LINUX_FONT "x"  — override default "sans-serif" on Linux
//   FTUI_DISABLE_EFFECTS — strip the Windows effects layer entirely
//
// Frame pacing:
//   Config::fps_limit defaults to 60. Set it to 0 for uncapped rendering.
//   On Linux, idle windows sleep until X11 reports an event or request_redraw()
//   asks for another frame.

// Global startup theme. Change this line to another built-in preset if you
// want a different default across all new FTUI windows.
#ifdef FTUI_DEFAULT_STYLE
#define FTUI_DEFAULT_STYLE_EXPLICIT 1
#else
#define FTUI_DEFAULT_STYLE ftui::one_dark_style
#endif

#ifndef FTUI_VERSION_MAJOR
#define FTUI_VERSION_MAJOR 1
#endif
#ifndef FTUI_VERSION_MINOR
#define FTUI_VERSION_MINOR 1
#endif
#ifndef FTUI_VERSION_PATCH
#define FTUI_VERSION_PATCH 0
#endif

#pragma once
#if defined(_WIN32) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <cmath>
#include <initializer_list>
#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <cassert>
#include <fstream>
#include <sstream>
#include <cctype>
#include <cstdlib>

// ============================================================
// Public API
// ============================================================

namespace ftui {

enum class Align { Start, Center, End };

static constexpr int KeyEnter = 13;
static constexpr int KeyNumpad0 = 96;

struct Color { float r, g, b, a; };

struct Style {
    Color background    = {0.055f,0.055f,0.059f,1.0f};
    Color panel         = {0.090f,0.094f,0.102f,1.0f};
    Color text          = {0.910f,0.910f,0.910f,1.0f};
    Color text_dim      = {0.663f,0.678f,0.702f,1.0f};
    Color border        = {0.169f,0.176f,0.192f,1.0f};
    Color button        = {0.090f,0.094f,0.102f,1.0f};
    Color button_hover  = {0.137f,0.149f,0.169f,1.0f};
    Color button_active = {0.176f,0.192f,0.220f,1.0f};
    Color input_bg      = {0.071f,0.075f,0.082f,1.0f};
    Color input_focus   = {0.310f,0.420f,0.780f,1.0f};
    Color accent        = {0.310f,0.420f,0.780f,1.0f};
    Color warning       = {0.894f,0.486f,0.353f,1.0f};
    Color success       = {0.420f,0.741f,0.522f,1.0f};
    float window_padding = 20.0f;
    float item_spacing   = 10.0f;
    float item_height    = 36.0f;
    float rounding       = 8.0f;
    float border_width   = 1.0f;
    float font_size      = 16.0f;
};

enum class ColorRole {
    Background,
    Panel,
    Text,
    TextDim,
    Border,
    Button,
    ButtonHover,
    ButtonActive,
    InputBg,
    InputFocus,
    Accent,
    Warning,
    Success,
};

Style default_dark_style();
Style catppuccin_mocha_style();
Style nord_style();
Style gruvbox_dark_style();
Style one_dark_style();
Style ghostty_green_style();
Style style_from_name(const char* name);
const char* style_name(const Style& style);
const char* current_style_name();

void         set_style(const Style& s);
const Style& get_style();
Color        color_from_hex(const char* hex);
Color        color_from_hex(std::string_view hex);
void         push_color(ColorRole role, Color color);
void         pop_color();
void         set_next_color(ColorRole role, Color color);

enum class BuiltinIcon {
    Symbol,
    SymbolWithText,
};

enum class BackdropEffect {
    Blur,
    BayerDither,
};

enum class WindowTransparency {
    Opaque,
    Plain,
    BayerDither,
    Blur,
};

struct Config {
    const char* title         = "FTUI App"; // UTF-8; converted internally on Windows
    int         width         = 960;
    int         height        = 640;
    int         fps_limit     = 60;         // 0 or below disables frame limiting
    bool        resizable     = true;
    bool        center_window = true;
    void*       icon          = nullptr;    // HICON on Windows; ignored on Linux
    bool        enable_effects = true;      // Smooth scrolling and widget animations
    BackdropEffect backdrop_effect = BackdropEffect::Blur; // Blur is Windows-only; dither is cross-platform
    int         dither_size   = 4;          // Soft dither motif size in pixels
    WindowTransparency window_transparency = WindowTransparency::Opaque;
    float       window_opacity = 0.92f;     // Plain/Blur whole-window opacity
};

bool create_window(const Config& cfg = {});
bool pump();
void begin();
void end();
void shutdown();
void request_redraw();
void set_quit_on_ctrl_q(bool enabled);
void set_fps_limit(int fps);
int  get_fps_limit();
void set_backdrop_effect(BackdropEffect effect);
void set_dither_size(int px);
void set_window_transparency(WindowTransparency mode);
void set_window_opacity(float opacity);
void set_window_size(int width, int height);
void set_window_icon(void* native_icon);
void set_window_icon_builtin(BuiltinIcon variant = BuiltinIcon::Symbol);
using CommandHandler = bool (*)(const char* command);
void set_command_handler(CommandHandler handler);

void text(const char* label);
void text_wrapped(const char* text);
void separator();
void spacing(float px = 8.0f);

enum class InputFlags : unsigned {
    Default = 0,
    Password = 1 << 0,
    ReadOnly = 1 << 1,
    CharsDecimal = 1 << 2,
    CharsHexadecimal = 1 << 3,
    CharsUppercase = 1 << 4,
    CharsNoBlank = 1 << 5,
};
inline InputFlags operator|(InputFlags a, InputFlags b) {
    return static_cast<InputFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(InputFlags a, InputFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class TextAreaFlags : unsigned {
    Default = 0,
    ReadOnly = 1 << 0,
    WordWrap = 1 << 1,
    AutoScrollBottom = 1 << 2,
};
inline TextAreaFlags operator|(TextAreaFlags a, TextAreaFlags b) {
    return static_cast<TextAreaFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(TextAreaFlags a, TextAreaFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class LogViewFlags : unsigned {
    Default = 0,
    WordWrap = 1 << 0,
    AutoScrollBottom = 1 << 1,
};
inline LogViewFlags operator|(LogViewFlags a, LogViewFlags b) {
    return static_cast<LogViewFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(LogViewFlags a, LogViewFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

bool input(const char* label, char* buffer, int buffer_size,
           InputFlags flags = InputFlags::Default, bool* enter_pressed = nullptr);

// Multi-line text input. Enter inserts newline; Tab cycles focus.
bool text_area(const char* label, char* buffer, int buffer_size, int rows = 5);
bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows = 5,
                  TextAreaFlags flags = TextAreaFlags::Default);
void log_view(const char* label, const char* text, int rows = 8,
              LogViewFlags flags = LogViewFlags::AutoScrollBottom);

bool checkbox(const char* label, bool* value);
bool slider_float(const char* label, float* value, float min_v, float max_v);
bool button(const char* label);
bool button(const char* label, ColorRole role);
bool button(const char* label, Color color);
bool dropdown(const char* label, const char* const* items, int count, int* selected, int popup_rows = 8);
bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows = 6);
bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns = 1);
bool collapsing_header(const char* label, bool* open = nullptr);
bool side_menu(const char* label, const char* const* items, int count, int* selected);
bool side_menu_drawer(const char* label, const char* const* items, int count, int* selected, float width = 260.0f);

// Horizontal tab bar. Returns true when selected changes.
bool tabs(const char* const* labels, int count, int* selected);

void row(int cols, std::function<void()> fn);
void row(std::initializer_list<float> weights, std::function<void()> fn);
void split(std::initializer_list<float> columns, std::function<void()> fn);
void side_layout(float side_width, std::function<void()> fn);
void content(std::function<void()> fn);
void scroll_area(const char* label, float height, std::function<void()> fn);
void set_next_width(float px);
void set_next_fill();
void set_next_percent(float pct);
void set_next_limits(float min_px, float max_px);
void set_next_align(Align align);
void open_modal(const char* label);
bool modal(const char* label, std::function<void()> fn);
void close_modal();
void begin_disabled();
void end_disabled();
void tooltip(const char* text);
void request_focus(const char* label);
float calc_text_width(const char* text);
float calc_text_height(const char* text, float wrap_width);

enum class ToastType {
    Info,
    Success,
    Warning,
    Error,
};

struct Toast {
    const char* message = "";
    ToastType   type = ToastType::Info;
    int         duration_ms = 3500;
    bool        dismissible = true;
};

void toast(const Toast& toast);
void toast(const char* message);
void toast_info(const char* message);
void toast_success(const char* message);
void toast_warning(const char* message);
void toast_error(const char* message);
void clear_toasts();

struct ProgressStyle {
    const char* label = nullptr;
    const char* mask_path = nullptr;
    const char* mask_svg = nullptr;
    const char* mask_shape = nullptr;
    ColorRole   fill_role = ColorRole::Accent;
    Color       fill_color = {-1.0f, -1.0f, -1.0f, -1.0f};
    float       height = 22.0f;
    bool        show_percent = true;
    bool        wave_front = false;
    bool        glint = false;
};

void progress_bar(float progress);
void progress_bar(float progress, const char* label_or_mask_path);
void progress_bar(float progress, const ProgressStyle& style);

// Opaque image handle — platform-specific payload in _impl.
struct ImageHandle { void* _impl = nullptr; };
void         image(ImageHandle* img, float width, float height);
ImageHandle* load_image(const char* utf8_path);
void         free_image(ImageHandle* img);

// File dialog — UTF-8 strings on both platforms.
// On Linux: requires zenity; returns "" if not available or cancelled.
struct FileFilter {
    const char* name; // e.g. "PNG Images"
    const char* spec; // e.g. "*.png;*.jpg"  (Windows wildcard; Linux space-sep)
};
std::string open_file_dialog(const char* title = "Open File",
                              const FileFilter* filters = nullptr,
                              int filter_count = 0);

void open_child_window(const Config& cfg, std::function<void()> fn);

struct DebugState {
    bool show_layout_rects = false;
    bool show_hovered_id   = false;
    bool show_active_id    = false;
    bool show_fps          = false;
    bool log_widget_calls  = false;
};
DebugState& debug();

} // namespace ftui


// ============================================================
// Implementation
// ============================================================

#ifdef FTUI_IMPLEMENTATION

#if defined(_WIN32) && !defined(FTUI_DISABLE_EFFECTS)
#define FTUI_WINDOWS_EFFECTS 1
#else
#define FTUI_WINDOWS_EFFECTS 0
#endif

#ifdef _WIN32

#if !defined(_WIN32_WINNT) || _WIN32_WINNT < 0x0A00
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#if !defined(NTDDI_VERSION) || NTDDI_VERSION < 0x0A000000
#undef NTDDI_VERSION
#define NTDDI_VERSION 0x0A000000
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include <dwrite.h>
#include <shobjidl.h>
#include <wincodec.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "uuid.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "windowscodecs.lib")
#ifndef FTUI_KEEP_CONSOLE
#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")
#endif

#elif defined(__linux__)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/keysym.h>
#include <X11/Xresource.h>
#ifdef InputFocus
#undef InputFocus
#endif
#ifdef Success
#undef Success
#endif
#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>
#include <errno.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#endif

namespace ftui {
namespace internal {

// ---- Shared utilities -----------------------------------------------

static Style startup_style() {
#ifdef FTUI_DEFAULT_STYLE_EXPLICIT
    return FTUI_DEFAULT_STYLE();
#else
    const char* env_theme = getenv("FTUI_THEME");
    if (env_theme && env_theme[0]) return style_from_name(env_theme);
    return FTUI_DEFAULT_STYLE();
#endif
}

static const Style& fallback_style() {
    static Style s = startup_style();
    return s;
}

static constexpr int kColorRoleCount = 13;
static constexpr int kColorOverrideStackCap = 64;

static int hash_str(const char* s) {
    unsigned h = 2166136261u;
    for (; *s; ++s) h = (h ^ (unsigned char)*s) * 16777619u;
    return (int)(h & 0x7fffffff) + 1;
}

static void split_label(const char* label, char* visible, int vis_len, const char** hash_src) {
    const char* sep = strstr(label, "##");
    int n = sep ? (int)(sep - label) : (int)strlen(label);
    if (n >= vis_len) n = vis_len - 1;
    memcpy(visible, label, n); visible[n] = '\0';
    *hash_src = label;
}

struct Rect { float x, y, w, h; };
static bool rect_contains(Rect r, float px, float py) {
    return px >= r.x && px < r.x + r.w && py >= r.y && py < r.y + r.h;
}

static int utf8_advance(const char* s, int pos) {
    if (!s[pos]) return pos;
    unsigned char c = (unsigned char)s[pos];
    return pos + (c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4);
}
static int utf8_retreat(const char* s, int pos) {
    if (pos == 0) return 0;
    do { --pos; } while (pos > 0 && ((unsigned char)s[pos] & 0xC0) == 0x80);
    return pos;
}
static int utf8_char_count(const char* s, int byte_offset) {
    int n = 0;
    for (int i = 0; i < byte_offset && s[i]; ) {
        unsigned char c = (unsigned char)s[i];
        i += c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4; n++;
    }
    return n;
}

// ---- Shared state structs -------------------------------------------

struct InputState {
    float mouse_x = 0, mouse_y = 0;
    bool  mouse_down = false, mouse_pressed = false, mouse_released = false;
    float wheel_y = 0;
    bool  key_backspace = false, key_enter = false;
    bool  key_space = false, key_escape = false;
    bool  key_tab = false, key_shift_tab = false;
    bool  key_left = false, key_right = false;
    bool  key_up   = false, key_down  = false;
    bool  key_ctrl_c = false, key_ctrl_v = false;
    bool  shift_held = false, ctrl_held = false;
    char  text_input[64] = {};
    int   text_input_count = 0;
    bool  focused = true;
};

struct RowContext {
    bool  active = false;
    bool  weighted = false;
    bool  split = false;
    int   cols = 0;
    float cell_w = 0, gap = 0, start_x = 0, start_y = 0;
    float total_w = 0, used_x = 0, total_weight = 0;
    float fixed_total = 0, flex_total = 0;
    const float* weights = nullptr;
    int   col_index = 0;
    float row_height = 0;
};

struct UIContext {
    int  hot_id = 0, active_id = 0, focused_input_id = 0, focused_widget_id = 0, frame_index = 0;
    Rect content_region = {};
    float cursor_x = 0, cursor_y = 0;
    RowContext row_ctx;
    std::vector<int> tab_stops, tab_stops_prev;
    Rect last_item_rect = {};
    int  last_item_id = 0;
    bool last_item_hovered = false;
    bool last_item_focused = false;
};

struct CmdState { bool active = false; char buf[64] = {}; int len = 0; };

struct MotionSlot {
    int   key = 0;
    float hover = 0;
    float active = 0;
    float focus = 0;
};

struct TabFxSlot {
    int   key = 0;
    int   selected = 0;
    int   previous = 0;
    int   dir = 1;
    bool  initialized = false;
    float switch_t = 1.0f;
    float underline_x = 0;
    float underline_w = 0;
};

struct FrameContentFx {
    bool  active = false;
    int   owner = 0;
    float start_y = 0;
    float progress = 1.0f;
    int   dir = 1;
};

struct ScrollSlot {
    int   key = 0;
    float current = 0;
    float target = 0;
    float content = 0;
    bool  dragging = false;
    float drag_mouse = 0;
    float drag_scroll0 = 0;
};

struct CollapseSlot {
    int   key = 0;
    bool  initialized = false;
    bool  open = false;
    float progress = 0;
    float body_height = 0;
};

struct ActiveCollapseFx {
    int   key = 0;
    float top_target = 0;
    float top_display = 0;
    float body_height = 0;
    float progress = 1.0f;
};

struct CollapseRectFx {
    Rect rect = {};
    Rect clip = {};
    bool clip_active = false;
};

struct NextLayoutState {
    bool  active = false;
    bool  fill = false;
    bool  has_width = false;
    bool  has_percent = false;
    bool  has_limits = false;
    float width = 0;
    float percent = 1.0f;
    float min = 0;
    float max = 0;
    Align align = Align::Start;
};

struct TooltipState {
    bool active = false;
    bool requested = false;
    bool visible = false;
    int  request_id = 0;
    int  hover_id = 0;
    Rect anchor = {};
    Rect request_anchor = {};
    std::string text;
    std::string request_text;
    float hover_time = 0;
    float opacity = 0;
    float visible_time = 0;
    float calm_time = 0;
    float static_x = 0;
    float static_y = 0;
    int   dismiss_streak = 0;
};

struct DropdownOverlayState {
    bool active = false;
    bool open = false;
    Rect popup_r = {};
    int count = 0;
    int selected_index = -1;
    float item_h = 0;
    float scroll = 0;
    float popup_h = 0;
    float popup_p = 1.0f;
    Color popup_fill = {};
    Color popup_border = {};
    Color item_hover = {};
    Color item_selected = {};
    Color item_text = {};
    Color item_text_selected = {};
    Color scrollbar_thumb = {};
    Color scrollbar_track = {};
    std::vector<std::string> labels;
};

struct SideDrawerState {
    bool active = false;
    bool open = false;
    int  owner = 0;
    int  selected = 0;
    int  count = 0;
    float width = 260.0f;
    float progress = 0.0f;
    float item_h = 34.0f;
    Rect launcher_rect = {};
    float launcher_hover = 0.0f;
    float launcher_active = 0.0f;
    std::string title;
    std::vector<std::string> labels;
};

struct ColorOverrideEntry {
    ColorRole role = ColorRole::Text;
    Color color = {};
};

struct ToastSlot {
    bool active = false;
    Toast toast = {};
    std::string message;
    float elapsed = 0.0f;
};

struct MaskData {
    std::string path;
    int w = 0;
    int h = 0;
    std::vector<unsigned char> bits;
    bool loaded = false;
};

struct ColorOverrideTable {
    bool used[kColorRoleCount] = {};
    Color colors[kColorRoleCount] = {};
};

// ---- Shared globals -------------------------------------------------

static InputState g_input;
static UIContext  g_ctx;
static DebugState g_debug;
static Style      g_style;
static std::string g_style_name;
static float      g_fps = 0, g_fps_accum = 0;
static int        g_fps_frames = 0;
static int        g_frame_limit_fps = 60;
static bool       g_redraw_requested = true;
static bool       g_shortcuts_enabled = true;
static CmdState   g_cmd;
static CommandHandler g_command_handler = nullptr;
static float      g_dt = 1.0f / 60.0f;
static bool       g_effects_enabled = false;
static BackdropEffect g_backdrop_effect = BackdropEffect::Blur;
static int        g_dither_size = 4;
static WindowTransparency g_window_transparency = WindowTransparency::Opaque;
static float      g_window_opacity = 0.92f;
static float      g_scroll_y = 0, g_content_height = 0;
static float      g_scroll_target_y = 0;
static bool       g_sb_dragging = false;
static float      g_sb_drag_mouse_y = 0, g_sb_drag_scroll0 = 0;
static int        g_text_cursor_id = 0, g_text_cursor = 0, g_text_sel_anchor = 0;
static int        g_ta_cursor_id = 0, g_ta_cursor = 0, g_ta_sel_anchor = 0;
static float      g_ta_scroll_y = 0;
static float      g_ta_scroll_target_y = 0;
static bool       g_drawing = false;
static MotionSlot g_motion_slots[256];
static TabFxSlot  g_tab_fx_slots[32];
static ScrollSlot g_scroll_slots[128];
static CollapseSlot g_collapse_slots[64];
static int        g_tab_content_owner = 0;
static FrameContentFx g_frame_content_fx;
static float      g_draw_fx_off_x = 0;
static float      g_draw_fx_off_y = 0;
static float      g_draw_fx_opacity = 1.0f;
static NextLayoutState g_next_layout;
static TooltipState g_tooltip;
static DropdownOverlayState g_dropdown_overlay;
static DropdownOverlayState g_dropdown_overlay_prev;
static SideDrawerState g_side_drawer;
static ToastSlot g_toasts[16];
static MaskData  g_mask_cache[16];
static ColorOverrideEntry g_color_stack[kColorOverrideStackCap];
static int        g_color_stack_count = 0;
static ColorOverrideTable g_next_color_pending;
static ColorOverrideTable g_active_widget_colors;
static int        g_active_widget_color_depth = 0;
static ActiveCollapseFx g_active_collapse_fx[16];
static int        g_active_collapse_fx_count = 0;
static int        g_pending_collapse_key = 0;
static float      g_pending_collapse_start_y = 0.0f;
static int        g_disabled_depth = 0;
static int        g_focus_request_id = 0;
static int        g_modal_open_id = 0;
static int        g_modal_request_id = 0;
static bool       g_inside_modal = false;
static bool       g_modal_drawn = false;
static int        g_dropdown_open_id = 0;
static bool       g_dropdown_capture_input = false;
static bool       g_side_drawer_capture_input = false;

// ---- Forward declarations of platform-specific functions -----------
// (defined in the platform blocks below; used by shared widget code)

static void        dbg(const char* fmt, ...);
static float       measure_text_width(const char* utf8);
static int         byte_from_x(const char* utf8, float rel_x);
static void        fill_round_rect(Rect r, float radius, Color c);
static void        stroke_round_rect(Rect r, float radius, float thickness, Color c);
static void        fill_rect(Rect r, Color c);
static void        draw_line(float x0, float y0, float x1, float y1, float thickness, Color c);
static void        fill_triangle(Rect r, int dir, Color c);
static void        draw_text_utf8(const char* utf8, Rect r, Color c);
static void        draw_text_utf8_centered(const char* utf8, Rect r, Color c);
static void        draw_image_handle(ImageHandle* img, Rect r);
static void        clipboard_set(const char* utf8);
static std::string clipboard_get();
static void        push_clip(Rect r);
static void        pop_clip();
static void        wait_frame_limit();
static void        wake_event_loop();
static void        sync_native_window_chrome();
static void        sync_native_window_transparency();
static void        apply_window_icon_handles(void* icon_big, void* icon_small, bool owned, BuiltinIcon variant);
static bool        load_mask_from_image(const char* path, MaskData& out);
static void        draw_toast_overlay();
static void        draw_side_drawer_overlay(int window_w, int window_h);
static void        draw_command_overlay(int window_w, int window_h);
static const char* gui_command_completion(const char* prefix);
static const char* gui_command_hint(const char* prefix);
static bool        toasts_active();
static void        draw_dither_backdrop_panel(Rect r, float opacity);
static void        draw_dither_pattern(Rect r, float opacity);

#if FTUI_WINDOWS_EFFECTS
static void        draw_previous_frame_overlay(Rect clip_r, float dx, float opacity);
static void        draw_previous_frame_blur_panel(Rect r, float opacity);
static void        release_frame_snapshot();
static void        capture_frame_snapshot();
#endif

// ---- Shared implementations (call platform fns above) ---------------

static float text_line_height() { return g_style.font_size + 6.0f; }

static float clamp01(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

static float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

static Color lerp_color(Color a, Color b, float t) {
    t = clamp01(t);
    return {
        lerpf(a.r, b.r, t),
        lerpf(a.g, b.g, t),
        lerpf(a.b, b.b, t),
        lerpf(a.a, b.a, t),
    };
}

static Color with_alpha(Color c, float a) {
    c.a = a;
    return c;
}

static int color_role_index(ColorRole role) {
    switch (role) {
        case ColorRole::Background:  return 0;
        case ColorRole::Panel:       return 1;
        case ColorRole::Text:        return 2;
        case ColorRole::TextDim:     return 3;
        case ColorRole::Border:      return 4;
        case ColorRole::Button:      return 5;
        case ColorRole::ButtonHover: return 6;
        case ColorRole::ButtonActive:return 7;
        case ColorRole::InputBg:     return 8;
        case ColorRole::InputFocus:  return 9;
        case ColorRole::Accent:      return 10;
        case ColorRole::Warning:     return 11;
        case ColorRole::Success:     return 12;
    }
    return 2;
}

static Color style_color_for_role(const Style& style, ColorRole role) {
    switch (role) {
        case ColorRole::Background:   return style.background;
        case ColorRole::Panel:        return style.panel;
        case ColorRole::Text:         return style.text;
        case ColorRole::TextDim:      return style.text_dim;
        case ColorRole::Border:       return style.border;
        case ColorRole::Button:       return style.button;
        case ColorRole::ButtonHover:  return style.button_hover;
        case ColorRole::ButtonActive: return style.button_active;
        case ColorRole::InputBg:      return style.input_bg;
        case ColorRole::InputFocus:   return style.input_focus;
        case ColorRole::Accent:       return style.accent;
        case ColorRole::Warning:      return style.warning;
        case ColorRole::Success:      return style.success;
    }
    return style.text;
}

static Color resolve_color(ColorRole role) {
    int idx = color_role_index(role);
    if (g_active_widget_color_depth > 0 && g_active_widget_colors.used[idx]) {
        return g_active_widget_colors.colors[idx];
    }
    for (int i = g_color_stack_count - 1; i >= 0; --i) {
        if (g_color_stack[i].role == role) return g_color_stack[i].color;
    }
    return style_color_for_role(g_style, role);
}

struct WidgetColorScope {
    ColorOverrideTable prev = {};
    ColorOverrideTable own = {};
    int prev_depth = 0;
    bool active = false;

    WidgetColorScope() {
        prev = g_active_widget_colors;
        prev_depth = g_active_widget_color_depth;
        own = g_next_color_pending;
        g_active_widget_colors = own;
        g_active_widget_color_depth = 1;
        g_next_color_pending = {};
        active = true;
    }

    void suspend_for_children() {
        if (!active) return;
        g_active_widget_colors = {};
        g_active_widget_color_depth = 0;
    }

    void resume_after_children() {
        if (!active) return;
        g_active_widget_colors = own;
        g_active_widget_color_depth = 1;
    }

    ~WidgetColorScope() {
        if (!active) return;
        g_active_widget_colors = prev;
        g_active_widget_color_depth = prev_depth;
    }
};

static int hex_nibble(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

static Color parse_hex_color(std::string_view hex) {
    constexpr Color fallback = {1.0f, 1.0f, 1.0f, 1.0f};
    auto fail = [&]() -> Color {
#ifndef NDEBUG
        assert(false && "ftui::color_from_hex: invalid hex color");
#endif
        return fallback;
    };

    if (hex.size() != 7 && hex.size() != 9) return fail();
    if (hex.empty() || hex[0] != '#') return fail();

    unsigned bytes[4] = {255u, 255u, 255u, 255u};
    int count = hex.size() == 7 ? 3 : 4;
    for (int i = 0; i < count; ++i) {
        int hi = hex_nibble(hex[1 + i * 2]);
        int lo = hex_nibble(hex[2 + i * 2]);
        if (hi < 0 || lo < 0) return fail();
        bytes[i] = (unsigned)((hi << 4) | lo);
    }

    return {
        bytes[0] / 255.0f,
        bytes[1] / 255.0f,
        bytes[2] / 255.0f,
        bytes[3] / 255.0f,
    };
}

static Rect offset_rect(Rect r, float dx, float dy) {
    r.x += dx; r.y += dy; return r;
}

static float ease_smooth(float t) {
    t = clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

static float step_anim(float value, float target, float dt, float sharpness) {
    if (dt <= 0.0f) return target;
    float a = 1.0f - expf(-sharpness * dt);
    float out = value + (target - value) * a;
    if (fabsf(out - target) < 0.001f) return target;
    return out;
}

static bool effects_enabled() {
    return g_effects_enabled;
}

static void reset_draw_fx() {
    g_draw_fx_off_x = 0;
    g_draw_fx_off_y = 0;
    g_draw_fx_opacity = 1.0f;
}

static void reset_effect_state(bool enable_effects) {
    g_effects_enabled = enable_effects;
    g_dt = 1.0f / 60.0f;
    g_scroll_target_y = g_scroll_y;
    g_ta_scroll_target_y = g_ta_scroll_y;
    memset(g_motion_slots, 0, sizeof(g_motion_slots));
    memset(g_tab_fx_slots, 0, sizeof(g_tab_fx_slots));
    g_tab_content_owner = 0;
    g_frame_content_fx = {};
    reset_draw_fx();
}

static bool effects_need_redraw() {
    if (!effects_enabled()) return false;
    if (fabsf(g_scroll_y - g_scroll_target_y) > 0.01f) return true;
    if (fabsf(g_ta_scroll_y - g_ta_scroll_target_y) > 0.01f) return true;
    for (const MotionSlot& slot : g_motion_slots) {
        if (!slot.key) continue;
        if ((slot.hover > 0.001f && slot.hover < 0.999f) ||
            (slot.active > 0.001f && slot.active < 0.999f) ||
            (slot.focus > 0.001f && slot.focus < 0.999f)) {
            return true;
        }
    }
    for (const TabFxSlot& slot : g_tab_fx_slots) {
        if (slot.key && slot.switch_t < 1.0f) return true;
    }
    for (const CollapseSlot& slot : g_collapse_slots) {
        if (slot.key && slot.progress > 0.001f && slot.progress < 0.999f) return true;
    }
    return false;
}

static float clampf(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

static Color disabled_color(Color c) {
    Color out = lerp_color(c, resolve_color(ColorRole::Panel), 0.42f);
    out.a *= 0.70f;
    return out;
}

static Color maybe_disabled(Color c) {
    return g_disabled_depth > 0 ? disabled_color(c) : c;
}

static void clear_text_focus() {
    g_ctx.focused_input_id = 0;
    g_text_cursor_id = 0;
    g_ta_cursor_id = 0;
}

static void set_widget_focus(int id, bool text_like = false) {
    g_ctx.focused_widget_id = id;
    if (text_like) g_ctx.focused_input_id = id;
    else clear_text_focus();
}

static bool is_widget_focused(int id) {
    return g_ctx.focused_widget_id == id;
}

static bool widget_interaction_enabled() {
    return g_disabled_depth == 0 && (!g_modal_open_id || g_inside_modal) &&
           !g_dropdown_capture_input && !g_side_drawer_capture_input;
}

static void register_focusable(int id) {
    g_ctx.tab_stops.push_back(id);
}

static void mark_last_item(int id, Rect r, bool hovered, bool focused) {
    g_ctx.last_item_id = id;
    g_ctx.last_item_rect = r;
    g_ctx.last_item_hovered = hovered;
    g_ctx.last_item_focused = focused;
}

static ScrollSlot& scroll_slot_for(int key) {
    unsigned idx = (unsigned)key & 127u;
    for (int n = 0; n < 128; ++n) {
        ScrollSlot& slot = g_scroll_slots[(idx + (unsigned)n) & 127u];
        if (slot.key == key || slot.key == 0) {
            if (slot.key == 0) slot.key = key;
            return slot;
        }
    }
    ScrollSlot& slot = g_scroll_slots[idx];
    slot = {};
    slot.key = key;
    return slot;
}

static CollapseSlot& collapse_slot_for(int key) {
    unsigned idx = (unsigned)key & 63u;
    for (int n = 0; n < 64; ++n) {
        CollapseSlot& slot = g_collapse_slots[(idx + (unsigned)n) & 63u];
        if (slot.key == key || slot.key == 0) {
            if (slot.key == 0) slot.key = key;
            return slot;
        }
    }
    CollapseSlot& slot = g_collapse_slots[idx];
    slot = {};
    slot.key = key;
    return slot;
}

static void finalize_pending_collapse_measure() {
    if (!g_pending_collapse_key) return;
    CollapseSlot& slot = collapse_slot_for(g_pending_collapse_key);
    float measured = g_ctx.cursor_y - g_pending_collapse_start_y;
    if (measured < 0.0f) measured = 0.0f;
    slot.body_height = measured;
    g_pending_collapse_key = 0;
    g_pending_collapse_start_y = 0.0f;
}

static MotionSlot& motion_slot_for(int key) {
    unsigned idx = (unsigned)key & 255u;
    for (int n = 0; n < 256; ++n) {
        MotionSlot& slot = g_motion_slots[(idx + (unsigned)n) & 255u];
        if (slot.key == key || slot.key == 0) {
            if (slot.key == 0) slot.key = key;
            return slot;
        }
    }
    MotionSlot& slot = g_motion_slots[idx];
    slot = {};
    slot.key = key;
    return slot;
}

static TabFxSlot& tab_fx_slot_for(int key) {
    unsigned idx = (unsigned)key & 31u;
    for (int n = 0; n < 32; ++n) {
        TabFxSlot& slot = g_tab_fx_slots[(idx + (unsigned)n) & 31u];
        if (slot.key == key || slot.key == 0) {
            if (slot.key == 0) slot.key = key;
            return slot;
        }
    }
    TabFxSlot& slot = g_tab_fx_slots[idx];
    slot = {};
    slot.key = key;
    return slot;
}

static void update_motion_slot(MotionSlot& slot, bool hover_on, bool active_on, bool focus_on) {
    if (!effects_enabled()) {
        slot.hover = hover_on ? 1.0f : 0.0f;
        slot.active = active_on ? 1.0f : 0.0f;
        slot.focus = focus_on ? 1.0f : 0.0f;
        return;
    }
    slot.hover = step_anim(slot.hover, hover_on ? 1.0f : 0.0f, g_dt, 16.0f);
    slot.active = step_anim(slot.active, active_on ? 1.0f : 0.0f, g_dt, 22.0f);
    slot.focus = step_anim(slot.focus, focus_on ? 1.0f : 0.0f, g_dt, 14.0f);
}

static void draw_widget_chrome(Rect r, float rounding, Color fill, Color border,
                               float hover_p, float active_p, float focus_p) {
    if (!effects_enabled()) {
        fill_round_rect(r, rounding, fill);
        stroke_round_rect(r, rounding, g_style.border_width, border);
        return;
    }

    float glow = clamp01(hover_p * 0.45f + active_p * 0.65f + focus_p * 0.80f);
    Color shadow = {0.0f, 0.0f, 0.0f, 0.07f + glow * 0.08f};
    Color shell = lerp_color(fill, resolve_color(ColorRole::Panel), 0.12f);
    shell = lerp_color(shell, resolve_color(ColorRole::InputFocus), hover_p * 0.03f + focus_p * 0.05f);
    shell.a = clamp01(0.94f + hover_p * 0.02f + focus_p * 0.02f);
    Color inner = {1.0f, 1.0f, 1.0f, 0.025f + hover_p * 0.015f + focus_p * 0.020f};
    Color rim = lerp_color(border, resolve_color(ColorRole::InputFocus), clamp01(active_p * 0.35f + focus_p * 0.55f));
    rim.a = clamp01(0.50f + glow * 0.18f);

    fill_round_rect(offset_rect(r, 0.0f, 1.0f), rounding + 0.5f, shadow);
    fill_round_rect(r, rounding, shell);
    if (r.w > 2.0f && r.h > 2.0f) {
        stroke_round_rect({r.x + 1.0f, r.y + 1.0f, r.w - 2.0f, r.h - 2.0f},
                          fmaxf(1.0f, rounding - 1.0f), 1.0f, inner);
    }
    stroke_round_rect(r, rounding, 1.0f + focus_p * 0.7f, rim);
}

static float measure_text_at(const char* utf8, int byte_len) {
    if (byte_len <= 0 || !utf8 || !utf8[0]) return 0.0f;
    std::string sub(utf8, utf8 + byte_len);
    return measure_text_width(sub.c_str());
}

struct TextRange {
    int start = 0;
    int end = 0;
};

static bool wrap_space(char c) {
    return c == ' ' || c == '\t';
}

static void compute_wrapped_ranges(const char* text, float max_width, bool word_wrap,
                                   std::vector<TextRange>& out) {
    out.clear();
    if (!text) { out.push_back({0, 0}); return; }

    int len = (int)strlen(text);
    if (len == 0) { out.push_back({0, 0}); return; }

    int line_start = 0;
    while (line_start <= len) {
        if (line_start == len) { out.push_back({len, len}); break; }

        int logical_end = line_start;
        while (logical_end < len && text[logical_end] != '\n') logical_end++;

        if (!word_wrap || max_width <= 1.0f || measure_text_at(text + line_start, logical_end - line_start) <= max_width) {
            out.push_back({line_start, logical_end});
            if (logical_end >= len) break;
            line_start = logical_end + 1;
            continue;
        }

        int seg_start = line_start;
        while (seg_start < logical_end) {
            int pos = seg_start;
            int last_break = -1;
            int best_end = seg_start;
            while (pos < logical_end) {
                int next = utf8_advance(text, pos);
                float w = measure_text_at(text + seg_start, next - seg_start);
                if (wrap_space(text[pos])) last_break = pos;
                if (w > max_width && best_end > seg_start) break;
                if (w > max_width) {
                    best_end = last_break >= seg_start ? last_break : pos;
                    break;
                }
                best_end = next;
                pos = next;
            }
            if (best_end <= seg_start) best_end = utf8_advance(text, seg_start);
            out.push_back({seg_start, best_end});
            seg_start = best_end;
            while (seg_start < logical_end && wrap_space(text[seg_start])) seg_start++;
        }

        if (logical_end >= len) break;
        line_start = logical_end + 1;
    }

    if (out.empty()) out.push_back({0, 0});
}

static void draw_tooltip_overlay() {
    if (g_tooltip.opacity <= 0.01f || g_tooltip.text.empty()) return;

    float pad = 8.0f;
    float max_w = fminf(320.0f, fmaxf(180.0f, g_ctx.content_region.w * 0.45f));
    std::vector<TextRange> lines;
    compute_wrapped_ranges(g_tooltip.text.c_str(), max_w - pad * 2.0f, true, lines);

    float lh = text_line_height();
    float box_h = pad * 2.0f + (float)lines.size() * lh;
    Rect r = {
        g_tooltip.anchor.x,
        g_tooltip.anchor.y + g_tooltip.anchor.h + 8.0f,
        max_w,
        box_h
    };

    float max_x = g_ctx.content_region.x + g_ctx.content_region.w - r.w - 8.0f;
    float max_y = g_ctx.content_region.y + g_ctx.content_region.h - r.h - 8.0f;
    if (r.x > max_x) r.x = max_x;
    if (r.y > max_y) r.y = g_tooltip.anchor.y - r.h - 8.0f;
    if (r.x < 8.0f) r.x = 8.0f;
    if (r.y < 8.0f) r.y = 8.0f;

    Color fill = lerp_color(resolve_color(ColorRole::Panel), resolve_color(ColorRole::Background), 0.18f);
    fill.a = 0.98f * g_tooltip.opacity;
    Color border = resolve_color(ColorRole::Border); border.a *= g_tooltip.opacity;
    draw_widget_chrome(r, fmaxf(4.0f, g_style.rounding * 0.75f), fill, border, 0.0f, 0.0f, 0.0f);

    for (size_t i = 0; i < lines.size(); ++i) {
        TextRange tr = lines[i];
        std::string line(g_tooltip.text.c_str() + tr.start, g_tooltip.text.c_str() + tr.end);
        Rect lr = {r.x + pad, r.y + pad + (float)i * lh, r.w - pad * 2.0f, lh};
        Color text = resolve_color(ColorRole::Text); text.a *= g_tooltip.opacity;
        draw_text_utf8(line.c_str(), lr, text);
    }
}

static void draw_dropdown_overlay() {
    if (!g_dropdown_overlay.active || g_dropdown_overlay.labels.empty() || g_dropdown_overlay.count <= 0) return;

    const DropdownOverlayState& ov = g_dropdown_overlay;
#if FTUI_WINDOWS_EFFECTS
    if (effects_enabled() && g_backdrop_effect == BackdropEffect::Blur) {
        draw_previous_frame_blur_panel(ov.popup_r, ov.popup_p);
    }
#endif
    if (g_backdrop_effect == BackdropEffect::BayerDither) {
        draw_dither_backdrop_panel(ov.popup_r, ov.popup_p);
    }
    draw_widget_chrome(ov.popup_r, g_style.rounding, maybe_disabled(ov.popup_fill), maybe_disabled(ov.popup_border), 0.0f, 0.0f, 0.0f);

    Rect clip_r = {ov.popup_r.x + 2.0f, ov.popup_r.y + 2.0f, ov.popup_r.w - 4.0f, ov.popup_r.h - 4.0f};
    push_clip(clip_r);
    for (int i = 0; i < ov.count; ++i) {
        Rect ir = {ov.popup_r.x + 4.0f, ov.popup_r.y + 4.0f - ov.scroll + i * ov.item_h, ov.popup_r.w - 8.0f, ov.item_h};
        if (ir.y + ir.h < ov.popup_r.y || ir.y > ov.popup_r.y + ov.popup_r.h) continue;
        bool item_hov = ov.open && rect_contains(ir, g_input.mouse_x, g_input.mouse_y);
        bool item_sel = (ov.selected_index == i);
        Color item_bg = item_sel ? ov.item_selected :
                        item_hov ? ov.item_hover :
                                   Color{0.0f, 0.0f, 0.0f, 0.0f};
        if (item_bg.a > 0.0f) fill_round_rect(ir, g_style.rounding * 0.6f, maybe_disabled(item_bg));
        Rect itr = {ir.x + 8.0f, ir.y, ir.w - 16.0f, ir.h};
        draw_text_utf8(ov.labels[i].c_str(), itr, maybe_disabled(item_sel ? ov.item_text_selected : ov.item_text));
    }
    pop_clip();

    float content_h = ov.count * ov.item_h;
    if (content_h > ov.popup_h - 8.0f) {
        float max_scroll = content_h - (ov.popup_h - 8.0f);
        if (max_scroll < 0.0f) max_scroll = 0.0f;
        if (max_scroll > 0.0f) {
            float visible_h = ov.popup_h - 8.0f;
            float thumb_h = (visible_h / content_h) * visible_h;
            if (thumb_h < 12.0f) thumb_h = 12.0f;
            float thumb_y = ov.popup_r.y + 4.0f + (ov.scroll / max_scroll) * (visible_h - thumb_h);
            Rect track_r = {ov.popup_r.x + ov.popup_r.w - 8.0f, ov.popup_r.y + 4.0f, 4.0f, ov.popup_r.h - 8.0f};
            Rect thumb_r = {track_r.x, thumb_y, track_r.w, thumb_h};
            fill_round_rect(track_r, 2.0f, maybe_disabled(ov.scrollbar_track));
            fill_round_rect(thumb_r, 2.0f, maybe_disabled(ov.scrollbar_thumb));
        }
    }
}

static void draw_side_drawer_overlay(int window_w, int window_h) {
    SideDrawerState& drawer = g_side_drawer;
    if (!drawer.active && !drawer.open && drawer.progress <= 0.01f) return;

    float p = drawer.progress * drawer.progress * (3.0f - 2.0f * drawer.progress);
    float ww = (float)window_w;
    float wh = (float)window_h;
    float panel_w = clampf(drawer.width, 180.0f, fmaxf(180.0f, ww - 36.0f));
    float x = -panel_w + panel_w * p;
    Rect screen = {0.0f, 0.0f, ww, wh};
    Rect panel = {x, 0.0f, panel_w, wh};

    if (p > 0.01f) {
        Color scrim = {0.0f, 0.0f, 0.0f, 0.34f * p};
        fill_rect(screen, scrim);

        Color shadow = {0.0f, 0.0f, 0.0f, 0.22f * p};
        fill_rect({panel.x + panel.w, 0.0f, 18.0f, panel.h}, shadow);

        Color bg = lerp_color(resolve_color(ColorRole::Panel), resolve_color(ColorRole::Background), 0.08f);
        bg.a = 0.98f;
        fill_rect(panel, bg);
    }

    Color accent = resolve_color(ColorRole::Accent);
    Color border = resolve_color(ColorRole::Border);
    if (p > 0.01f) {
        border.a = 0.70f * p;
        fill_rect({panel.x + panel.w - 1.0f, 0.0f, 1.0f, panel.h}, border);
        fill_rect({panel.x, 0.0f, 4.0f, panel.h}, with_alpha(accent, 0.55f * p));

        push_clip(panel);
        float pad = 18.0f;
        float title_h = text_line_height() + 22.0f;
        Rect title_r = {panel.x + 62.0f, 14.0f, panel.w - 80.0f, title_h};
        draw_text_utf8(drawer.title.empty() ? "Navigation" : drawer.title.c_str(), title_r,
                       with_alpha(resolve_color(ColorRole::Text), p));
        fill_rect({panel.x + pad, title_r.y + title_h - 7.0f, panel.w - pad * 2.0f, 1.0f},
                  with_alpha(border, 0.55f * p));

        Color text = resolve_color(ColorRole::Text);
        Color text_dim = resolve_color(ColorRole::TextDim);
        Color button = resolve_color(ColorRole::Button);
        Color button_hover = resolve_color(ColorRole::ButtonHover);
        Color selected_bg = lerp_color(resolve_color(ColorRole::ButtonActive), accent, 0.18f);
        selected_bg.a = 0.92f * p;

        float y = 72.0f;
        for (int i = 0; i < drawer.count && i < (int)drawer.labels.size(); ++i) {
            Rect ir = {panel.x + 10.0f, y + i * drawer.item_h, panel.w - 20.0f, drawer.item_h - 5.0f};
            bool hov = drawer.open && rect_contains(ir, g_input.mouse_x, g_input.mouse_y);
            bool sel = i == drawer.selected;
            if (sel) {
                fill_round_rect(ir, fmaxf(4.0f, g_style.rounding * 0.75f), selected_bg);
                fill_round_rect({ir.x + 6.0f, ir.y + 7.0f, 4.0f, ir.h - 14.0f}, 2.0f, with_alpha(accent, 0.95f * p));
            } else if (hov) {
                Color hover = lerp_color(button, button_hover, 0.75f);
                hover.a = 0.80f * p;
                fill_round_rect(ir, fmaxf(4.0f, g_style.rounding * 0.75f), hover);
            }
            Rect tr = {ir.x + 20.0f, ir.y, ir.w - 30.0f, ir.h};
            draw_text_utf8(drawer.labels[i].c_str(), tr,
                           with_alpha(lerp_color(text_dim, text, sel ? 0.78f : (hov ? 0.35f : 0.0f)), p));
        }
        pop_clip();
    }

    Rect lr = drawer.launcher_rect;
    if (lr.w <= 0.0f || lr.h <= 0.0f) lr = {12.0f, 12.0f, 40.0f, 40.0f};
    Color lb = lerp_color(resolve_color(ColorRole::Panel), resolve_color(ColorRole::Button), 0.32f);
    lb = lerp_color(lb, resolve_color(ColorRole::ButtonHover), drawer.launcher_hover * 0.55f);
    lb = lerp_color(lb, resolve_color(ColorRole::ButtonActive), drawer.launcher_active * 0.55f);
    Color lborder = lerp_color(resolve_color(ColorRole::Border), accent, 0.20f + drawer.launcher_hover * 0.22f);
    draw_widget_chrome(lr, fmaxf(8.0f, g_style.rounding), lb, lborder,
                       drawer.launcher_hover, drawer.launcher_active, drawer.open ? 1.0f : 0.0f);
    Color icon = lerp_color(resolve_color(ColorRole::TextDim), resolve_color(ColorRole::Text), 0.60f + drawer.launcher_hover * 0.28f);
    if (drawer.open) {
        draw_line(lr.x + 13.0f, lr.y + 13.0f, lr.x + lr.w - 13.0f, lr.y + lr.h - 13.0f, 2.0f, icon);
        draw_line(lr.x + lr.w - 13.0f, lr.y + 13.0f, lr.x + 13.0f, lr.y + lr.h - 13.0f, 2.0f, icon);
    } else {
        float x0 = lr.x + 12.0f, x1 = lr.x + lr.w - 12.0f;
        draw_line(x0, lr.y + 13.0f, x1, lr.y + 13.0f, 2.0f, icon);
        draw_line(x0, lr.y + 20.0f, x1, lr.y + 20.0f, 2.0f, icon);
        draw_line(x0, lr.y + 27.0f, x1, lr.y + 27.0f, 2.0f, icon);
    }
}

static void draw_command_overlay(int window_w, int window_h) {
    if (!g_cmd.active) return;

    char cmd[96];
    snprintf(cmd, sizeof(cmd), ":%s", g_cmd.buf);
    const char* completion = gui_command_completion(g_cmd.buf);
    const char* suffix = "";
    if (completion && strncmp(completion, g_cmd.buf, strlen(g_cmd.buf)) == 0) {
        suffix = completion + strlen(g_cmd.buf);
    }
    const char* hint = gui_command_hint(g_cmd.buf);
    float lh = text_line_height();
    float pad_x = 12.0f;
    float pad_y = 8.0f;
    float cmd_w = measure_text_width(cmd);
    float suffix_w = measure_text_width(suffix);
    float hint_w = measure_text_width(hint);
    float max_w = fmaxf(220.0f, (float)window_w - g_style.window_padding * 2.0f);
    float w = fminf(max_w, fmaxf(280.0f, cmd_w + suffix_w + hint_w + pad_x * 4.0f + 12.0f));
    float h = lh + pad_y * 2.0f;
    Rect r = {g_style.window_padding, (float)window_h - g_style.window_padding - h, w, h};

    Color bg = lerp_color(resolve_color(ColorRole::Panel), resolve_color(ColorRole::Background), 0.18f);
    bg.a = 0.98f;
    Color border = resolve_color(ColorRole::Border);
    Color accent = resolve_color(ColorRole::Accent);
    draw_widget_chrome(r, fmaxf(6.0f, g_style.rounding), bg, border, 0.0f, 0.0f, 1.0f);
    fill_round_rect({r.x + 7.0f, r.y + 7.0f, 4.0f, r.h - 14.0f}, 2.0f, accent);

    Rect cmd_r = {r.x + pad_x + 10.0f, r.y + pad_y, cmd_w + 4.0f, lh};
    draw_text_utf8(cmd, cmd_r, resolve_color(ColorRole::Text));
    if (suffix && suffix[0]) {
        Rect suffix_r = {cmd_r.x + cmd_w, r.y + pad_y, suffix_w + 4.0f, lh};
        draw_text_utf8(suffix, suffix_r, resolve_color(ColorRole::TextDim));
    }

    Rect hint_r = {r.x + r.w - hint_w - pad_x, r.y + pad_y, hint_w, lh};
    draw_text_utf8(hint, hint_r, resolve_color(ColorRole::TextDim));
}

static float widget_label_height(const char* vis) {
    return (vis && vis[0]) ? (text_line_height() + 6.0f) : 0.0f;
}

static Rect next_rect(float height);

static Rect next_labeled_rect(const char* vis, float body_h, Rect* body_out) {
    float label_h = widget_label_height(vis);
    Rect outer = next_rect(body_h + label_h);
    if (body_out) *body_out = {outer.x, outer.y + label_h, outer.w, body_h};
    return outer;
}

static void draw_widget_label(const char* vis, Rect outer, bool emphasized = false) {
    if (!vis || !vis[0]) return;
    float label_h = widget_label_height(vis);
    Rect lr = {outer.x, outer.y, outer.w, label_h};
    draw_text_utf8(vis, lr, maybe_disabled(lerp_color(resolve_color(ColorRole::TextDim), resolve_color(ColorRole::Text), emphasized ? 0.22f : 0.0f)));
}

static Rect rect_intersection(Rect a, Rect b) {
    float x0 = a.x > b.x ? a.x : b.x;
    float y0 = a.y > b.y ? a.y : b.y;
    float x1 = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    float y1 = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    if (x1 < x0) x1 = x0;
    if (y1 < y0) y1 = y0;
    return {x0, y0, x1 - x0, y1 - y0};
}

static float transformed_layout_y(float target_y) {
    float y = target_y;
    for (int i = 0; i < g_active_collapse_fx_count; ++i) {
        const ActiveCollapseFx& fx = g_active_collapse_fx[i];
        if (fx.body_height <= 0.0f) continue;
        float body_end = fx.top_target + fx.body_height;
        if (target_y >= body_end - 0.5f) {
            y -= fx.body_height * (1.0f - fx.progress);
        }
    }
    return y;
}

static CollapseRectFx collapse_rect_fx(Rect target) {
    CollapseRectFx out = {};
    out.rect = target;

    for (int i = 0; i < g_active_collapse_fx_count; ++i) {
        const ActiveCollapseFx& fx = g_active_collapse_fx[i];
        if (fx.body_height <= 0.0f) continue;
        float body_end = fx.top_target + fx.body_height;
        float hidden_h = fx.body_height * (1.0f - fx.progress);
        if (target.y >= body_end - 0.5f) {
            out.rect.y -= hidden_h;
        } else if (target.y + target.h > fx.top_target && target.y < body_end) {
            float visible_h = fx.body_height * fx.progress;
            Rect clip = {
                g_ctx.content_region.x - 4.0f,
                fx.top_display,
                g_ctx.content_region.w + 8.0f,
                visible_h
            };
            if (out.rect.y < clip.y) {
                float delta = clip.y - out.rect.y;
                out.rect.y = clip.y;
                out.rect.h -= delta;
            }
            float clip_bottom = clip.y + clip.h;
            float rect_bottom = out.rect.y + out.rect.h;
            if (rect_bottom > clip_bottom) out.rect.h = clip_bottom - out.rect.y;
            if (out.rect.h < 0.0f) out.rect.h = 0.0f;
            out.clip = out.clip_active ? rect_intersection(out.clip, clip) : clip;
            out.clip_active = true;
        }
    }

    return out;
}

static Rect labeled_body_rect(const char* vis, Rect outer) {
    float label_h = widget_label_height(vis);
    float shown_label_h = outer.h < label_h ? outer.h : label_h;
    float body_h = outer.h - shown_label_h;
    if (body_h < 0.0f) body_h = 0.0f;
    return {outer.x, outer.y + shown_label_h, outer.w, body_h};
}

static float tooltip_delay_seconds() {
    if (g_tooltip.dismiss_streak >= 4) return 10.0f;
    if (g_tooltip.dismiss_streak >= 2) return 5.0f;
    return 1.0f;
}

static void begin_tooltip_frame() {
    g_tooltip.requested = false;
    g_tooltip.request_id = 0;
    g_tooltip.request_text.clear();
}

static void finalize_tooltip_frame() {
    if (g_tooltip.requested) {
        if (g_tooltip.hover_id != g_tooltip.request_id) {
            if (g_tooltip.visible && g_tooltip.visible_time < 0.8f) g_tooltip.dismiss_streak++;
            g_tooltip.hover_id = g_tooltip.request_id;
            g_tooltip.hover_time = 0.0f;
            g_tooltip.visible_time = 0.0f;
            g_tooltip.calm_time = 0.0f;
            g_tooltip.static_x = g_input.mouse_x;
            g_tooltip.static_y = g_input.mouse_y;
        } else {
            float dx = g_input.mouse_x - g_tooltip.static_x;
            float dy = g_input.mouse_y - g_tooltip.static_y;
            float moved2 = dx * dx + dy * dy;
            if (moved2 <= 4.0f) {
                g_tooltip.hover_time += g_dt;
            } else {
                if (g_tooltip.visible && g_tooltip.visible_time < 0.8f) g_tooltip.dismiss_streak++;
                g_tooltip.hover_time = 0.0f;
                g_tooltip.visible_time = 0.0f;
                g_tooltip.calm_time = 0.0f;
                g_tooltip.static_x = g_input.mouse_x;
                g_tooltip.static_y = g_input.mouse_y;
            }
        }
        g_tooltip.anchor = g_tooltip.request_anchor;
        g_tooltip.text = g_tooltip.request_text;
    } else {
        if (g_tooltip.visible && g_tooltip.visible_time < 0.8f) g_tooltip.dismiss_streak++;
        g_tooltip.hover_id = 0;
        g_tooltip.hover_time = 0.0f;
        g_tooltip.visible_time = 0.0f;
        g_tooltip.calm_time = 0.0f;
    }

    g_tooltip.visible = g_tooltip.requested && g_tooltip.hover_time >= tooltip_delay_seconds();
    if (g_tooltip.visible) {
        g_tooltip.visible_time += g_dt;
        g_tooltip.calm_time += g_dt;
        if (g_tooltip.calm_time >= 1.25f && g_tooltip.dismiss_streak > 0) {
            g_tooltip.dismiss_streak--;
            g_tooltip.calm_time = 0.0f;
        }
    }
    g_tooltip.opacity = step_anim(g_tooltip.opacity, g_tooltip.visible ? 1.0f : 0.0f, g_dt, 10.0f);
    g_tooltip.active = g_tooltip.opacity > 0.01f;
}

static bool filter_ascii_input_char(char c, InputFlags flags, char& out) {
    out = c;
    if (flags & InputFlags::CharsUppercase) {
        if (c >= 'a' && c <= 'z') out = (char)(c - 'a' + 'A');
    }
    if (flags & InputFlags::CharsNoBlank) {
        if (c == ' ' || c == '\t') return false;
    }
    if (flags & InputFlags::CharsDecimal) {
        if (!((out >= '0' && out <= '9') || out == '.' || out == '-' || out == '+')) return false;
    }
    if (flags & InputFlags::CharsHexadecimal) {
        if (!((out >= '0' && out <= '9') || (out >= 'a' && out <= 'f') || (out >= 'A' && out <= 'F'))) return false;
    }
    return true;
}

static Rect next_rect(float height) {
    float region_x = g_ctx.cursor_x;
    float region_y = g_ctx.cursor_y;
    float region_w = g_ctx.content_region.w;

    if (g_ctx.row_ctx.active) {
        auto& rc = g_ctx.row_ctx;
        int row_index = rc.col_index < rc.cols ? rc.col_index : (rc.cols - 1);
        region_x = rc.start_x + rc.used_x;
        region_y = rc.start_y;
        if (rc.split && rc.weights) {
            float spec = rc.weights[row_index];
            if (spec >= 16.0f) {
                region_w = spec;
            } else {
                float available = rc.total_w - rc.gap * (rc.cols - 1) - rc.fixed_total;
                if (available < 0.0f) available = 0.0f;
                region_w = available * ((spec > 0.0f ? spec : 1.0f) / rc.flex_total);
            }
        } else {
            region_w = rc.weighted && rc.weights
            ? (rc.total_w - rc.gap * (rc.cols - 1)) * (rc.weights[row_index] / rc.total_weight)
            : rc.cell_w;
        }
        rc.used_x += region_w + (rc.col_index + 1 < rc.cols ? rc.gap : 0.0f);
        rc.col_index++;
        if (height > rc.row_height) rc.row_height = height;
    } else {
        g_ctx.cursor_y += height + g_style.item_spacing;
    }

    float width = region_w;
    if (g_next_layout.active) {
        if (g_next_layout.has_width) width = g_next_layout.width;
        else if (g_next_layout.has_percent) width = region_w * g_next_layout.percent;
        else if (g_next_layout.fill) width = region_w;

        if (g_next_layout.has_limits) {
            width = clampf(width, g_next_layout.min, g_next_layout.max);
        }
        if (width > region_w) width = region_w;
        if (width < 1.0f) width = 1.0f;
    }

    float x = region_x;
    Align align = g_next_layout.active ? g_next_layout.align : Align::Start;
    if (width < region_w) {
        if (align == Align::Center) x += (region_w - width) * 0.5f;
        else if (align == Align::End) x += (region_w - width);
    }

    Rect r = {x, region_y, width, height};
    g_next_layout = {};

    if (g_ctx.row_ctx.active) return r;
    return r;
}

static void cmd_clear() { g_cmd.active = false; g_cmd.len = 0; g_cmd.buf[0] = '\0'; }

struct GuiCommandSpec {
    const char* command;
    const char* hint;
};

static const GuiCommandSpec k_gui_commands[] = {
    {"q", "quit"},
    {"quit", "quit"},
    {"td", "default dark"},
    {"tc", "catppuccin"},
    {"tn", "nord"},
    {"tg", "gruvbox"},
    {"to", "one dark"},
    {"th", "ghostty green"},
    {"fps60", "set fps"},
    {"fps30", "set fps"},
    {"ds4", "dither size"},
    {"op90", "opacity"},
    {"df", "toggle fps"},
    {"dl", "layout rects"},
    {"di", "debug ids"},
    {"dw", "widget log"},
    {"fx", "toggle effects"},
    {"fx0", "effects off"},
    {"fx1", "effects on"},
    {"rr", "redraw"},
    {"ct", "clear toasts"},
    {"blur", "blur window"},
    {"dit", "dither window"},
    {"dither", "dither window"},
    {"plain", "plain transparency"},
    {"opaque", "opaque window"},
    {"bb", "blur backdrop"},
    {"bd", "dither backdrop"},
    {"web", "web server"},
    {"ws", "web server"},
    {"server", "web server"},
    {"webserver", "web server"},
};

static const char* gui_command_completion(const char* prefix) {
    if (!prefix || !*prefix) return "";
    const char* best = "";
    size_t prefix_len = strlen(prefix);
    for (const GuiCommandSpec& spec : k_gui_commands) {
        if (strncmp(spec.command, prefix, prefix_len) == 0) {
            if (!*best || strlen(spec.command) < strlen(best)) best = spec.command;
        }
    }
    return best;
}

static const char* gui_command_hint(const char* prefix) {
    if (!prefix || !*prefix) return "tab complete  enter run  esc cancel";
    for (const GuiCommandSpec& spec : k_gui_commands) {
        if (strcmp(spec.command, prefix) == 0) return spec.hint;
    }
    const char* completion = gui_command_completion(prefix);
    if (completion && *completion) {
        for (const GuiCommandSpec& spec : k_gui_commands) {
            if (strcmp(spec.command, completion) == 0) return spec.hint;
        }
    }
    return "tab complete  enter run  esc cancel";
}

static void gui_command_accept_completion() {
    const char* completion = gui_command_completion(g_cmd.buf);
    if (!completion || !*completion) return;
    snprintf(g_cmd.buf, sizeof(g_cmd.buf), "%s", completion);
    g_cmd.len = (int)strlen(g_cmd.buf);
}

static void set_theme_env(const char* name) {
    if (!name || !name[0]) return;
#ifdef _WIN32
    _putenv_s("FTUI_THEME", name);
#else
    setenv("FTUI_THEME", name, 1);
#endif
}

static void apply_named_theme(const char* name, bool persist) {
    g_style = style_from_name(name);
    g_style_name = style_name(g_style);
    if (persist) set_theme_env(g_style_name.c_str());
    g_redraw_requested = true;
    wake_event_loop();
    sync_native_window_chrome();
}

static void apply_cmd_theme() {
    if      (strcmp(g_cmd.buf, "td") == 0) apply_named_theme("default-dark", true);
    else if (strcmp(g_cmd.buf, "tc") == 0) apply_named_theme("catppuccin-mocha", true);
    else if (strcmp(g_cmd.buf, "tn") == 0) apply_named_theme("nord", true);
    else if (strcmp(g_cmd.buf, "tg") == 0) apply_named_theme("gruvbox-dark", true);
    else if (strcmp(g_cmd.buf, "to") == 0) apply_named_theme("one-dark", true);
    else if (strcmp(g_cmd.buf, "th") == 0) apply_named_theme("ghostty-green", true);
}

static bool parse_cmd_int(const char* prefix, int* out) {
    size_t n = strlen(prefix);
    if (strncmp(g_cmd.buf, prefix, n) != 0 || !g_cmd.buf[n]) return false;
    char* end = nullptr;
    long v = strtol(g_cmd.buf + n, &end, 10);
    if (!end || *end != '\0') return false;
    *out = (int)v;
    return true;
}

static bool apply_cmd_action() {
    if (!g_cmd.buf[0]) return false;
    if (g_command_handler && g_command_handler(g_cmd.buf)) {
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }

    const std::string before_theme = g_style_name;
    apply_cmd_theme();
    if (g_style_name != before_theme || strcmp(g_cmd.buf, "td") == 0 ||
        strcmp(g_cmd.buf, "tc") == 0 || strcmp(g_cmd.buf, "tn") == 0 ||
        strcmp(g_cmd.buf, "tg") == 0 || strcmp(g_cmd.buf, "to") == 0 ||
        strcmp(g_cmd.buf, "th") == 0) {
        return true;
    }

    int value = 0;
    if (parse_cmd_int("fps", &value)) {
        g_frame_limit_fps = value;
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }
    if (parse_cmd_int("ds", &value)) {
        g_dither_size = value < 1 ? 1 : (value > 32 ? 32 : value);
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }
    if (parse_cmd_int("op", &value)) {
        g_window_opacity = clampf((float)value / 100.0f, 0.20f, 1.0f);
        sync_native_window_transparency();
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }

    if (strcmp(g_cmd.buf, "df") == 0) { g_debug.show_fps = !g_debug.show_fps; g_redraw_requested = true; return true; }
    if (strcmp(g_cmd.buf, "dl") == 0) { g_debug.show_layout_rects = !g_debug.show_layout_rects; g_redraw_requested = true; return true; }
    if (strcmp(g_cmd.buf, "di") == 0) {
        bool next = !(g_debug.show_hovered_id || g_debug.show_active_id);
        g_debug.show_hovered_id = next;
        g_debug.show_active_id = next;
        g_redraw_requested = true;
        return true;
    }
    if (strcmp(g_cmd.buf, "dw") == 0) { g_debug.log_widget_calls = !g_debug.log_widget_calls; g_redraw_requested = true; return true; }
    if (strcmp(g_cmd.buf, "fx") == 0) { g_effects_enabled = !g_effects_enabled; g_redraw_requested = true; wake_event_loop(); return true; }
    if (strcmp(g_cmd.buf, "fx0") == 0) { g_effects_enabled = false; g_redraw_requested = true; return true; }
    if (strcmp(g_cmd.buf, "fx1") == 0) { g_effects_enabled = true; g_redraw_requested = true; wake_event_loop(); return true; }
    if (strcmp(g_cmd.buf, "rr") == 0) { g_redraw_requested = true; wake_event_loop(); return true; }
    if (strcmp(g_cmd.buf, "ct") == 0) {
        for (ToastSlot& s : g_toasts) s = {};
        g_redraw_requested = true;
        return true;
    }

    if (strcmp(g_cmd.buf, "blur") == 0) {
        g_window_transparency = WindowTransparency::Blur;
        sync_native_window_transparency();
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }
    if (strcmp(g_cmd.buf, "dit") == 0 || strcmp(g_cmd.buf, "dither") == 0) {
        g_window_transparency = WindowTransparency::BayerDither;
        sync_native_window_transparency();
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }
    if (strcmp(g_cmd.buf, "plain") == 0) {
        g_window_transparency = WindowTransparency::Plain;
        sync_native_window_transparency();
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }
    if (strcmp(g_cmd.buf, "opaque") == 0) {
        g_window_transparency = WindowTransparency::Opaque;
        sync_native_window_transparency();
        g_redraw_requested = true;
        wake_event_loop();
        return true;
    }
    if (strcmp(g_cmd.buf, "bb") == 0) { g_backdrop_effect = BackdropEffect::Blur; g_redraw_requested = true; return true; }
    if (strcmp(g_cmd.buf, "bd") == 0) { g_backdrop_effect = BackdropEffect::BayerDither; g_redraw_requested = true; return true; }

    return false;
}

static bool has_ext(const char* path, const char* ext) {
    if (!path || !ext) return false;
    size_t lp = strlen(path), le = strlen(ext);
    if (lp < le) return false;
    const char* p = path + lp - le;
    for (size_t i = 0; i < le; ++i) {
        char a = (char)tolower((unsigned char)p[i]);
        char b = (char)tolower((unsigned char)ext[i]);
        if (a != b) return false;
    }
    return true;
}

static bool looks_like_mask_path(const char* s) {
    return has_ext(s, ".svg") || has_ext(s, ".png") || has_ext(s, ".jpg") ||
           has_ext(s, ".jpeg") || has_ext(s, ".bmp");
}

static float attr_float(const std::string& tag, const char* name, float fallback) {
    std::string key(name);
    size_t p = tag.find(key);
    while (p != std::string::npos) {
        size_t q = p + key.size();
        while (q < tag.size() && isspace((unsigned char)tag[q])) q++;
        if (q < tag.size() && tag[q] == '=') {
            q++;
            while (q < tag.size() && isspace((unsigned char)tag[q])) q++;
            if (q < tag.size() && (tag[q] == '"' || tag[q] == '\'')) q++;
            return (float)strtod(tag.c_str() + q, nullptr);
        }
        p = tag.find(key, p + key.size());
    }
    return fallback;
}

static std::string attr_string(const std::string& tag, const char* name) {
    std::string key(name);
    size_t p = tag.find(key);
    while (p != std::string::npos) {
        size_t q = p + key.size();
        while (q < tag.size() && isspace((unsigned char)tag[q])) q++;
        if (q < tag.size() && tag[q] == '=') {
            q++;
            while (q < tag.size() && isspace((unsigned char)tag[q])) q++;
            if (q >= tag.size()) return "";
            char quote = tag[q];
            if (quote != '"' && quote != '\'') return "";
            q++;
            size_t e = tag.find(quote, q);
            if (e == std::string::npos) return "";
            return tag.substr(q, e - q);
        }
        p = tag.find(key, p + key.size());
    }
    return "";
}

struct MaskPoint { float x = 0.0f, y = 0.0f; };

static void fill_mask_rect(MaskData& m, int x0, int y0, int x1, int y1) {
    if (m.w <= 0 || m.h <= 0) return;
    if (x0 > x1) { int t = x0; x0 = x1; x1 = t; }
    if (y0 > y1) { int t = y0; y0 = y1; y1 = t; }
    x0 = x0 < 0 ? 0 : (x0 > m.w ? m.w : x0);
    x1 = x1 < 0 ? 0 : (x1 > m.w ? m.w : x1);
    y0 = y0 < 0 ? 0 : (y0 > m.h ? m.h : y0);
    y1 = y1 < 0 ? 0 : (y1 > m.h ? m.h : y1);
    for (int y = y0; y < y1; ++y)
        for (int x = x0; x < x1; ++x)
            m.bits[(size_t)y * (size_t)m.w + (size_t)x] = 255;
}

static void fill_mask_ellipse(MaskData& m, float cx, float cy, float rx, float ry) {
    if (rx <= 0.0f || ry <= 0.0f) return;
    int x0 = (int)floorf(cx - rx), x1 = (int)ceilf(cx + rx);
    int y0 = (int)floorf(cy - ry), y1 = (int)ceilf(cy + ry);
    for (int y = y0; y <= y1; ++y) {
        if (y < 0 || y >= m.h) continue;
        for (int x = x0; x <= x1; ++x) {
            if (x < 0 || x >= m.w) continue;
            float dx = ((float)x + 0.5f - cx) / rx;
            float dy = ((float)y + 0.5f - cy) / ry;
            if (dx * dx + dy * dy <= 1.0f) m.bits[(size_t)y * (size_t)m.w + (size_t)x] = 255;
        }
    }
}

static void fill_mask_polygon(MaskData& m, const std::vector<MaskPoint>& pts) {
    if (pts.size() < 3 || m.w <= 0 || m.h <= 0) return;
    float min_y = pts[0].y, max_y = pts[0].y;
    for (const MaskPoint& p : pts) { if (p.y < min_y) min_y = p.y; if (p.y > max_y) max_y = p.y; }
    int y0 = (int)floorf(min_y), y1 = (int)ceilf(max_y);
    for (int y = y0; y <= y1; ++y) {
        if (y < 0 || y >= m.h) continue;
        float scan = (float)y + 0.5f;
        std::vector<float> xs;
        for (size_t i = 0, j = pts.size() - 1; i < pts.size(); j = i++) {
            const MaskPoint& a = pts[j];
            const MaskPoint& b = pts[i];
            if ((a.y > scan) != (b.y > scan)) {
                float t = (scan - a.y) / (b.y - a.y);
                xs.push_back(a.x + t * (b.x - a.x));
            }
        }
        for (size_t i = 0; i + 1 < xs.size(); i += 2) {
            for (size_t j = i + 1; j < xs.size(); ++j) {
                if (xs[j] < xs[i]) { float t = xs[i]; xs[i] = xs[j]; xs[j] = t; }
            }
            int x0 = (int)floorf(xs[i]), x1 = (int)ceilf(xs[i + 1]);
            if (x0 < 0) x0 = 0;
            if (x1 > m.w) x1 = m.w;
            for (int x = x0; x < x1; ++x) m.bits[(size_t)y * (size_t)m.w + (size_t)x] = 255;
        }
    }
}

static const char* skip_svg_sep(const char* p) {
    while (*p && (isspace((unsigned char)*p) || *p == ',')) p++;
    return p;
}

static bool parse_svg_number(const char*& p, float& out) {
    p = skip_svg_sep(p);
    if (!*p) return false;
    char* end = nullptr;
    out = (float)strtod(p, &end);
    if (end == p) return false;
    p = end;
    return true;
}

static void fill_svg_path(MaskData& m, const std::string& d) {
    const char* p = d.c_str();
    char cmd = 0;
    MaskPoint cur = {}, start = {};
    std::vector<MaskPoint> poly;
    while (*p) {
        p = skip_svg_sep(p);
        if (!*p) break;
        if (isalpha((unsigned char)*p)) cmd = *p++;
        bool rel = cmd >= 'a' && cmd <= 'z';
        char c = (char)tolower((unsigned char)cmd);
        if (c == 'z') {
            if (poly.size() >= 3) fill_mask_polygon(m, poly);
            poly.clear(); cur = start; cmd = 0; continue;
        }
        if (c == 'm' || c == 'l') {
            float x = 0, y = 0;
            if (!parse_svg_number(p, x) || !parse_svg_number(p, y)) break;
            if (rel) { x += cur.x; y += cur.y; }
            cur = {x, y};
            if (c == 'm') { if (poly.size() >= 3) fill_mask_polygon(m, poly); poly.clear(); start = cur; cmd = rel ? 'l' : 'L'; }
            poly.push_back(cur);
        } else if (c == 'h') {
            float x = 0; if (!parse_svg_number(p, x)) break;
            cur.x = rel ? cur.x + x : x; poly.push_back(cur);
        } else if (c == 'v') {
            float y = 0; if (!parse_svg_number(p, y)) break;
            cur.y = rel ? cur.y + y : y; poly.push_back(cur);
        } else if (c == 'c' || c == 'q' || c == 's' || c == 't') {
            int pairs = c == 'c' ? 3 : c == 's' || c == 'q' ? 2 : 1;
            float x = cur.x, y = cur.y;
            for (int i = 0; i < pairs; ++i) {
                if (!parse_svg_number(p, x) || !parse_svg_number(p, y)) break;
            }
            if (rel) { x += cur.x; y += cur.y; }
            cur = {x, y}; poly.push_back(cur);
        } else {
            p++;
        }
    }
    if (poly.size() >= 3) fill_mask_polygon(m, poly);
}

static bool load_mask_from_svg_text(const char* key, const std::string& s, MaskData& out) {
    float w = 0.0f, h = 0.0f;
    size_t svg_p = s.find("<svg");
    if (svg_p != std::string::npos) {
        size_t svg_e = s.find('>', svg_p);
        std::string tag = s.substr(svg_p, svg_e == std::string::npos ? 256 : svg_e - svg_p + 1);
        w = attr_float(tag, "width", 0.0f);
        h = attr_float(tag, "height", 0.0f);
        std::string vb = attr_string(tag, "viewBox");
        if ((w <= 0.0f || h <= 0.0f) && !vb.empty()) {
            std::stringstream vs(vb);
            float x0, y0, vw, vh;
            if (vs >> x0 >> y0 >> vw >> vh) { w = vw; h = vh; }
        }
    }
    if (w <= 0.0f) w = 128.0f;
    if (h <= 0.0f) h = 32.0f;
    out.w = (int)clampf(w, 8.0f, 512.0f);
    out.h = (int)clampf(h, 8.0f, 512.0f);
    out.bits.assign((size_t)out.w * (size_t)out.h, 0);

    for (size_t p = 0; (p = s.find("<rect", p)) != std::string::npos; ++p) {
        size_t e = s.find('>', p); if (e == std::string::npos) break;
        std::string tag = s.substr(p, e - p + 1);
        float x = attr_float(tag, "x", 0), y = attr_float(tag, "y", 0);
        float rw = attr_float(tag, "width", (float)out.w), rh = attr_float(tag, "height", (float)out.h);
        fill_mask_rect(out, (int)x, (int)y, (int)(x + rw), (int)(y + rh));
    }
    for (size_t p = 0; (p = s.find("<circle", p)) != std::string::npos; ++p) {
        size_t e = s.find('>', p); if (e == std::string::npos) break;
        std::string tag = s.substr(p, e - p + 1);
        float cx = attr_float(tag, "cx", out.w * 0.5f), cy = attr_float(tag, "cy", out.h * 0.5f);
        float r = attr_float(tag, "r", fminf(out.w, out.h) * 0.5f);
        fill_mask_ellipse(out, cx, cy, r, r);
    }
    for (size_t p = 0; (p = s.find("<ellipse", p)) != std::string::npos; ++p) {
        size_t e = s.find('>', p); if (e == std::string::npos) break;
        std::string tag = s.substr(p, e - p + 1);
        fill_mask_ellipse(out, attr_float(tag, "cx", out.w * 0.5f), attr_float(tag, "cy", out.h * 0.5f),
                          attr_float(tag, "rx", out.w * 0.5f), attr_float(tag, "ry", out.h * 0.5f));
    }
    for (size_t p = 0; (p = s.find("<path", p)) != std::string::npos; ++p) {
        size_t e = s.find('>', p); if (e == std::string::npos) break;
        std::string tag = s.substr(p, e - p + 1);
        std::string d = attr_string(tag, "d");
        if (!d.empty()) fill_svg_path(out, d);
    }
    out.loaded = true;
    out.path = key ? key : "";
    return true;
}

static bool load_mask_from_svg(const char* path, MaskData& out) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;
    std::stringstream ss; ss << in.rdbuf();
    return load_mask_from_svg_text(path, ss.str(), out);
}

static bool is_builtin_mask_shape(const char* shape) {
    return shape &&
           (strcmp(shape, "battery") == 0 || strcmp(shape, "tank") == 0 ||
            strcmp(shape, "pill") == 0 || strcmp(shape, "circle") == 0 ||
            strcmp(shape, "logo") == 0);
}

static bool load_builtin_mask_shape(const char* shape, MaskData& out) {
    if (!shape) return false;
    const int s = 4;
    out.w = 160 * s;
    out.h = 64 * s;
    out.bits.assign((size_t)out.w * (size_t)out.h, 0);
    if (strcmp(shape, "battery") == 0) {
        fill_mask_rect(out, 4 * s, 12 * s, 136 * s, 52 * s);
        fill_mask_rect(out, 140 * s, 24 * s, 156 * s, 40 * s);
    } else if (strcmp(shape, "tank") == 0) {
        fill_mask_rect(out, 8 * s, 16 * s, 152 * s, 54 * s);
        fill_mask_rect(out, 24 * s, 8 * s, 136 * s, 20 * s);
    } else if (strcmp(shape, "pill") == 0) {
        fill_mask_rect(out, 32 * s, 8 * s, 128 * s, 56 * s);
        fill_mask_ellipse(out, 32.0f * s, 32.0f * s, 24.0f * s, 24.0f * s);
        fill_mask_ellipse(out, 128.0f * s, 32.0f * s, 24.0f * s, 24.0f * s);
    } else if (strcmp(shape, "circle") == 0) {
        out.w = 96 * s; out.h = 96 * s; out.bits.assign((size_t)out.w * (size_t)out.h, 0);
        fill_mask_ellipse(out, 48.0f * s, 48.0f * s, 44.0f * s, 44.0f * s);
    } else if (strcmp(shape, "logo") == 0) {
        std::vector<MaskPoint> tri = {{80.0f * s, 4.0f * s}, {156.0f * s, 58.0f * s}, {4.0f * s, 58.0f * s}};
        fill_mask_polygon(out, tri);
        fill_mask_rect(out, 68 * s, 22 * s, 92 * s, 60 * s);
    } else {
        return false;
    }
    out.loaded = true;
    out.path = std::string("shape:") + shape;
    return true;
}

static MaskData* mask_for_key(const char* key) {
    if (!key || !key[0]) return nullptr;
    for (MaskData& m : g_mask_cache) {
        if (m.loaded && m.path == key) return &m;
    }
    MaskData* slot = &g_mask_cache[0];
    for (MaskData& m : g_mask_cache) {
        if (!m.loaded) { slot = &m; break; }
    }
    return slot;
}

static MaskData* mask_for_path(const char* path) {
    MaskData* slot = mask_for_key(path);
    if (!slot) return nullptr;
    if (slot->loaded) return slot;
    *slot = {};
    bool ok = has_ext(path, ".svg") ? load_mask_from_svg(path, *slot) : load_mask_from_image(path, *slot);
    if (!ok || slot->bits.empty()) { *slot = {}; return nullptr; }
    slot->path = path;
    slot->loaded = true;
    return slot;
}

static MaskData* mask_for_svg(const char* svg) {
    if (!svg || !svg[0]) return nullptr;
    std::string key = std::string("svg:") + svg;
    MaskData* slot = mask_for_key(key.c_str());
    if (!slot) return nullptr;
    if (slot->loaded) return slot;
    *slot = {};
    bool ok = load_mask_from_svg_text(key.c_str(), svg, *slot);
    if (!ok || slot->bits.empty()) { *slot = {}; return nullptr; }
    return slot;
}

static MaskData* mask_for_shape(const char* shape) {
    if (!shape || !shape[0]) return nullptr;
    std::string key = std::string("shape:") + shape;
    MaskData* slot = mask_for_key(key.c_str());
    if (!slot) return nullptr;
    if (slot->loaded) return slot;
    *slot = {};
    bool ok = load_builtin_mask_shape(shape, *slot);
    if (!ok || slot->bits.empty()) { *slot = {}; return nullptr; }
    return slot;
}

static Color toast_accent(ToastType type) {
    switch (type) {
        case ToastType::Success: return resolve_color(ColorRole::Success);
        case ToastType::Warning: return resolve_color(ColorRole::Warning);
        case ToastType::Error:   return {0.941f, 0.267f, 0.267f, 1.0f};
        case ToastType::Info:    return resolve_color(ColorRole::Accent);
    }
    return resolve_color(ColorRole::Accent);
}

static unsigned soft_dither_hash(unsigned x, unsigned y) {
    unsigned h = x * 0x8da6b343u ^ y * 0xd8163841u ^ 0x9e3779b9u;
    h ^= h >> 16;
    h *= 0x7feb352du;
    h ^= h >> 15;
    h *= 0x846ca68bu;
    h ^= h >> 16;
    return h;
}

static unsigned char byte01(float v) {
    v = clamp01(v);
    return (unsigned char)(v * 255.0f + 0.5f);
}

static void write_premul_bgra(std::vector<unsigned char>& px, int w, int h, int x, int y, Color c) {
    if (x < 0 || y < 0 || x >= w || y >= h) return;
    c.a = clamp01(c.a);
    size_t p = ((size_t)y * (size_t)w + (size_t)x) * 4u;
    px[p + 0] = byte01(c.b * c.a);
    px[p + 1] = byte01(c.g * c.a);
    px[p + 2] = byte01(c.r * c.a);
    px[p + 3] = byte01(c.a);
}

static void build_soft_dither_tile(int cell, std::vector<unsigned char>& px, int& tile_w, int& tile_h) {
    cell = cell < 1 ? 1 : (cell > 32 ? 32 : cell);
    const int cells = 16;
    tile_w = cell * cells;
    tile_h = tile_w;
    px.assign((size_t)tile_w * (size_t)tile_h * 4u, 0);

    Color accent = resolve_color(ColorRole::Accent);
    Color dim = resolve_color(ColorRole::TextDim);
    for (int gy = 0; gy < cells; ++gy) {
        for (int gx = 0; gx < cells; ++gx) {
            unsigned h = soft_dither_hash((unsigned)gx, (unsigned)gy);
            float t = (float)(h & 255u) / 255.0f;
            Color c = lerp_color(dim, accent, 0.24f + t * 0.56f);
            c.a = 0.030f + t * 0.050f;

            int sx = gx * cell + (int)((h >> 8) % (unsigned)cell);
            int sy = gy * cell + (int)((h >> 13) % (unsigned)cell);
            int len = cell <= 2 ? 1 : (cell * 3) / 4;
            for (int k = 0; k < len; ++k) {
                int x = gx * cell + ((sx - gx * cell + k) % cell);
                int y = gy * cell + ((sy - gy * cell + (k / 2)) % cell);
                write_premul_bgra(px, tile_w, tile_h, x, y, c);
                if (cell >= 7 && (k & 1) == 0) write_premul_bgra(px, tile_w, tile_h, x + 1, y, with_alpha(c, c.a * 0.55f));
            }

            if (((h >> 21) & 3u) == 0u) {
                Color dot = lerp_color(dim, accent, 0.72f);
                dot.a = 0.055f + t * 0.030f;
                int dx = gx * cell + (int)((h >> 24) % (unsigned)cell);
                int dy = gy * cell + (int)((h >> 28) % (unsigned)cell);
                write_premul_bgra(px, tile_w, tile_h, dx, dy, dot);
            }
        }
    }
}

static void draw_dither_backdrop_panel(Rect r, float opacity) {
    if (opacity <= 0.001f) return;
    Color bg = lerp_color(resolve_color(ColorRole::Background), resolve_color(ColorRole::Panel), 0.45f);
    bg.a = 0.56f * opacity;
    fill_round_rect(r, g_style.rounding, bg);
    push_clip(r);
    draw_dither_pattern(r, opacity);
    pop_clip();
}

static bool toasts_active() {
    for (const ToastSlot& s : g_toasts) if (s.active) return true;
    return false;
}

static void draw_toast_overlay() {
    if (!g_drawing) return;
    float margin = 18.0f;
    float max_w = fminf(380.0f, fmaxf(240.0f, g_ctx.content_region.w * 0.42f));
    float y = g_ctx.content_region.y + margin;

    for (ToastSlot& slot : g_toasts) {
        if (!slot.active) continue;
        slot.elapsed += g_dt;
        float duration = slot.toast.duration_ms > 0 ? (float)slot.toast.duration_ms / 1000.0f : 999999.0f;
        float in_t = clamp01(slot.elapsed / 0.18f);
        float out_t = slot.elapsed > duration ? clamp01((slot.elapsed - duration) / 0.22f) : 0.0f;
        float alpha = ease_smooth(in_t) * (1.0f - ease_smooth(out_t));
        if (alpha <= 0.01f && slot.elapsed > duration) {
            slot.active = false;
            continue;
        }

        std::vector<TextRange> lines;
        compute_wrapped_ranges(slot.message.c_str(), max_w - 54.0f, true, lines);
        float lh = text_line_height();
        float h = fmaxf(48.0f, 18.0f + (float)lines.size() * lh);
        Rect r = {
            g_ctx.content_region.x + g_ctx.content_region.w - max_w - margin + (1.0f - in_t) * 18.0f,
            y,
            max_w,
            h
        };
        y += h + 10.0f;

        Color panel = lerp_color(resolve_color(ColorRole::Panel), resolve_color(ColorRole::Background), 0.12f);
        Color border = resolve_color(ColorRole::Border);
        Color accent = toast_accent(slot.toast.type);
        panel.a = 0.96f * alpha;
        border = lerp_color(border, accent, 0.35f);
        border.a *= alpha;
        accent.a *= alpha;
        draw_widget_chrome(r, fmaxf(6.0f, g_style.rounding), panel, border, 0.0f, 0.0f, 0.0f);
        fill_round_rect({r.x + 8.0f, r.y + 9.0f, 5.0f, r.h - 18.0f}, 2.5f, accent);

        Color text = resolve_color(ColorRole::Text); text.a *= alpha;
        for (size_t i = 0; i < lines.size(); ++i) {
            TextRange tr = lines[i];
            std::string line(slot.message.c_str() + tr.start, slot.message.c_str() + tr.end);
            Rect lr = {r.x + 24.0f, r.y + 9.0f + (float)i * lh, r.w - 58.0f, lh};
            draw_text_utf8(line.c_str(), lr, text);
        }

        if (slot.toast.dismissible) {
            Rect xr = {r.x + r.w - 30.0f, r.y + 9.0f, 20.0f, 20.0f};
            bool hov = rect_contains(xr, g_input.mouse_x, g_input.mouse_y);
            Color xcol = lerp_color(resolve_color(ColorRole::TextDim), resolve_color(ColorRole::Text), hov ? 0.8f : 0.0f);
            xcol.a *= alpha;
            draw_line(xr.x + 6.0f, xr.y + 6.0f, xr.x + xr.w - 6.0f, xr.y + xr.h - 6.0f, 1.6f, xcol);
            draw_line(xr.x + xr.w - 6.0f, xr.y + 6.0f, xr.x + 6.0f, xr.y + xr.h - 6.0f, 1.6f, xcol);
            if (hov && g_input.mouse_pressed) slot.elapsed = duration + 0.25f;
        }
    }
}

} // namespace internal

// ---- Public helpers (before platform split) -------------------------

void set_quit_on_ctrl_q(bool e) { internal::g_shortcuts_enabled = e; }
void set_fps_limit(int fps) { internal::g_frame_limit_fps = fps; }
int  get_fps_limit() { return internal::g_frame_limit_fps; }
void set_backdrop_effect(BackdropEffect effect) { internal::g_backdrop_effect = effect; internal::g_redraw_requested = true; internal::wake_event_loop(); }
void set_dither_size(int px) {
    internal::g_dither_size = px < 1 ? 1 : (px > 32 ? 32 : px);
    internal::g_redraw_requested = true;
    internal::wake_event_loop();
}
void set_window_transparency(WindowTransparency mode) {
    internal::g_window_transparency = mode;
    internal::sync_native_window_transparency();
    internal::g_redraw_requested = true;
    internal::wake_event_loop();
}
void set_window_opacity(float opacity) {
    internal::g_window_opacity = internal::clampf(opacity, 0.20f, 1.0f);
    internal::sync_native_window_transparency();
    internal::g_redraw_requested = true;
    internal::wake_event_loop();
}
void request_redraw() { internal::g_redraw_requested = true; internal::wake_event_loop(); }
void set_command_handler(CommandHandler handler) { internal::g_command_handler = handler; }
void set_style(const Style& s)  {
    internal::g_style = s;
    internal::g_style_name = style_name(s);
    internal::g_redraw_requested = true;
    internal::wake_event_loop();
    internal::sync_native_window_chrome();
}
const Style& get_style()         { return internal::g_style; }
Color color_from_hex(const char* hex) { return internal::parse_hex_color(hex ? std::string_view(hex) : std::string_view{}); }
Color color_from_hex(std::string_view hex) { return internal::parse_hex_color(hex); }
void push_color(ColorRole role, Color color) {
    if (internal::g_color_stack_count >= internal::kColorOverrideStackCap) {
#ifndef NDEBUG
        assert(false && "ftui::push_color: override stack overflow");
#endif
        return;
    }
    internal::g_color_stack[internal::g_color_stack_count++] = {role, color};
}
void pop_color() {
    if (internal::g_color_stack_count <= 0) {
#ifndef NDEBUG
        assert(false && "ftui::pop_color: override stack underflow");
#endif
        return;
    }
    internal::g_color_stack_count--;
}
void set_next_color(ColorRole role, Color color) {
    int idx = internal::color_role_index(role);
    internal::g_next_color_pending.used[idx] = true;
    internal::g_next_color_pending.colors[idx] = color;
}
void set_window_icon(void* native_icon) { internal::apply_window_icon_handles(native_icon, native_icon, false, BuiltinIcon::Symbol); }
void set_window_icon_builtin(BuiltinIcon variant) { internal::apply_window_icon_handles(nullptr, nullptr, true, variant); }
DebugState&  debug()             { return internal::g_debug; }
void set_next_width(float px) {
    internal::g_next_layout.active = true;
    internal::g_next_layout.has_width = true;
    internal::g_next_layout.width = px;
}
void set_next_fill() {
    internal::g_next_layout.active = true;
    internal::g_next_layout.fill = true;
}
void set_next_percent(float pct) {
    internal::g_next_layout.active = true;
    internal::g_next_layout.has_percent = true;
    internal::g_next_layout.percent = pct < 0.0f ? 0.0f : (pct > 1.0f ? 1.0f : pct);
}
void set_next_limits(float min_px, float max_px) {
    internal::g_next_layout.active = true;
    internal::g_next_layout.has_limits = true;
    internal::g_next_layout.min = min_px < 0.0f ? 0.0f : min_px;
    internal::g_next_layout.max = max_px < min_px ? min_px : max_px;
}
void set_next_align(Align align) {
    internal::g_next_layout.active = true;
    internal::g_next_layout.align = align;
}
void begin_disabled() { internal::g_disabled_depth++; }
void end_disabled() { if (internal::g_disabled_depth > 0) internal::g_disabled_depth--; }
void request_focus(const char* label) { if (label) internal::g_focus_request_id = internal::hash_str(label); }
void open_modal(const char* label) { if (label) internal::g_modal_request_id = internal::hash_str(label); }
void close_modal() { internal::g_modal_open_id = 0; internal::g_modal_request_id = 0; }
float calc_text_width(const char* text) { return internal::measure_text_width(text ? text : ""); }
float calc_text_height(const char* text, float wrap_width) {
    std::vector<internal::TextRange> lines;
    internal::compute_wrapped_ranges(text ? text : "", wrap_width, true, lines);
    return (float)lines.size() * internal::text_line_height();
}
void tooltip(const char* text) {
    if (!text || !text[0]) return;
    if (!internal::g_ctx.last_item_hovered) return;
    internal::g_tooltip.requested = true;
    internal::g_tooltip.request_id = internal::g_ctx.last_item_id;
    internal::g_tooltip.request_anchor = internal::g_ctx.last_item_rect;
    internal::g_tooltip.request_text = text;
}
void toast(const Toast& t) {
    if (!t.message || !t.message[0]) return;
    internal::ToastSlot* slot = nullptr;
    for (internal::ToastSlot& s : internal::g_toasts) {
        if (!s.active) { slot = &s; break; }
    }
    if (!slot) {
        slot = &internal::g_toasts[0];
        for (int i = 1; i < 16; ++i) internal::g_toasts[i - 1] = internal::g_toasts[i];
        slot = &internal::g_toasts[15];
    }
    slot->active = true;
    slot->toast = t;
    slot->message = t.message;
    slot->elapsed = 0.0f;
    internal::g_redraw_requested = true;
    internal::wake_event_loop();
}
void toast(const char* message) { Toast t; t.message = message; toast(t); }
void toast_info(const char* message) { toast(message); }
void toast_success(const char* message) { Toast t; t.message = message; t.type = ToastType::Success; toast(t); }
void toast_warning(const char* message) { Toast t; t.message = message; t.type = ToastType::Warning; toast(t); }
void toast_error(const char* message) { Toast t; t.message = message; t.type = ToastType::Error; toast(t); }
void clear_toasts() {
    for (internal::ToastSlot& s : internal::g_toasts) s = {};
    internal::g_redraw_requested = true;
    internal::wake_event_loop();
}

// ============================================================
// Themes
// ============================================================

Style default_dark_style() {
    Style s;
    s.background = {0.055f,0.055f,0.059f,1}; s.panel = {0.090f,0.094f,0.102f,1};
    s.text = {0.910f,0.910f,0.910f,1}; s.text_dim = {0.663f,0.678f,0.702f,1};
    s.border = {0.169f,0.176f,0.192f,1}; s.button = {0.090f,0.094f,0.102f,1};
    s.button_hover = {0.137f,0.149f,0.169f,1}; s.button_active = {0.176f,0.192f,0.220f,1};
    s.input_bg = {0.071f,0.075f,0.082f,1}; s.input_focus = {0.310f,0.420f,0.780f,1};
    s.accent = s.input_focus; s.warning = {0.894f,0.486f,0.353f,1}; s.success = {0.420f,0.741f,0.522f,1};
    s.window_padding=20; s.item_spacing=10; s.item_height=36; s.rounding=8; s.border_width=1; s.font_size=16;
    return s;
}
Style catppuccin_mocha_style() {
    Style s;
    s.background = {0.118f,0.118f,0.180f,1}; s.panel = {0.192f,0.196f,0.267f,1};
    s.text = {0.804f,0.839f,0.957f,1}; s.text_dim = {0.729f,0.761f,0.871f,1};
    s.border = {0.271f,0.278f,0.353f,1}; s.button = {0.192f,0.196f,0.267f,1};
    s.button_hover = {0.271f,0.278f,0.353f,1}; s.button_active = {0.341f,0.349f,0.431f,1};
    s.input_bg = {0.149f,0.149f,0.220f,1}; s.input_focus = {0.537f,0.706f,0.980f,1};
    s.accent = s.input_focus; s.warning = {0.961f,0.471f,0.561f,1}; s.success = {0.651f,0.890f,0.631f,1};
    s.window_padding=20; s.item_spacing=10; s.item_height=36; s.rounding=8; s.border_width=1; s.font_size=16;
    return s;
}
Style nord_style() {
    Style s;
    s.background = {0.180f,0.204f,0.251f,1}; s.panel = {0.231f,0.259f,0.322f,1};
    s.text = {0.847f,0.871f,0.914f,1}; s.text_dim = {0.596f,0.635f,0.702f,1};
    s.border = {0.263f,0.298f,0.369f,1}; s.button = {0.231f,0.259f,0.322f,1};
    s.button_hover = {0.263f,0.298f,0.369f,1}; s.button_active = {0.298f,0.337f,0.416f,1};
    s.input_bg = {0.200f,0.227f,0.282f,1}; s.input_focus = {0.533f,0.753f,0.816f,1};
    s.accent = s.input_focus; s.warning = {0.816f,0.529f,0.440f,1}; s.success = {0.639f,0.745f,0.549f,1};
    s.window_padding=20; s.item_spacing=10; s.item_height=36; s.rounding=6; s.border_width=1; s.font_size=16;
    return s;
}
Style gruvbox_dark_style() {
    Style s;
    s.background = {0.157f,0.157f,0.157f,1}; s.panel = {0.235f,0.220f,0.212f,1};
    s.text = {0.922f,0.859f,0.698f,1}; s.text_dim = {0.835f,0.769f,0.576f,1};
    s.border = {0.314f,0.294f,0.282f,1}; s.button = {0.235f,0.220f,0.212f,1};
    s.button_hover = {0.314f,0.294f,0.282f,1}; s.button_active = {0.400f,0.373f,0.329f,1};
    s.input_bg = {0.196f,0.188f,0.188f,1}; s.input_focus = {0.271f,0.522f,0.533f,1};
    s.accent = s.input_focus; s.warning = {0.984f,0.490f,0.247f,1}; s.success = {0.722f,0.733f,0.247f,1};
    s.window_padding=20; s.item_spacing=10; s.item_height=36; s.rounding=4; s.border_width=1; s.font_size=16;
    return s;
}
Style one_dark_style() {
    Style s;
    s.background = {0.157f,0.173f,0.204f,1}; s.panel = {0.173f,0.192f,0.227f,1};
    s.text = {0.671f,0.698f,0.749f,1}; s.text_dim = {0.435f,0.467f,0.522f,1};
    s.border = {0.208f,0.231f,0.271f,1}; s.button = {0.173f,0.192f,0.227f,1};
    s.button_hover = {0.208f,0.231f,0.271f,1}; s.button_active = {0.247f,0.278f,0.329f,1};
    s.input_bg = {0.145f,0.161f,0.192f,1}; s.input_focus = {0.380f,0.686f,0.937f,1};
    s.accent = s.input_focus; s.warning = {0.878f,0.423f,0.458f,1}; s.success = {0.596f,0.757f,0.420f,1};
    s.window_padding=20; s.item_spacing=10; s.item_height=36; s.rounding=6; s.border_width=1; s.font_size=16;
    return s;
}
Style ghostty_green_style() {
    Style s;
    s.background = {0.027f,0.035f,0.031f,1}; s.panel = {0.055f,0.071f,0.063f,1};
    s.text = {0.850f,0.925f,0.860f,1}; s.text_dim = {0.533f,0.643f,0.561f,1};
    s.border = {0.129f,0.204f,0.157f,1}; s.button = {0.066f,0.090f,0.078f,1};
    s.button_hover = {0.102f,0.145f,0.122f,1}; s.button_active = {0.145f,0.216f,0.176f,1};
    s.input_bg = {0.035f,0.047f,0.043f,1}; s.input_focus = {0.392f,0.820f,0.514f,1};
    s.accent = {0.451f,0.878f,0.529f,1}; s.warning = {0.918f,0.698f,0.353f,1}; s.success = {0.420f,0.831f,0.514f,1};
    s.window_padding=20; s.item_spacing=10; s.item_height=36; s.rounding=5; s.border_width=1; s.font_size=16;
    return s;
}

static bool style_eq(Color a, Color b) {
    return fabsf(a.r - b.r) < 0.0001f && fabsf(a.g - b.g) < 0.0001f &&
           fabsf(a.b - b.b) < 0.0001f && fabsf(a.a - b.a) < 0.0001f;
}

const char* style_name(const Style& style) {
    struct NamedStyle { const char* name; Style (*fn)(); };
    static NamedStyle styles[] = {
        {"default-dark", default_dark_style},
        {"catppuccin-mocha", catppuccin_mocha_style},
        {"nord", nord_style},
        {"gruvbox-dark", gruvbox_dark_style},
        {"one-dark", one_dark_style},
        {"ghostty-green", ghostty_green_style},
    };
    for (const NamedStyle& ns : styles) {
        Style s = ns.fn();
        if (style_eq(style.background, s.background) && style_eq(style.panel, s.panel) &&
            style_eq(style.text, s.text) && style_eq(style.accent, s.accent) &&
            fabsf(style.rounding - s.rounding) < 0.0001f) {
            return ns.name;
        }
    }
    return "custom";
}

Style style_from_name(const char* name) {
    if (!name || !name[0]) return FTUI_DEFAULT_STYLE();
    if (strcmp(name, "default-dark") == 0 || strcmp(name, "dark") == 0 || strcmp(name, "td") == 0) return default_dark_style();
    if (strcmp(name, "catppuccin-mocha") == 0 || strcmp(name, "catppuccin") == 0 || strcmp(name, "tc") == 0) return catppuccin_mocha_style();
    if (strcmp(name, "nord") == 0 || strcmp(name, "tn") == 0) return nord_style();
    if (strcmp(name, "gruvbox-dark") == 0 || strcmp(name, "gruvbox") == 0 || strcmp(name, "tg") == 0) return gruvbox_dark_style();
    if (strcmp(name, "one-dark") == 0 || strcmp(name, "onedark") == 0 || strcmp(name, "to") == 0) return one_dark_style();
    if (strcmp(name, "ghostty-green") == 0 || strcmp(name, "ghostty") == 0 || strcmp(name, "green") == 0 || strcmp(name, "th") == 0) return ghostty_green_style();
    return FTUI_DEFAULT_STYLE();
}

const char* current_style_name() {
    if (!internal::g_style_name.empty()) return internal::g_style_name.c_str();
    return style_name(internal::g_style);
}


// ============================================================
// Windows Implementation
// ============================================================
#ifdef _WIN32

namespace internal {

static std::wstring utf8_to_wide(const char* u) {
    if (!u || !u[0]) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, u, -1, nullptr, 0);
    if (n <= 0) return L"";
    std::wstring w(n - 1, 0);
    MultiByteToWideChar(CP_UTF8, 0, u, -1, &w[0], n);
    return w;
}

struct PlatformState {
    HWND hwnd = nullptr; HINSTANCE instance = nullptr;
    bool running = false; int width = 0, height = 0;
    HICON icon_big = nullptr;
    HICON icon_small = nullptr;
    bool  icon_owned = false;
    BuiltinIcon icon_variant = BuiltinIcon::Symbol;
};
struct RendererState {
    ID2D1Factory*           d2d_factory      = nullptr;
    ID2D1HwndRenderTarget*  target           = nullptr;
    ID2D1SolidColorBrush*   brush            = nullptr;
    IDWriteFactory*         dwrite_factory   = nullptr;
    IDWriteTextFormat*      text_format      = nullptr;
    IDWriteRenderingParams* rendering_params = nullptr;
    IWICImagingFactory*     wic_factory      = nullptr;
    float dpi_scale = 1.0f; bool com_inited = false;
};

static PlatformState  g_platform;
static RendererState  g_renderer;
static LARGE_INTEGER  g_freq, g_last_time;
static ID2D1Bitmap*   g_prev_frame_bitmap = nullptr;
static std::vector<BYTE> g_prev_frame_pixels;
static UINT           g_prev_frame_w = 0, g_prev_frame_h = 0;
static ID2D1Bitmap*      g_dither_bitmap = nullptr;
static ID2D1BitmapBrush* g_dither_brush = nullptr;
static int               g_dither_cache_cell = 0;
static Color             g_dither_cache_accent = {};
static Color             g_dither_cache_dim = {};

static void dbg(const char* fmt, ...) {
    char buf[512]; va_list a; va_start(a, fmt); vsnprintf(buf, sizeof(buf), fmt, a); va_end(a);
    OutputDebugStringA(buf);
}

static D2D1_COLOR_F tod(Color c) { return {c.r, c.g, c.b, c.a}; }
static Color apply_draw_color(Color c) {
    c.a *= g_draw_fx_opacity;
    return c;
}
static Rect apply_draw_rect(Rect r) {
    r.x += g_draw_fx_off_x;
    r.y += g_draw_fx_off_y;
    return r;
}
static void set_brush(Color c)   { g_renderer.brush->SetColor(tod(apply_draw_color(c))); }
static void clear_bg(Color c)    { g_renderer.target->Clear(tod(c)); }

static void fill_round_rect(Rect r, float rad, Color c) {
    r = apply_draw_rect(r);
    set_brush(c);
    D2D1_RECT_F rc = {r.x, r.y, r.x+r.w, r.y+r.h};
    g_renderer.target->FillRoundedRectangle(D2D1::RoundedRect(rc, rad, rad), g_renderer.brush);
}
static void stroke_round_rect(Rect r, float rad, float thick, Color c) {
    r = apply_draw_rect(r);
    set_brush(c);
    D2D1_RECT_F rc = {r.x, r.y, r.x+r.w, r.y+r.h};
    g_renderer.target->DrawRoundedRectangle(D2D1::RoundedRect(rc, rad, rad), g_renderer.brush, thick);
}
static void fill_rect(Rect r, Color c) {
    r = apply_draw_rect(r);
    set_brush(c);
    D2D1_RECT_F rc = {r.x, r.y, r.x+r.w, r.y+r.h};
    g_renderer.target->FillRectangle(rc, g_renderer.brush);
}
static void release_dither_cache() {
    if (g_dither_brush) { g_dither_brush->Release(); g_dither_brush = nullptr; }
    if (g_dither_bitmap) { g_dither_bitmap->Release(); g_dither_bitmap = nullptr; }
    g_dither_cache_cell = 0;
    g_dither_cache_accent = {};
    g_dither_cache_dim = {};
}
static bool ensure_dither_brush() {
    if (!g_renderer.target) return false;
    int cell = g_dither_size < 1 ? 1 : (g_dither_size > 32 ? 32 : g_dither_size);
    Color accent = resolve_color(ColorRole::Accent);
    Color dim = resolve_color(ColorRole::TextDim);
    if (g_dither_brush && g_dither_bitmap && g_dither_cache_cell == cell &&
        style_eq(g_dither_cache_accent, accent) && style_eq(g_dither_cache_dim, dim)) {
        return true;
    }
    release_dither_cache();
    std::vector<unsigned char> px;
    int tw = 0, th = 0;
    build_soft_dither_tile(cell, px, tw, th);
    if (px.empty() || tw <= 0 || th <= 0) return false;

    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        96.0f, 96.0f);
    HRESULT hr = g_renderer.target->CreateBitmap(D2D1::SizeU((UINT32)tw, (UINT32)th),
                                                 px.data(), (UINT32)tw * 4u,
                                                 props, &g_dither_bitmap);
    if (FAILED(hr) || !g_dither_bitmap) { release_dither_cache(); return false; }
    D2D1_BITMAP_BRUSH_PROPERTIES bp = D2D1::BitmapBrushProperties(
        D2D1_EXTEND_MODE_WRAP, D2D1_EXTEND_MODE_WRAP, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
    hr = g_renderer.target->CreateBitmapBrush(g_dither_bitmap, bp, &g_dither_brush);
    if (FAILED(hr) || !g_dither_brush) { release_dither_cache(); return false; }
    g_dither_cache_cell = cell;
    g_dither_cache_accent = accent;
    g_dither_cache_dim = dim;
    return true;
}
static void draw_dither_pattern(Rect r, float opacity) {
    if (opacity <= 0.001f || !ensure_dither_brush()) return;
    r = apply_draw_rect(r);
    g_dither_brush->SetOpacity(clamp01(opacity));
    D2D1_RECT_F rc = {r.x, r.y, r.x + r.w, r.y + r.h};
    g_renderer.target->FillRectangle(rc, g_dither_brush);
}
static void fill_triangle(Rect r, int dir, Color c) {
    r = apply_draw_rect(r);
    ID2D1PathGeometry* tri = nullptr;
    if (FAILED(g_renderer.d2d_factory->CreatePathGeometry(&tri)) || !tri) return;
    ID2D1GeometrySink* sink = nullptr;
    if (FAILED(tri->Open(&sink)) || !sink) { tri->Release(); return; }

    D2D1_POINT_2F pts[3];
    if (dir == 0) {
        pts[0] = {r.x + r.w * 0.5f, r.y + r.h};
        pts[1] = {r.x, r.y};
        pts[2] = {r.x + r.w, r.y};
    } else if (dir == 1) {
        pts[0] = {r.x + r.w * 0.5f, r.y};
        pts[1] = {r.x, r.y + r.h};
        pts[2] = {r.x + r.w, r.y + r.h};
    } else {
        pts[0] = {r.x + r.w, r.y + r.h * 0.5f};
        pts[1] = {r.x, r.y};
        pts[2] = {r.x, r.y + r.h};
    }
    sink->BeginFigure(pts[0], D2D1_FIGURE_BEGIN_FILLED);
    sink->AddLine(pts[1]);
    sink->AddLine(pts[2]);
    sink->EndFigure(D2D1_FIGURE_END_CLOSED);
    sink->Close();
    sink->Release();

    set_brush(c);
    g_renderer.target->FillGeometry(tri, g_renderer.brush);
    tri->Release();
}
static void draw_line(float x0, float y0, float x1, float y1, float thick, Color c) {
    set_brush(c);
    g_renderer.target->DrawLine({x0 + g_draw_fx_off_x, y0 + g_draw_fx_off_y},
                                {x1 + g_draw_fx_off_x, y1 + g_draw_fx_off_y},
                                g_renderer.brush, thick);
}
static void push_clip(Rect r) {
    r = apply_draw_rect(r);
    D2D1_RECT_F rc = {r.x, r.y, r.x+r.w, r.y+r.h};
    g_renderer.target->PushAxisAlignedClip(rc, D2D1_ANTIALIAS_MODE_ALIASED);
}
static void pop_clip() { g_renderer.target->PopAxisAlignedClip(); }

static void draw_text_utf8(const char* utf8, Rect r, Color c) {
    std::wstring w = utf8_to_wide(utf8);
    if (w.empty()) return;
    r = apply_draw_rect(r);
    set_brush(c);
    D2D1_RECT_F rc = {r.x, r.y, r.x+r.w, r.y+r.h};
    DWRITE_TRIMMING trim = {DWRITE_TRIMMING_GRANULARITY_NONE,0,0};
    g_renderer.text_format->SetTrimming(&trim, nullptr);
    g_renderer.target->DrawText(w.c_str(),(UINT32)w.size(),g_renderer.text_format,rc,g_renderer.brush,D2D1_DRAW_TEXT_OPTIONS_CLIP);
}
static void draw_text_utf8_centered(const char* utf8, Rect r, Color c) {
    g_renderer.text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    draw_text_utf8(utf8, r, c);
    g_renderer.text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
}

static float measure_text_width(const char* utf8) {
    if (!utf8 || !utf8[0]) return 0.0f;
    std::wstring w = utf8_to_wide(utf8);
    IDWriteTextLayout* lay = nullptr;
    if (FAILED(g_renderer.dwrite_factory->CreateTextLayout(w.c_str(),(UINT32)w.size(),g_renderer.text_format,10000,1000,&lay))||!lay) return 0;
    DWRITE_TEXT_METRICS m; lay->GetMetrics(&m); lay->Release(); return m.width;
}
static int byte_from_x(const char* utf8, float rel_x) {
    if (!utf8||!utf8[0]||rel_x<=0) return 0;
    std::wstring w = utf8_to_wide(utf8);
    IDWriteTextLayout* lay = nullptr;
    if (FAILED(g_renderer.dwrite_factory->CreateTextLayout(w.c_str(),(UINT32)w.size(),g_renderer.text_format,10000,1000,&lay))||!lay) return (int)strlen(utf8);
    BOOL trail=0,inside=0; DWRITE_HIT_TEST_METRICS m{};
    lay->HitTestPoint(rel_x,0,&trail,&inside,&m);
    UINT32 wpos = m.textPosition + (trail?1u:0u);
    if (wpos > (UINT32)w.size()) wpos=(UINT32)w.size();
    lay->Release();
    const char* p = utf8; UINT32 wc=0;
    while (*p && wc < wpos) {
        unsigned char ch=(unsigned char)*p;
        int sl = ch<0x80?1:ch<0xE0?2:ch<0xF0?3:4;
        wc += (sl==4)?2:1; p+=sl;
    }
    return (int)(p-utf8);
}

static void draw_image_handle(ImageHandle* img, Rect r) {
    auto* bmp = img ? static_cast<ID2D1Bitmap*>(img->_impl) : nullptr;
    if (!bmp) return;
    r = apply_draw_rect(r);
    D2D1_RECT_F dst = {r.x, r.y, r.x+r.w, r.y+r.h};
    g_renderer.target->DrawBitmap(bmp, dst, g_draw_fx_opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
}

static void clipboard_set(const char* utf8) {
    if (!g_platform.hwnd||!utf8) return;
    std::wstring w = utf8_to_wide(utf8);
    size_t n = (w.size()+1)*sizeof(wchar_t);
    HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE,n);
    if (!hg) return;
    memcpy(GlobalLock(hg),w.c_str(),n); GlobalUnlock(hg);
    if (OpenClipboard(g_platform.hwnd)) { EmptyClipboard(); SetClipboardData(CF_UNICODETEXT,hg); CloseClipboard(); }
    else GlobalFree(hg);
}
static std::string clipboard_get() {
    if (!g_platform.hwnd||!OpenClipboard(g_platform.hwnd)) return "";
    std::string result;
    HGLOBAL hg = GetClipboardData(CF_UNICODETEXT);
    if (hg) {
        wchar_t* w=(wchar_t*)GlobalLock(hg);
        if (w) { int n=WideCharToMultiByte(CP_UTF8,0,w,-1,nullptr,0,nullptr,nullptr);
                 if (n>1){result.resize(n-1);WideCharToMultiByte(CP_UTF8,0,w,-1,&result[0],n,nullptr,nullptr);}
                 GlobalUnlock(hg); }
    }
    CloseClipboard(); return result;
}

static void release_frame_snapshot() {
    if (g_prev_frame_bitmap) { g_prev_frame_bitmap->Release(); g_prev_frame_bitmap = nullptr; }
    g_prev_frame_pixels.clear();
    g_prev_frame_w = g_prev_frame_h = 0;
}

static void draw_previous_frame_overlay(Rect clip_r, float dx, float opacity) {
    if (!g_prev_frame_bitmap || opacity <= 0.001f) return;
    D2D1_RECT_F clip = {clip_r.x, clip_r.y, clip_r.x + clip_r.w, clip_r.y + clip_r.h};
    D2D1_RECT_F dst = {dx, 0.0f, dx + (float)g_prev_frame_w, (float)g_prev_frame_h};
    g_renderer.target->PushAxisAlignedClip(clip, D2D1_ANTIALIAS_MODE_ALIASED);
    g_renderer.target->DrawBitmap(g_prev_frame_bitmap, dst, opacity, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
    g_renderer.target->PopAxisAlignedClip();
}

static void draw_previous_frame_blur_panel(Rect r, float opacity) {
    if (!g_prev_frame_bitmap || opacity <= 0.001f) return;
    r = apply_draw_rect(r);
    D2D1_RECT_F dst = {r.x, r.y, r.x + r.w, r.y + r.h};
    struct BlurSample { float ox, oy, weight; };
    static const BlurSample samples[] = {
        { 0.0f,  0.0f, 0.24f},
        {-2.0f,  0.0f, 0.10f}, { 2.0f,  0.0f, 0.10f},
        { 0.0f, -2.0f, 0.10f}, { 0.0f,  2.0f, 0.10f},
        {-1.5f, -1.5f, 0.08f}, { 1.5f, -1.5f, 0.08f},
        {-1.5f,  1.5f, 0.08f}, { 1.5f,  1.5f, 0.08f},
        {-3.0f,  0.0f, 0.04f}, { 3.0f,  0.0f, 0.04f},
        { 0.0f, -3.0f, 0.04f}, { 0.0f,  3.0f, 0.04f}
    };
    g_renderer.target->PushAxisAlignedClip(dst, D2D1_ANTIALIAS_MODE_ALIASED);
    for (const BlurSample& s : samples) {
        D2D1_RECT_F src = {
            dst.left + s.ox,
            dst.top + s.oy,
            dst.right + s.ox,
            dst.bottom + s.oy
        };
        g_renderer.target->DrawBitmap(g_prev_frame_bitmap, dst, opacity * s.weight,
                                      D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src);
    }
    g_renderer.target->PopAxisAlignedClip();
}

static void capture_frame_snapshot() {
    if (!effects_enabled() || !g_platform.hwnd || !g_renderer.target ||
        g_tab_content_owner != 0 || g_dropdown_open_id != 0) return;

    RECT rc{};
    GetClientRect(g_platform.hwnd, &rc);
    int w = rc.right - rc.left;
    int h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) { release_frame_snapshot(); return; }

    UINT stride = (UINT)w * 4u;
    size_t bytes = (size_t)stride * (size_t)h;
    if (g_prev_frame_pixels.size() != bytes) g_prev_frame_pixels.resize(bytes);

    HDC src_dc = GetDC(g_platform.hwnd);
    if (!src_dc) return;

    BITMAPINFO bi{};
    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth = w;
    bi.bmiHeader.biHeight = -h;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP dib = CreateDIBSection(src_dc, &bi, DIB_RGB_COLORS, &bits, nullptr, 0);
    HDC mem_dc = CreateCompatibleDC(src_dc);
    if (!dib || !mem_dc || !bits) {
        if (mem_dc) DeleteDC(mem_dc);
        if (dib) DeleteObject(dib);
        ReleaseDC(g_platform.hwnd, src_dc);
        return;
    }

    HGDIOBJ old = SelectObject(mem_dc, dib);
    BitBlt(mem_dc, 0, 0, w, h, src_dc, 0, 0, SRCCOPY);
    memcpy(g_prev_frame_pixels.data(), bits, bytes);

    SelectObject(mem_dc, old);
    DeleteDC(mem_dc);
    DeleteObject(dib);
    ReleaseDC(g_platform.hwnd, src_dc);

    UINT dpi = GetDpiForWindow(g_platform.hwnd); if (!dpi) dpi = 96;
    D2D1_BITMAP_PROPERTIES props = D2D1::BitmapProperties(
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE),
        (float)dpi, (float)dpi);

    if (!g_prev_frame_bitmap || g_prev_frame_w != (UINT)w || g_prev_frame_h != (UINT)h) {
        if (g_prev_frame_bitmap) { g_prev_frame_bitmap->Release(); g_prev_frame_bitmap = nullptr; }
        if (FAILED(g_renderer.target->CreateBitmap(D2D1::SizeU((UINT32)w, (UINT32)h),
                                                   g_prev_frame_pixels.data(), stride,
                                                   props, &g_prev_frame_bitmap))) {
            g_prev_frame_w = g_prev_frame_h = 0;
            return;
        }
        g_prev_frame_w = (UINT)w;
        g_prev_frame_h = (UINT)h;
    } else {
        g_prev_frame_bitmap->CopyFromMemory(nullptr, g_prev_frame_pixels.data(), stride);
    }
}

static bool init_d2d() {
    D2D1_FACTORY_OPTIONS opts = {};
    if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,__uuidof(ID2D1Factory),&opts,(void**)&g_renderer.d2d_factory))) return false;
    if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED,__uuidof(IDWriteFactory),(IUnknown**)&g_renderer.dwrite_factory))) return false;
    return true;
}
static bool create_render_target() {
    RECT rc; GetClientRect(g_platform.hwnd, &rc);
    D2D1_SIZE_U psz = {(UINT32)(rc.right-rc.left),(UINT32)(rc.bottom-rc.top)};
    UINT dpi = GetDpiForWindow(g_platform.hwnd); if (!dpi) dpi=96;
    g_renderer.dpi_scale = (float)dpi/96.0f;
    g_platform.width  = (int)(psz.width  / g_renderer.dpi_scale + 0.5f);
    g_platform.height = (int)(psz.height / g_renderer.dpi_scale + 0.5f);
    D2D1_RENDER_TARGET_PROPERTIES rtp = D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT,D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN,D2D1_ALPHA_MODE_UNKNOWN),(float)dpi,(float)dpi);
    D2D1_HWND_RENDER_TARGET_PROPERTIES hwp = D2D1::HwndRenderTargetProperties(g_platform.hwnd,psz);
    if (FAILED(g_renderer.d2d_factory->CreateHwndRenderTarget(rtp,hwp,&g_renderer.target))) return false;
    if (FAILED(g_renderer.target->CreateSolidColorBrush(D2D1::ColorF(1,1,1,1),&g_renderer.brush))) return false;
    g_renderer.target->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_CLEARTYPE);
    IDWriteRenderingParams* dp=nullptr; g_renderer.dwrite_factory->CreateRenderingParams(&dp);
    if (dp) { g_renderer.dwrite_factory->CreateCustomRenderingParams(dp->GetGamma(),dp->GetEnhancedContrast(),dp->GetClearTypeLevel(),dp->GetPixelGeometry(),DWRITE_RENDERING_MODE_CLEARTYPE_NATURAL_SYMMETRIC,&g_renderer.rendering_params); dp->Release(); }
    if (g_renderer.rendering_params) g_renderer.target->SetTextRenderingParams(g_renderer.rendering_params);
    return true;
}
static bool create_text_format() {
    if (g_renderer.text_format) { g_renderer.text_format->Release(); g_renderer.text_format=nullptr; }
    if (FAILED(g_renderer.dwrite_factory->CreateTextFormat(L"Segoe UI",nullptr,DWRITE_FONT_WEIGHT_NORMAL,DWRITE_FONT_STYLE_NORMAL,DWRITE_FONT_STRETCH_NORMAL,g_style.font_size,L"en-us",&g_renderer.text_format))) return false;
    g_renderer.text_format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    g_renderer.text_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    g_renderer.text_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    return true;
}

static COLORREF to_colorref(Color c) {
    int r = (int)(clampf(c.r, 0.0f, 1.0f) * 255.0f + 0.5f);
    int g = (int)(clampf(c.g, 0.0f, 1.0f) * 255.0f + 0.5f);
    int b = (int)(clampf(c.b, 0.0f, 1.0f) * 255.0f + 0.5f);
    return RGB(r, g, b);
}

static void destroy_owned_icons() {
    if (!g_platform.icon_owned) return;
    if (g_platform.icon_big) { DestroyIcon(g_platform.icon_big); g_platform.icon_big = nullptr; }
    if (g_platform.icon_small) { DestroyIcon(g_platform.icon_small); g_platform.icon_small = nullptr; }
    g_platform.icon_owned = false;
}

static HICON make_ftui_icon(BuiltinIcon variant, int size) {
    if (!g_renderer.d2d_factory||!g_renderer.dwrite_factory) return nullptr;
    HDC screen_dc=GetDC(nullptr); HDC hdc=CreateCompatibleDC(screen_dc); ReleaseDC(nullptr,screen_dc);
    BITMAPV5HEADER bmi={}; bmi.bV5Size=sizeof(bmi); bmi.bV5Width=size; bmi.bV5Height=-size;
    bmi.bV5Planes=1; bmi.bV5BitCount=32; bmi.bV5Compression=BI_BITFIELDS;
    bmi.bV5RedMask=0x00FF0000; bmi.bV5GreenMask=0x0000FF00; bmi.bV5BlueMask=0x000000FF; bmi.bV5AlphaMask=0xFF000000;
    void* bits=nullptr; HBITMAP hbm=CreateDIBSection(hdc,(BITMAPINFO*)&bmi,DIB_RGB_COLORS,&bits,nullptr,0);
    if (!hbm){DeleteDC(hdc);return nullptr;}
    HGDIOBJ old=SelectObject(hdc,hbm);
    D2D1_RENDER_TARGET_PROPERTIES rtp=D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_SOFTWARE,D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED));
    ID2D1DCRenderTarget* dc_rt=nullptr; g_renderer.d2d_factory->CreateDCRenderTarget(&rtp,&dc_rt);
    if (!dc_rt){SelectObject(hdc,old);DeleteObject(hbm);DeleteDC(hdc);return nullptr;}
    RECT br={0,0,size,size}; dc_rt->BindDC(hdc,&br);
    ID2D1SolidColorBrush* ibr=nullptr; dc_rt->CreateSolidColorBrush(D2D1::ColorF(1,1,1,1),&ibr);
    float s=(float)size, margin=s*0.1114f, sw=fmaxf(1.f,s/56.f);
    D2D1_COLOR_F orange={1.f,130.f/255.f,0.f,1.f};
    D2D1_COLOR_F black={0.f,0.f,0.f,1.f};
    dc_rt->BeginDraw(); dc_rt->Clear(D2D1::ColorF(0,0,0,0));
    { ID2D1PathGeometry* tri=nullptr; g_renderer.d2d_factory->CreatePathGeometry(&tri);
      ID2D1GeometrySink* sink=nullptr; tri->Open(&sink);
      sink->BeginFigure({s*.5f,margin},D2D1_FIGURE_BEGIN_FILLED);
      D2D1_POINT_2F pts[2]={{s-margin,s-margin},{margin,s-margin}}; sink->AddLines(pts,2);
      sink->EndFigure(D2D1_FIGURE_END_CLOSED); sink->Close(); sink->Release();
      ibr->SetColor(D2D1::ColorF(1,1,1,1)); dc_rt->FillGeometry(tri,ibr); tri->Release(); }
    ibr->SetColor(orange); dc_rt->DrawLine({margin,margin},{s-margin,s-margin},ibr,sw*1.5f);
    { D2D1_RECT_F brd={margin,margin,s-margin,s-margin};
      ibr->SetColor(black); dc_rt->DrawRectangle(brd,ibr,sw*1.35f);
      ibr->SetColor(orange); dc_rt->DrawRectangle(brd,ibr,sw); }
    if (variant == BuiltinIcon::SymbolWithText) {
        IDWriteTextFormat* tf = nullptr;
        IDWriteTextLayout* tl = nullptr;
        float font_sz = size * 0.26f;
        if (SUCCEEDED(g_renderer.dwrite_factory->CreateTextFormat(
                L"Segoe UI Semibold", nullptr,
                DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
                font_sz, L"en-us", &tf)) && tf) {
            tf->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
            tf->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
            if (SUCCEEDED(g_renderer.dwrite_factory->CreateTextLayout(
                    L"FTUI", 4, tf, s - margin * 2.0f, size * 0.22f, &tl)) && tl) {
                D2D1_POINT_2F origin = {margin, s * 0.70f};
                ibr->SetColor(orange);
                dc_rt->DrawTextLayout(origin, tl, ibr, D2D1_DRAW_TEXT_OPTIONS_NONE);
                tl->Release();
            }
            tf->Release();
        }
    }
    dc_rt->EndDraw(); ibr->Release(); dc_rt->Release();
    int ms=((size+31)/32)*4; std::vector<BYTE> mb(ms*size,0);
    HBITMAP hbm_mask=CreateBitmap(size,size,1,1,mb.data());
    ICONINFO ii={TRUE,0,0,hbm_mask,hbm}; HICON icon=CreateIconIndirect(&ii);
    SelectObject(hdc,old); DeleteObject(hbm_mask); DeleteObject(hbm); DeleteDC(hdc);
    return icon;
}

static void apply_window_icon_handles(void* icon_big, void* icon_small, bool owned, BuiltinIcon variant) {
    if (owned && !g_renderer.d2d_factory) {
        g_platform.icon_owned = true;
        g_platform.icon_variant = variant;
        g_platform.icon_big = nullptr;
        g_platform.icon_small = nullptr;
        return;
    }
    if (owned) {
        destroy_owned_icons();
        g_platform.icon_big = (HICON)(icon_big ? icon_big : make_ftui_icon(variant, 256));
        g_platform.icon_small = (HICON)(icon_small ? icon_small : make_ftui_icon(variant, 32));
        g_platform.icon_owned = true;
        g_platform.icon_variant = variant;
    } else {
        destroy_owned_icons();
        g_platform.icon_big = (HICON)icon_big;
        g_platform.icon_small = (HICON)(icon_small ? icon_small : icon_big);
        g_platform.icon_owned = false;
    }
    if (g_platform.hwnd) {
        SendMessageW(g_platform.hwnd, WM_SETICON, ICON_BIG, (LPARAM)g_platform.icon_big);
        SendMessageW(g_platform.hwnd, WM_SETICON, ICON_SMALL, (LPARAM)g_platform.icon_small);
    }
}

static void sync_native_window_chrome() {
    if (!g_platform.hwnd) return;
    HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
    if (!dwm) return;
    using DwmSetWindowAttributeFn = HRESULT (WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
    auto fn = (DwmSetWindowAttributeFn)GetProcAddress(dwm, "DwmSetWindowAttribute");
    if (!fn) { FreeLibrary(dwm); return; }

    BOOL dark = TRUE;
    COLORREF caption = to_colorref(lerp_color(g_style.panel, g_style.background, 0.15f));
    COLORREF text = RGB(255, 255, 255);
    COLORREF border = to_colorref(g_style.border);
    fn(g_platform.hwnd, 20, &dark, sizeof(dark));
    fn(g_platform.hwnd, 34, &border, sizeof(border));
    fn(g_platform.hwnd, 35, &caption, sizeof(caption));
    fn(g_platform.hwnd, 36, &text, sizeof(text));
    FreeLibrary(dwm);
}

static void sync_native_window_transparency() {
    if (!g_platform.hwnd) return;

    HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
    if (dwm) {
        struct DwmBlurBehind {
            DWORD dwFlags;
            BOOL fEnable;
            HRGN hRgnBlur;
            BOOL fTransitionOnMaximized;
        };
        using DwmEnableBlurBehindWindowFn = HRESULT (WINAPI*)(HWND, const DwmBlurBehind*);
        auto blur_fn = (DwmEnableBlurBehindWindowFn)GetProcAddress(dwm, "DwmEnableBlurBehindWindow");
        if (blur_fn) {
            DwmBlurBehind bb{};
            bb.dwFlags = 0x00000001;
            bb.fEnable = (g_window_transparency == WindowTransparency::Blur) ? TRUE : FALSE;
            blur_fn(g_platform.hwnd, &bb);
        }
        FreeLibrary(dwm);
    }

    LONG_PTR ex = GetWindowLongPtrW(g_platform.hwnd, GWL_EXSTYLE);
    bool alpha_mode = g_window_transparency == WindowTransparency::Plain ||
                      g_window_transparency == WindowTransparency::BayerDither ||
                      g_window_transparency == WindowTransparency::Blur;
    if (alpha_mode) {
        SetWindowLongPtrW(g_platform.hwnd, GWL_EXSTYLE, ex | WS_EX_LAYERED);
        BYTE alpha = (BYTE)(clampf(g_window_opacity, 0.20f, 1.0f) * 255.0f + 0.5f);
        SetLayeredWindowAttributes(g_platform.hwnd, 0, alpha, LWA_ALPHA);
    } else if (ex & WS_EX_LAYERED) {
        SetLayeredWindowAttributes(g_platform.hwnd, 0, 255, LWA_ALPHA);
        SetWindowLongPtrW(g_platform.hwnd, GWL_EXSTYLE, ex & ~WS_EX_LAYERED);
    }
}

static void release_render_target() {
    release_dither_cache();
    if (g_renderer.rendering_params){g_renderer.rendering_params->Release();g_renderer.rendering_params=nullptr;}
    if (g_renderer.brush){g_renderer.brush->Release();g_renderer.brush=nullptr;}
    if (g_renderer.target){g_renderer.target->Release();g_renderer.target=nullptr;}
    release_frame_snapshot();
}

static void wait_frame_limit() {
    if (g_frame_limit_fps <= 0 || g_freq.QuadPart <= 0) return;
    const double target = 1.0 / (double)g_frame_limit_fps;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    double elapsed = (double)(now.QuadPart - g_last_time.QuadPart) / (double)g_freq.QuadPart;
    double remaining = target - elapsed;
    if (remaining <= 0.0) return;

    DWORD ms = (DWORD)(remaining * 1000.0);
    if (ms > 1) Sleep(ms - 1);
    do {
        Sleep(0);
        QueryPerformanceCounter(&now);
        elapsed = (double)(now.QuadPart - g_last_time.QuadPart) / (double)g_freq.QuadPart;
    } while (elapsed < target);
}

static void wait_for_win32_event() {
    if (!g_platform.running) return;
    WaitMessage();
}

static void wake_event_loop() {
    if (g_platform.hwnd) PostMessageW(g_platform.hwnd, WM_NULL, 0, 0);
}

static void execute_command() {
    if (strcmp(g_cmd.buf,"q")==0 || strcmp(g_cmd.buf,"quit")==0) PostQuitMessage(0);
    else apply_cmd_action();
    cmd_clear();
}

static LRESULT CALLBACK wndproc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (hwnd!=g_platform.hwnd&&g_platform.hwnd) return DefWindowProcW(hwnd,msg,wp,lp);
    switch(msg) {
    case WM_DESTROY: g_platform.running=false; PostQuitMessage(0); return 0;
    case WM_SIZE: {
        int pw=LOWORD(lp),ph=HIWORD(lp);
        g_platform.width=(int)(pw/g_renderer.dpi_scale+.5f); g_platform.height=(int)(ph/g_renderer.dpi_scale+.5f);
        if (g_renderer.target&&pw>0&&ph>0) g_renderer.target->Resize({(UINT32)pw,(UINT32)ph});
        return 0; }
    case WM_DPICHANGED: {
        UINT ndpi=HIWORD(wp); g_renderer.dpi_scale=(float)ndpi/96.f;
        if (g_renderer.target) g_renderer.target->SetDpi((float)ndpi,(float)ndpi);
        const RECT* r=(const RECT*)lp;
        SetWindowPos(hwnd,nullptr,r->left,r->top,r->right-r->left,r->bottom-r->top,SWP_NOZORDER|SWP_NOACTIVATE);
        return 0; }
    case WM_MOUSEMOVE:
        g_input.mouse_x=GET_X_LPARAM(lp)/g_renderer.dpi_scale;
        g_input.mouse_y=GET_Y_LPARAM(lp)/g_renderer.dpi_scale; return 0;
    case WM_LBUTTONDOWN:
        SetCapture(hwnd);
        g_input.mouse_down=g_input.mouse_pressed=true;
        g_input.mouse_x=GET_X_LPARAM(lp)/g_renderer.dpi_scale;
        g_input.mouse_y=GET_Y_LPARAM(lp)/g_renderer.dpi_scale;
        g_input.shift_held=(GetKeyState(VK_SHIFT)&0x8000)!=0;
        if (g_cmd.active) cmd_clear(); return 0;
    case WM_LBUTTONUP:
        ReleaseCapture();
        g_input.mouse_down=false; g_input.mouse_released=true;
        g_input.mouse_x=GET_X_LPARAM(lp)/g_renderer.dpi_scale;
        g_input.mouse_y=GET_Y_LPARAM(lp)/g_renderer.dpi_scale; return 0;
    case WM_CHAR: {
        wchar_t wc=(wchar_t)wp;
        if (g_cmd.active) {
            if (wc==L'\b'){if(g_cmd.len>0)g_cmd.buf[--g_cmd.len]='\0';}
            else if(wc==L'\r'||wc==L'\n') execute_command();
            else if(wc==27) cmd_clear();
            else if(wc>=32&&g_cmd.len<(int)sizeof(g_cmd.buf)-1){g_cmd.buf[g_cmd.len++]=(char)wc;g_cmd.buf[g_cmd.len]='\0';}
            return 0;
        }
        if (wc==L'\b') g_input.key_backspace=true;
        else if(wc==L'\r'||wc==L'\n') g_input.key_enter=true;
        else if(wc==L':'&&g_ctx.focused_input_id==0&&g_shortcuts_enabled){g_cmd.active=true;g_cmd.len=0;g_cmd.buf[0]='\0';}
        else if(wc>=32){
            char buf[8]={}; int n=WideCharToMultiByte(CP_UTF8,0,&wc,1,buf,sizeof(buf)-1,nullptr,nullptr);
            if(n>0&&g_input.text_input_count+n<(int)sizeof(g_input.text_input)){memcpy(g_input.text_input+g_input.text_input_count,buf,n);g_input.text_input_count+=n;}
        }
        return 0; }
    case WM_KEYDOWN:
        g_input.shift_held=(GetKeyState(VK_SHIFT)&0x8000)!=0;
        g_input.ctrl_held=(GetKeyState(VK_CONTROL)&0x8000)!=0;
        if (g_cmd.active){
            if(wp==VK_ESCAPE)cmd_clear();
            else if(wp==VK_TAB)gui_command_accept_completion();
            return 0;
        }
        if(wp==VK_BACK)  g_input.key_backspace=true;
        if(wp==VK_RETURN)g_input.key_enter=true;
        if(wp==VK_SPACE) g_input.key_space=true;
        if(wp==VK_ESCAPE)g_input.key_escape=true;
        if(wp==VK_TAB){if(g_input.shift_held)g_input.key_shift_tab=true;else g_input.key_tab=true;}
        if(wp==VK_LEFT) g_input.key_left=true;
        if(wp==VK_RIGHT)g_input.key_right=true;
        if(wp==VK_UP)   g_input.key_up=true;
        if(wp==VK_DOWN) g_input.key_down=true;
        if(g_input.ctrl_held){
            if(wp=='A'&&g_ctx.focused_input_id!=0){
                g_text_cursor_id=g_ctx.focused_input_id; g_text_sel_anchor=0; g_text_cursor=0x7FFFFFFF;
                g_ta_cursor_id  =g_ctx.focused_input_id; g_ta_sel_anchor  =0; g_ta_cursor  =0x7FFFFFFF;
            }
            if(wp=='C')g_input.key_ctrl_c=true;
            if(wp=='V')g_input.key_ctrl_v=true;
            if(wp=='Q'&&g_shortcuts_enabled)PostQuitMessage(0);
        }
        return 0;
    case WM_KEYUP:
        g_input.shift_held=(GetKeyState(VK_SHIFT)&0x8000)!=0;
        g_input.ctrl_held=(GetKeyState(VK_CONTROL)&0x8000)!=0; return 0;
    case WM_MOUSEWHEEL: {
        short delta=(short)HIWORD(wp);
        if (effects_enabled()) {
            g_input.wheel_y += (float)delta / WHEEL_DELTA * 80.0f;
        } else {
            g_scroll_y -= (float)delta/WHEEL_DELTA*80.f;
            g_scroll_target_y = g_scroll_y;
            if(g_scroll_y<0)g_scroll_y=0;
        }
        return 0; }
    case WM_SETFOCUS:   g_input.focused=true;  return 0;
    case WM_KILLFOCUS:  g_input.focused=false;  g_ctx.focused_input_id=0; g_ctx.focused_widget_id=0; return 0;
    case WM_ERASEBKGND: return 1;
    default: return DefWindowProcW(hwnd,msg,wp,lp);
    }
}

} // namespace internal

bool create_window(const Config& cfg) {
    using namespace internal;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    g_renderer.com_inited = (hr==S_OK||hr==S_FALSE);
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    g_style = startup_style();
    g_style_name = style_name(g_style);
    g_frame_limit_fps = cfg.fps_limit;
    g_redraw_requested = true;
    g_backdrop_effect = cfg.backdrop_effect;
    g_dither_size = cfg.dither_size < 1 ? 1 : (cfg.dither_size > 32 ? 32 : cfg.dither_size);
    g_window_transparency = cfg.window_transparency;
    g_window_opacity = clampf(cfg.window_opacity, 0.20f, 1.0f);
    g_scroll_y = g_scroll_target_y = 0;
    g_ta_scroll_y = g_ta_scroll_target_y = 0;
    reset_effect_state(cfg.enable_effects && FTUI_WINDOWS_EFFECTS);
    g_platform.instance = GetModuleHandleW(nullptr);

    WNDCLASSEXW wc={sizeof(wc)}; wc.style=CS_HREDRAW|CS_VREDRAW; wc.lpfnWndProc=wndproc;
    wc.hInstance=g_platform.instance; wc.hCursor=LoadCursorW(nullptr,(LPCWSTR)IDC_ARROW);
    wc.lpszClassName=L"FTUI_Window";
    wc.hIcon=cfg.icon?(HICON)cfg.icon:LoadIconW(nullptr,(LPCWSTR)IDI_APPLICATION); wc.hIconSm=wc.hIcon;
    if (!RegisterClassExW(&wc)&&GetLastError()!=ERROR_CLASS_ALREADY_EXISTS) return false;

    DWORD style=WS_OVERLAPPEDWINDOW; if(!cfg.resizable)style&=~(WS_THICKFRAME|WS_MAXIMIZEBOX);
    UINT sys_dpi=GetDpiForSystem(); float pre=(float)sys_dpi/96.f;
    RECT rc={0,0,(LONG)(cfg.width*pre+.5f),(LONG)(cfg.height*pre+.5f)};
    using AdjFn=BOOL(WINAPI*)(LPRECT,DWORD,BOOL,DWORD,UINT);
    auto adj=(AdjFn)GetProcAddress(GetModuleHandleW(L"user32.dll"),"AdjustWindowRectExForDpi");
    if(adj) adj(&rc,style,FALSE,0,sys_dpi); else AdjustWindowRect(&rc,style,FALSE);
    int w=rc.right-rc.left,h=rc.bottom-rc.top,x=CW_USEDEFAULT,y=CW_USEDEFAULT;
    if(cfg.center_window){int sw=GetSystemMetrics(SM_CXSCREEN),sh=GetSystemMetrics(SM_CYSCREEN);x=(sw-w)/2;y=(sh-h)/2;}

    std::wstring wtitle = utf8_to_wide(cfg.title ? cfg.title : "FTUI App");
    g_platform.hwnd=CreateWindowExW(0,L"FTUI_Window",wtitle.c_str(),style,x,y,w,h,nullptr,nullptr,g_platform.instance,nullptr);
    if(!g_platform.hwnd) return false;
    g_platform.width=cfg.width; g_platform.height=cfg.height; g_platform.running=true;
    if(!init_d2d()||!create_render_target()||!create_text_format()) return false;
    if(g_platform.width!=cfg.width||g_platform.height!=cfg.height){
        RECT rc2={0,0,(LONG)(cfg.width*g_renderer.dpi_scale+.5f),(LONG)(cfg.height*g_renderer.dpi_scale+.5f)};
        UINT adpi=(UINT)(g_renderer.dpi_scale*96+.5f);
        if(adj)adj(&rc2,style,FALSE,0,adpi);else AdjustWindowRect(&rc2,style,FALSE);
        int nw=rc2.right-rc2.left,nh=rc2.bottom-rc2.top,nx=x,ny=y;
        if(cfg.center_window){nx=(GetSystemMetrics(SM_CXSCREEN)-nw)/2;ny=(GetSystemMetrics(SM_CYSCREEN)-nh)/2;}
        SetWindowPos(g_platform.hwnd,nullptr,nx,ny,nw,nh,SWP_NOZORDER|SWP_NOACTIVATE);
    }
    if (cfg.icon) apply_window_icon_handles(cfg.icon, cfg.icon, false, g_platform.icon_variant);
    else if (!g_platform.icon_owned && (g_platform.icon_big || g_platform.icon_small))
        apply_window_icon_handles(g_platform.icon_big, g_platform.icon_small, false, g_platform.icon_variant);
    else
        apply_window_icon_handles(nullptr, nullptr, true, g_platform.icon_variant);
    sync_native_window_chrome();
    sync_native_window_transparency();
    ShowWindow(g_platform.hwnd,SW_SHOW); UpdateWindow(g_platform.hwnd);
    QueryPerformanceFrequency(&g_freq); QueryPerformanceCounter(&g_last_time);
    return true;
}

void set_window_size(int width, int height) {
    using namespace internal;
    if (!g_platform.hwnd || width <= 0 || height <= 0) return;
    DWORD style = (DWORD)GetWindowLongPtrW(g_platform.hwnd, GWL_STYLE);
    DWORD ex_style = (DWORD)GetWindowLongPtrW(g_platform.hwnd, GWL_EXSTYLE);
    RECT rc = {0, 0, (LONG)(width * g_renderer.dpi_scale + 0.5f), (LONG)(height * g_renderer.dpi_scale + 0.5f)};
    UINT dpi = (UINT)(g_renderer.dpi_scale * 96.0f + 0.5f);
    using AdjFn = BOOL(WINAPI*)(LPRECT, DWORD, BOOL, DWORD, UINT);
    auto adj = (AdjFn)GetProcAddress(GetModuleHandleW(L"user32.dll"), "AdjustWindowRectExForDpi");
    if (adj) adj(&rc, style, FALSE, ex_style, dpi);
    else AdjustWindowRectEx(&rc, style, FALSE, ex_style);
    SetWindowPos(g_platform.hwnd, nullptr, 0, 0, rc.right - rc.left, rc.bottom - rc.top,
                 SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    g_redraw_requested = true;
}

bool pump() {
    using namespace internal;
    g_input.mouse_pressed=g_input.mouse_released=false;
    g_input.wheel_y = 0;
    g_input.key_backspace=g_input.key_enter=g_input.key_space=g_input.key_escape=g_input.key_tab=g_input.key_shift_tab=false;
    g_input.key_left=g_input.key_right=g_input.key_up=g_input.key_down=false;
    g_input.key_ctrl_c=g_input.key_ctrl_v=false;
    g_input.text_input_count=0; memset(g_input.text_input,0,sizeof(g_input.text_input));
    if (!g_redraw_requested) wait_for_win32_event();
    bool saw_event = false;
    MSG msg;
    while(PeekMessageW(&msg,nullptr,0,0,PM_REMOVE)){
        saw_event = true;
        if(msg.message==WM_QUIT){g_platform.running=false;return false;}
        TranslateMessage(&msg); DispatchMessageW(&msg);
    }
    if (saw_event) g_redraw_requested = true;
    return g_platform.running;
}

void begin() {
    using namespace internal;
    g_ctx.hot_id=0;
    g_frame_content_fx = {};
    g_active_collapse_fx_count = 0;
    g_pending_collapse_key = 0;
    g_pending_collapse_start_y = 0.0f;
    g_dropdown_capture_input = g_dropdown_overlay_prev.active;
    g_dropdown_overlay = {};
    g_side_drawer_capture_input = g_side_drawer.open || g_side_drawer.progress > 0.01f ||
                                  (g_side_drawer.active && rect_contains(g_side_drawer.launcher_rect, g_input.mouse_x, g_input.mouse_y));
    g_side_drawer.active = false;
    begin_tooltip_frame();
    g_modal_drawn = false;
    reset_draw_fx();
    g_redraw_requested = false;
    if((g_input.key_tab||g_input.key_shift_tab)&&!g_ctx.tab_stops_prev.empty()){
        auto& stops=g_ctx.tab_stops_prev;
        int cur=g_ctx.focused_widget_id,idx=-1;
        for(int i=0;i<(int)stops.size();i++){if(stops[i]==cur){idx=i;break;}}
        int next_id = g_input.key_shift_tab?stops[(idx<=0?(int)stops.size():idx)-1]:stops[(idx+1)%(int)stops.size()];
        set_widget_focus(next_id, false);
    }
    if (g_focus_request_id) {
        set_widget_focus(g_focus_request_id, false);
        g_focus_request_id = 0;
    }
    g_ctx.tab_stops.clear();
    float pad=g_style.window_padding;
    g_ctx.content_region={pad,pad,(float)g_platform.width-2*pad,(float)g_platform.height-2*pad};
    g_ctx.cursor_x=g_ctx.content_region.x;
    const float kSW=14;
    float vh=g_ctx.content_region.h;
    bool nsb=g_content_height>vh+1;
    float ms=nsb?g_content_height-vh:0;
    LARGE_INTEGER now; QueryPerformanceCounter(&now);
    g_dt=(float)(now.QuadPart-g_last_time.QuadPart)/(float)g_freq.QuadPart;
    g_last_time=now; g_fps_accum+=g_dt; g_fps_frames++;
    if(g_fps_accum>=0.5f){g_fps=(float)g_fps_frames/g_fps_accum;g_fps_frames=0;g_fps_accum=0;}

    if(!nsb) { g_scroll_y=0; g_scroll_target_y=0; }
    if (effects_enabled()) {
        g_scroll_target_y=g_scroll_target_y<0?0:(g_scroll_target_y>ms?ms:g_scroll_target_y);
        g_scroll_y = step_anim(g_scroll_y, g_scroll_target_y, g_dt, 16.0f);
    } else {
        g_scroll_target_y = g_scroll_y;
    }
    g_scroll_y=g_scroll_y<0?0:(g_scroll_y>ms?ms:g_scroll_y);
    if(nsb)g_ctx.content_region.w-=kSW+pad;
    g_ctx.cursor_y=g_ctx.content_region.y-g_scroll_y;
    if(!g_renderer.target) return;
    g_renderer.target->BeginDraw(); g_drawing=true;
    clear_bg(g_style.background);
    if (g_window_transparency == WindowTransparency::BayerDither) {
        draw_dither_backdrop_panel({0.0f, 0.0f, (float)g_platform.width, (float)g_platform.height}, 1.0f);
    }
    D2D1_RECT_F cr={0,g_ctx.content_region.y,(float)g_platform.width,g_ctx.content_region.y+vh};
    g_renderer.target->PushAxisAlignedClip(cr,D2D1_ANTIALIAS_MODE_ALIASED);
}

void end() {
    using namespace internal;
    if(!g_drawing) return;
    finalize_pending_collapse_measure();
    g_renderer.target->PopAxisAlignedClip();

    if (g_frame_content_fx.active) {
        reset_draw_fx();
    }

    float new_ch=g_ctx.cursor_y+g_scroll_y-g_ctx.content_region.y;
    if(new_ch<0)new_ch=0;
    {
        const float kSW=14; float vh=g_ctx.content_region.h;
        bool nsb=g_content_height>vh+1;
        if(nsb){
            float ms=g_content_height-vh;
            if (effects_enabled() && g_input.wheel_y != 0.0f) {
                g_scroll_target_y -= g_input.wheel_y;
                g_input.wheel_y = 0.0f;
            }
            g_scroll_target_y=g_scroll_target_y<0?0:(g_scroll_target_y>ms?ms:g_scroll_target_y);
            float tx=g_ctx.content_region.x+g_ctx.content_region.w+g_style.window_padding;
            Rect track={tx,g_ctx.content_region.y,kSW,vh};
            float th=fmaxf(20,(vh/g_content_height)*vh);
            float tt=ms>0?g_scroll_y/ms:0;
            float ty2=g_ctx.content_region.y+tt*(vh-th);
            Rect thumb={tx,ty2,kSW,th};
            bool thov=rect_contains(thumb,g_input.mouse_x,g_input.mouse_y);
            bool trhov=rect_contains(track,g_input.mouse_x,g_input.mouse_y);
            if(g_input.mouse_pressed&&thov){g_sb_dragging=true;g_sb_drag_mouse_y=g_input.mouse_y;g_sb_drag_scroll0=g_scroll_y;}
            if(g_sb_dragging){
                if(g_input.mouse_down||g_input.mouse_released){
                    float sc=ms/fmaxf(1,vh-th);
                    g_scroll_y=g_sb_drag_scroll0+(g_input.mouse_y-g_sb_drag_mouse_y)*sc;
                    g_scroll_y=g_scroll_y<0?0:(g_scroll_y>ms?ms:g_scroll_y);
                    g_scroll_target_y = g_scroll_y;
                }
                if(g_input.mouse_released)g_sb_dragging=false;
            }
            if(g_input.mouse_pressed&&trhov&&!thov){
                if (effects_enabled()) {
                    g_scroll_target_y += (g_input.mouse_y<ty2?-vh:vh);
                    g_scroll_target_y=g_scroll_target_y<0?0:(g_scroll_target_y>ms?ms:g_scroll_target_y);
                } else {
                    g_scroll_y+=(g_input.mouse_y<ty2?-vh:vh);
                    g_scroll_target_y = g_scroll_y;
                    g_scroll_y=g_scroll_y<0?0:(g_scroll_y>ms?ms:g_scroll_y);
                }
            }
            float thumb_hover = thov ? 1.0f : 0.0f;
            draw_widget_chrome(track, 6, g_style.input_bg, g_style.border, 0.0f, 0.0f, 0.0f);
            draw_widget_chrome(thumb, 6,
                               g_sb_dragging ? g_style.button_active : g_style.button_hover,
                               g_sb_dragging ? g_style.input_focus : g_style.border,
                               thumb_hover, g_sb_dragging ? 1.0f : 0.0f, 0.0f);
        } else {
            if (effects_enabled() && g_input.wheel_y != 0.0f) {
                g_scroll_target_y = 0;
                g_input.wheel_y = 0.0f;
            }
        }
        g_content_height=new_ch;
    }
    if(g_debug.show_fps){
        char buf[64]; snprintf(buf,sizeof(buf),"FPS: %.0f  frame:%d  DPI:%.0f%%",g_fps,g_ctx.frame_index,g_renderer.dpi_scale*100);
        float lh=text_line_height(); Rect r={4,4,300,lh}; fill_rect(r,{0,0,0,.6f}); draw_text_utf8(buf,r,g_style.text_dim);
    }
    if(g_debug.show_hovered_id||g_debug.show_active_id){
        char buf[128]; snprintf(buf,sizeof(buf),"hot=%d active=%d focused=%d",g_ctx.hot_id,g_ctx.active_id,g_ctx.focused_widget_id);
        float lh=text_line_height(),oy=g_debug.show_fps?lh+6:4; Rect r={4,oy,360,lh}; fill_rect(r,{0,0,0,.6f}); draw_text_utf8(buf,r,g_style.text_dim);
    }
    draw_side_drawer_overlay(g_platform.width, g_platform.height);
    draw_command_overlay(g_platform.width, g_platform.height);
    draw_dropdown_overlay();
    finalize_tooltip_frame();
    if (g_modal_open_id && !g_modal_drawn) close_modal();
    draw_tooltip_overlay();
    draw_toast_overlay();
    HRESULT hr=g_renderer.target->EndDraw(); g_drawing=false;
    if(hr==D2DERR_RECREATE_TARGET){release_render_target();create_render_target();}
    bool keep_redrawing = g_redraw_requested || g_debug.show_fps ||
                          g_debug.show_hovered_id || g_debug.show_active_id ||
                          g_input.mouse_down || g_sb_dragging ||
                          g_cmd.active || g_side_drawer.open || g_side_drawer.progress > 0.01f ||
                          g_tooltip.requested || g_tooltip.active ||
                          toasts_active() ||
                          g_dropdown_overlay.active || g_dropdown_overlay_prev.active ||
                          effects_need_redraw() || hr==D2DERR_RECREATE_TARGET;
    wait_frame_limit();
    g_redraw_requested = keep_redrawing;
    g_dropdown_overlay_prev = g_dropdown_overlay;
    g_ctx.tab_stops_prev=g_ctx.tab_stops; g_ctx.frame_index++;
}

void shutdown() {
    using namespace internal;
    release_render_target();
    if(g_renderer.wic_factory)   {g_renderer.wic_factory->Release();   g_renderer.wic_factory=nullptr;}
    if(g_renderer.text_format)   {g_renderer.text_format->Release();   g_renderer.text_format=nullptr;}
    if(g_renderer.dwrite_factory){g_renderer.dwrite_factory->Release();g_renderer.dwrite_factory=nullptr;}
    if(g_renderer.d2d_factory)   {g_renderer.d2d_factory->Release();   g_renderer.d2d_factory=nullptr;}
    destroy_owned_icons();
    if(g_platform.hwnd)          {DestroyWindow(g_platform.hwnd);      g_platform.hwnd=nullptr;}
    if(g_renderer.com_inited)    {CoUninitialize();g_renderer.com_inited=false;}
}

void open_child_window(const Config& cfg, std::function<void()> fn) {
    using namespace internal;
    if(!g_platform.hwnd) return;
    struct Snap {
        PlatformState p; RendererState r; InputState in; UIContext ctx; Style sty; std::string sty_name; DebugState dbg;
        float sc,sct,ch,sbmy,sbms0; bool sbd;
        CmdState cmd; float fps,fpsa; int fpsf, frame_limit_fps; bool redraw_requested;
        LARGE_INTEGER lt;
        int tci,tc,tsa,taci,tac,taas,dither_size; float tasc,tasct,dt,window_opacity;
        BackdropEffect backdrop_effect; WindowTransparency window_transparency;
        bool effects;
        MotionSlot motion[256];
        TabFxSlot tab_fx[32];
        int tab_owner;
        FrameContentFx frame_fx;
        float draw_off_x,draw_off_y,draw_opacity;
        ID2D1Bitmap* prev_frame_bitmap;
        std::vector<BYTE> prev_frame_pixels;
        UINT prev_frame_w, prev_frame_h;
        ScrollSlot scroll_slots[128];
        CollapseSlot collapse_slots[64];
        ActiveCollapseFx active_collapse_fx[16];
        int active_collapse_fx_count;
        int pending_collapse_key;
        float pending_collapse_start_y;
        NextLayoutState next_layout;
        ColorOverrideEntry color_stack[kColorOverrideStackCap];
        int color_stack_count;
        ColorOverrideTable next_color_pending, active_widget_colors;
        int active_widget_color_depth;
        TooltipState tooltip;
        DropdownOverlayState dropdown_overlay, dropdown_overlay_prev;
        int disabled_depth, focus_request_id, modal_open_id, modal_request_id, dropdown_open_id;
        bool inside_modal, modal_drawn, dropdown_capture_input;
        bool shortcuts,drawing;
    } s;
    s.p=g_platform;s.r=g_renderer;s.in=g_input;s.ctx=g_ctx;s.sty=g_style;s.sty_name=g_style_name;s.dbg=g_debug;
    s.sc=g_scroll_y;s.sct=g_scroll_target_y;s.ch=g_content_height;s.sbd=g_sb_dragging;s.sbmy=g_sb_drag_mouse_y;s.sbms0=g_sb_drag_scroll0;
    s.cmd=g_cmd;s.fps=g_fps;s.fpsa=g_fps_accum;s.fpsf=g_fps_frames;s.lt=g_last_time;
    s.frame_limit_fps=g_frame_limit_fps;s.redraw_requested=g_redraw_requested;
    s.backdrop_effect=g_backdrop_effect;s.dither_size=g_dither_size;s.window_transparency=g_window_transparency;s.window_opacity=g_window_opacity;
    s.tci=g_text_cursor_id;s.tc=g_text_cursor;s.tsa=g_text_sel_anchor;
    s.taci=g_ta_cursor_id;s.tac=g_ta_cursor;s.taas=g_ta_sel_anchor;s.tasc=g_ta_scroll_y;s.tasct=g_ta_scroll_target_y;s.dt=g_dt;
    s.effects=g_effects_enabled;
    memcpy(s.motion, g_motion_slots, sizeof(g_motion_slots));
    memcpy(s.tab_fx, g_tab_fx_slots, sizeof(g_tab_fx_slots));
    s.tab_owner=g_tab_content_owner;
    s.frame_fx=g_frame_content_fx;
    s.draw_off_x=g_draw_fx_off_x;s.draw_off_y=g_draw_fx_off_y;s.draw_opacity=g_draw_fx_opacity;
    s.prev_frame_bitmap=g_prev_frame_bitmap;
    s.prev_frame_pixels=std::move(g_prev_frame_pixels);
    s.prev_frame_w=g_prev_frame_w;s.prev_frame_h=g_prev_frame_h;
    memcpy(s.scroll_slots, g_scroll_slots, sizeof(g_scroll_slots));
    memcpy(s.collapse_slots, g_collapse_slots, sizeof(g_collapse_slots));
    memcpy(s.active_collapse_fx, g_active_collapse_fx, sizeof(g_active_collapse_fx));
    s.active_collapse_fx_count = g_active_collapse_fx_count;
    s.pending_collapse_key = g_pending_collapse_key;
    s.pending_collapse_start_y = g_pending_collapse_start_y;
    s.next_layout=g_next_layout;
    memcpy(s.color_stack, g_color_stack, sizeof(g_color_stack));
    s.color_stack_count = g_color_stack_count;
    s.next_color_pending = g_next_color_pending;
    s.active_widget_colors = g_active_widget_colors;
    s.active_widget_color_depth = g_active_widget_color_depth;
    s.tooltip=g_tooltip;
    s.dropdown_overlay=g_dropdown_overlay;
    s.dropdown_overlay_prev=g_dropdown_overlay_prev;
    s.disabled_depth=g_disabled_depth;
    s.focus_request_id=g_focus_request_id;
    s.modal_open_id=g_modal_open_id;
    s.modal_request_id=g_modal_request_id;
    s.dropdown_open_id=g_dropdown_open_id;
    s.inside_modal=g_inside_modal;
    s.modal_drawn=g_modal_drawn;
    s.dropdown_capture_input=g_dropdown_capture_input;
    s.shortcuts=g_shortcuts_enabled;s.drawing=g_drawing;

    auto* d2d = g_renderer.d2d_factory;
    auto* wic = g_renderer.wic_factory;
    auto* dw = g_renderer.dwrite_factory;
    auto  inst = g_platform.instance;
    auto  owner = g_platform.hwnd;

    g_platform={}; g_platform.instance=inst;
    g_renderer={}; g_renderer.d2d_factory=d2d; g_renderer.dwrite_factory=(IDWriteFactory*)dw; g_renderer.wic_factory=(IWICImagingFactory*)wic; g_renderer.dpi_scale=1;
    g_input={}; g_ctx={}; g_style=s.sty; g_style_name=s.sty_name; g_debug=s.dbg;
    g_scroll_y=g_content_height=0; g_sb_dragging=false; g_cmd={};
    g_fps=g_fps_accum=0; g_fps_frames=0;
    g_frame_limit_fps=cfg.fps_limit; g_redraw_requested=true;
    g_backdrop_effect=cfg.backdrop_effect; g_dither_size=cfg.dither_size < 1 ? 1 : (cfg.dither_size > 32 ? 32 : cfg.dither_size);
    g_window_transparency=cfg.window_transparency; g_window_opacity=clampf(cfg.window_opacity, 0.20f, 1.0f);
    g_text_cursor_id=g_text_cursor=g_text_sel_anchor=0;
    g_ta_cursor_id=g_ta_cursor=g_ta_sel_anchor=0; g_ta_scroll_y=0;
    g_prev_frame_bitmap=nullptr; g_prev_frame_pixels.clear(); g_prev_frame_w=0; g_prev_frame_h=0;
    memset(g_scroll_slots, 0, sizeof(g_scroll_slots));
    memset(g_collapse_slots, 0, sizeof(g_collapse_slots));
    memset(g_active_collapse_fx, 0, sizeof(g_active_collapse_fx));
    g_active_collapse_fx_count = 0;
    g_pending_collapse_key = 0;
    g_pending_collapse_start_y = 0.0f;
    g_next_layout={}; g_tooltip={}; g_dropdown_overlay={}; g_dropdown_overlay_prev={}; g_disabled_depth=0; g_focus_request_id=0;
    memcpy(g_color_stack, s.color_stack, sizeof(g_color_stack));
    g_color_stack_count = s.color_stack_count;
    g_next_color_pending = s.next_color_pending;
    g_active_widget_colors = s.active_widget_colors;
    g_active_widget_color_depth = s.active_widget_color_depth;
    g_modal_open_id=0; g_modal_request_id=0; g_dropdown_open_id=0; g_inside_modal=false; g_modal_drawn=false;
    g_dropdown_capture_input=false;
    reset_effect_state(cfg.enable_effects);
    g_shortcuts_enabled=s.shortcuts; g_drawing=false;

    DWORD ws=WS_OVERLAPPEDWINDOW; if(!cfg.resizable)ws&=~(WS_THICKFRAME|WS_MAXIMIZEBOX);
    UINT sd=GetDpiForSystem(); float ps=(float)sd/96;
    RECT rc={0,0,(LONG)(cfg.width*ps+.5f),(LONG)(cfg.height*ps+.5f)};
    using AdjFn=BOOL(WINAPI*)(LPRECT,DWORD,BOOL,DWORD,UINT);
    auto adj=(AdjFn)GetProcAddress(GetModuleHandleW(L"user32.dll"),"AdjustWindowRectExForDpi");
    if(adj)adj(&rc,ws,FALSE,0,sd);else AdjustWindowRect(&rc,ws,FALSE);
    int cw=rc.right-rc.left,ch2=rc.bottom-rc.top,cx=CW_USEDEFAULT,cy=CW_USEDEFAULT;
    if(cfg.center_window){cx=(GetSystemMetrics(SM_CXSCREEN)-cw)/2;cy=(GetSystemMetrics(SM_CYSCREEN)-ch2)/2;}
    std::wstring wt=utf8_to_wide(cfg.title?cfg.title:"FTUI");
    g_platform.hwnd=CreateWindowExW(0,L"FTUI_Window",wt.c_str(),ws,cx,cy,cw,ch2,owner,nullptr,inst,nullptr);
    if(g_platform.hwnd){
        g_platform.width=cfg.width; g_platform.height=cfg.height; g_platform.running=true;
        if(create_render_target()&&create_text_format()){
            QueryPerformanceFrequency(&g_freq); QueryPerformanceCounter(&g_last_time);
            if (cfg.icon) apply_window_icon_handles(cfg.icon, cfg.icon, false, s.p.icon_variant);
            else if (!s.p.icon_owned && (s.p.icon_big || s.p.icon_small))
                apply_window_icon_handles(s.p.icon_big, s.p.icon_small, false, s.p.icon_variant);
            else
                apply_window_icon_handles(nullptr, nullptr, true, s.p.icon_variant);
            sync_native_window_chrome();
            sync_native_window_transparency();
            ShowWindow(g_platform.hwnd,SW_SHOW); UpdateWindow(g_platform.hwnd);
            while(pump()){begin();fn();end();}
        }
        release_render_target();
        if(g_renderer.text_format){g_renderer.text_format->Release();g_renderer.text_format=nullptr;}
        g_renderer.d2d_factory=nullptr; g_renderer.dwrite_factory=nullptr; g_renderer.wic_factory=nullptr;
        if(g_platform.hwnd){DestroyWindow(g_platform.hwnd);g_platform.hwnd=nullptr;}
    }
    destroy_owned_icons();
    g_platform=s.p;g_renderer=s.r;g_input=s.in;g_ctx=s.ctx;g_style=s.sty;g_style_name=s.sty_name;g_debug=s.dbg;
    g_scroll_y=s.sc;g_scroll_target_y=s.sct;g_content_height=s.ch;g_sb_dragging=s.sbd;g_sb_drag_mouse_y=s.sbmy;g_sb_drag_scroll0=s.sbms0;
    g_cmd=s.cmd;g_fps=s.fps;g_fps_accum=s.fpsa;g_fps_frames=s.fpsf;g_last_time=s.lt;
    g_frame_limit_fps=s.frame_limit_fps;g_redraw_requested=s.redraw_requested;
    g_backdrop_effect=s.backdrop_effect;g_dither_size=s.dither_size;g_window_transparency=s.window_transparency;g_window_opacity=s.window_opacity;
    g_text_cursor_id=s.tci;g_text_cursor=s.tc;g_text_sel_anchor=s.tsa;
    g_ta_cursor_id=s.taci;g_ta_cursor=s.tac;g_ta_sel_anchor=s.taas;g_ta_scroll_y=s.tasc;g_ta_scroll_target_y=s.tasct;g_dt=s.dt;
    g_effects_enabled=s.effects;
    memcpy(g_motion_slots, s.motion, sizeof(g_motion_slots));
    memcpy(g_tab_fx_slots, s.tab_fx, sizeof(g_tab_fx_slots));
    g_tab_content_owner=s.tab_owner;
    g_frame_content_fx=s.frame_fx;
    g_draw_fx_off_x=s.draw_off_x;g_draw_fx_off_y=s.draw_off_y;g_draw_fx_opacity=s.draw_opacity;
    g_prev_frame_bitmap=s.prev_frame_bitmap;
    g_prev_frame_pixels=std::move(s.prev_frame_pixels);
    g_prev_frame_w=s.prev_frame_w;g_prev_frame_h=s.prev_frame_h;
    memcpy(g_scroll_slots, s.scroll_slots, sizeof(g_scroll_slots));
    memcpy(g_collapse_slots, s.collapse_slots, sizeof(g_collapse_slots));
    memcpy(g_active_collapse_fx, s.active_collapse_fx, sizeof(g_active_collapse_fx));
    g_active_collapse_fx_count = s.active_collapse_fx_count;
    g_pending_collapse_key = s.pending_collapse_key;
    g_pending_collapse_start_y = s.pending_collapse_start_y;
    g_next_layout=s.next_layout;
    memcpy(g_color_stack, s.color_stack, sizeof(g_color_stack));
    g_color_stack_count = s.color_stack_count;
    g_next_color_pending = s.next_color_pending;
    g_active_widget_colors = s.active_widget_colors;
    g_active_widget_color_depth = s.active_widget_color_depth;
    g_tooltip=s.tooltip;
    g_dropdown_overlay=s.dropdown_overlay;
    g_dropdown_overlay_prev=s.dropdown_overlay_prev;
    g_disabled_depth=s.disabled_depth;
    g_focus_request_id=s.focus_request_id;
    g_modal_open_id=s.modal_open_id;
    g_modal_request_id=s.modal_request_id;
    g_dropdown_open_id=s.dropdown_open_id;
    g_inside_modal=s.inside_modal;
    g_modal_drawn=s.modal_drawn;
    g_dropdown_capture_input=s.dropdown_capture_input;
    g_shortcuts_enabled=s.shortcuts;g_drawing=s.drawing;
    InvalidateRect(owner,nullptr,FALSE);
}

ImageHandle* load_image(const char* utf8_path) {
    using namespace internal;
    if(!g_renderer.target||!utf8_path||!utf8_path[0]) return nullptr;
    if(!g_renderer.wic_factory){
        if(FAILED(CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&g_renderer.wic_factory)))) return nullptr;
    }
    std::wstring wp=utf8_to_wide(utf8_path);
    IWICBitmapDecoder* dec=nullptr;
    if(FAILED(g_renderer.wic_factory->CreateDecoderFromFilename(wp.c_str(),nullptr,GENERIC_READ,WICDecodeMetadataCacheOnLoad,&dec))) return nullptr;
    IWICBitmapFrameDecode* frame=nullptr;
    HRESULT hr=dec->GetFrame(0,&frame); dec->Release(); if(FAILED(hr)) return nullptr;
    IWICFormatConverter* conv=nullptr; g_renderer.wic_factory->CreateFormatConverter(&conv);
    hr=conv->Initialize(frame,GUID_WICPixelFormat32bppPBGRA,WICBitmapDitherTypeNone,nullptr,0,WICBitmapPaletteTypeMedianCut);
    frame->Release(); if(FAILED(hr)){conv->Release();return nullptr;}
    ID2D1Bitmap* bmp=nullptr;
    hr=g_renderer.target->CreateBitmapFromWicBitmap(conv,nullptr,&bmp); conv->Release();
    if(FAILED(hr)) return nullptr;
    ImageHandle* h=new ImageHandle(); h->_impl=bmp; return h;
}
namespace internal {
static bool load_mask_from_image(const char* path, MaskData& out) {
    if (!path || !path[0]) return false;
    if (!g_renderer.wic_factory) {
        if (FAILED(CoCreateInstance(CLSID_WICImagingFactory,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&g_renderer.wic_factory)))) return false;
    }
    std::wstring wp = utf8_to_wide(path);
    IWICBitmapDecoder* dec = nullptr;
    if (FAILED(g_renderer.wic_factory->CreateDecoderFromFilename(wp.c_str(), nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &dec))) return false;
    IWICBitmapFrameDecode* frame = nullptr;
    HRESULT hr = dec->GetFrame(0, &frame);
    dec->Release();
    if (FAILED(hr) || !frame) return false;
    IWICFormatConverter* conv = nullptr;
    if (FAILED(g_renderer.wic_factory->CreateFormatConverter(&conv)) || !conv) { frame->Release(); return false; }
    hr = conv->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, nullptr, 0.0, WICBitmapPaletteTypeMedianCut);
    frame->Release();
    if (FAILED(hr)) { conv->Release(); return false; }
    UINT w = 0, h = 0;
    conv->GetSize(&w, &h);
    if (w == 0 || h == 0 || w > 2048 || h > 2048) { conv->Release(); return false; }
    std::vector<unsigned char> px((size_t)w * (size_t)h * 4u);
    hr = conv->CopyPixels(nullptr, w * 4u, (UINT)px.size(), px.data());
    conv->Release();
    if (FAILED(hr)) return false;
    out.w = (int)w; out.h = (int)h;
    out.bits.assign((size_t)out.w * (size_t)out.h, 0);
    bool has_bright_mask = false;
    for (size_t i = 0, p = 0; i < out.bits.size(); ++i, p += 4) {
        int lum = ((int)px[p] * 54 + (int)px[p + 1] * 183 + (int)px[p + 2] * 19) / 256;
        int alpha = px[p + 3];
        if (alpha > 32 && lum > 96) { has_bright_mask = true; break; }
    }
    for (size_t i = 0, p = 0; i < out.bits.size(); ++i, p += 4) {
        int lum = ((int)px[p] * 54 + (int)px[p + 1] * 183 + (int)px[p + 2] * 19) / 256;
        int alpha = px[p + 3];
        out.bits[i] = (alpha > 32 && (has_bright_mask ? lum > 96 : true)) ? 255 : 0;
    }
    return true;
}
} // namespace internal
void free_image(ImageHandle* img) {
    if(!img) return;
    if(img->_impl){static_cast<ID2D1Bitmap*>(img->_impl)->Release();img->_impl=nullptr;}
    delete img;
}

std::string open_file_dialog(const char* title, const FileFilter* filters, int filter_count) {
    using namespace internal;
    IFileOpenDialog* dlg=nullptr;
    if(FAILED(CoCreateInstance(CLSID_FileOpenDialog,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&dlg)))) return "";
    if(title){std::wstring wt=utf8_to_wide(title);dlg->SetTitle(wt.c_str());}
    if(filters&&filter_count>0){
        std::vector<COMDLG_FILTERSPEC> specs(filter_count);
        std::vector<std::wstring> wnames(filter_count),wspecs(filter_count);
        for(int i=0;i<filter_count;i++){
            wnames[i]=utf8_to_wide(filters[i].name?filters[i].name:"");
            wspecs[i]=utf8_to_wide(filters[i].spec?filters[i].spec:"*.*");
            specs[i]={wnames[i].c_str(),wspecs[i].c_str()};
        }
        dlg->SetFileTypes((UINT)filter_count,specs.data()); dlg->SetFileTypeIndex(1);
    }
    std::string result;
    if(SUCCEEDED(dlg->Show(g_platform.hwnd))){
        IShellItem* item=nullptr;
        if(SUCCEEDED(dlg->GetResult(&item))){
            PWSTR wpath=nullptr;
            if(SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH,&wpath))){
                int n=WideCharToMultiByte(CP_UTF8,0,wpath,-1,nullptr,0,nullptr,nullptr);
                if(n>1){result.resize(n-1);WideCharToMultiByte(CP_UTF8,0,wpath,-1,&result[0],n,nullptr,nullptr);}
                CoTaskMemFree(wpath);
            }
            item->Release();
        }
    }
    dlg->Release(); return result;
}


// ============================================================
// Linux Implementation (X11 + Cairo)
// ============================================================
#elif defined(__linux__)

namespace internal {

struct LinuxImage { cairo_surface_t* surface = nullptr; };

struct PlatformState {
    Display* display = nullptr; Window window = 0;
    Atom wm_delete = 0; bool running = false; int width = 0, height = 0;
};
struct RendererState {
    cairo_surface_t* xlib_surf = nullptr;
    cairo_surface_t* back_surf = nullptr;
    Pixmap           back_px   = 0;
    cairo_t*         cr        = nullptr;
    float            dpi_scale = 1.0f;
    char             font_face[64] = "sans-serif";
};

static PlatformState  g_platform;
static RendererState  g_renderer;
static struct timespec g_last_time;
static XIM  g_xim = nullptr;
static XIC  g_xic = nullptr;
static std::string g_clipboard_buf;
static cairo_surface_t* g_dither_surface = nullptr;
static cairo_pattern_t* g_dither_pattern = nullptr;
static int             g_dither_cache_cell = 0;
static Color           g_dither_cache_accent = {};
static Color           g_dither_cache_dim = {};

static void dbg(const char* fmt, ...) {
    char buf[512]; va_list a; va_start(a,fmt); vsnprintf(buf,sizeof(buf),fmt,a); va_end(a);
    fputs(buf, stderr);
}

static void set_color(Color c) { cairo_set_source_rgba(g_renderer.cr,c.r,c.g,c.b,c.a); }
static void sync_native_window_chrome() {}
static void sync_native_window_transparency() {
    if (!g_platform.display || !g_platform.window) return;
    Atom opacity = XInternAtom(g_platform.display, "_NET_WM_WINDOW_OPACITY", False);
    if (g_window_transparency == WindowTransparency::Plain ||
        g_window_transparency == WindowTransparency::BayerDither) {
        unsigned long value = (unsigned long)(clampf(g_window_opacity, 0.20f, 1.0f) * 4294967295.0f + 0.5f);
        XChangeProperty(g_platform.display, g_platform.window, opacity, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char*)&value, 1);
    } else {
        XDeleteProperty(g_platform.display, g_platform.window, opacity);
    }
    XFlush(g_platform.display);
}
static void apply_window_icon_handles(void*, void*, bool, BuiltinIcon) {}

static void rrect_path(cairo_t* cr, float x, float y, float w, float h, float r) {
    if (r <= 0.0f || w <= 0.0f || h <= 0.0f) { cairo_rectangle(cr,x,y,w,h); return; }
    float rr = r < w*0.5f ? (r < h*0.5f ? r : h*0.5f) : w*0.5f;
    const double PI = M_PI;
    cairo_new_sub_path(cr);
    cairo_arc(cr, x+w-rr, y+rr,     rr, -PI/2,  0);
    cairo_arc(cr, x+w-rr, y+h-rr,   rr,  0,      PI/2);
    cairo_arc(cr, x+rr,   y+h-rr,   rr,  PI/2,   PI);
    cairo_arc(cr, x+rr,   y+rr,     rr,  PI,      3*PI/2);
    cairo_close_path(cr);
}

static void fill_round_rect(Rect r, float rad, Color c) {
    set_color(c); rrect_path(g_renderer.cr, r.x,r.y,r.w,r.h,rad); cairo_fill(g_renderer.cr);
}
static void stroke_round_rect(Rect r, float rad, float thick, Color c) {
    set_color(c); cairo_set_line_width(g_renderer.cr, thick);
    rrect_path(g_renderer.cr, r.x,r.y,r.w,r.h,rad); cairo_stroke(g_renderer.cr);
}
static void fill_rect(Rect r, Color c) {
    set_color(c); cairo_rectangle(g_renderer.cr,r.x,r.y,r.w,r.h); cairo_fill(g_renderer.cr);
}
static void release_dither_cache() {
    if (g_dither_pattern) { cairo_pattern_destroy(g_dither_pattern); g_dither_pattern = nullptr; }
    if (g_dither_surface) { cairo_surface_destroy(g_dither_surface); g_dither_surface = nullptr; }
    g_dither_cache_cell = 0;
    g_dither_cache_accent = {};
    g_dither_cache_dim = {};
}
static bool ensure_dither_pattern() {
    int cell = g_dither_size < 1 ? 1 : (g_dither_size > 32 ? 32 : g_dither_size);
    Color accent = resolve_color(ColorRole::Accent);
    Color dim = resolve_color(ColorRole::TextDim);
    if (g_dither_pattern && g_dither_surface && g_dither_cache_cell == cell &&
        style_eq(g_dither_cache_accent, accent) && style_eq(g_dither_cache_dim, dim)) {
        return true;
    }
    release_dither_cache();
    std::vector<unsigned char> px;
    int tw = 0, th = 0;
    build_soft_dither_tile(cell, px, tw, th);
    if (px.empty() || tw <= 0 || th <= 0) return false;
    g_dither_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, tw, th);
    if (!g_dither_surface || cairo_surface_status(g_dither_surface) != CAIRO_STATUS_SUCCESS) {
        release_dither_cache();
        return false;
    }
    unsigned char* data = cairo_image_surface_get_data(g_dither_surface);
    int stride = cairo_image_surface_get_stride(g_dither_surface);
    for (int y = 0; y < th; ++y) {
        memcpy(data + (size_t)y * (size_t)stride, px.data() + (size_t)y * (size_t)tw * 4u, (size_t)tw * 4u);
    }
    cairo_surface_mark_dirty(g_dither_surface);
    g_dither_pattern = cairo_pattern_create_for_surface(g_dither_surface);
    if (!g_dither_pattern || cairo_pattern_status(g_dither_pattern) != CAIRO_STATUS_SUCCESS) {
        release_dither_cache();
        return false;
    }
    cairo_pattern_set_extend(g_dither_pattern, CAIRO_EXTEND_REPEAT);
    cairo_pattern_set_filter(g_dither_pattern, CAIRO_FILTER_NEAREST);
    g_dither_cache_cell = cell;
    g_dither_cache_accent = accent;
    g_dither_cache_dim = dim;
    return true;
}
static void draw_dither_pattern(Rect r, float opacity) {
    if (opacity <= 0.001f || !ensure_dither_pattern()) return;
    cairo_t* cr = g_renderer.cr;
    cairo_save(cr);
    cairo_rectangle(cr, r.x, r.y, r.w, r.h);
    cairo_clip(cr);
    cairo_set_source(cr, g_dither_pattern);
    cairo_paint_with_alpha(cr, clamp01(opacity));
    cairo_restore(cr);
}
static void fill_triangle(Rect r, int dir, Color c) {
    set_color(c);
    cairo_new_path(g_renderer.cr);
    if (dir == 0) {
        cairo_move_to(g_renderer.cr, r.x + r.w * 0.5f, r.y + r.h);
        cairo_line_to(g_renderer.cr, r.x, r.y);
        cairo_line_to(g_renderer.cr, r.x + r.w, r.y);
    } else if (dir == 1) {
        cairo_move_to(g_renderer.cr, r.x + r.w * 0.5f, r.y);
        cairo_line_to(g_renderer.cr, r.x, r.y + r.h);
        cairo_line_to(g_renderer.cr, r.x + r.w, r.y + r.h);
    } else {
        cairo_move_to(g_renderer.cr, r.x + r.w, r.y + r.h * 0.5f);
        cairo_line_to(g_renderer.cr, r.x, r.y);
        cairo_line_to(g_renderer.cr, r.x, r.y + r.h);
    }
    cairo_close_path(g_renderer.cr);
    cairo_fill(g_renderer.cr);
}
static void draw_line(float x0, float y0, float x1, float y1, float thick, Color c) {
    set_color(c); cairo_set_line_width(g_renderer.cr,thick);
    cairo_move_to(g_renderer.cr,x0,y0); cairo_line_to(g_renderer.cr,x1,y1); cairo_stroke(g_renderer.cr);
}
static void push_clip(Rect r) {
    cairo_save(g_renderer.cr); cairo_rectangle(g_renderer.cr,r.x,r.y,r.w,r.h); cairo_clip(g_renderer.cr);
}
static void pop_clip() { cairo_restore(g_renderer.cr); }

static void apply_font() {
#ifdef FTUI_LINUX_FONT
    cairo_select_font_face(g_renderer.cr, FTUI_LINUX_FONT, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
#else
    cairo_select_font_face(g_renderer.cr, g_renderer.font_face, CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
#endif
    cairo_set_font_size(g_renderer.cr, g_style.font_size);
}

static void draw_text_utf8(const char* utf8, Rect r, Color c) {
    if (!utf8||!utf8[0]) return;
    cairo_t* cr = g_renderer.cr;
    apply_font();
    cairo_font_extents_t fe; cairo_font_extents(cr,&fe);
    double by = r.y + (r.h - fe.height)*0.5 + fe.ascent;
    set_color(c);
    cairo_save(cr); cairo_rectangle(cr,r.x,r.y,r.w,r.h); cairo_clip(cr);
    cairo_move_to(cr, r.x, by); cairo_show_text(cr, utf8); cairo_restore(cr);
}

static void draw_text_utf8_centered(const char* utf8, Rect r, Color c) {
    if (!utf8||!utf8[0]) return;
    cairo_t* cr = g_renderer.cr;
    apply_font();
    cairo_text_extents_t te; cairo_text_extents(cr,utf8,&te);
    cairo_font_extents_t fe; cairo_font_extents(cr,&fe);
    double cx = r.x + (r.w - te.x_advance)*0.5;
    double by = r.y + (r.h - fe.height)*0.5 + fe.ascent;
    set_color(c);
    cairo_save(cr); cairo_rectangle(cr,r.x,r.y,r.w,r.h); cairo_clip(cr);
    cairo_move_to(cr,cx,by); cairo_show_text(cr,utf8); cairo_restore(cr);
}

static float measure_text_width(const char* utf8) {
    if (!utf8||!utf8[0]) return 0;
    apply_font();
    cairo_text_extents_t te; cairo_text_extents(g_renderer.cr,utf8,&te);
    return (float)te.x_advance;
}

static int byte_from_x(const char* utf8, float rel_x) {
    if (!utf8||!utf8[0]||rel_x<=0) return 0;
    int len=(int)strlen(utf8), pos=0;
    float prev=0;
    while (pos<len) {
        int np=utf8_advance(utf8,pos);
        float w=measure_text_at(utf8,np);
        float mid=(prev+w)*0.5f;
        if (rel_x<mid) return pos;
        prev=w; pos=np;
    }
    return len;
}

static void draw_image_scaled(cairo_surface_t* surf, Rect dst) {
    if (!surf) return;
    double sw=cairo_image_surface_get_width(surf);
    double sh=cairo_image_surface_get_height(surf);
    if (sw<=0||sh<=0) return;
    cairo_t* cr=g_renderer.cr;
    cairo_save(cr);
    cairo_translate(cr,dst.x,dst.y);
    cairo_scale(cr,dst.w/sw,dst.h/sh);
    cairo_set_source_surface(cr,surf,0,0);
    cairo_pattern_set_filter(cairo_get_source(cr),CAIRO_FILTER_BILINEAR);
    cairo_rectangle(cr,0,0,sw,sh); cairo_fill(cr);
    cairo_restore(cr);
}

static void draw_image_handle(ImageHandle* img, Rect r) {
    if (!img||!img->_impl) return;
    auto* li=static_cast<LinuxImage*>(img->_impl);
    if (li&&li->surface) draw_image_scaled(li->surface,r);
}

static float detect_dpi(Display* dpy) {
    char* xrm=XResourceManagerString(dpy);
    if (xrm) {
        XrmDatabase db=XrmGetStringDatabase(xrm);
        if (db) {
            XrmValue val; char* type=nullptr;
            if (XrmGetResource(db,"Xft.dpi","Xft.Dpi",&type,&val)&&val.addr) {
                float dpi=(float)atof(val.addr); XrmDestroyDatabase(db);
                if (dpi>0) return dpi;
            }
            XrmDestroyDatabase(db);
        }
    }
    int s=DefaultScreen(dpy);
    int pw=DisplayWidth(dpy,s),mm=DisplayWidthMM(dpy,s);
    if (mm>0){float d=(float)pw/((float)mm/25.4f);if(d>0&&d<1000)return d;}
    return 96.0f;
}

static bool create_back_buffer(int w, int h) {
    if (!g_platform.display||!g_platform.window) return false;
    Display* dpy=g_platform.display; int s=DefaultScreen(dpy);
    if (g_renderer.cr)       { cairo_destroy(g_renderer.cr);          g_renderer.cr=nullptr; }
    if (g_renderer.back_surf){ cairo_surface_destroy(g_renderer.back_surf); g_renderer.back_surf=nullptr; }
    if (g_renderer.back_px)  { XFreePixmap(dpy,g_renderer.back_px);  g_renderer.back_px=0; }
    g_renderer.back_px=XCreatePixmap(dpy,g_platform.window,w,h,DefaultDepth(dpy,s));
    g_renderer.back_surf=cairo_xlib_surface_create(dpy,g_renderer.back_px,DefaultVisual(dpy,s),w,h);
    g_renderer.cr=cairo_create(g_renderer.back_surf);
    return cairo_status(g_renderer.cr)==CAIRO_STATUS_SUCCESS;
}

static bool init_cairo() {
    Display* dpy=g_platform.display; int s=DefaultScreen(dpy);
    g_renderer.xlib_surf=cairo_xlib_surface_create(dpy,g_platform.window,DefaultVisual(dpy,s),g_platform.width,g_platform.height);
    return create_back_buffer(g_platform.width,g_platform.height);
}

static void teardown_cairo() {
    release_dither_cache();
    if (g_renderer.cr)       { cairo_destroy(g_renderer.cr);          g_renderer.cr=nullptr; }
    if (g_renderer.back_surf){ cairo_surface_destroy(g_renderer.back_surf); g_renderer.back_surf=nullptr; }
    if (g_renderer.back_px&&g_platform.display){ XFreePixmap(g_platform.display,g_renderer.back_px); g_renderer.back_px=0; }
    if (g_renderer.xlib_surf){ cairo_surface_destroy(g_renderer.xlib_surf); g_renderer.xlib_surf=nullptr; }
}

static void swap_buffers() {
    if (!g_renderer.xlib_surf||!g_renderer.back_surf) return;
    cairo_t* fc=cairo_create(g_renderer.xlib_surf);
    cairo_set_source_surface(fc,g_renderer.back_surf,0,0);
    cairo_paint(fc); cairo_destroy(fc);
    XFlush(g_platform.display);
}

static void wait_frame_limit() {
    if (g_frame_limit_fps <= 0) return;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    double elapsed = (double)(now.tv_sec - g_last_time.tv_sec) +
                     (double)(now.tv_nsec - g_last_time.tv_nsec) * 1e-9;
    double remaining = (1.0 / (double)g_frame_limit_fps) - elapsed;
    if (remaining <= 0.0) return;

    struct timespec ts;
    ts.tv_sec = (time_t)remaining;
    ts.tv_nsec = (long)((remaining - (double)ts.tv_sec) * 1000000000.0);
    while (nanosleep(&ts, &ts) == -1 && errno == EINTR) {}
}

static void wait_for_x_event() {
    if (!g_platform.display) return;
    int fd = ConnectionNumber(g_platform.display);
    while (g_platform.running && !XPending(g_platform.display)) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        int r = select(fd + 1, &fds, nullptr, nullptr, nullptr);
        if (r < 0 && errno == EINTR) continue;
        if (r <= 0) break;
    }
}

static void wake_event_loop() {}

static void clipboard_set(const char* utf8) {
    if (!utf8) return;
    g_clipboard_buf=utf8;
    Atom clip=XInternAtom(g_platform.display,"CLIPBOARD",False);
    XSetSelectionOwner(g_platform.display,clip,g_platform.window,CurrentTime);
    XSetSelectionOwner(g_platform.display,XA_PRIMARY,g_platform.window,CurrentTime);
}

static void serve_selection(XEvent& ev) {
    XSelectionRequestEvent* req=&ev.xselectionrequest;
    XEvent resp={}; resp.xselection.type=SelectionNotify;
    resp.xselection.display=req->display; resp.xselection.requestor=req->requestor;
    resp.xselection.selection=req->selection; resp.xselection.target=req->target;
    resp.xselection.time=req->time; resp.xselection.property=None;
    Atom utf8a=XInternAtom(g_platform.display,"UTF8_STRING",False);
    Atom tgts=XInternAtom(g_platform.display,"TARGETS",False);
    if (req->target==tgts) {
        Atom t[]={tgts,utf8a,XA_STRING};
        XChangeProperty(req->display,req->requestor,req->property,XA_ATOM,32,PropModeReplace,(unsigned char*)t,3);
        resp.xselection.property=req->property;
    } else if (req->target==utf8a||req->target==XA_STRING) {
        XChangeProperty(req->display,req->requestor,req->property,req->target,8,PropModeReplace,(unsigned char*)g_clipboard_buf.c_str(),(int)g_clipboard_buf.size());
        resp.xselection.property=req->property;
    }
    XSendEvent(req->display,req->requestor,False,0,&resp);
}

static std::string clipboard_get() {
    Display* dpy=g_platform.display; Window win=g_platform.window;
    Atom clip=XInternAtom(dpy,"CLIPBOARD",False);
    if (XGetSelectionOwner(dpy,clip)==win) return g_clipboard_buf;
    Atom utf8a=XInternAtom(dpy,"UTF8_STRING",False);
    Atom prop=XInternAtom(dpy,"FTUI_CLIP",False);
    XConvertSelection(dpy,clip,utf8a,prop,win,CurrentTime); XFlush(dpy);
    for (int i=0;i<50;i++) {
        if (XPending(dpy)) {
            XEvent ev; XNextEvent(dpy,&ev);
            if (ev.type==SelectionNotify) {
                if (ev.xselection.property==None) return "";
                Atom at; int fmt; unsigned long n,ba; unsigned char* data=nullptr;
                XGetWindowProperty(dpy,win,prop,0,0x7fffffff,True,AnyPropertyType,&at,&fmt,&n,&ba,&data);
                if (data) { std::string r((char*)data,n); XFree(data); return r; }
                return "";
            }
            if (ev.type==SelectionRequest) serve_selection(ev);
        }
        struct timespec ts={0,2000000}; nanosleep(&ts,nullptr);
    }
    return "";
}

static void execute_command() {
    if (strcmp(g_cmd.buf,"q")==0 || strcmp(g_cmd.buf,"quit")==0) g_platform.running=false;
    else apply_cmd_action();
    cmd_clear();
}

static void handle_xevent(XEvent& ev) {
    if (ev.type==SelectionRequest) { serve_selection(ev); return; }
    if (ev.xany.window!=g_platform.window) return;
    switch (ev.type) {
    case ConfigureNotify: {
        int nw=ev.xconfigure.width,nh=ev.xconfigure.height;
        if (nw!=g_platform.width||nh!=g_platform.height) {
            g_platform.width=nw; g_platform.height=nh;
            if (g_renderer.xlib_surf) cairo_xlib_surface_set_size(g_renderer.xlib_surf,nw,nh);
            create_back_buffer(nw,nh);
        }
        break; }
    case ButtonPress:
        if (ev.xbutton.button==Button1) {
            g_input.mouse_pressed=g_input.mouse_down=true;
            g_input.mouse_x=(float)ev.xbutton.x; g_input.mouse_y=(float)ev.xbutton.y;
            if (g_cmd.active) cmd_clear();
        } else if (ev.xbutton.button==Button4) {
            if (effects_enabled()) g_input.wheel_y += 80.0f;
            else { g_scroll_y-=80; if(g_scroll_y<0)g_scroll_y=0; g_scroll_target_y = g_scroll_y; }
        } else if (ev.xbutton.button==Button5) {
            if (effects_enabled()) g_input.wheel_y -= 80.0f;
            else { g_scroll_y+=80; g_scroll_target_y = g_scroll_y; }
        }
        break;
    case ButtonRelease:
        if (ev.xbutton.button==Button1) {
            g_input.mouse_released=true; g_input.mouse_down=false;
            g_input.mouse_x=(float)ev.xbutton.x; g_input.mouse_y=(float)ev.xbutton.y;
        }
        break;
    case MotionNotify:
        g_input.mouse_x=(float)ev.xmotion.x; g_input.mouse_y=(float)ev.xmotion.y; break;
    case KeyRelease:
        g_input.shift_held=(ev.xkey.state&ShiftMask)!=0;
        g_input.ctrl_held =(ev.xkey.state&ControlMask)!=0; break;
    case KeyPress: {
        g_input.shift_held=(ev.xkey.state&ShiftMask)!=0;
        g_input.ctrl_held =(ev.xkey.state&ControlMask)!=0;
        char buf[32]={}; KeySym ks=0;
        int n=0;
        if (g_xic) {
            Status st; n=Xutf8LookupString(g_xic,&ev.xkey,buf,sizeof(buf)-1,&ks,&st);
            if (st==XLookupNone||st==XLookupChars) { if(st==XLookupNone) n=0; ks=XLookupKeysym(&ev.xkey,0); }
        } else {
            ks=XLookupKeysym(&ev.xkey,0);
            n=XLookupString(&ev.xkey,buf,sizeof(buf)-1,nullptr,nullptr);
        }
        // navigation keys
        if (ks==XK_BackSpace) g_input.key_backspace=true;
        if (ks==XK_Return||ks==XK_KP_Enter) g_input.key_enter=true;
        if (ks==XK_space) g_input.key_space=true;
        if (ks==XK_Escape) g_input.key_escape=true;
        if (ks==XK_Tab) { if(g_input.shift_held)g_input.key_shift_tab=true;else g_input.key_tab=true; }
        if (ks==XK_Left)  g_input.key_left=true;
        if (ks==XK_Right) g_input.key_right=true;
        if (ks==XK_Up)    g_input.key_up=true;
        if (ks==XK_Down)  g_input.key_down=true;
        if (ks==XK_Escape && g_cmd.active) { cmd_clear(); break; }
        if (g_input.ctrl_held) {
            if (ks==XK_q||ks==XK_Q) { if(g_shortcuts_enabled)g_platform.running=false; break; }
            if (ks==XK_c||ks==XK_C) g_input.key_ctrl_c=true;
            if (ks==XK_v||ks==XK_V) g_input.key_ctrl_v=true;
            if ((ks==XK_a||ks==XK_A)&&g_ctx.focused_input_id!=0) {
                g_text_cursor_id=g_ctx.focused_input_id; g_text_sel_anchor=0; g_text_cursor=0x7FFFFFFF;
                g_ta_cursor_id  =g_ctx.focused_input_id; g_ta_sel_anchor  =0; g_ta_cursor  =0x7FFFFFFF;
            }
            break;
        }
        if (g_cmd.active) {
            if (ks==XK_BackSpace){if(g_cmd.len>0)g_cmd.buf[--g_cmd.len]='\0';}
            else if(ks==XK_Return||ks==XK_KP_Enter) execute_command();
            else if(ks==XK_Tab) gui_command_accept_completion();
            else if(n>0&&buf[0]>=32&&g_cmd.len<(int)sizeof(g_cmd.buf)-1){g_cmd.buf[g_cmd.len++]=buf[0];g_cmd.buf[g_cmd.len]='\0';}
            break;
        }
        if (n>0&&buf[0]==':'&&g_ctx.focused_input_id==0&&g_shortcuts_enabled) {
            g_cmd.active=true; g_cmd.len=0; g_cmd.buf[0]='\0'; break;
        }
        if (n>0&&(unsigned char)buf[0]>=32) {
            if (g_input.text_input_count+n<(int)sizeof(g_input.text_input)) {
                memcpy(g_input.text_input+g_input.text_input_count,buf,n);
                g_input.text_input_count+=n;
            }
        }
        break; }
    case FocusIn:  g_input.focused=true; break;
    case FocusOut: g_input.focused=false; g_ctx.focused_input_id=0; g_ctx.focused_widget_id=0; break;
    case ClientMessage:
        if ((Atom)ev.xclient.data.l[0]==g_platform.wm_delete) g_platform.running=false; break;
    }
}

} // namespace internal

bool create_window(const Config& cfg) {
    using namespace internal;
    Display* dpy=XOpenDisplay(nullptr); if (!dpy) return false;
    g_platform.display=dpy;
    g_renderer.dpi_scale=detect_dpi(dpy)/96.0f;
    int s=DefaultScreen(dpy); Window root=RootWindow(dpy,s);
    int x=0,y=0;
    if (cfg.center_window){x=(DisplayWidth(dpy,s)-cfg.width)/2;y=(DisplayHeight(dpy,s)-cfg.height)/2;}
    g_platform.window=XCreateSimpleWindow(dpy,root,x,y,cfg.width,cfg.height,0,BlackPixel(dpy,s),BlackPixel(dpy,s));
    g_platform.wm_delete=XInternAtom(dpy,"WM_DELETE_WINDOW",False);
    XSetWMProtocols(dpy,g_platform.window,&g_platform.wm_delete,1);
    const char* title=cfg.title?cfg.title:"FTUI App";
    XStoreName(dpy,g_platform.window,title);
    Atom net_wm_name=XInternAtom(dpy,"_NET_WM_NAME",False);
    Atom utf8str=XInternAtom(dpy,"UTF8_STRING",False);
    XChangeProperty(dpy,g_platform.window,net_wm_name,utf8str,8,PropModeReplace,(unsigned char*)title,(int)strlen(title));
    XClassHint ch={(char*)"ftui",(char*)"ftui"}; XSetClassHint(dpy,g_platform.window,&ch);
    if (!cfg.resizable){XSizeHints sh={};sh.flags=PMinSize|PMaxSize;sh.min_width=sh.max_width=cfg.width;sh.min_height=sh.max_height=cfg.height;XSetWMNormalHints(dpy,g_platform.window,&sh);}
    XSelectInput(dpy,g_platform.window,ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|KeyPressMask|KeyReleaseMask|StructureNotifyMask|FocusChangeMask);
    g_xim=XOpenIM(dpy,nullptr,nullptr,nullptr);
    if (g_xim) g_xic=XCreateIC(g_xim,XNInputStyle,(XIMPreeditNothing|XIMStatusNothing),XNClientWindow,g_platform.window,XNFocusWindow,g_platform.window,(void*)nullptr);
    XMapWindow(dpy,g_platform.window); XFlush(dpy);
    g_platform.width=cfg.width; g_platform.height=cfg.height; g_platform.running=true;
    g_style=startup_style();
    g_style_name=style_name(g_style);
    g_frame_limit_fps = cfg.fps_limit;
    g_redraw_requested = true;
    g_backdrop_effect = cfg.backdrop_effect;
    g_dither_size = cfg.dither_size < 1 ? 1 : (cfg.dither_size > 32 ? 32 : cfg.dither_size);
    g_window_transparency = cfg.window_transparency;
    g_window_opacity = clampf(cfg.window_opacity, 0.20f, 1.0f);
    g_scroll_y = g_scroll_target_y = 0;
    g_ta_scroll_y = g_ta_scroll_target_y = 0;
    reset_effect_state(cfg.enable_effects);
    if (!init_cairo()) return false;
    sync_native_window_transparency();
    clock_gettime(CLOCK_MONOTONIC,&g_last_time);
    return true;
}

void set_window_size(int width, int height) {
    using namespace internal;
    if (!g_platform.display || !g_platform.window || width <= 0 || height <= 0) return;
    XResizeWindow(g_platform.display, g_platform.window, (unsigned)width, (unsigned)height);
    XFlush(g_platform.display);
    g_redraw_requested = true;
    wake_event_loop();
}

bool pump() {
    using namespace internal;
    g_input.mouse_pressed=g_input.mouse_released=false;
    g_input.wheel_y = 0;
    g_input.key_backspace=g_input.key_enter=g_input.key_space=g_input.key_escape=g_input.key_tab=g_input.key_shift_tab=false;
    g_input.key_left=g_input.key_right=g_input.key_up=g_input.key_down=false;
    g_input.key_ctrl_c=g_input.key_ctrl_v=false;
    g_input.text_input_count=0; memset(g_input.text_input,0,sizeof(g_input.text_input));
    if (!g_redraw_requested) wait_for_x_event();
    bool saw_event = false;
    while (XPending(g_platform.display)) {
        saw_event = true;
        XEvent ev; XNextEvent(g_platform.display,&ev); handle_xevent(ev);
        if (!g_platform.running) return false;
    }
    if (saw_event) g_redraw_requested = true;
    return g_platform.running;
}

void begin() {
    using namespace internal;
    g_ctx.hot_id=0;
    g_frame_content_fx = {};
    g_active_collapse_fx_count = 0;
    g_pending_collapse_key = 0;
    g_pending_collapse_start_y = 0.0f;
    g_dropdown_capture_input = g_dropdown_overlay_prev.active;
    g_dropdown_overlay = {};
    g_side_drawer_capture_input = g_side_drawer.open || g_side_drawer.progress > 0.01f ||
                                  (g_side_drawer.active && rect_contains(g_side_drawer.launcher_rect, g_input.mouse_x, g_input.mouse_y));
    g_side_drawer.active = false;
    begin_tooltip_frame();
    g_modal_drawn = false;
    reset_draw_fx();
    g_redraw_requested = false;
    if ((g_input.key_tab||g_input.key_shift_tab)&&!g_ctx.tab_stops_prev.empty()) {
        auto& stops=g_ctx.tab_stops_prev;
        int cur=g_ctx.focused_widget_id,idx=-1;
        for(int i=0;i<(int)stops.size();i++){if(stops[i]==cur){idx=i;break;}}
        int next_id=g_input.key_shift_tab?stops[(idx<=0?(int)stops.size():idx)-1]:stops[(idx+1)%(int)stops.size()];
        set_widget_focus(next_id, false);
    }
    if (g_focus_request_id) {
        set_widget_focus(g_focus_request_id, false);
        g_focus_request_id = 0;
    }
    g_ctx.tab_stops.clear();
    float pad=g_style.window_padding;
    g_ctx.content_region={pad,pad,(float)g_platform.width-2*pad,(float)g_platform.height-2*pad};
    g_ctx.cursor_x=g_ctx.content_region.x;
    const float kSW=14; float vh=g_ctx.content_region.h;
    bool nsb=g_content_height>vh+1;
    float ms=nsb?g_content_height-vh:0;
    struct timespec now; clock_gettime(CLOCK_MONOTONIC,&now);
    g_dt=(now.tv_sec-g_last_time.tv_sec)+(now.tv_nsec-g_last_time.tv_nsec)*1e-9f;
    g_last_time=now; g_fps_accum+=g_dt; g_fps_frames++;
    if(g_fps_accum>=0.5f){g_fps=(float)g_fps_frames/g_fps_accum;g_fps_frames=0;g_fps_accum=0;}
    if(!nsb) {
        g_scroll_y=0; g_scroll_target_y=0;
    } else if (effects_enabled()) {
        g_scroll_target_y=g_scroll_target_y<0?0:(g_scroll_target_y>ms?ms:g_scroll_target_y);
        g_scroll_y = step_anim(g_scroll_y, g_scroll_target_y, g_dt, 16.0f);
    } else {
        g_scroll_target_y = g_scroll_y;
    }
    g_scroll_y=g_scroll_y<0?0:(g_scroll_y>ms?ms:g_scroll_y);
    if(nsb)g_ctx.content_region.w-=kSW+pad;
    g_ctx.cursor_y=g_ctx.content_region.y-g_scroll_y;
    if(!g_renderer.cr) return;
    apply_font();
    // clear
    set_color(g_style.background); cairo_paint(g_renderer.cr);
    if (g_window_transparency == WindowTransparency::BayerDither) {
        draw_dither_backdrop_panel({0.0f, 0.0f, (float)g_platform.width, (float)g_platform.height}, 1.0f);
    }
    // content clip
    cairo_save(g_renderer.cr);
    cairo_rectangle(g_renderer.cr,0,g_ctx.content_region.y,(float)g_platform.width,vh);
    cairo_clip(g_renderer.cr);
    g_drawing=true;
}

void end() {
    using namespace internal;
    if (!g_drawing) return;
    finalize_pending_collapse_measure();
    cairo_restore(g_renderer.cr); // pop content clip
    float new_ch=g_ctx.cursor_y+g_scroll_y-g_ctx.content_region.y;
    if(new_ch<0)new_ch=0;
    {
        const float kSW=14; float vh=g_ctx.content_region.h;
        bool nsb=g_content_height>vh+1;
        if(nsb){
            float ms=g_content_height-vh;
            if (effects_enabled() && g_input.wheel_y != 0.0f) {
                g_scroll_target_y -= g_input.wheel_y;
                g_input.wheel_y = 0.0f;
            } else if (!effects_enabled() && g_input.wheel_y != 0.0f) {
                g_scroll_y -= g_input.wheel_y;
                g_scroll_target_y = g_scroll_y;
                g_input.wheel_y = 0.0f;
            }
            g_scroll_target_y=g_scroll_target_y<0?0:(g_scroll_target_y>ms?ms:g_scroll_target_y);
            float tx=g_ctx.content_region.x+g_ctx.content_region.w+g_style.window_padding;
            Rect track={tx,g_ctx.content_region.y,kSW,vh};
            float th=fmaxf(20,(vh/g_content_height)*vh);
            float tt=ms>0?g_scroll_y/ms:0;
            float ty2=g_ctx.content_region.y+tt*(vh-th);
            Rect thumb={tx,ty2,kSW,th};
            bool thov=rect_contains(thumb,g_input.mouse_x,g_input.mouse_y);
            bool trhov=rect_contains(track,g_input.mouse_x,g_input.mouse_y);
            if(g_input.mouse_pressed&&thov){g_sb_dragging=true;g_sb_drag_mouse_y=g_input.mouse_y;g_sb_drag_scroll0=g_scroll_y;}
            if(g_sb_dragging){
                if(g_input.mouse_down||g_input.mouse_released){float sc=ms/fmaxf(1,vh-th);g_scroll_y=g_sb_drag_scroll0+(g_input.mouse_y-g_sb_drag_mouse_y)*sc;g_scroll_y=g_scroll_y<0?0:(g_scroll_y>ms?ms:g_scroll_y);g_scroll_target_y=g_scroll_y;}
                if(g_input.mouse_released)g_sb_dragging=false;
            }
            if(g_input.mouse_pressed&&trhov&&!thov){
                if (effects_enabled()) {
                    g_scroll_target_y+=(g_input.mouse_y<ty2?-vh:vh);
                    g_scroll_target_y=g_scroll_target_y<0?0:(g_scroll_target_y>ms?ms:g_scroll_target_y);
                } else {
                    g_scroll_y+=(g_input.mouse_y<ty2?-vh:vh);
                    g_scroll_target_y = g_scroll_y;
                    g_scroll_y=g_scroll_y<0?0:(g_scroll_y>ms?ms:g_scroll_y);
                }
            }
            fill_round_rect(track,6,g_style.input_bg);
            fill_round_rect(thumb,6,g_sb_dragging?g_style.input_focus:thov?g_style.button_hover:g_style.border);
        }
        else if (effects_enabled() && g_input.wheel_y != 0.0f) {
            g_scroll_target_y = 0;
            g_input.wheel_y = 0.0f;
        }
        g_content_height=new_ch;
    }
    if(g_debug.show_fps){
        char buf[64]; snprintf(buf,sizeof(buf),"FPS: %.0f  frame:%d  DPI:%.0f%%",g_fps,g_ctx.frame_index,g_renderer.dpi_scale*100);
        float lh=text_line_height(); Rect r={4,4,300,lh}; fill_rect(r,{0,0,0,.6f}); draw_text_utf8(buf,r,g_style.text_dim);
    }
    if(g_debug.show_hovered_id||g_debug.show_active_id){
        char buf[128]; snprintf(buf,sizeof(buf),"hot=%d active=%d focused=%d",g_ctx.hot_id,g_ctx.active_id,g_ctx.focused_widget_id);
        float lh=text_line_height(),oy=g_debug.show_fps?lh+6:4; Rect r={4,oy,360,lh}; fill_rect(r,{0,0,0,.6f}); draw_text_utf8(buf,r,g_style.text_dim);
    }
    draw_side_drawer_overlay(g_platform.width, g_platform.height);
    draw_command_overlay(g_platform.width, g_platform.height);
    draw_dropdown_overlay();
    finalize_tooltip_frame();
    if (g_modal_open_id && !g_modal_drawn) close_modal();
    draw_tooltip_overlay();
    draw_toast_overlay();
    g_drawing=false;
    swap_buffers();
    bool keep_redrawing = g_redraw_requested || g_debug.show_fps ||
                          g_input.mouse_down || g_sb_dragging ||
                          g_cmd.active || g_side_drawer.open || g_side_drawer.progress > 0.01f ||
                          g_tooltip.requested || g_tooltip.active ||
                          toasts_active() ||
                          g_dropdown_overlay.active || g_dropdown_overlay_prev.active ||
                          effects_need_redraw();
    wait_frame_limit();
    g_redraw_requested = keep_redrawing;
    g_dropdown_overlay_prev = g_dropdown_overlay;
    g_ctx.tab_stops_prev=g_ctx.tab_stops; g_ctx.frame_index++;
}

void shutdown() {
    using namespace internal;
    teardown_cairo();
    if(g_xic){XDestroyIC(g_xic);g_xic=nullptr;}
    if(g_xim){XCloseIM(g_xim);g_xim=nullptr;}
    if(g_platform.window){XDestroyWindow(g_platform.display,g_platform.window);g_platform.window=0;}
    if(g_platform.display){XCloseDisplay(g_platform.display);g_platform.display=nullptr;}
}

void open_child_window(const Config& cfg, std::function<void()> fn) {
    using namespace internal;
    if(!g_platform.display) return;
    struct Snap {
        PlatformState p; RendererState r; InputState in; UIContext ctx; Style sty; std::string sty_name; DebugState dbg;
        float sc,sct,ch,sbmy,sbms0; bool sbd;
        CmdState cmd; float fps,fpsa; int fpsf, frame_limit_fps; bool redraw_requested;
        struct timespec lt;
        int tci,tc,tsa,taci,tac,taas,dither_size; float tasc,tasct,dt,window_opacity;
        BackdropEffect backdrop_effect; WindowTransparency window_transparency;
        bool effects;
        MotionSlot motion[256];
        TabFxSlot tab_fx[32];
        int tab_owner;
        FrameContentFx frame_fx;
        float draw_off_x,draw_off_y,draw_opacity;
        ScrollSlot scroll_slots[128];
        CollapseSlot collapse_slots[64];
        ActiveCollapseFx active_collapse_fx[16];
        int active_collapse_fx_count;
        int pending_collapse_key;
        float pending_collapse_start_y;
        NextLayoutState next_layout;
        ColorOverrideEntry color_stack[kColorOverrideStackCap];
        int color_stack_count;
        ColorOverrideTable next_color_pending, active_widget_colors;
        int active_widget_color_depth;
        TooltipState tooltip;
        DropdownOverlayState dropdown_overlay, dropdown_overlay_prev;
        int disabled_depth, focus_request_id, modal_open_id, modal_request_id, dropdown_open_id;
        bool inside_modal, modal_drawn, dropdown_capture_input;
        bool shortcuts,drawing;
        XIM xim; XIC xic; std::string cb;
    } s;
    s.p=g_platform;s.r=g_renderer;s.in=g_input;s.ctx=g_ctx;s.sty=g_style;s.sty_name=g_style_name;s.dbg=g_debug;
    s.sc=g_scroll_y;s.sct=g_scroll_target_y;s.ch=g_content_height;s.sbd=g_sb_dragging;s.sbmy=g_sb_drag_mouse_y;s.sbms0=g_sb_drag_scroll0;
    s.cmd=g_cmd;s.fps=g_fps;s.fpsa=g_fps_accum;s.fpsf=g_fps_frames;s.lt=g_last_time;
    s.frame_limit_fps=g_frame_limit_fps;s.redraw_requested=g_redraw_requested;
    s.backdrop_effect=g_backdrop_effect;s.dither_size=g_dither_size;s.window_transparency=g_window_transparency;s.window_opacity=g_window_opacity;
    s.tci=g_text_cursor_id;s.tc=g_text_cursor;s.tsa=g_text_sel_anchor;
    s.taci=g_ta_cursor_id;s.tac=g_ta_cursor;s.taas=g_ta_sel_anchor;s.tasc=g_ta_scroll_y;s.tasct=g_ta_scroll_target_y;s.dt=g_dt;
    s.effects=g_effects_enabled;
    memcpy(s.motion, g_motion_slots, sizeof(g_motion_slots));
    memcpy(s.tab_fx, g_tab_fx_slots, sizeof(g_tab_fx_slots));
    s.tab_owner=g_tab_content_owner;
    s.frame_fx=g_frame_content_fx;
    s.draw_off_x=g_draw_fx_off_x;s.draw_off_y=g_draw_fx_off_y;s.draw_opacity=g_draw_fx_opacity;
    memcpy(s.scroll_slots, g_scroll_slots, sizeof(g_scroll_slots));
    memcpy(s.collapse_slots, g_collapse_slots, sizeof(g_collapse_slots));
    memcpy(s.active_collapse_fx, g_active_collapse_fx, sizeof(g_active_collapse_fx));
    s.active_collapse_fx_count = g_active_collapse_fx_count;
    s.pending_collapse_key = g_pending_collapse_key;
    s.pending_collapse_start_y = g_pending_collapse_start_y;
    s.next_layout=g_next_layout;
    memcpy(s.color_stack, g_color_stack, sizeof(g_color_stack));
    s.color_stack_count = g_color_stack_count;
    s.next_color_pending = g_next_color_pending;
    s.active_widget_colors = g_active_widget_colors;
    s.active_widget_color_depth = g_active_widget_color_depth;
    s.tooltip=g_tooltip;
    s.dropdown_overlay=g_dropdown_overlay;
    s.dropdown_overlay_prev=g_dropdown_overlay_prev;
    s.disabled_depth=g_disabled_depth;
    s.focus_request_id=g_focus_request_id;
    s.modal_open_id=g_modal_open_id;
    s.modal_request_id=g_modal_request_id;
    s.dropdown_open_id=g_dropdown_open_id;
    s.inside_modal=g_inside_modal;
    s.modal_drawn=g_modal_drawn;
    s.dropdown_capture_input=g_dropdown_capture_input;
    s.shortcuts=g_shortcuts_enabled;s.drawing=g_drawing;s.xim=g_xim;s.xic=g_xic;s.cb=g_clipboard_buf;
    Display* dpy=g_platform.display; Window parent=g_platform.window;
    g_platform={}; g_platform.display=dpy;
    g_renderer={}; g_input={}; g_ctx={}; g_style=s.sty; g_style_name=s.sty_name; g_debug=s.dbg;
    g_scroll_y=g_content_height=0; g_sb_dragging=false; g_cmd={};
    g_fps=g_fps_accum=0; g_fps_frames=0;
    g_frame_limit_fps=cfg.fps_limit; g_redraw_requested=true;
    g_backdrop_effect=cfg.backdrop_effect; g_dither_size=cfg.dither_size < 1 ? 1 : (cfg.dither_size > 32 ? 32 : cfg.dither_size);
    g_window_transparency=cfg.window_transparency; g_window_opacity=clampf(cfg.window_opacity, 0.20f, 1.0f);
    g_text_cursor_id=g_text_cursor=g_text_sel_anchor=0;
    g_ta_cursor_id=g_ta_cursor=g_ta_sel_anchor=0; g_ta_scroll_y=0;
    memset(g_scroll_slots, 0, sizeof(g_scroll_slots));
    memset(g_collapse_slots, 0, sizeof(g_collapse_slots));
    memset(g_active_collapse_fx, 0, sizeof(g_active_collapse_fx));
    g_active_collapse_fx_count = 0;
    g_pending_collapse_key = 0;
    g_pending_collapse_start_y = 0.0f;
    g_next_layout={}; g_tooltip={}; g_dropdown_overlay={}; g_dropdown_overlay_prev={}; g_disabled_depth=0; g_focus_request_id=0;
    memcpy(g_color_stack, s.color_stack, sizeof(g_color_stack));
    g_color_stack_count = s.color_stack_count;
    g_next_color_pending = s.next_color_pending;
    g_active_widget_colors = s.active_widget_colors;
    g_active_widget_color_depth = s.active_widget_color_depth;
    g_modal_open_id=0; g_modal_request_id=0; g_dropdown_open_id=0; g_inside_modal=false; g_modal_drawn=false;
    g_dropdown_capture_input=false;
    reset_effect_state(cfg.enable_effects);
    g_shortcuts_enabled=s.shortcuts; g_drawing=false; g_clipboard_buf=s.cb;
    g_xim=nullptr; g_xic=nullptr;
    int screen=DefaultScreen(dpy); int cx=0,cy=0;
    if(cfg.center_window){cx=(DisplayWidth(dpy,screen)-cfg.width)/2;cy=(DisplayHeight(dpy,screen)-cfg.height)/2;}
    g_platform.window=XCreateSimpleWindow(dpy,RootWindow(dpy,screen),cx,cy,cfg.width,cfg.height,0,BlackPixel(dpy,screen),BlackPixel(dpy,screen));
    g_platform.wm_delete=XInternAtom(dpy,"WM_DELETE_WINDOW",False);
    XSetWMProtocols(dpy,g_platform.window,&g_platform.wm_delete,1);
    XSetTransientForHint(dpy,g_platform.window,parent);
    const char* t=cfg.title?cfg.title:"FTUI"; XStoreName(dpy,g_platform.window,t);
    Atom nwn=XInternAtom(dpy,"_NET_WM_NAME",False),us=XInternAtom(dpy,"UTF8_STRING",False);
    XChangeProperty(dpy,g_platform.window,nwn,us,8,PropModeReplace,(unsigned char*)t,(int)strlen(t));
    if(!cfg.resizable){XSizeHints sh={};sh.flags=PMinSize|PMaxSize;sh.min_width=sh.max_width=cfg.width;sh.min_height=sh.max_height=cfg.height;XSetWMNormalHints(dpy,g_platform.window,&sh);}
    XSelectInput(dpy,g_platform.window,ExposureMask|ButtonPressMask|ButtonReleaseMask|PointerMotionMask|KeyPressMask|KeyReleaseMask|StructureNotifyMask|FocusChangeMask);
    g_xim=XOpenIM(dpy,nullptr,nullptr,nullptr);
    if(g_xim) g_xic=XCreateIC(g_xim,XNInputStyle,(XIMPreeditNothing|XIMStatusNothing),XNClientWindow,g_platform.window,XNFocusWindow,g_platform.window,(void*)nullptr);
    g_platform.width=cfg.width;g_platform.height=cfg.height;g_platform.running=true;
    XMapWindow(dpy,g_platform.window); XFlush(dpy);
    g_renderer.dpi_scale=s.r.dpi_scale;
    if(init_cairo()){sync_native_window_transparency();clock_gettime(CLOCK_MONOTONIC,&g_last_time);while(pump()){begin();fn();end();}}
    teardown_cairo();
    if(g_xic){XDestroyIC(g_xic);g_xic=nullptr;}
    if(g_xim){XCloseIM(g_xim);g_xim=nullptr;}
    if(g_platform.window){XDestroyWindow(dpy,g_platform.window);g_platform.window=0;} XFlush(dpy);
    g_platform=s.p;g_renderer=s.r;g_input=s.in;g_ctx=s.ctx;g_style=s.sty;g_style_name=s.sty_name;g_debug=s.dbg;
    g_scroll_y=s.sc;g_scroll_target_y=s.sct;g_content_height=s.ch;g_sb_dragging=s.sbd;g_sb_drag_mouse_y=s.sbmy;g_sb_drag_scroll0=s.sbms0;
    g_cmd=s.cmd;g_fps=s.fps;g_fps_accum=s.fpsa;g_fps_frames=s.fpsf;g_last_time=s.lt;
    g_frame_limit_fps=s.frame_limit_fps;g_redraw_requested=s.redraw_requested;
    g_backdrop_effect=s.backdrop_effect;g_dither_size=s.dither_size;g_window_transparency=s.window_transparency;g_window_opacity=s.window_opacity;
    g_text_cursor_id=s.tci;g_text_cursor=s.tc;g_text_sel_anchor=s.tsa;
    g_ta_cursor_id=s.taci;g_ta_cursor=s.tac;g_ta_sel_anchor=s.taas;g_ta_scroll_y=s.tasc;g_ta_scroll_target_y=s.tasct;g_dt=s.dt;
    g_effects_enabled=s.effects;
    memcpy(g_motion_slots, s.motion, sizeof(g_motion_slots));
    memcpy(g_tab_fx_slots, s.tab_fx, sizeof(g_tab_fx_slots));
    g_tab_content_owner=s.tab_owner;
    g_frame_content_fx=s.frame_fx;
    g_draw_fx_off_x=s.draw_off_x;g_draw_fx_off_y=s.draw_off_y;g_draw_fx_opacity=s.draw_opacity;
    memcpy(g_scroll_slots, s.scroll_slots, sizeof(g_scroll_slots));
    memcpy(g_collapse_slots, s.collapse_slots, sizeof(g_collapse_slots));
    memcpy(g_active_collapse_fx, s.active_collapse_fx, sizeof(g_active_collapse_fx));
    g_active_collapse_fx_count = s.active_collapse_fx_count;
    g_pending_collapse_key = s.pending_collapse_key;
    g_pending_collapse_start_y = s.pending_collapse_start_y;
    g_next_layout=s.next_layout;
    memcpy(g_color_stack, s.color_stack, sizeof(g_color_stack));
    g_color_stack_count = s.color_stack_count;
    g_next_color_pending = s.next_color_pending;
    g_active_widget_colors = s.active_widget_colors;
    g_active_widget_color_depth = s.active_widget_color_depth;
    g_tooltip=s.tooltip;
    g_dropdown_overlay=s.dropdown_overlay;
    g_dropdown_overlay_prev=s.dropdown_overlay_prev;
    g_disabled_depth=s.disabled_depth;
    g_focus_request_id=s.focus_request_id;
    g_modal_open_id=s.modal_open_id;
    g_modal_request_id=s.modal_request_id;
    g_dropdown_open_id=s.dropdown_open_id;
    g_inside_modal=s.inside_modal;
    g_modal_drawn=s.modal_drawn;
    g_dropdown_capture_input=s.dropdown_capture_input;
    g_shortcuts_enabled=s.shortcuts;g_drawing=s.drawing;g_xim=s.xim;g_xic=s.xic;g_clipboard_buf=s.cb;
    // trigger repaint of parent
    XExposeEvent xe={}; xe.type=Expose; xe.window=g_platform.window; xe.count=0;
    XSendEvent(dpy,g_platform.window,False,ExposureMask,(XEvent*)&xe); XFlush(dpy);
}

ImageHandle* load_image(const char* utf8_path) {
    if (!utf8_path||!utf8_path[0]) return nullptr;
    cairo_surface_t* surf=cairo_image_surface_create_from_png(utf8_path);
    if (!surf||cairo_surface_status(surf)!=CAIRO_STATUS_SUCCESS) {
        if(surf)cairo_surface_destroy(surf);
        internal::dbg("ftui: load_image failed for \"%s\"\n",utf8_path);
        return nullptr;
    }
    auto* li=new internal::LinuxImage{surf};
    auto* h=new ImageHandle(); h->_impl=li; return h;
}
namespace internal {
static bool load_mask_from_image(const char* path, MaskData& out) {
    if (!path || !path[0]) return false;
    cairo_surface_t* surf = cairo_image_surface_create_from_png(path);
    if (!surf || cairo_surface_status(surf) != CAIRO_STATUS_SUCCESS) {
        if (surf) cairo_surface_destroy(surf);
        return false;
    }
    cairo_surface_t* img = surf;
    if (cairo_image_surface_get_format(surf) != CAIRO_FORMAT_ARGB32 &&
        cairo_image_surface_get_format(surf) != CAIRO_FORMAT_RGB24) {
        int w0 = cairo_image_surface_get_width(surf);
        int h0 = cairo_image_surface_get_height(surf);
        img = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w0, h0);
        cairo_t* cr = cairo_create(img);
        cairo_set_source_surface(cr, surf, 0, 0);
        cairo_paint(cr);
        cairo_destroy(cr);
        cairo_surface_destroy(surf);
    }
    cairo_surface_flush(img);
    int w = cairo_image_surface_get_width(img);
    int h = cairo_image_surface_get_height(img);
    int stride = cairo_image_surface_get_stride(img);
    if (w <= 0 || h <= 0 || w > 2048 || h > 2048) { cairo_surface_destroy(img); return false; }
    unsigned char* data = cairo_image_surface_get_data(img);
    out.w = w; out.h = h;
    out.bits.assign((size_t)w * (size_t)h, 0);
    bool has_bright_mask = false;
    for (int y = 0; y < h && !has_bright_mask; ++y) {
        unsigned char* row = data + y * stride;
        for (int x = 0; x < w; ++x) {
            unsigned char b = row[x * 4 + 0];
            unsigned char g = row[x * 4 + 1];
            unsigned char r = row[x * 4 + 2];
            unsigned char a = row[x * 4 + 3];
            int lum = ((int)r * 54 + (int)g * 183 + (int)b * 19) / 256;
            if (a > 32 && lum > 96) { has_bright_mask = true; break; }
        }
    }
    for (int y = 0; y < h; ++y) {
        unsigned char* row = data + y * stride;
        for (int x = 0; x < w; ++x) {
            unsigned char b = row[x * 4 + 0];
            unsigned char g = row[x * 4 + 1];
            unsigned char r = row[x * 4 + 2];
            unsigned char a = row[x * 4 + 3];
            int lum = ((int)r * 54 + (int)g * 183 + (int)b * 19) / 256;
            out.bits[(size_t)y * (size_t)w + (size_t)x] = (a > 32 && (has_bright_mask ? lum > 96 : true)) ? 255 : 0;
        }
    }
    cairo_surface_destroy(img);
    return true;
}
} // namespace internal
void free_image(ImageHandle* img) {
    if (!img) return;
    if (img->_impl) { auto* li=static_cast<internal::LinuxImage*>(img->_impl); if(li->surface)cairo_surface_destroy(li->surface); delete li; img->_impl=nullptr; }
    delete img;
}

std::string open_file_dialog(const char* title, const FileFilter* filters, int filter_count) {
    std::string cmd="zenity --file-selection";
    if (title&&title[0]) { cmd+=" --title='"; for(const char*p=title;*p;p++){if(*p=='\'')cmd+="'\\''";else cmd+=*p;} cmd+="'"; }
    for (int i=0;i<filter_count;i++) {
        cmd+=" --file-filter='";
        if(filters[i].name){for(const char*p=filters[i].name;*p;p++){if(*p=='\'')cmd+="'\\''";else cmd+=*p;}}
        cmd+=" | ";
        if(filters[i].spec){std::string sp=filters[i].spec;for(char&c:sp)if(c==';')c=' ';cmd+=sp;}
        cmd+="'";
    }
    cmd+=" 2>/dev/null";
    FILE* f=popen(cmd.c_str(),"r"); if(!f) return "";
    std::string result; char buf[4096];
    while(fgets(buf,sizeof(buf),f)) result+=buf;
    pclose(f);
    while(!result.empty()&&(result.back()=='\n'||result.back()=='\r')) result.pop_back();
    return result;
}

#else
    #error "ftui.hpp: unsupported platform (Windows and Linux only)"
#endif // platform


// ============================================================
// Shared Widget Implementations
// ============================================================

using namespace internal;

void text(const char* label) {
    if (!g_drawing) return;
    WidgetColorScope color_scope;
    char vis[256]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    float h = g_style.item_height;
    Rect r = next_rect(h);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    if (cfx.clip_active) push_clip(cfx.clip);
    if (g_debug.show_layout_rects) stroke_round_rect(r, 0, 1, {1,0,0,0.4f});
    draw_text_utf8(vis, r, maybe_disabled(resolve_color(ColorRole::Text)));
    if (cfx.clip_active) pop_clip();
    mark_last_item(0, r, rect_contains(r, g_input.mouse_x, g_input.mouse_y), false);
}

void text_wrapped(const char* text) {
    if (!g_drawing) return;
    WidgetColorScope color_scope;
    std::vector<TextRange> lines;
    compute_wrapped_ranges(text ? text : "", g_ctx.content_region.w, true, lines);
    float lh = text_line_height();
    Rect r = next_rect((float)lines.size() * lh);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    if (cfx.clip_active) push_clip(cfx.clip);
    Color text_col = maybe_disabled(resolve_color(ColorRole::Text));
    float y = r.y;
    for (size_t i = 0; i < lines.size(); ++i) {
        std::string line((text ? text : "") + lines[i].start, (text ? text : "") + lines[i].end);
        Rect lr = {r.x, y, r.w, lh};
        draw_text_utf8(line.c_str(), lr, text_col);
        y += lh;
    }
    if (cfx.clip_active) pop_clip();
    mark_last_item(0, r, rect_contains(r, g_input.mouse_x, g_input.mouse_y), false);
}

void separator() {
    if (!g_drawing) return;
    WidgetColorScope color_scope;
    float h = g_style.item_spacing * 2 + 1;
    Rect r = next_rect(h);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    if (cfx.clip_active) push_clip(cfx.clip);
    float my = r.y + r.h * 0.5f;
    draw_line(r.x, my, r.x + r.w, my, 1.0f, resolve_color(ColorRole::Border));
    if (cfx.clip_active) pop_clip();
}

void spacing(float px) {
    if (!g_drawing) return;
    next_rect(px);
}

static bool button_impl(const char* label, const Color* tint_override, const ColorRole* tint_role) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[256]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    register_focusable(id);

    Rect r = next_rect(g_style.item_height);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    bool raw_hov = rect_contains(r, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;
    bool clicked = false;

    if (hov && g_input.mouse_pressed) { g_ctx.active_id = id; set_widget_focus(id, false); }
    if (g_ctx.active_id == id && g_input.mouse_released) {
        if (hov) clicked = true;
        g_ctx.active_id = 0;
    }
    if (hov) g_ctx.hot_id = id;
    bool focused = is_widget_focused(id);
    if (focused && enabled && (g_input.key_enter || g_input.key_space)) clicked = true;

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, raw_hov, enabled && g_ctx.active_id == id, focused);

    Color panel = resolve_color(ColorRole::Panel);
    Color tint = tint_override ? *tint_override : (tint_role ? resolve_color(*tint_role) : resolve_color(ColorRole::Button));
    Color button = tint_override || tint_role ? tint : resolve_color(ColorRole::Button);
    Color button_hover = tint_override || tint_role ? lerp_color(tint, resolve_color(ColorRole::ButtonHover), 0.35f)
                                       : resolve_color(ColorRole::ButtonHover);
    Color button_active = tint_override || tint_role ? lerp_color(tint, resolve_color(ColorRole::ButtonActive), 0.30f)
                                        : resolve_color(ColorRole::ButtonActive);
    Color border_base = tint_override || tint_role ? lerp_color(resolve_color(ColorRole::Border), tint, 0.10f)
                                      : resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color text = resolve_color(ColorRole::Text);

    Color bg = lerp_color(button, panel, 0.30f);
    bg = lerp_color(bg, button_hover, 0.20f + motion.hover * 0.35f);
    bg = lerp_color(bg, focus, 0.06f + motion.hover * 0.05f + motion.active * 0.14f);
    bg = lerp_color(bg, button_active, motion.active * 0.35f);
    Color border = lerp_color(border_base, focus, 0.12f + motion.hover * 0.14f + motion.active * 0.22f);
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(bg), maybe_disabled(border), motion.hover, motion.active, focused ? 1.0f : 0.0f);
    Rect text_r = offset_rect(r, 0.0f, motion.active * 1.0f);
    draw_text_utf8_centered(vis, text_r, maybe_disabled(lerp_color(text_dim, text, 0.72f + motion.hover * 0.20f + motion.active * 0.08f)));
    if (cfx.clip_active) pop_clip();

    mark_last_item(id, r, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(r, 0, 1, {0,1,0,0.4f});
    return clicked;
}

bool button(const char* label) {
    return button_impl(label, nullptr, nullptr);
}

bool button(const char* label, ColorRole role) {
    return button_impl(label, nullptr, &role);
}

bool button(const char* label, Color color) {
    return button_impl(label, &color, nullptr);
}

bool side_menu(const char* label, const char* const* items, int count, int* selected) {
    if (!g_drawing || !items || count <= 0 || !selected) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label ? label : "Navigation", vis, sizeof(vis), &hs);
    int base_id = hash_str(hs) ^ 0x5195;

    float title_h = vis[0] ? text_line_height() + 12.0f : 8.0f;
    float item_h = fmaxf(34.0f, g_style.item_height - 2.0f);
    Rect outer = next_rect(title_h + 10.0f + item_h * count + 12.0f);
    CollapseRectFx cfx = collapse_rect_fx(outer);
    outer = cfx.rect;
    bool enabled = widget_interaction_enabled();
    bool changed = false;
    int current = *selected < 0 ? 0 : (*selected >= count ? count - 1 : *selected);

    std::vector<int> ids(count);
    int focused_idx = -1;
    for (int i = 0; i < count; ++i) {
        ids[i] = (hash_str(items[i] ? items[i] : "") ^ base_id ^ (i * 977)) & 0x7fffffff;
        if (!ids[i]) ids[i] = base_id + i + 1;
        register_focusable(ids[i]);
        if (is_widget_focused(ids[i])) focused_idx = i;
    }

    if (focused_idx >= 0 && enabled) {
        int next = focused_idx;
        if (g_input.key_up) next = focused_idx > 0 ? focused_idx - 1 : 0;
        if (g_input.key_down) next = focused_idx + 1 < count ? focused_idx + 1 : count - 1;
        if (next != focused_idx) {
            set_widget_focus(ids[next], false);
            current = next;
            changed = true;
        } else if ((g_input.key_enter || g_input.key_space) && current != focused_idx) {
            current = focused_idx;
            changed = true;
        }
    }

    if (cfx.clip_active) push_clip(cfx.clip);
    Color panel = lerp_color(resolve_color(ColorRole::Panel), resolve_color(ColorRole::Background), 0.10f);
    Color border = resolve_color(ColorRole::Border);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color accent = resolve_color(ColorRole::Accent);
    draw_widget_chrome(outer, fmaxf(6.0f, g_style.rounding), maybe_disabled(panel), maybe_disabled(border), 0.0f, 0.0f, 0.0f);

    float y = outer.y + 8.0f;
    if (vis[0]) {
        Rect title_r = {outer.x + 14.0f, y, outer.w - 28.0f, title_h - 8.0f};
        draw_text_utf8(vis, title_r, maybe_disabled(text_dim));
        y += title_h;
    }

    Rect last_rect = outer;
    int last_id = base_id;
    bool last_hov = false, last_focus = false;
    for (int i = 0; i < count; ++i) {
        Rect ir = {outer.x + 8.0f, y + i * item_h, outer.w - 16.0f, item_h - 4.0f};
        bool raw_hov = rect_contains(ir, g_input.mouse_x, g_input.mouse_y);
        bool hov = raw_hov && enabled;
        int id = ids[i];
        if (hov && g_input.mouse_pressed) {
            set_widget_focus(id, false);
            g_ctx.active_id = id;
        }
        if (g_ctx.active_id == id && g_input.mouse_released) {
            if (hov && current != i) { current = i; changed = true; }
            g_ctx.active_id = 0;
        }
        if (hov) g_ctx.hot_id = id;
        bool focused = is_widget_focused(id);
        bool sel = current == i;
        MotionSlot& motion = motion_slot_for(id);
        update_motion_slot(motion, raw_hov, enabled && g_ctx.active_id == id, focused || sel);

        Color bg = {0,0,0,0};
        if (sel) {
            bg = lerp_color(resolve_color(ColorRole::ButtonActive), accent, 0.16f);
            bg.a = 0.92f;
        } else if (motion.hover > 0.01f || focused) {
            bg = lerp_color(resolve_color(ColorRole::Button), resolve_color(ColorRole::ButtonHover), motion.hover);
            bg.a = 0.78f;
        }
        if (bg.a > 0.0f) fill_round_rect(ir, fmaxf(4.0f, g_style.rounding * 0.75f), maybe_disabled(bg));
        if (sel) fill_round_rect({ir.x + 5.0f, ir.y + 7.0f, 4.0f, ir.h - 14.0f}, 2.0f, maybe_disabled(accent));

        Rect tr = {ir.x + 18.0f, ir.y, ir.w - 26.0f, ir.h};
        draw_text_utf8(items[i] ? items[i] : "", tr,
                       maybe_disabled(lerp_color(text_dim, text, (sel ? 0.72f : 0.0f) + motion.hover * 0.24f + (focused ? 0.30f : 0.0f))));
        if (focused) stroke_round_rect(ir, fmaxf(4.0f, g_style.rounding * 0.75f), 1.0f, maybe_disabled(with_alpha(accent, 0.55f)));
        if (raw_hov || focused || sel) { last_rect = ir; last_id = id; last_hov = raw_hov; last_focus = focused; }
    }
    if (cfx.clip_active) pop_clip();

    *selected = current;
    mark_last_item(last_id, last_rect, last_hov, last_focus);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0.2f,0.8f,1.0f,0.4f});
    return changed;
}

bool side_menu_drawer(const char* label, const char* const* items, int count, int* selected, float width) {
    if (!g_drawing || !items || count <= 0 || !selected) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label ? label : "Navigation", vis, sizeof(vis), &hs);
    int id = (hash_str(hs) ^ 0x6d31) & 0x7fffffff;
    if (!id) id = 0x6d31;

    SideDrawerState& drawer = g_side_drawer;
    if (drawer.owner != id) {
        drawer = {};
        drawer.owner = id;
    }

    drawer.active = true;
    drawer.title = vis[0] ? vis : "Navigation";
    drawer.count = count;
    drawer.selected = *selected < 0 ? 0 : (*selected >= count ? count - 1 : *selected);
    drawer.width = clampf(width, 180.0f, 420.0f);
    drawer.item_h = fmaxf(34.0f, g_style.item_height - 2.0f);
    drawer.labels.clear();
    drawer.labels.reserve((size_t)count);
    for (int i = 0; i < count; ++i) drawer.labels.push_back(items[i] ? items[i] : "");

    bool changed = false;
    int launcher_id = id ^ 0x5151;
    float btn = fmaxf(38.0f, fminf(44.0f, g_style.item_height + 4.0f));
    drawer.launcher_rect = {12.0f, 12.0f, btn, btn};
    bool launcher_hov = rect_contains(drawer.launcher_rect, g_input.mouse_x, g_input.mouse_y);
    if (launcher_hov && g_input.mouse_pressed) {
        g_ctx.active_id = launcher_id;
        g_side_drawer_capture_input = true;
    }
    if (g_ctx.active_id == launcher_id && g_input.mouse_released) {
        if (launcher_hov) drawer.open = !drawer.open;
        g_ctx.active_id = 0;
    }
    if (launcher_hov) {
        g_ctx.hot_id = launcher_id;
        g_side_drawer_capture_input = true;
    }
    MotionSlot& launcher_motion = motion_slot_for(launcher_id);
    update_motion_slot(launcher_motion, launcher_hov, g_ctx.active_id == launcher_id, drawer.open);
    drawer.launcher_hover = launcher_motion.hover;
    drawer.launcher_active = launcher_motion.active;

    float target = drawer.open ? 1.0f : 0.0f;
    if (effects_enabled()) drawer.progress = step_anim(drawer.progress, target, g_dt, 16.0f);
    else drawer.progress = target;
    if (fabsf(drawer.progress - target) > 0.01f) g_redraw_requested = true;

    float p = drawer.progress * drawer.progress * (3.0f - 2.0f * drawer.progress);
    float window_h = g_ctx.content_region.y + g_ctx.content_region.h + g_style.window_padding;
    float panel_w = drawer.width;
    Rect panel = {-panel_w + panel_w * p, 0.0f, panel_w, window_h};

    if (drawer.open) {
        if (g_input.key_escape) drawer.open = false;
        if (g_input.key_up) {
            int next = drawer.selected > 0 ? drawer.selected - 1 : 0;
            if (next != drawer.selected) { drawer.selected = next; changed = true; }
        }
        if (g_input.key_down) {
            int next = drawer.selected + 1 < count ? drawer.selected + 1 : count - 1;
            if (next != drawer.selected) { drawer.selected = next; changed = true; }
        }
        if (g_input.key_enter || g_input.key_space) drawer.open = false;

        if (g_input.mouse_pressed) {
            bool inside_panel = rect_contains(panel, g_input.mouse_x, g_input.mouse_y);
            if (!inside_panel && drawer.progress > 0.55f) {
                drawer.open = false;
            } else if (inside_panel && drawer.progress > 0.80f) {
                float y = 72.0f;
                for (int i = 0; i < count; ++i) {
                    Rect ir = {panel.x + 10.0f, y + i * drawer.item_h, panel.w - 20.0f, drawer.item_h - 5.0f};
                    if (rect_contains(ir, g_input.mouse_x, g_input.mouse_y)) {
                        if (drawer.selected != i) { drawer.selected = i; changed = true; }
                        drawer.open = false;
                        break;
                    }
                }
            }
        }
    }

    *selected = drawer.selected;
    if (drawer.open || drawer.progress > 0.01f) {
        g_side_drawer_capture_input = true;
        g_redraw_requested = true;
    }
    return changed;
}

bool input(const char* label, char* buffer, int buffer_size,
           InputFlags flags, bool* enter_pressed) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);

    register_focusable(id);

    Rect target_r = {}, target_outer = next_labeled_rect(vis, g_style.item_height, &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;

    // Click to focus
    if (hov && g_input.mouse_pressed) {
        set_widget_focus(id, true);
        g_text_cursor_id = id;
        int len = (int)strlen(buffer);
        // Place cursor at click position
        float inner_x = r.x + 8.0f;
        g_text_cursor = byte_from_x(buffer, g_input.mouse_x - inner_x);
        g_text_sel_anchor = g_text_cursor;
    }

    bool focused = is_widget_focused(id);
    bool changed = false;

    if (focused) {
        g_ctx.focused_input_id = id;
        if (g_text_cursor_id != id) {
            g_text_cursor_id = id;
            g_text_cursor = (int)strlen(buffer);
            g_text_sel_anchor = g_text_cursor;
        }
        int len = (int)strlen(buffer);
        if (g_text_cursor > len) g_text_cursor = len;
        if (g_text_sel_anchor > len) g_text_sel_anchor = len;

        bool ro = (flags & InputFlags::ReadOnly);

        // Ctrl+A: select all
        if (g_input.ctrl_held && g_input.text_input_count > 0 && g_input.text_input[0] == 1) {
            g_text_sel_anchor = 0; g_text_cursor = len;
        }

        // Arrow keys
        if (g_input.key_left) {
            if (g_input.shift_held) {
                g_text_cursor = utf8_retreat(buffer, g_text_cursor);
            } else if (g_text_cursor != g_text_sel_anchor) {
                g_text_cursor = g_text_sel_anchor < g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                g_text_sel_anchor = g_text_cursor;
            } else {
                g_text_cursor = utf8_retreat(buffer, g_text_cursor);
                g_text_sel_anchor = g_text_cursor;
            }
        }
        if (g_input.key_right) {
            if (g_input.shift_held) {
                g_text_cursor = utf8_advance(buffer, g_text_cursor);
            } else if (g_text_cursor != g_text_sel_anchor) {
                g_text_cursor = g_text_sel_anchor > g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                g_text_sel_anchor = g_text_cursor;
            } else {
                g_text_cursor = utf8_advance(buffer, g_text_cursor);
                g_text_sel_anchor = g_text_cursor;
            }
        }

        // Ctrl+C
        if (g_input.key_ctrl_c && g_text_sel_anchor != g_text_cursor) {
            int lo = g_text_sel_anchor < g_text_cursor ? g_text_sel_anchor : g_text_cursor;
            int hi = g_text_sel_anchor > g_text_cursor ? g_text_sel_anchor : g_text_cursor;
            std::string sel(buffer + lo, buffer + hi);
            clipboard_set(sel.c_str());
        }

        // Ctrl+V
        if (g_input.key_ctrl_v && !ro) {
            std::string cb = clipboard_get();
            if (!cb.empty()) {
                int lo = g_text_sel_anchor < g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                int hi = g_text_sel_anchor > g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                if (lo != hi) { memmove(buffer+lo, buffer+hi, len-hi+1); len -= (hi-lo); g_text_cursor = lo; }
                int ins = (int)cb.size();
                if (len + ins < buffer_size - 1) {
                    memmove(buffer+g_text_cursor+ins, buffer+g_text_cursor, len-g_text_cursor+1);
                    memcpy(buffer+g_text_cursor, cb.c_str(), ins);
                    g_text_cursor += ins;
                }
                g_text_sel_anchor = g_text_cursor;
                changed = true;
            }
        }

        // Backspace
        if (g_input.key_backspace && !ro) {
            if (g_text_cursor != g_text_sel_anchor) {
                int lo = g_text_sel_anchor < g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                int hi = g_text_sel_anchor > g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                memmove(buffer+lo, buffer+hi, len-hi+1);
                g_text_cursor = g_text_sel_anchor = lo;
                changed = true;
            } else if (g_text_cursor > 0) {
                int prev = utf8_retreat(buffer, g_text_cursor);
                memmove(buffer+prev, buffer+g_text_cursor, len-g_text_cursor+1);
                g_text_cursor = g_text_sel_anchor = prev;
                changed = true;
            }
        }

        // Printable input
        if (!ro && g_input.text_input_count > 0) {
            // Delete selection first
            if (g_text_cursor != g_text_sel_anchor) {
                int lo = g_text_sel_anchor < g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                int hi = g_text_sel_anchor > g_text_cursor ? g_text_sel_anchor : g_text_cursor;
                memmove(buffer+lo, buffer+hi, len-hi+1);
                len -= (hi-lo); g_text_cursor = g_text_sel_anchor = lo;
            }
            for (int i = 0; i < g_input.text_input_count && g_input.text_input[i]; ) {
                unsigned char c = (unsigned char)g_input.text_input[i];
                int cs = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
                if ((flags & InputFlags::CharsDecimal) || (flags & InputFlags::CharsHexadecimal) ||
                    (flags & InputFlags::CharsUppercase) || (flags & InputFlags::CharsNoBlank)) {
                    if (cs == 1) {
                        char outc = (char)c;
                        if (!filter_ascii_input_char((char)c, flags, outc)) { i += cs; continue; }
                        if (len + 1 < buffer_size - 1) {
                            memmove(buffer+g_text_cursor+1, buffer+g_text_cursor, len-g_text_cursor+1);
                            buffer[g_text_cursor] = outc;
                            g_text_cursor += 1; len += 1;
                        }
                        i += cs;
                        continue;
                    }
                    if ((flags & InputFlags::CharsDecimal) || (flags & InputFlags::CharsHexadecimal)) { i += cs; continue; }
                }
                if (len + cs < buffer_size - 1) {
                    memmove(buffer+g_text_cursor+cs, buffer+g_text_cursor, len-g_text_cursor+1);
                    memcpy(buffer+g_text_cursor, g_input.text_input+i, cs);
                    g_text_cursor += cs; len += cs;
                }
                i += cs;
            }
            g_text_sel_anchor = g_text_cursor;
            changed = true;
        }

        // Enter
        if (g_input.key_enter) {
            if (enter_pressed) *enter_pressed = true;
        }
    }

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, raw_hov, enabled && hov && g_input.mouse_down, focused);

    // Draw
    Color input_bg = resolve_color(ColorRole::InputBg);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text = resolve_color(ColorRole::Text);
    Color panel_col = lerp_color(input_bg, panel, motion.hover * 0.18f + motion.focus * 0.08f);
    Color border_col = lerp_color(border, focus, motion.focus);
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(panel_col), maybe_disabled(border_col), motion.hover, motion.active, motion.focus);

    float pad = 8.0f;
    float inner_w = r.w - pad * 2.0f - 4.0f;
    if (inner_w < 4.0f) inner_w = 4.0f;
    Rect inner = {r.x + pad, r.y, inner_w, r.h};

    // Build display string
    int len = (int)strlen(buffer);
    std::string disp;
    if (flags & InputFlags::Password) disp = std::string(utf8_char_count(buffer, len), '*');
    else disp = buffer;

    // Scroll so cursor is visible
    static float s_scroll[256] = {}; // per-hash scroll offset (limited slots)
    int slot = (id ^ (id >> 8)) & 0xFF;
    float& scroll_x = s_scroll[slot];
    if (focused) {
        float cx = measure_text_at(disp.c_str(), g_text_cursor);
        float vis_w = inner.w;
        if (cx - scroll_x > vis_w - 4) scroll_x = cx - vis_w + 4;
        if (cx - scroll_x < 0)         scroll_x = cx;
        if (scroll_x < 0) scroll_x = 0;
    }

    push_clip(inner);

    // Selection highlight
    if (focused && g_text_sel_anchor != g_text_cursor) {
        int lo = g_text_sel_anchor < g_text_cursor ? g_text_sel_anchor : g_text_cursor;
        int hi = g_text_sel_anchor > g_text_cursor ? g_text_sel_anchor : g_text_cursor;
        float sx = inner.x - scroll_x + measure_text_at(disp.c_str(), lo);
        float ex = inner.x - scroll_x + measure_text_at(disp.c_str(), hi);
        Rect sel_r = {sx, r.y + 4, ex - sx, r.h - 8};
        Color sel_col = focus; sel_col.a = 0.35f;
        fill_rect(sel_r, sel_col);
    }

    Rect text_r = {inner.x - scroll_x, inner.y, inner.w + scroll_x, inner.h};
    draw_text_utf8(disp.c_str(), text_r, maybe_disabled(text));

    // Cursor blink
    if (focused && (g_ctx.frame_index / 30) % 2 == 0) {
        float cx = inner.x - scroll_x + measure_text_at(disp.c_str(), g_text_cursor);
        draw_line(cx, r.y + 5, cx, r.y + r.h - 5, 1.5f, maybe_disabled(text));
    }

    pop_clip();

    draw_widget_label(vis, outer, focused);
    if (cfx.clip_active) pop_clip();
    mark_last_item(id, outer, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0,0,1,0.4f});
    return changed;
}

bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows, TextAreaFlags flags) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    bool ro = (flags & TextAreaFlags::ReadOnly);
    bool wrap = (flags & TextAreaFlags::WordWrap);

    register_focusable(id);
    ScrollSlot& tslot = scroll_slot_for(id ^ 0x54A54A);
    float& ta_scroll_y = tslot.current;
    float& ta_scroll_target_y = tslot.target;

    float lh = text_line_height();
    float widget_h = rows * lh + 8.0f;
    Rect target_r = {}, target_outer = next_labeled_rect(vis, widget_h, &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;

    if (hov && g_input.mouse_pressed) {
        set_widget_focus(id, true);
        g_ta_cursor_id = id;
    }

    bool focused = is_widget_focused(id);
    bool changed = false;

    float pad_x = 6.0f, pad_y = 4.0f;
    float text_w = r.w - pad_x * 2.0f - 8.0f;

    auto rebuild_lines = [&](std::vector<TextRange>& lines) {
        compute_wrapped_ranges(buffer, text_w, wrap, lines);
        if (lines.empty()) lines.push_back({0, 0});
    };
    auto line_index_for = [&](const std::vector<TextRange>& lines, int pos) {
        if (lines.empty()) return 0;
        for (size_t i = 0; i < lines.size(); ++i) {
            if (pos >= lines[i].start && pos <= lines[i].end) return (int)i;
        }
        return (int)lines.size() - 1;
    };

    std::vector<TextRange> lines;
    rebuild_lines(lines);

    if (focused) {
        g_ctx.focused_input_id = id;
        int len = (int)strlen(buffer);
        if (g_ta_cursor_id != id) {
            g_ta_cursor_id = id;
            g_ta_cursor = len;
            g_ta_sel_anchor = g_ta_cursor;
            ta_scroll_y = 0;
            ta_scroll_target_y = 0;
        }
        if (g_ta_cursor > len) g_ta_cursor = len;
        if (g_ta_sel_anchor > len) g_ta_sel_anchor = len;

        if (g_input.ctrl_held && g_input.text_input_count > 0 && g_input.text_input[0] == 1) {
            g_ta_sel_anchor = 0;
            g_ta_cursor = len;
        }

        if (g_input.key_ctrl_c && g_ta_sel_anchor != g_ta_cursor) {
            int lo = g_ta_sel_anchor < g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
            int hi = g_ta_sel_anchor > g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
            std::string sel(buffer + lo, buffer + hi);
            clipboard_set(sel.c_str());
        }

        auto delete_selection = [&]() {
            if (g_ta_cursor == g_ta_sel_anchor) return;
            int lo = g_ta_sel_anchor < g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
            int hi = g_ta_sel_anchor > g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
            memmove(buffer + lo, buffer + hi, len - hi + 1);
            len -= (hi - lo);
            g_ta_cursor = g_ta_sel_anchor = lo;
            changed = true;
        };

        if (!ro && g_input.key_ctrl_v) {
            std::string cb = clipboard_get();
            if (!cb.empty()) {
                delete_selection();
                int ins = (int)cb.size();
                if (len + ins < buffer_size - 1) {
                    memmove(buffer + g_ta_cursor + ins, buffer + g_ta_cursor, len - g_ta_cursor + 1);
                    memcpy(buffer + g_ta_cursor, cb.c_str(), ins);
                    g_ta_cursor += ins;
                    len += ins;
                    g_ta_sel_anchor = g_ta_cursor;
                    changed = true;
                }
            }
        }

        if (!ro && g_input.key_backspace) {
            if (g_ta_cursor != g_ta_sel_anchor) {
                delete_selection();
            } else if (g_ta_cursor > 0) {
                int prev = utf8_retreat(buffer, g_ta_cursor);
                int removed = g_ta_cursor - prev;
                memmove(buffer + prev, buffer + g_ta_cursor, len - g_ta_cursor + 1);
                g_ta_cursor = g_ta_sel_anchor = prev;
                len -= removed;
                changed = true;
            }
        }

        if (!ro && g_input.key_enter) {
            delete_selection();
            if (len + 1 < buffer_size - 1) {
                memmove(buffer + g_ta_cursor + 1, buffer + g_ta_cursor, len - g_ta_cursor + 1);
                buffer[g_ta_cursor] = '\n';
                g_ta_cursor++;
                g_ta_sel_anchor = g_ta_cursor;
                len += 1;
                changed = true;
            }
        }

        if (g_input.key_left) {
            if (g_input.shift_held) {
                g_ta_cursor = utf8_retreat(buffer, g_ta_cursor);
            } else if (g_ta_cursor != g_ta_sel_anchor) {
                g_ta_cursor = g_ta_sel_anchor < g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
                g_ta_sel_anchor = g_ta_cursor;
            } else {
                g_ta_cursor = utf8_retreat(buffer, g_ta_cursor);
                g_ta_sel_anchor = g_ta_cursor;
            }
        }
        if (g_input.key_right) {
            if (g_input.shift_held) {
                g_ta_cursor = utf8_advance(buffer, g_ta_cursor);
            } else if (g_ta_cursor != g_ta_sel_anchor) {
                g_ta_cursor = g_ta_sel_anchor > g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
                g_ta_sel_anchor = g_ta_cursor;
            } else {
                g_ta_cursor = utf8_advance(buffer, g_ta_cursor);
                g_ta_sel_anchor = g_ta_cursor;
            }
        }

        rebuild_lines(lines);

        if (g_input.key_up || g_input.key_down) {
            int cur_line = line_index_for(lines, g_ta_cursor);
            int next_line = g_input.key_up ? cur_line - 1 : cur_line + 1;
            if (next_line >= 0 && next_line < (int)lines.size()) {
                TextRange cur = lines[cur_line];
                TextRange dst = lines[next_line];
                float col_x = measure_text_at(buffer + cur.start, g_ta_cursor - cur.start);
                int new_pos = dst.start + byte_from_x(buffer + dst.start, col_x);
                if (new_pos > dst.end) new_pos = dst.end;
                g_ta_cursor = new_pos;
                if (!g_input.shift_held) g_ta_sel_anchor = g_ta_cursor;
            }
        }

        if (hov && g_input.mouse_pressed) {
            int line_idx = (int)((g_input.mouse_y - (r.y + pad_y) + ta_scroll_y) / lh);
            if (line_idx < 0) line_idx = 0;
            if (line_idx >= (int)lines.size()) line_idx = (int)lines.size() - 1;
            TextRange tr = lines.empty() ? TextRange{} : lines[line_idx];
            int clicked = tr.start + byte_from_x(buffer + tr.start, g_input.mouse_x - (r.x + pad_x));
            if (clicked > tr.end) clicked = tr.end;
            g_ta_cursor = clicked;
            g_ta_sel_anchor = clicked;
        }

        if (!ro && g_input.text_input_count > 0) {
            delete_selection();
            for (int i = 0; i < g_input.text_input_count && g_input.text_input[i]; ) {
                unsigned char c = (unsigned char)g_input.text_input[i];
                int cs = c < 0x80 ? 1 : c < 0xE0 ? 2 : c < 0xF0 ? 3 : 4;
                if (len + cs < buffer_size - 1) {
                    memmove(buffer + g_ta_cursor + cs, buffer + g_ta_cursor, len - g_ta_cursor + 1);
                    memcpy(buffer + g_ta_cursor, g_input.text_input + i, cs);
                    g_ta_cursor += cs;
                    len += cs;
                    changed = true;
                }
                i += cs;
            }
            g_ta_sel_anchor = g_ta_cursor;
        }

        rebuild_lines(lines);
        float visible_h = r.h - 8.0f;
        if (visible_h < lh) visible_h = lh;
        int cur_line = line_index_for(lines, g_ta_cursor);
        float cursor_y_in_widget = (float)cur_line * lh;
        if (cursor_y_in_widget - ta_scroll_target_y < 0.0f) ta_scroll_target_y = cursor_y_in_widget;
        if (cursor_y_in_widget + lh - ta_scroll_target_y > visible_h) ta_scroll_target_y = cursor_y_in_widget + lh - visible_h;
        if (ta_scroll_target_y < 0.0f) ta_scroll_target_y = 0.0f;
    }

    if (hov && g_input.wheel_y != 0.0f) {
        ta_scroll_target_y -= g_input.wheel_y;
        g_input.wheel_y = 0.0f;
    }

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, raw_hov, enabled && hov && g_input.mouse_down, focused);
    Color input_bg = resolve_color(ColorRole::InputBg);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color border_col = lerp_color(border, focus, motion.focus);
    Color panel_col = lerp_color(input_bg, panel, motion.hover * 0.18f + motion.focus * 0.08f);
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(panel_col), maybe_disabled(border_col), motion.hover, motion.active, motion.focus);

    Rect clip_r = {r.x + 2, r.y + 2, r.w - 4, r.h - 4};
    if (clip_r.w < 0.0f) clip_r.w = 0.0f;
    if (clip_r.h < 0.0f) clip_r.h = 0.0f;
    push_clip(clip_r);

    rebuild_lines(lines);
    float total_h = (float)lines.size() * lh;
    float ta_visible_h = r.h - 8.0f;
    if (ta_visible_h < lh) ta_visible_h = lh;
    float ta_max_scroll = total_h > ta_visible_h ? (total_h - ta_visible_h) : 0.0f;
    if ((flags & TextAreaFlags::AutoScrollBottom) && !focused) ta_scroll_target_y = ta_max_scroll;
    ta_scroll_target_y = clampf(ta_scroll_target_y, 0.0f, ta_max_scroll);
    if (effects_enabled()) ta_scroll_y = step_anim(ta_scroll_y, ta_scroll_target_y, g_dt, 18.0f);
    else ta_scroll_y = ta_scroll_target_y;
    ta_scroll_y = clampf(ta_scroll_y, 0.0f, ta_max_scroll);

    int sel_lo = g_ta_sel_anchor < g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
    int sel_hi = g_ta_sel_anchor > g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;

    for (size_t li = 0; li < lines.size(); ++li) {
        TextRange tr = lines[li];
        float line_y = r.y + pad_y - ta_scroll_y + (float)li * lh;
        if (line_y + lh < r.y || line_y > r.y + r.h) continue;

        if (focused && sel_lo != sel_hi) {
            int line_sel_lo = sel_lo < tr.start ? tr.start : sel_lo;
            int line_sel_hi = sel_hi > tr.end ? tr.end : sel_hi;
            if (line_sel_lo < line_sel_hi) {
                float sx = r.x + pad_x + measure_text_at(buffer + tr.start, line_sel_lo - tr.start);
                float ex = r.x + pad_x + measure_text_at(buffer + tr.start, line_sel_hi - tr.start);
                Rect sel_r = {sx, line_y, ex - sx, lh};
                Color sc = focus; sc.a = 0.35f;
                fill_rect(sel_r, sc);
            }
        }

        if (tr.end > tr.start) {
            std::string line_str(buffer + tr.start, buffer + tr.end);
            Rect lr = {r.x + pad_x, line_y, text_w, lh};
            draw_text_utf8(line_str.c_str(), lr, maybe_disabled(text));
        }

        if (focused && g_ta_cursor >= tr.start && g_ta_cursor <= tr.end && (g_ctx.frame_index / 30) % 2 == 0) {
            float cx = r.x + pad_x + measure_text_at(buffer + tr.start, g_ta_cursor - tr.start);
            draw_line(cx, line_y + 2, cx, line_y + lh - 2, 1.5f, maybe_disabled(text));
        }
    }

    pop_clip();

    if (total_h > r.h - 8.0f) {
        float sb_w = 6.0f, sb_x = r.x + r.w - sb_w - 2.0f;
        float visible_h = r.h - 8.0f;
        float thumb_h = (visible_h / total_h) * visible_h;
        if (thumb_h < 12.0f) thumb_h = 12.0f;
        float thumb_y = r.y + 4.0f + (ta_scroll_y / (total_h - visible_h)) * (visible_h - thumb_h);
        Rect track_r = {sb_x, r.y + 4.0f, sb_w, visible_h};
        Rect thumb_r = {sb_x, thumb_y, sb_w, thumb_h};
        Color track_c = border; track_c.a = 0.3f;
        fill_round_rect(track_r, sb_w * 0.5f, maybe_disabled(track_c));
        fill_round_rect(thumb_r, sb_w * 0.5f, maybe_disabled(text_dim));
    }

    draw_widget_label(vis, outer, focused);
    if (cfx.clip_active) pop_clip();

    mark_last_item(id, outer, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0,0,1,0.4f});
    return changed;
}

bool text_area(const char* label, char* buffer, int buffer_size, int rows) {
    return text_area_ex(label, buffer, buffer_size, rows, TextAreaFlags::Default);
}

void log_view(const char* label, const char* text, int rows, LogViewFlags flags) {
    if (!g_drawing) return;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    register_focusable(id);
    ScrollSlot& slot = scroll_slot_for(id ^ 0x6F6A5601);

    float lh = text_line_height();
    float widget_h = rows * lh + 8.0f;
    Rect target_r = {}, target_outer = next_labeled_rect(vis, widget_h, &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;
    bool focused = is_widget_focused(id);

    const char* src = text ? text : "";
    int len = (int)strlen(src);

    if (hov && g_input.mouse_pressed) {
        set_widget_focus(id, false);
        focused = true;
        g_ta_cursor_id = id;
    }
    if (focused && g_ta_cursor_id != id) {
        g_ta_cursor_id = id;
        g_ta_cursor = len;
        g_ta_sel_anchor = len;
    }

    float pad_x = 6.0f, pad_y = 4.0f;
    float text_w = r.w - pad_x * 2.0f - 8.0f;
    std::vector<TextRange> lines;
    compute_wrapped_ranges(src, text_w, (flags & LogViewFlags::WordWrap), lines);

    float visible_h = r.h - 8.0f;
    if (visible_h < lh) visible_h = lh;
    float total_h = (float)lines.size() * lh;
    float max_scroll = total_h > visible_h ? (total_h - visible_h) : 0.0f;
    if ((flags & LogViewFlags::AutoScrollBottom) && !focused) slot.target = max_scroll;
    if (effects_enabled()) slot.current = step_anim(slot.current, slot.target, g_dt, 18.0f);
    else slot.current = slot.target;
    slot.current = clampf(slot.current, 0.0f, max_scroll);
    slot.target = clampf(slot.target, 0.0f, max_scroll);

    if (hov && g_input.wheel_y != 0.0f) {
        slot.target -= g_input.wheel_y;
        slot.target = clampf(slot.target, 0.0f, max_scroll);
        g_input.wheel_y = 0.0f;
    }

    if (focused) {
        if (g_input.ctrl_held && g_input.text_input_count > 0 && g_input.text_input[0] == 1) {
            g_ta_sel_anchor = 0;
            g_ta_cursor = len;
        }
        if (g_input.key_ctrl_c && g_ta_sel_anchor != g_ta_cursor) {
            int lo = g_ta_sel_anchor < g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
            int hi = g_ta_sel_anchor > g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
            std::string sel(src + lo, src + hi);
            clipboard_set(sel.c_str());
        }
        if (hov && g_input.mouse_pressed) {
            int line_idx = (int)((g_input.mouse_y - (r.y + pad_y) + slot.current) / lh);
            line_idx = line_idx < 0 ? 0 : (line_idx >= (int)lines.size() ? (int)lines.size() - 1 : line_idx);
            TextRange tr = lines.empty() ? TextRange{} : lines[line_idx];
            int clicked = tr.start + byte_from_x(src + tr.start, g_input.mouse_x - (r.x + pad_x));
            if (clicked > tr.end) clicked = tr.end;
            g_ta_cursor = clicked;
            g_ta_sel_anchor = clicked;
        }
    }

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, raw_hov, false, focused);
    Color input_bg = resolve_color(ColorRole::InputBg);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text_col = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color border_col = lerp_color(border, focus, focused ? 0.7f : 0.0f);
    Color panel_col = lerp_color(input_bg, panel, motion.hover * 0.18f + (focused ? 0.08f : 0.0f));
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(panel_col), maybe_disabled(border_col), motion.hover, 0.0f, focused ? 1.0f : 0.0f);

    Rect clip_r = {r.x + 2, r.y + 2, r.w - 4, r.h - 4};
    if (clip_r.w < 0.0f) clip_r.w = 0.0f;
    if (clip_r.h < 0.0f) clip_r.h = 0.0f;
    push_clip(clip_r);

    int sel_lo = g_ta_sel_anchor < g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;
    int sel_hi = g_ta_sel_anchor > g_ta_cursor ? g_ta_sel_anchor : g_ta_cursor;

    for (size_t i = 0; i < lines.size(); ++i) {
        float line_y = r.y + pad_y - slot.current + (float)i * lh;
        if (line_y + lh < r.y || line_y > r.y + r.h) continue;

        TextRange tr = lines[i];
        if (focused && sel_lo != sel_hi) {
            int line_sel_lo = sel_lo < tr.start ? tr.start : sel_lo;
            int line_sel_hi = sel_hi > tr.end ? tr.end : sel_hi;
            if (line_sel_lo < line_sel_hi) {
                float sx = r.x + pad_x + measure_text_at(src + tr.start, line_sel_lo - tr.start);
                float ex = r.x + pad_x + measure_text_at(src + tr.start, line_sel_hi - tr.start);
                Rect sel_r = {sx, line_y, ex - sx, lh};
                Color sc = focus; sc.a = 0.30f;
                fill_rect(sel_r, sc);
            }
        }

        std::string line(src + tr.start, src + tr.end);
        Rect lr = {r.x + pad_x, line_y, text_w, lh};
        draw_text_utf8(line.c_str(), lr, maybe_disabled(text_col));
    }
    pop_clip();

    if (total_h > visible_h) {
        float sb_w = 6.0f, sb_x = r.x + r.w - sb_w - 2;
        float thumb_h = (visible_h / total_h) * visible_h;
        if (thumb_h < 12) thumb_h = 12;
        float thumb_y = r.y + 4 + (slot.current / (total_h - visible_h)) * (visible_h - thumb_h);
        Rect track_r = {sb_x, r.y + 4, sb_w, visible_h};
        Rect thumb_r = {sb_x, thumb_y, sb_w, thumb_h};
        Color track_c = border; track_c.a = 0.3f;
        fill_round_rect(track_r, sb_w * 0.5f, maybe_disabled(track_c));
        fill_round_rect(thumb_r, sb_w * 0.5f, maybe_disabled(text_dim));
    }

    draw_widget_label(vis, outer, focused);
    if (cfx.clip_active) pop_clip();

    mark_last_item(id, outer, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0.3f,0.7f,1,0.4f});
}

bool checkbox(const char* label, bool* value) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    register_focusable(id);

    Rect r = next_rect(g_style.item_height);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    bool raw_hov = rect_contains(r, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;
    bool focused = is_widget_focused(id);
    bool clicked = false;

    if (hov && g_input.mouse_pressed) {
        set_widget_focus(id, false);
        g_ctx.active_id = id;
    }
    if (focused && enabled && (g_input.key_space || g_input.key_enter)) {
        *value = !*value;
        clicked = true;
    }
    if (g_ctx.active_id == id && g_input.mouse_released) {
        if (hov) {
            *value = !*value;
            clicked = true;
            set_widget_focus(id, false);
        }
        g_ctx.active_id = 0;
    }

    float box_sz = g_style.item_height - 10.0f;
    Rect box = {r.x, r.y + (r.h - box_sz) * 0.5f, box_sz, box_sz};
    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, hov, enabled && g_ctx.active_id == id, focused || *value);
    Color input_bg = resolve_color(ColorRole::InputBg);
    Color panel = resolve_color(ColorRole::Panel);
    Color button_hover = resolve_color(ColorRole::ButtonHover);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color border_base = resolve_color(ColorRole::Border);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color bg = lerp_color(input_bg, panel, 0.14f);
    bg = lerp_color(bg, button_hover, motion.hover * 0.28f);
    bg = lerp_color(bg, focus, *value ? 0.14f : 0.04f);
    Color border = lerp_color(border_base, focus, focused ? 0.85f : (*value ? 0.35f : 0.0f));
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(box, fmaxf(2.0f, g_style.rounding * 0.35f),
                       maybe_disabled(bg), maybe_disabled(border),
                       motion.hover, motion.active, focused ? 1.0f : 0.0f);

    if (*value) {
        float inset = 5.0f;
        Rect mark = {box.x + inset, box.y + inset, box.w - inset * 2.0f, box.h - inset * 2.0f};
        Color mark_fill = focus;
        mark_fill.a = 0.82f;
        fill_rect(mark, maybe_disabled(mark_fill));
    }

    Rect lbl_r = {box.x + box_sz + 8.0f, r.y, r.w - box_sz - 8.0f, r.h};
    draw_text_utf8(vis, lbl_r, maybe_disabled(lerp_color(text_dim, text, 0.52f + motion.hover * 0.42f)));
    if (cfx.clip_active) pop_clip();

    mark_last_item(id, r, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(r, 0, 1, {1,0,1,0.4f});
    return clicked;
}

bool slider_float(const char* label, float* value, float min_v, float max_v) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    register_focusable(id);

    if (max_v < min_v) { float tmp = min_v; min_v = max_v; max_v = tmp; }
    Rect target_r = {}, target_outer = next_labeled_rect(vis, g_style.item_height, &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    float value_w = 80.0f;
    Rect track_r = {r.x, r.y + r.h * 0.5f - 3.0f, r.w - value_w, 6.0f};
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;
    bool focused = is_widget_focused(id);
    bool changed = false;

    if (hov && g_input.mouse_pressed) {
        set_widget_focus(id, false);
        g_ctx.active_id = id;
    }
    if (g_ctx.active_id == id) {
        if (g_input.mouse_released) g_ctx.active_id = 0;
        else if (enabled) {
            float t = (g_input.mouse_x - track_r.x) / track_r.w;
            t = clampf(t, 0.0f, 1.0f);
            float nv = min_v + t * (max_v - min_v);
            if (nv != *value) { *value = nv; changed = true; }
        }
    }

    if (focused && enabled && max_v > min_v) {
        float step = (max_v - min_v) / 100.0f;
        if (step <= 0.0f) step = (max_v - min_v) / 20.0f;
        if (step <= 0.0f) step = 1.0f;
        if (g_input.key_left) {
            float nv = *value - step;
            if (nv < min_v) nv = min_v;
            if (nv != *value) { *value = nv; changed = true; }
        }
        if (g_input.key_right) {
            float nv = *value + step;
            if (nv > max_v) nv = max_v;
            if (nv != *value) { *value = nv; changed = true; }
        }
    }

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, hov, enabled && g_ctx.active_id == id, focused);

    Color input_bg = resolve_color(ColorRole::InputBg);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color button = resolve_color(ColorRole::Button);
    Color button_hover = resolve_color(ColorRole::ButtonHover);
    Color button_active = resolve_color(ColorRole::ButtonActive);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color track_bg = lerp_color(input_bg, panel, motion.hover * 0.18f);
    if (cfx.clip_active) push_clip(cfx.clip);
    fill_round_rect(track_r, 3.0f, maybe_disabled(track_bg));
    stroke_round_rect(track_r, 3.0f, g_style.border_width,
                      maybe_disabled(lerp_color(border, focus, focused ? 0.65f : motion.active * 0.25f)));

    float t = max_v > min_v ? ((*value - min_v) / (max_v - min_v)) : 0.0f;
    t = clampf(t, 0.0f, 1.0f);
    Rect fill_r = {track_r.x, track_r.y, track_r.w * t, track_r.h};
    if (fill_r.w > 0.0f) fill_round_rect(fill_r, 3.0f, maybe_disabled(with_alpha(focus, 0.78f + motion.active * 0.18f)));

    float knob_sz = 14.0f;
    Rect knob = {track_r.x + track_r.w * t - knob_sz * 0.5f, r.y + r.h * 0.5f - knob_sz * 0.5f, knob_sz, knob_sz};
    Color knob_col = lerp_color(button, button_hover, motion.hover);
    knob_col = lerp_color(knob_col, button_active, motion.active);
    draw_widget_chrome(knob, knob_sz * 0.5f, maybe_disabled(knob_col), maybe_disabled(focus),
                       motion.hover, motion.active, focused ? 1.0f : 0.0f);

    char val_str[48];
    snprintf(val_str, sizeof(val_str), "%.2f", *value);
    Rect val_r = {r.x + r.w - value_w + 8.0f, r.y, value_w - 8.0f, r.h};
    draw_text_utf8(val_str, val_r, maybe_disabled(lerp_color(text_dim, text, motion.hover * 0.30f + (focused ? 0.35f : 0.0f))));
    draw_widget_label(vis, outer, focused);
    if (cfx.clip_active) pop_clip();

    mark_last_item(id, outer, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {1,1,0,0.4f});
    return changed;
}

static void draw_masked_progress(Rect r, float progress, const ProgressStyle& style, Color fill) {
    MaskData* mask = style.mask_svg && style.mask_svg[0] ? mask_for_svg(style.mask_svg) :
                     style.mask_shape && style.mask_shape[0] ? mask_for_shape(style.mask_shape) :
                     mask_for_path(style.mask_path);
    if (!mask || mask->w <= 0 || mask->h <= 0) return;

    Color base = resolve_color(ColorRole::InputBg);
    Color border = resolve_color(ColorRole::Border);
    draw_widget_chrome(r, fmaxf(3.0f, g_style.rounding * 0.55f), maybe_disabled(base), maybe_disabled(border), 0.0f, 0.0f, 0.0f);
    Rect inner = {r.x + 3.0f, r.y + 3.0f, r.w - 6.0f, r.h - 6.0f};
    if (inner.w <= 0.0f || inner.h <= 0.0f) return;

    Color ghost = fill;
    ghost.a = 0.16f;
    Color filled = fill;
    filled.a = 0.92f;
    float p = clamp01(progress);
    if (style.wave_front || style.glint) g_redraw_requested = true;
    float fill_px = p * inner.w;
    float phase = (float)g_ctx.frame_index * 0.085f;
    float wave_amp_px = 0.0f;
    if (style.wave_front && p > 0.02f && p < 0.995f) {
        wave_amp_px = fminf(7.0f, fmaxf(2.0f, inner.h * 0.18f));
    }

    int out_w = (int)ceilf(inner.w);
    int out_h = (int)ceilf(inner.h);
    if (out_w < 1) out_w = 1;
    if (out_h < 1) out_h = 1;
    auto mask_on = [&](int ox, int oy) -> bool {
        int mx = (int)(((float)ox + 0.5f) * (float)mask->w / (float)out_w);
        int my = (int)(((float)oy + 0.5f) * (float)mask->h / (float)out_h);
        if (mx < 0) mx = 0; else if (mx >= mask->w) mx = mask->w - 1;
        if (my < 0) my = 0; else if (my >= mask->h) my = mask->h - 1;
        return mask->bits[(size_t)my * (size_t)mask->w + (size_t)mx] > 0;
    };
    auto row_front = [&](int oy) -> float {
        float front = fill_px;
        if (wave_amp_px > 0.0f) front += sinf((float)oy * 0.34f + phase) * wave_amp_px;
        return clampf(front, 0.0f, inner.w);
    };

    for (int oy = 0; oy < out_h; ++oy) {
        float sy0 = inner.y + inner.h * ((float)oy / (float)out_h);
        float sy1 = inner.y + inner.h * ((float)(oy + 1) / (float)out_h);
        float front = row_front(oy);
        int run_start = -1;
        bool run_fill = false;
        for (int ox = 0; ox <= out_w; ++ox) {
            bool on = ox < out_w && mask_on(ox, oy);
            float px = inner.w * ((float)ox / (float)out_w);
            bool fill_on = on && px <= front;
            if (on && run_start < 0) { run_start = ox; run_fill = fill_on; }
            bool boundary = !on || fill_on != run_fill;
            if (run_start >= 0 && boundary) {
                float sx0 = inner.x + inner.w * ((float)run_start / (float)out_w);
                float sx1 = inner.x + inner.w * ((float)ox / (float)out_w);
                fill_rect({sx0, sy0, sx1 - sx0, sy1 - sy0}, maybe_disabled(run_fill ? filled : ghost));
                run_start = on ? ox : -1;
                run_fill = fill_on;
            }
        }
    }

    if (style.glint && p > 0.03f) {
        float gx = inner.x + fmodf((float)g_ctx.frame_index * 0.85f, inner.w + 120.0f) - 60.0f;
        for (int oy = 0; oy < out_h; ++oy) {
            float sy0 = inner.y + inner.h * ((float)oy / (float)out_h);
            float sy1 = inner.y + inner.h * ((float)(oy + 1) / (float)out_h);
            float front = row_front(oy);
            int run_start = -1;
            float run_alpha = 0.0f;
            for (int ox = 0; ox <= out_w; ++ox) {
                float px = inner.w * ((float)ox / (float)out_w);
                float sx = inner.x + px;
                bool on = ox < out_w && mask_on(ox, oy) && px <= front;
                float dist = fabsf((sx + inner.w / (float)out_w * 0.5f) - (gx + 5.0f));
                float a = on ? fmaxf(0.0f, 1.0f - dist / 8.0f) * 0.055f : 0.0f;
                if (on && oy < 2 && px > 4.0f && px < front - 4.0f) a = fmaxf(a, 0.09f);
                bool draw = a > 0.004f;
                if (draw && run_start < 0) { run_start = ox; run_alpha = a; }
                bool boundary = !draw || fabsf(a - run_alpha) > 0.018f;
                if (run_start >= 0 && boundary) {
                    float sx0 = inner.x + inner.w * ((float)run_start / (float)out_w);
                    float sx1 = inner.x + inner.w * ((float)ox / (float)out_w);
                    fill_rect({sx0, sy0, sx1 - sx0, sy1 - sy0}, maybe_disabled(Color{1.0f, 1.0f, 1.0f, run_alpha}));
                    run_start = draw ? ox : -1;
                    run_alpha = a;
                }
            }
        }
    }
}

void progress_bar(float progress) {
    ProgressStyle style;
    progress_bar(progress, style);
}

void progress_bar(float progress, const char* label_or_mask_path) {
    ProgressStyle style;
    if (label_or_mask_path && is_builtin_mask_shape(label_or_mask_path)) style.mask_shape = label_or_mask_path;
    else if (label_or_mask_path && looks_like_mask_path(label_or_mask_path)) style.mask_path = label_or_mask_path;
    else style.label = label_or_mask_path;
    progress_bar(progress, style);
}

void progress_bar(float progress, const ProgressStyle& style) {
    if (!g_drawing) return;
    WidgetColorScope color_scope;
    float p = clamp01(progress);
    const char* label = style.label;
    float h = fmaxf(12.0f, style.height);
    float label_h = label && label[0] ? widget_label_height(label) : 0.0f;
    Rect outer = next_rect(h + label_h);
    CollapseRectFx cfx = collapse_rect_fx(outer);
    outer = cfx.rect;
    Rect r = {outer.x, outer.y + label_h, outer.w, h};
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);

    Color fill = style.fill_color.a >= 0.0f ? style.fill_color : resolve_color(style.fill_role);
    if (cfx.clip_active) push_clip(cfx.clip);
    if (label && label[0]) draw_widget_label(label, outer, false);

    if ((style.mask_path && style.mask_path[0]) ||
        (style.mask_svg && style.mask_svg[0]) ||
        (style.mask_shape && style.mask_shape[0])) {
        draw_masked_progress(r, p, style, fill);
    } else {
        Color track = resolve_color(ColorRole::InputBg);
        Color border = resolve_color(ColorRole::Border);
        draw_widget_chrome(r, fmaxf(3.0f, g_style.rounding * 0.55f), maybe_disabled(track), maybe_disabled(border), 0.0f, 0.0f, 0.0f);
        Rect inner = {r.x + 3.0f, r.y + 3.0f, r.w - 6.0f, r.h - 6.0f};
        if (inner.w > 0.0f && inner.h > 0.0f && p > 0.0f) {
            float fill_w = inner.w * p;
            if (style.wave_front && p < 0.995f) {
                float amp = fminf(5.0f, fmaxf(1.5f, inner.h * 0.18f));
                float core_w = clampf(fill_w - amp - 1.0f, 0.0f, inner.w);
                if (core_w > 0.0f) {
                    fill_round_rect({inner.x, inner.y, core_w, inner.h}, fmaxf(2.0f, g_style.rounding * 0.4f), maybe_disabled(fill));
                }
                int strips = (int)fminf(32.0f, fmaxf(8.0f, inner.h));
                float phase = (float)g_ctx.frame_index * 0.085f;
                push_clip(inner);
                for (int i = 0; i < strips; ++i) {
                    float y0 = inner.y + inner.h * ((float)i / (float)strips);
                    float y1 = inner.y + inner.h * ((float)(i + 1) / (float)strips);
                    float front = clampf(fill_w + sinf((float)i * 0.42f + phase) * amp, 0.0f, inner.w);
                    if (front > core_w) {
                        fill_rect({inner.x + core_w, y0, front - core_w, y1 - y0 + 0.5f}, maybe_disabled(fill));
                    }
                }
                pop_clip();
                g_redraw_requested = true;
            } else {
                fill_round_rect({inner.x, inner.y, fill_w, inner.h}, fmaxf(2.0f, g_style.rounding * 0.4f), maybe_disabled(fill));
            }
            if (style.glint) {
                g_redraw_requested = true;
                push_clip({inner.x, inner.y, fill_w, inner.h});
                fill_rect({inner.x + 4.0f, inner.y + 2.0f, fmaxf(0.0f, fill_w - 8.0f), 1.0f}, Color{1.0f, 1.0f, 1.0f, 0.10f});
                float gx = inner.x + fmodf((float)g_ctx.frame_index * 0.85f, inner.w + 110.0f) - 55.0f;
                fill_rect({gx, inner.y, 10.0f, inner.h}, Color{1.0f, 1.0f, 1.0f, 0.035f});
                pop_clip();
            }
        }
    }

    if (style.show_percent) {
        char pct[32];
        snprintf(pct, sizeof(pct), "%.0f%%", p * 100.0f);
        float text_w = measure_text_width(pct);
        float lh = text_line_height();
        Rect chip = {
            r.x + r.w * 0.5f - text_w * 0.5f - 8.0f,
            r.y + r.h * 0.5f - lh * 0.5f - 1.0f,
            text_w + 16.0f,
            lh + 2.0f
        };
        Color chip_bg = lerp_color(resolve_color(ColorRole::Background), resolve_color(ColorRole::Panel), 0.35f);
        chip_bg.a = 0.56f;
        fill_round_rect(chip, fmaxf(3.0f, g_style.rounding * 0.45f), maybe_disabled(chip_bg));
        Rect shadow_r = {r.x + 1.0f, r.y + 1.0f, r.w, r.h};
        draw_text_utf8_centered(pct, shadow_r, Color{0.0f, 0.0f, 0.0f, 0.40f});
        draw_text_utf8_centered(pct, r, maybe_disabled(resolve_color(ColorRole::Text)));
    }
    if (cfx.clip_active) pop_clip();
    mark_last_item(0, outer, raw_hov, false);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0.6f,0.8f,1.0f,0.4f});
}

void image(ImageHandle* img, float width, float height) {
    if (!g_drawing) return;
    Rect r = next_rect(height);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    if (width < r.w) r.w = width;
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_image_handle(img, r);
    if (cfx.clip_active) pop_clip();
    mark_last_item(0, r, false, false);
    if (g_debug.show_layout_rects) stroke_round_rect(r, 0, 1, {0,1,1,0.4f});
}

bool tabs(const char* const* labels, int count, int* selected) {
    if (!g_drawing || count <= 0) return false;
    WidgetColorScope color_scope;
    float tab_h = g_style.item_height;
    Rect r = next_rect(tab_h);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    float tab_w = r.w / count;
    bool changed = false;
    size_t ptr_bits = (size_t)(const void*)selected;
    int bar_id = (int)(((ptr_bits >> 4) ^ (ptr_bits >> 9) ^ ((size_t)count * 2654435761u)) & 0x7fffffff);
    if (bar_id == 0) bar_id = count ? count : 1;
    TabFxSlot& fx = tab_fx_slot_for(bar_id);
    if (!fx.initialized) {
        fx.initialized = true;
        fx.selected = *selected;
        fx.previous = *selected;
        fx.switch_t = 1.0f;
    }

    int current_selected = *selected;
    Rect selected_rect = {};
    bool have_selected_rect = false;
    bool enabled = widget_interaction_enabled();
    std::vector<int> ids(count);
    int focused_idx = -1;
    Rect last_rect = r;
    int last_id = bar_id;
    bool last_hov = false;
    bool last_focus = false;

    for (int i = 0; i < count; ++i) {
        char vis[128]; const char* hs;
        split_label(labels[i], vis, sizeof(vis), &hs);
        ids[i] = hash_str(hs) ^ (i * 31337);
        register_focusable(ids[i]);
        if (is_widget_focused(ids[i])) focused_idx = i;
    }

    if (focused_idx >= 0 && enabled) {
        int next_idx = focused_idx;
        if (g_input.key_left) next_idx = focused_idx > 0 ? focused_idx - 1 : 0;
        if (g_input.key_right) next_idx = focused_idx + 1 < count ? focused_idx + 1 : count - 1;
        if (next_idx != focused_idx) {
            current_selected = next_idx;
            set_widget_focus(ids[next_idx], false);
            changed = true;
        } else if ((g_input.key_space || g_input.key_enter) && current_selected != focused_idx) {
            current_selected = focused_idx;
            changed = true;
        }
    }

    if (cfx.clip_active) push_clip(cfx.clip);
    Color button = resolve_color(ColorRole::Button);
    Color button_hover = resolve_color(ColorRole::ButtonHover);
    Color button_active = resolve_color(ColorRole::ButtonActive);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    for (int i = 0; i < count; i++) {
        char vis[128]; const char* hs;
        split_label(labels[i], vis, sizeof(vis), &hs);
        int id = ids[i];

        Rect tr = {r.x + i * tab_w, r.y, tab_w, tab_h};
        bool raw_hov = rect_contains(tr, g_input.mouse_x, g_input.mouse_y);
        bool hov = raw_hov && enabled;

        if (hov && g_input.mouse_pressed) {
            set_widget_focus(id, false);
            g_ctx.active_id = id;
        }
        if (g_ctx.active_id == id && g_input.mouse_released) {
            if (hov && current_selected != i) { current_selected = i; changed = true; }
            g_ctx.active_id = 0;
        }

        bool focused = is_widget_focused(id);
        bool sel = (current_selected == i);
        if (sel) { selected_rect = tr; have_selected_rect = true; }
        if (raw_hov || focused) {
            last_rect = tr;
            last_id = id;
            last_hov = raw_hov;
            last_focus = focused;
        }

        MotionSlot& motion = motion_slot_for(id);
        update_motion_slot(motion, hov, enabled && g_ctx.active_id == id, focused || sel);

        Color bg = lerp_color(button, button_hover, motion.hover * 0.45f);
        bg = lerp_color(bg, panel, focused ? 0.18f : 0.0f);
        bg = lerp_color(bg, button_active, sel ? 0.22f : motion.active * 0.75f);
        Color border_col = lerp_color(border, focus, (focused ? 0.55f : 0.0f) + (sel ? 0.18f : 0.0f));
        draw_widget_chrome(tr, g_style.rounding, maybe_disabled(bg), maybe_disabled(border_col), motion.hover, motion.active, focused ? 1.0f : 0.0f);
        draw_text_utf8_centered(vis, tr, maybe_disabled(lerp_color(text_dim, text, clamp01((sel ? 0.55f : 0.0f) + motion.hover * 0.35f + (focused ? 0.35f : 0.0f)))));
    }

    *selected = current_selected;

    if (current_selected != fx.selected) {
#if FTUI_WINDOWS_EFFECTS
        if (effects_enabled()) capture_frame_snapshot();
#endif
        fx.previous = fx.selected;
        fx.selected = current_selected;
        fx.dir = fx.selected >= fx.previous ? 1 : -1;
        fx.switch_t = 0.0f;
        if (effects_enabled() && (g_tab_content_owner == 0 || g_tab_content_owner == bar_id)) g_tab_content_owner = bar_id;
    }

    if (effects_enabled()) {
        if (fx.switch_t < 1.0f) {
            fx.switch_t += g_dt / 0.18f;
            if (fx.switch_t > 1.0f) fx.switch_t = 1.0f;
        }
    } else {
        fx.switch_t = 1.0f;
    }

    if (have_selected_rect) {
        float target_x = selected_rect.x + 4.0f;
        float target_w = fmaxf(18.0f, selected_rect.w - 8.0f);
        if (fx.underline_w <= 0.0f) {
            fx.underline_x = target_x;
            fx.underline_w = target_w;
        } else if (effects_enabled()) {
            fx.underline_x = step_anim(fx.underline_x, target_x, g_dt, 20.0f);
            fx.underline_w = step_anim(fx.underline_w, target_w, g_dt, 20.0f);
        } else {
            fx.underline_x = target_x;
            fx.underline_w = target_w;
        }
        Rect accent = {fx.underline_x, r.y + r.h - 3, fx.underline_w, 3};
        fill_round_rect(accent, 1.5f, with_alpha(focus, 0.78f));
    }

    if (effects_enabled() && g_tab_content_owner == bar_id && fx.switch_t < 1.0f && !g_frame_content_fx.active) {
        float p = ease_smooth(fx.switch_t);
        g_frame_content_fx.active = true;
        g_frame_content_fx.owner = bar_id;
        g_frame_content_fx.start_y = g_ctx.cursor_y;
        g_frame_content_fx.progress = fx.switch_t;
        g_frame_content_fx.dir = fx.dir;
        g_draw_fx_off_x = (float)fx.dir * (1.0f - p) * 18.0f;
        g_draw_fx_opacity = 1.0f;
    } else if (g_tab_content_owner == bar_id && fx.switch_t >= 1.0f) {
        g_tab_content_owner = 0;
    }
    if (cfx.clip_active) pop_clip();

    mark_last_item(last_id, last_rect, last_hov, last_focus);
    if (g_debug.show_layout_rects) stroke_round_rect(r, 0, 1, {0.5f,0,1,0.4f});
    return changed;
}

bool dropdown(const char* label, const char* const* items, int count, int* selected, int popup_rows) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    register_focusable(id);

    if (count < 0) count = 0;
    if (popup_rows <= 0) popup_rows = 8;
    if (selected && count > 0) {
        if (*selected < 0) *selected = 0;
        if (*selected >= count) *selected = count - 1;
    }

    Rect target_r = {}, target_outer = next_labeled_rect(vis, g_style.item_height, &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    bool open = (g_dropdown_open_id == id);
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);
    bool base_enabled = (g_disabled_depth == 0) && (!g_modal_open_id || g_inside_modal);
    bool enabled = base_enabled && (!g_dropdown_capture_input || open);
    bool hov = raw_hov && enabled;
    bool focused = is_widget_focused(id);
    bool changed = false;
    ScrollSlot& slot = scroll_slot_for(id ^ 0x330011);
    CollapseSlot& popup_fx = collapse_slot_for(id ^ 0x330177);

    if (hov && g_input.mouse_pressed) {
        set_widget_focus(id, false);
        g_ctx.active_id = id;
    }
    if (g_ctx.active_id == id && g_input.mouse_released) {
        if (hov) {
            bool was_open = open;
#if FTUI_WINDOWS_EFFECTS
            if (effects_enabled() && !was_open) capture_frame_snapshot();
#endif
            g_dropdown_open_id = open ? 0 : id;
            open = !open;
        }
        g_ctx.active_id = 0;
    }
    if (focused && enabled && (g_input.key_space || g_input.key_enter)) {
        bool was_open = open;
#if FTUI_WINDOWS_EFFECTS
        if (effects_enabled() && !was_open) capture_frame_snapshot();
#endif
        g_dropdown_open_id = open ? 0 : id;
        open = !open;
    }
    if (open && g_input.key_escape) {
        g_dropdown_open_id = 0;
        open = false;
    }

    float item_h = g_style.item_height - 4.0f;
    int visible_count = popup_rows < count ? popup_rows : count;
    float full_popup_h = visible_count > 0 ? visible_count * item_h + 8.0f : item_h + 8.0f;
    float space_below = g_ctx.content_region.y + g_ctx.content_region.h - (r.y + r.h) - 4.0f;
    float space_above = r.y - g_ctx.content_region.y - 4.0f;
    bool open_up = full_popup_h > space_below && space_above > space_below;
    float max_popup_h = open_up ? space_above : space_below;
    if (max_popup_h < item_h + 8.0f) max_popup_h = item_h + 8.0f;
    float popup_h = full_popup_h < max_popup_h ? full_popup_h : max_popup_h;

    if (open && focused && count > 0) {
        if (g_input.key_up) {
            int nv = *selected > 0 ? *selected - 1 : 0;
            if (nv != *selected) { *selected = nv; changed = true; }
        }
        if (g_input.key_down) {
            int nv = *selected + 1 < count ? *selected + 1 : count - 1;
            if (nv != *selected) { *selected = nv; changed = true; }
        }
    }

    if (effects_enabled()) popup_fx.progress = step_anim(popup_fx.progress, open ? 1.0f : 0.0f, g_dt, 18.0f);
    else popup_fx.progress = open ? 1.0f : 0.0f;
    float popup_p = ease_smooth(popup_fx.progress);
    bool popup_visible = open || popup_fx.progress > 0.01f;
    float popup_anim_h = popup_h * popup_p;
    Rect popup_r = open_up
        ? Rect{r.x, r.y - 4.0f - popup_anim_h, r.w, popup_anim_h}
        : Rect{r.x, r.y + r.h + 4.0f, r.w, popup_anim_h};
    bool popup_hov = popup_visible && rect_contains(popup_r, g_input.mouse_x, g_input.mouse_y);
    bool consume_mouse = false;

    if (open && g_input.mouse_pressed && !raw_hov && !popup_hov) {
        g_dropdown_open_id = 0;
        open = false;
        consume_mouse = true;
        g_ctx.active_id = 0;
    } else if (open && popup_hov && g_input.mouse_pressed) {
        g_ctx.active_id = 0;
    }

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, hov || popup_hov, enabled && g_ctx.active_id == id, focused);
    Color input_bg = resolve_color(ColorRole::InputBg);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color button_hover = resolve_color(ColorRole::ButtonHover);
    Color panel_col = lerp_color(input_bg, panel, motion.hover * 0.18f + (open ? 0.10f : 0.0f));
    Color border_col = lerp_color(border, focus, (focused ? 0.65f : 0.0f) + (open ? 0.20f : 0.0f));
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(panel_col), maybe_disabled(border_col),
                       motion.hover, motion.active, focused ? 1.0f : 0.0f);

    const char* current = (count > 0 && selected) ? items[*selected] : "(empty)";
    Rect text_r = {r.x + 10.0f, r.y, r.w - 34.0f, r.h};
    draw_text_utf8(current ? current : "", text_r, maybe_disabled(text));
    Rect arrow_r = {r.x + r.w - 22.0f, r.y + (r.h - 8.0f) * 0.5f, 10.0f, 8.0f};
    fill_triangle(arrow_r, (open && open_up) ? 1 : 0, maybe_disabled(text_dim));
    if (cfx.clip_active) pop_clip();

    if (popup_visible && popup_anim_h > 8.0f) {
        float max_scroll = count * item_h - (popup_h - 8.0f);
        if (max_scroll < 0.0f) max_scroll = 0.0f;
        if (open && popup_hov && g_input.wheel_y != 0.0f) {
            slot.target -= g_input.wheel_y;
            slot.target = clampf(slot.target, 0.0f, max_scroll);
            g_input.wheel_y = 0.0f;
        }
        if (effects_enabled()) slot.current = step_anim(slot.current, slot.target, g_dt, 18.0f);
        else slot.current = slot.target;
        slot.current = clampf(slot.current, 0.0f, max_scroll);

        for (int i = 0; i < count; ++i) {
            Rect ir = {popup_r.x + 4.0f, popup_r.y + 4.0f - slot.current + i * item_h, popup_r.w - 8.0f, item_h};
            if (ir.y + ir.h < popup_r.y || ir.y > popup_r.y + popup_r.h) continue;
            bool item_hov = open && rect_contains(ir, g_input.mouse_x, g_input.mouse_y);
            if (item_hov && g_input.mouse_pressed && enabled) {
                if (selected && *selected != i) changed = true;
                if (selected) *selected = i;
                g_dropdown_open_id = 0;
                open = false;
                consume_mouse = true;
                g_ctx.active_id = 0;
            }
        }
        g_dropdown_overlay.active = true;
        g_dropdown_overlay.open = open;
        g_dropdown_overlay.popup_r = popup_r;
        g_dropdown_overlay.count = count;
        g_dropdown_overlay.selected_index = selected ? *selected : -1;
        g_dropdown_overlay.item_h = item_h;
        g_dropdown_overlay.scroll = slot.current;
        g_dropdown_overlay.popup_h = popup_h;
        g_dropdown_overlay.popup_p = popup_p;
        g_dropdown_overlay.popup_fill = with_alpha(panel, effects_enabled() ? 0.96f : 0.98f);
        g_dropdown_overlay.popup_border = with_alpha(border, effects_enabled() ? 0.92f : 1.0f);
        g_dropdown_overlay.item_hover = with_alpha(button_hover, 0.34f);
        g_dropdown_overlay.item_selected = with_alpha(focus, 0.24f);
        g_dropdown_overlay.item_text = text_dim;
        g_dropdown_overlay.item_text_selected = text;
        Color scrollbar_track = border; scrollbar_track.a = 0.30f;
        g_dropdown_overlay.scrollbar_track = scrollbar_track;
        g_dropdown_overlay.scrollbar_thumb = text_dim;
        g_dropdown_overlay.labels.clear();
        g_dropdown_overlay.labels.reserve((size_t)count);
        for (int i = 0; i < count; ++i) {
            g_dropdown_overlay.labels.emplace_back(items && items[i] ? items[i] : "");
        }
    }

    if ((open || popup_visible) && (popup_hov || raw_hov || g_input.mouse_pressed || g_input.mouse_released || g_input.mouse_down)) {
        consume_mouse = true;
    }
    if (consume_mouse) {
        g_input.mouse_pressed = false;
        g_input.mouse_released = false;
        g_input.mouse_down = false;
    }

    draw_widget_label(vis, outer, focused);
    mark_last_item(id, outer, raw_hov || popup_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0.2f,0.8f,0.8f,0.4f});
    return changed;
}

bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    register_focusable(id);

    if (count < 0) count = 0;
    if (visible_rows <= 0) visible_rows = 6;
    if (selected && count > 0) {
        if (*selected < 0) *selected = 0;
        if (*selected >= count) *selected = count - 1;
    }

    float item_h = g_style.item_height - 4.0f;
    Rect target_r = {}, target_outer = next_labeled_rect(vis, visible_rows * item_h + 8.0f, &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;
    bool focused = is_widget_focused(id);
    bool changed = false;
    ScrollSlot& slot = scroll_slot_for(id ^ 0x774411);

    if (hov && g_input.mouse_pressed) set_widget_focus(id, false);
    if (focused && enabled && selected && count > 0) {
        if (g_input.key_up) {
            int nv = *selected > 0 ? *selected - 1 : 0;
            if (nv != *selected) { *selected = nv; changed = true; }
        }
        if (g_input.key_down) {
            int nv = *selected + 1 < count ? *selected + 1 : count - 1;
            if (nv != *selected) { *selected = nv; changed = true; }
        }
    }

    float visible_h = r.h - 8.0f;
    float total_h = count * item_h;
    float max_scroll = total_h > visible_h ? (total_h - visible_h) : 0.0f;
    if (selected && count > 0) {
        float item_top = *selected * item_h;
        float item_bot = item_top + item_h;
        if (item_top < slot.target) slot.target = item_top;
        if (item_bot > slot.target + visible_h) slot.target = item_bot - visible_h;
    }
    slot.target = clampf(slot.target, 0.0f, max_scroll);
    if (hov && g_input.wheel_y != 0.0f) {
        slot.target -= g_input.wheel_y;
        slot.target = clampf(slot.target, 0.0f, max_scroll);
        g_input.wheel_y = 0.0f;
    }
    if (effects_enabled()) slot.current = step_anim(slot.current, slot.target, g_dt, 18.0f);
    else slot.current = slot.target;
    slot.current = clampf(slot.current, 0.0f, max_scroll);

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, hov, false, focused);
    Color input_bg = resolve_color(ColorRole::InputBg);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color button_hover = resolve_color(ColorRole::ButtonHover);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color panel_col = lerp_color(input_bg, panel, motion.hover * 0.18f + (focused ? 0.08f : 0.0f));
    Color border_col = lerp_color(border, focus, focused ? 0.7f : 0.0f);
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(panel_col), maybe_disabled(border_col),
                       motion.hover, 0.0f, focused ? 1.0f : 0.0f);

    Rect clip_r = {r.x + 2.0f, r.y + 2.0f, r.w - 4.0f, r.h - 4.0f};
    if (clip_r.w < 0.0f) clip_r.w = 0.0f;
    if (clip_r.h < 0.0f) clip_r.h = 0.0f;
    push_clip(clip_r);
    for (int i = 0; i < count; ++i) {
        Rect ir = {r.x + 4.0f, r.y + 4.0f - slot.current + i * item_h, r.w - 12.0f, item_h};
        if (ir.y + ir.h < r.y || ir.y > r.y + r.h) continue;
        bool item_hov = rect_contains(ir, g_input.mouse_x, g_input.mouse_y) && enabled;
        if (item_hov && g_input.mouse_pressed) {
            set_widget_focus(id, false);
            if (selected && *selected != i) changed = true;
            if (selected) *selected = i;
        }
        bool item_sel = selected && *selected == i;
        Color item_bg = item_sel ? with_alpha(focus, 0.22f) :
                        item_hov ? with_alpha(button_hover, 0.30f) :
                                   with_alpha(panel, 0.0f);
        if (item_bg.a > 0.0f) fill_round_rect(ir, g_style.rounding * 0.6f, maybe_disabled(item_bg));
        Rect item_text_r = {ir.x + 8.0f, ir.y, ir.w - 16.0f, ir.h};
        draw_text_utf8(items[i] ? items[i] : "", item_text_r, maybe_disabled(item_sel ? text : text_dim));
    }
    if (count == 0) {
        Rect empty_r = {r.x + 8.0f, r.y, r.w - 16.0f, r.h};
        draw_text_utf8("(empty)", empty_r, maybe_disabled(text_dim));
    }
    pop_clip();

    if (total_h > visible_h) {
        float sb_w = 6.0f, sb_x = r.x + r.w - sb_w - 2.0f;
        float thumb_h = (visible_h / total_h) * visible_h;
        if (thumb_h < 12.0f) thumb_h = 12.0f;
        float thumb_y = r.y + 4.0f + (slot.current / (total_h - visible_h)) * (visible_h - thumb_h);
        Rect track_r = {sb_x, r.y + 4.0f, sb_w, visible_h};
        Rect thumb_r = {sb_x, thumb_y, sb_w, thumb_h};
        Color track_c = border; track_c.a = 0.3f;
        fill_round_rect(track_r, sb_w * 0.5f, maybe_disabled(track_c));
        fill_round_rect(thumb_r, sb_w * 0.5f, maybe_disabled(text_dim));
    }

    draw_widget_label(vis, outer, focused);
    if (cfx.clip_active) pop_clip();

    mark_last_item(id, outer, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0.0f,0.9f,0.5f,0.4f});
    return changed;
}

bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns) {
    if (!g_drawing || count <= 0 || !selected) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int base_id = hash_str(hs);
    if (columns <= 0) columns = 1;
    if (columns > count) columns = count;
    if (*selected < 0) *selected = 0;
    if (*selected >= count) *selected = count - 1;

    int rows = (count + columns - 1) / columns;
    float gap = g_style.item_spacing;
    float item_h = g_style.item_height;
    Rect target_r = {}, target_outer = next_labeled_rect(vis, rows * item_h + gap * (rows - 1), &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    float cell_w = (r.w - gap * (columns - 1)) / columns;
    bool enabled = widget_interaction_enabled();
    bool changed = false;
    std::vector<int> ids(count);
    int focused_idx = -1;
    Rect last_rect = r;
    int last_id = base_id;
    bool last_hov = false;
    bool last_focus = false;

    if (cfx.clip_active) push_clip(cfx.clip);
    for (int i = 0; i < count; ++i) {
        ids[i] = base_id ^ (i * 911);
        register_focusable(ids[i]);
        if (is_widget_focused(ids[i])) focused_idx = i;
    }
    if (cfx.clip_active) pop_clip();

    if (focused_idx >= 0 && enabled) {
        int next_idx = focused_idx;
        if (g_input.key_left && (focused_idx % columns) > 0) next_idx = focused_idx - 1;
        if (g_input.key_right && (focused_idx % columns) + 1 < columns && focused_idx + 1 < count) next_idx = focused_idx + 1;
        if (g_input.key_up && focused_idx - columns >= 0) next_idx = focused_idx - columns;
        if (g_input.key_down && focused_idx + columns < count) next_idx = focused_idx + columns;
        if (next_idx != focused_idx) {
            set_widget_focus(ids[next_idx], false);
            if (*selected != next_idx) { *selected = next_idx; changed = true; }
        } else if ((g_input.key_space || g_input.key_enter) && *selected != focused_idx) {
            *selected = focused_idx;
            changed = true;
        }
    }

    Color button = resolve_color(ColorRole::Button);
    Color button_hover = resolve_color(ColorRole::ButtonHover);
    Color button_active = resolve_color(ColorRole::ButtonActive);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    for (int i = 0; i < count; ++i) {
        int row_i = i / columns;
        int col_i = i % columns;
        Rect ir = {r.x + col_i * (cell_w + gap), r.y + row_i * (item_h + gap), cell_w, item_h};
        bool raw_hov = rect_contains(ir, g_input.mouse_x, g_input.mouse_y);
        bool hov = raw_hov && enabled;
        bool focused = is_widget_focused(ids[i]);
        bool sel = (*selected == i);

        if (hov && g_input.mouse_pressed) {
            set_widget_focus(ids[i], false);
            g_ctx.active_id = ids[i];
        }
        if (g_ctx.active_id == ids[i] && g_input.mouse_released) {
            if (hov && !sel) { *selected = i; changed = true; }
            g_ctx.active_id = 0;
        }
        if (raw_hov || focused) {
            last_rect = ir;
            last_id = ids[i];
            last_hov = raw_hov;
            last_focus = focused;
        }

        MotionSlot& motion = motion_slot_for(ids[i]);
        update_motion_slot(motion, hov, enabled && g_ctx.active_id == ids[i], focused || sel);
        Color bg = lerp_color(button, button_hover, motion.hover * 0.42f);
        bg = lerp_color(bg, button_active, sel ? 0.20f : motion.active * 0.65f);
        Color border_col = lerp_color(border, focus, (sel ? 0.28f : 0.0f) + (focused ? 0.55f : 0.0f));
        draw_widget_chrome(ir, g_style.rounding, maybe_disabled(bg), maybe_disabled(border_col), motion.hover, motion.active, focused ? 1.0f : 0.0f);

        float box_sz = 12.0f;
        Rect box = {ir.x + 10.0f, ir.y + (ir.h - box_sz) * 0.5f, box_sz, box_sz};
        stroke_round_rect(box, 3.0f, 1.0f, maybe_disabled(sel ? focus : border));
        if (sel) {
            Rect inner = {box.x + 3.0f, box.y + 3.0f, box.w - 6.0f, box.h - 6.0f};
            fill_rect(inner, maybe_disabled(focus));
        }

        Rect tr = {box.x + box.w + 8.0f, ir.y, ir.w - box.w - 18.0f, ir.h};
        draw_text_utf8(items[i] ? items[i] : "", tr, maybe_disabled(sel ? text : text_dim));
    }

    draw_widget_label(vis, outer, false);

    mark_last_item(last_id, outer, last_hov, last_focus);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0.7f,0.5f,0.1f,0.4f});
    return changed;
}

bool collapsing_header(const char* label, bool* open) {
    if (!g_drawing) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    register_focusable(id);

    CollapseSlot& slot = collapse_slot_for(id);
    if (!slot.initialized) { slot.initialized = true; slot.open = false; }
    bool state = open ? *open : slot.open;

    finalize_pending_collapse_measure();

    Rect r = next_rect(g_style.item_height);
    CollapseRectFx cfx = collapse_rect_fx(r);
    r = cfx.rect;
    bool raw_hov = rect_contains(r, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;
    bool focused = is_widget_focused(id);

    if (hov && g_input.mouse_pressed) {
        set_widget_focus(id, false);
        g_ctx.active_id = id;
    }
    if (focused && enabled && (g_input.key_space || g_input.key_enter)) state = !state;
    if (g_ctx.active_id == id && g_input.mouse_released) {
        if (hov) state = !state;
        g_ctx.active_id = 0;
    }

    if (open) *open = state;
    else slot.open = state;
    if (effects_enabled()) slot.progress = step_anim(slot.progress, state ? 1.0f : 0.0f, g_dt, 18.0f);
    else slot.progress = state ? 1.0f : 0.0f;
    bool show_body = state || (effects_enabled() && slot.progress > 0.001f);

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, hov, enabled && g_ctx.active_id == id, focused || state);
    Color button = resolve_color(ColorRole::Button);
    Color button_hover = resolve_color(ColorRole::ButtonHover);
    Color panel = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text = resolve_color(ColorRole::Text);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color bg = lerp_color(button, button_hover, motion.hover * 0.40f);
    bg = lerp_color(bg, panel, state ? 0.10f : 0.0f);
    Color border_col = lerp_color(border, focus, (focused ? 0.55f : 0.0f) + (state ? 0.16f : 0.0f));
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(bg), maybe_disabled(border_col), motion.hover, motion.active, focused ? 1.0f : 0.0f);

    Rect arrow_r = {r.x + 10.0f, r.y + (r.h - 10.0f) * 0.5f, 10.0f, 10.0f};
    fill_triangle(arrow_r, slot.progress > 0.5f ? 0 : 2, maybe_disabled(text_dim));
    Rect text_r = {r.x + 28.0f, r.y, r.w - 32.0f, r.h};
    draw_text_utf8(vis, text_r, maybe_disabled(text));
    if (cfx.clip_active) pop_clip();

    if (show_body) {
        float body_top_target = g_ctx.cursor_y;
        float body_top_display = transformed_layout_y(body_top_target);
        if (effects_enabled() && slot.body_height > 0.5f && g_active_collapse_fx_count < 16) {
            ActiveCollapseFx& fx = g_active_collapse_fx[g_active_collapse_fx_count++];
            fx.key = id;
            fx.top_target = body_top_target;
            fx.top_display = body_top_display;
            fx.body_height = slot.body_height;
            fx.progress = ease_smooth(slot.progress);
        }
        g_pending_collapse_key = id;
        g_pending_collapse_start_y = body_top_target;
    }

    mark_last_item(id, r, raw_hov, focused);
    if (g_debug.show_layout_rects) stroke_round_rect(r, 0, 1, {0.8f,0.4f,0.1f,0.4f});
    return show_body;
}

void scroll_area(const char* label, float height, std::function<void()> fn) {
    if (!g_drawing || height <= 0.0f || !fn) return;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);
    ScrollSlot& slot = scroll_slot_for(id ^ 0x442211);

    Rect target_r = {}, target_outer = next_labeled_rect(vis, height, &target_r);
    CollapseRectFx cfx = collapse_rect_fx(target_outer);
    Rect outer = cfx.rect;
    Rect r = labeled_body_rect(vis, outer);
    bool raw_hov = rect_contains(outer, g_input.mouse_x, g_input.mouse_y);
    bool enabled = widget_interaction_enabled();
    bool hov = raw_hov && enabled;

    bool had_scroll_prev = slot.content > (r.h - 12.0f);
    float sb_reserve = had_scroll_prev ? 10.0f : 0.0f;
    Rect inner = {r.x + 6.0f, r.y + 6.0f, r.w - 12.0f - sb_reserve, r.h - 12.0f};
    if (inner.w < 0.0f) inner.w = 0.0f;
    if (inner.h < 0.0f) inner.h = 0.0f;

    if (hov && g_input.wheel_y != 0.0f) {
        slot.target -= g_input.wheel_y;
        g_input.wheel_y = 0.0f;
    }

    MotionSlot& motion = motion_slot_for(id);
    update_motion_slot(motion, hov, false, false);
    Color panel = resolve_color(ColorRole::Panel);
    Color input_bg = resolve_color(ColorRole::InputBg);
    Color border = resolve_color(ColorRole::Border);
    Color focus = resolve_color(ColorRole::InputFocus);
    Color text_dim = resolve_color(ColorRole::TextDim);
    Color panel_col = lerp_color(panel, input_bg, motion.hover * 0.12f);
    if (cfx.clip_active) push_clip(cfx.clip);
    draw_widget_chrome(r, g_style.rounding, maybe_disabled(panel_col), maybe_disabled(border), motion.hover, 0.0f, 0.0f);

    Rect clip_r = {r.x + 2.0f, r.y + 2.0f, r.w - 4.0f, r.h - 4.0f};
    if (clip_r.w < 0.0f) clip_r.w = 0.0f;
    if (clip_r.h < 0.0f) clip_r.h = 0.0f;
    push_clip(clip_r);

    Rect saved_region = g_ctx.content_region;
    float saved_cursor_x = g_ctx.cursor_x;
    float saved_cursor_y = g_ctx.cursor_y;
    RowContext saved_row = g_ctx.row_ctx;

    g_ctx.content_region = inner;
    g_ctx.cursor_x = inner.x;
    g_ctx.cursor_y = inner.y - slot.current;
    g_ctx.row_ctx = {};
    color_scope.suspend_for_children();
    fn();
    color_scope.resume_after_children();
    slot.content = g_ctx.cursor_y + slot.current - inner.y;

    g_ctx.content_region = saved_region;
    g_ctx.cursor_x = saved_cursor_x;
    g_ctx.cursor_y = saved_cursor_y;
    g_ctx.row_ctx = saved_row;
    pop_clip();

    float max_scroll = slot.content > inner.h ? (slot.content - inner.h) : 0.0f;
    slot.target = clampf(slot.target, 0.0f, max_scroll);
    if (effects_enabled()) slot.current = step_anim(slot.current, slot.target, g_dt, 18.0f);
    else slot.current = slot.target;
    slot.current = clampf(slot.current, 0.0f, max_scroll);

    if (slot.content > inner.h) {
        float sb_w = 6.0f, visible_h = inner.h;
        float thumb_h = (visible_h / slot.content) * visible_h;
        if (thumb_h < 12.0f) thumb_h = 12.0f;
        float thumb_y = inner.y + (slot.current / (slot.content - visible_h)) * (visible_h - thumb_h);
        Rect track_r = {r.x + r.w - sb_w - 4.0f, inner.y, sb_w, visible_h};
        Rect thumb_r = {track_r.x, thumb_y, sb_w, thumb_h};
        bool thumb_hov = rect_contains(thumb_r, g_input.mouse_x, g_input.mouse_y) && enabled;
        bool track_hov = rect_contains(track_r, g_input.mouse_x, g_input.mouse_y) && enabled;
        if (g_input.mouse_pressed && thumb_hov) {
            slot.dragging = true;
            slot.drag_mouse = g_input.mouse_y;
            slot.drag_scroll0 = slot.current;
        }
        if (slot.dragging) {
            if (g_input.mouse_down || g_input.mouse_released) {
                float scale = (slot.content - visible_h) / fmaxf(1.0f, visible_h - thumb_h);
                slot.current = slot.drag_scroll0 + (g_input.mouse_y - slot.drag_mouse) * scale;
                slot.current = clampf(slot.current, 0.0f, max_scroll);
                slot.target = slot.current;
            }
            if (g_input.mouse_released) slot.dragging = false;
        }
        if (g_input.mouse_pressed && track_hov && !thumb_hov) {
            slot.target += (g_input.mouse_y < thumb_y ? -visible_h : visible_h);
            slot.target = clampf(slot.target, 0.0f, max_scroll);
        }
        Color track_c = border; track_c.a = 0.3f;
        fill_round_rect(track_r, sb_w * 0.5f, maybe_disabled(track_c));
        fill_round_rect(thumb_r, sb_w * 0.5f, maybe_disabled(slot.dragging ? focus : text_dim));
    }

    draw_widget_label(vis, outer, false);
    if (cfx.clip_active) pop_clip();

    mark_last_item(id, outer, raw_hov, false);
    if (g_debug.show_layout_rects) stroke_round_rect(outer, 0, 1, {0.3f,1.0f,0.4f,0.4f});
}

bool modal(const char* label, std::function<void()> fn) {
    if (!g_drawing || !label || !fn) return false;
    WidgetColorScope color_scope;
    char vis[128]; const char* hs;
    split_label(label, vis, sizeof(vis), &hs);
    int id = hash_str(hs);

    if (g_modal_request_id == id) {
        g_modal_open_id = id;
        g_modal_request_id = 0;
    }
    if (g_modal_open_id != id) return false;
    if (g_input.key_escape) {
        close_modal();
        return false;
    }

    g_modal_drawn = true;
    Rect overlay = {g_ctx.content_region.x, g_ctx.content_region.y, g_ctx.content_region.w, g_ctx.content_region.h};
    Color veil = {0.02f, 0.03f, 0.05f, 0.58f};
    fill_rect(overlay, veil);

    float modal_w = fminf(460.0f, fmaxf(280.0f, overlay.w * 0.56f));
    float modal_h = fminf(320.0f, fmaxf(180.0f, overlay.h * 0.42f));
    Rect panel = {
        overlay.x + (overlay.w - modal_w) * 0.5f,
        overlay.y + (overlay.h - modal_h) * 0.5f,
        modal_w,
        modal_h
    };
    Color panel_col = resolve_color(ColorRole::Panel);
    Color border = resolve_color(ColorRole::Border);
    Color text = resolve_color(ColorRole::Text);
    draw_widget_chrome(panel, fmaxf(g_style.rounding, 10.0f), panel_col, border, 0.0f, 0.0f, 1.0f);

    Rect title_r = {panel.x + 16.0f, panel.y + 10.0f, panel.w - 32.0f, g_style.item_height};
    draw_text_utf8(vis, title_r, text);

    Rect saved_region = g_ctx.content_region;
    float saved_cursor_x = g_ctx.cursor_x;
    float saved_cursor_y = g_ctx.cursor_y;
    RowContext saved_row = g_ctx.row_ctx;
    bool saved_inside_modal = g_inside_modal;

    g_inside_modal = true;
    g_ctx.content_region = {panel.x + 16.0f, panel.y + g_style.item_height + 16.0f, panel.w - 32.0f, panel.h - g_style.item_height - 26.0f};
    g_ctx.cursor_x = g_ctx.content_region.x;
    g_ctx.cursor_y = g_ctx.content_region.y;
    g_ctx.row_ctx = {};
    color_scope.suspend_for_children();
    fn();
    color_scope.resume_after_children();

    g_ctx.content_region = saved_region;
    g_ctx.cursor_x = saved_cursor_x;
    g_ctx.cursor_y = saved_cursor_y;
    g_ctx.row_ctx = saved_row;
    g_inside_modal = saved_inside_modal;

    mark_last_item(id, panel, rect_contains(panel, g_input.mouse_x, g_input.mouse_y), true);
    if (g_debug.show_layout_rects) stroke_round_rect(panel, 0, 1, {1.0f,0.4f,0.2f,0.4f});
    return g_modal_open_id == id;
}

void row(int cols, std::function<void()> fn) {
    if (!g_drawing || cols <= 0) return;
    auto& rc = g_ctx.row_ctx;
    float gap = g_style.item_spacing;
    rc.active   = true;
    rc.weighted = false;
    rc.cols     = cols;
    rc.gap      = gap;
    rc.cell_w   = (g_ctx.content_region.w - gap * (cols - 1)) / cols;
    rc.start_x  = g_ctx.cursor_x;
    rc.start_y  = g_ctx.cursor_y;
    rc.total_w  = g_ctx.content_region.w;
    rc.used_x   = 0;
    rc.total_weight = (float)cols;
    rc.weights  = nullptr;
    rc.col_index = 0;
    rc.row_height = 0;
    fn();
    rc.active = false;
    g_ctx.cursor_y += rc.row_height + g_style.item_spacing;
}

void row(std::initializer_list<float> weights, std::function<void()> fn) {
    if (!g_drawing || !fn || weights.size() == 0) return;
    auto& rc = g_ctx.row_ctx;
    float gap = g_style.item_spacing;
    rc.active = true;
    rc.weighted = true;
    rc.cols = (int)weights.size();
    rc.gap = gap;
    rc.cell_w = 0.0f;
    rc.start_x = g_ctx.cursor_x;
    rc.start_y = g_ctx.cursor_y;
    rc.total_w = g_ctx.content_region.w;
    rc.used_x = 0.0f;
    rc.total_weight = 0.0f;
    rc.weights = weights.begin();
    for (float w : weights) rc.total_weight += w > 0.0f ? w : 0.0f;
    if (rc.total_weight <= 0.0f) rc.total_weight = (float)rc.cols;
    rc.col_index = 0;
    rc.row_height = 0.0f;
    fn();
    rc.active = false;
    g_ctx.cursor_y += rc.row_height + g_style.item_spacing;
}

void split(std::initializer_list<float> columns, std::function<void()> fn) {
    if (!g_drawing || !fn || columns.size() == 0) return;
    auto& rc = g_ctx.row_ctx;
    float gap = g_style.item_spacing;
    rc.active = true;
    rc.weighted = true;
    rc.split = true;
    rc.cols = (int)columns.size();
    rc.gap = gap;
    rc.cell_w = 0.0f;
    rc.start_x = g_ctx.cursor_x;
    rc.start_y = g_ctx.cursor_y;
    rc.total_w = g_ctx.content_region.w;
    rc.used_x = 0.0f;
    rc.total_weight = 0.0f;
    rc.fixed_total = 0.0f;
    rc.flex_total = 0.0f;
    rc.weights = columns.begin();
    for (float c : columns) {
        if (c >= 16.0f) rc.fixed_total += c;
        else rc.flex_total += c > 0.0f ? c : 1.0f;
    }
    if (rc.flex_total <= 0.0f) rc.flex_total = 1.0f;
    rc.col_index = 0;
    rc.row_height = 0.0f;
    fn();
    float row_h = rc.row_height;
    rc = {};
    g_ctx.cursor_y += row_h + g_style.item_spacing;
}

void side_layout(float side_width, std::function<void()> fn) {
    split({side_width, 1.0f}, fn);
}

void content(std::function<void()> fn) {
    if (!g_drawing || !fn) return;
    Rect cell = next_rect(0.0f);
    RowContext parent_row = g_ctx.row_ctx;
    Rect saved_region = g_ctx.content_region;
    float saved_cursor_x = g_ctx.cursor_x;
    float saved_cursor_y = g_ctx.cursor_y;

    float available_h = saved_region.y + saved_region.h - cell.y;
    if (available_h < 0.0f) available_h = 0.0f;
    g_ctx.content_region = {cell.x, cell.y, cell.w, available_h};
    g_ctx.cursor_x = cell.x;
    g_ctx.cursor_y = cell.y;
    g_ctx.row_ctx = {};
    fn();
    float used_h = g_ctx.cursor_y - cell.y;

    g_ctx.content_region = saved_region;
    g_ctx.cursor_x = saved_cursor_x;
    g_ctx.cursor_y = saved_cursor_y;
    if (used_h > parent_row.row_height) parent_row.row_height = used_h;
    g_ctx.row_ctx = parent_row;
}

} // namespace ftui

#endif // FTUI_IMPLEMENTATION


#ifdef None
#undef None
#endif

// ============================================================
// Embedded ftti.hpp
// ============================================================

// ftti.hpp - Fuck This Terminal Interface
// Single-header immediate-mode terminal UI for C++17.
//
// Usage:
//   #define FTTI_IMPLEMENTATION
//   #include "ftti.hpp"
//
// Linux/macOS:
//   c++ app.cpp -std=c++17 -o app
//
// Windows:
//   cl /std:c++17 app.cpp
//
// Define FTTI_IMPLEMENTATION in exactly one translation unit.

#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifndef FTTI_DEFAULT_STYLE
#define FTTI_DEFAULT_STYLE ftti::default_dark_style
#endif

#ifndef FTTI_VERSION_MAJOR
#define FTTI_VERSION_MAJOR 0
#endif
#ifndef FTTI_VERSION_MINOR
#define FTTI_VERSION_MINOR 1
#endif
#ifndef FTTI_VERSION_PATCH
#define FTTI_VERSION_PATCH 0
#endif

namespace ftti {

enum class Align { Start, Center, End };

enum class BuiltinIcon {
    Symbol,
    SymbolWithText,
};

enum class BackdropEffect {
    Blur,
    BayerDither,
};

struct Color {
    float r = 1.0f;
    float g = 1.0f;
    float b = 1.0f;
    float a = 1.0f;
};

struct Style {
    Color background = {12, 14, 18};
    Color panel = {25, 29, 38};
    Color panel_alt = {34, 40, 52};
    Color text = {230, 235, 245};
    Color text_dim = {142, 154, 171};
    Color border = {73, 84, 104};
    Color button = {34, 40, 52};
    Color button_hover = {52, 61, 78};
    Color button_active = {77, 91, 117};
    Color input_bg = {10, 12, 16};
    Color input_focus = {101, 143, 255};
    Color accent = {101, 143, 255};
    Color warning = {239, 161, 99};
    Color success = {99, 202, 139};
    float window_padding = 2.0f;
    float item_spacing = 1.0f;
    float item_height = 3.0f;
    float rounding = 0.0f;
    float border_width = 1.0f;
    float font_size = 1.0f;
    bool boxed_widgets = true;
};

enum class ColorRole {
    Background,
    Panel,
    PanelAlt,
    Text,
    TextDim,
    Border,
    Button,
    ButtonHover,
    ButtonActive,
    InputBg,
    InputFocus,
    Accent,
    Warning,
    Success,
};

Style default_dark_style();
Style light_style();
Style terminal_green_style();
Style midnight_style();
Style amber_style();
Style catppuccin_mocha_style();
Style nord_style();
Style gruvbox_dark_style();
Style one_dark_style();
Style ghostty_green_style();

void set_style(const Style& style);
const Style& get_style();
Color color_from_hex(const char* hex);
Color color_from_hex(std::string_view hex);
void push_color(ColorRole role, Color color);
void pop_color();
void set_next_color(ColorRole role, Color color);

struct Config {
    const char* title = "FTTI App";
    int width = 960;
    int height = 640;
    int fps_limit = 30;
    bool resizable = true;
    bool center_window = true;
    void* icon = nullptr;
    bool enable_effects = true;
    BackdropEffect backdrop_effect = BackdropEffect::Blur;
    int dither_size = 4;
    bool alternate_screen = true;
    bool enable_mouse = true;
    bool quit_on_ctrl_q = true;
    bool animate = true;
};

bool create_window(const Config& cfg = {});
bool pump();
void begin();
void end();
void shutdown();
void request_redraw();
void set_quit_on_ctrl_q(bool enabled);
void set_fps_limit(int fps);
int get_fps_limit();
void set_backdrop_effect(BackdropEffect effect);
void set_dither_size(int px);
void set_window_icon(void* native_icon);
void set_window_icon_builtin(BuiltinIcon variant = BuiltinIcon::Symbol);
using CommandHandler = bool (*)(const char* command);
void set_command_handler(CommandHandler handler);

enum class InputFlags : unsigned {
    Default = 0,
    Password = 1 << 0,
    ReadOnly = 1 << 1,
    CharsDecimal = 1 << 2,
    CharsHexadecimal = 1 << 3,
    CharsUppercase = 1 << 4,
    CharsNoBlank = 1 << 5,
};

inline InputFlags operator|(InputFlags a, InputFlags b) {
    return static_cast<InputFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline bool operator&(InputFlags a, InputFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class TextAreaFlags : unsigned {
    Default = 0,
    ReadOnly = 1 << 0,
    WordWrap = 1 << 1,
    AutoScrollBottom = 1 << 2,
};

inline TextAreaFlags operator|(TextAreaFlags a, TextAreaFlags b) {
    return static_cast<TextAreaFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline bool operator&(TextAreaFlags a, TextAreaFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class LogViewFlags : unsigned {
    Default = 0,
    WordWrap = 1 << 0,
    AutoScrollBottom = 1 << 1,
};

inline LogViewFlags operator|(LogViewFlags a, LogViewFlags b) {
    return static_cast<LogViewFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}

inline bool operator&(LogViewFlags a, LogViewFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

void label(const char* text);
void text(const char* text);
void text_wrapped(const char* text);
void separator();
void spacing(int rows = 1);
void spacing(float rows);

bool button(const char* label);
bool button(const char* label, ColorRole role);
bool button(const char* label, Color color);
bool input(const char* label, char* buffer, int buffer_size,
           InputFlags flags = InputFlags::Default, bool* enter_pressed = nullptr);
bool text_area(const char* label, char* buffer, int buffer_size, int rows = 5);
bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows = 5,
                  TextAreaFlags flags = TextAreaFlags::Default);
void log_view(const char* label, const char* text, int rows = 8,
              LogViewFlags flags = LogViewFlags::AutoScrollBottom);
bool checkbox(const char* label, bool* value);
bool slider_float(const char* label, float* value, float min_v, float max_v);
bool tabs(const char* const* labels, int count, int* selected);
bool side_menu(const char* label, const char* const* items, int count, int* selected);
bool dropdown(const char* label, const char* const* items, int count, int* selected, int popup_rows = 8);
bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows = 6);
bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns = 1);
bool collapsing_header(const char* label, bool* open = nullptr);
void row(int cols, std::function<void()> fn);
void row(std::initializer_list<float> weights, std::function<void()> fn);
void split(std::initializer_list<float> columns, std::function<void()> fn);
void side_layout(float side_width, std::function<void()> fn);
void content(std::function<void()> fn);
void scroll_area(const char* label, float height, std::function<void()> fn);
void set_next_width(float cols);
void set_next_fill();
void set_next_percent(float pct);
void set_next_limits(float min_cols, float max_cols);
void set_next_align(Align align);
void open_modal(const char* label);
bool modal(const char* label, std::function<void()> fn);
void close_modal();
void begin_disabled();
void end_disabled();
void tooltip(const char* text);
void request_focus(const char* label);
float calc_text_width(const char* text);
float calc_text_height(const char* text, float wrap_width);

enum class ToastType {
    Info,
    Success,
    Warning,
    Error,
};

struct Toast {
    const char* message = "";
    ToastType type = ToastType::Info;
    int duration_ms = 3500;
    bool dismissible = true;
};

void toast(const Toast& toast);
void toast(const char* message);
void toast_info(const char* message);
void toast_success(const char* message);
void toast_warning(const char* message);
void toast_error(const char* message);
void clear_toasts();

struct ProgressStyle {
    const char* label = nullptr;
    const char* mask_path = nullptr;
    ColorRole fill_role = ColorRole::Accent;
    Color fill_color = {-1.0f, -1.0f, -1.0f, -1.0f};
    float height = 1.0f;
    bool show_percent = true;
    bool wave_front = false;
    bool glint = false;
};

void progress_bar(float progress);
void progress_bar(float progress, const char* label_or_mask_path);
void progress_bar(float progress, const ProgressStyle& style);

struct ImageHandle { void* _impl = nullptr; };
void image(ImageHandle* img, float width, float height);
ImageHandle* load_image(const char* utf8_path);
void free_image(ImageHandle* img);

struct FileFilter {
    const char* name;
    const char* spec;
};
std::string open_file_dialog(const char* title = "Open File",
                             const FileFilter* filters = nullptr,
                             int filter_count = 0);

void open_child_window(const Config& cfg, std::function<void()> fn);

struct DebugState {
    bool show_layout_rects = false;
    bool show_hovered_id = false;
    bool show_active_id = false;
    bool show_fps = false;
    bool log_widget_calls = false;
};
DebugState& debug();

float animation_time();
int terminal_width();
int terminal_height();

} // namespace ftti

#ifdef FTTI_IMPLEMENTATION

#include <cstdlib>
#include <thread>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <conio.h>
#include <io.h>
#include <windows.h>
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#else
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>
#endif

namespace ftti {
namespace detail {

struct Cell {
    char ch = ' ';
    Color fg = {230, 235, 245};
    Color bg = {12, 14, 18};
    bool bold = false;
};

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

struct InputState {
    bool enter = false;
    bool space = false;
    bool backspace = false;
    bool tab = false;
    bool shift_tab = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;
    bool ctrl_q = false;
    bool escape = false;
    bool mouse_pressed = false;
    bool mouse_released = false;
    bool mouse_event = false;
    bool mouse_down = false;
    int wheel = 0;
    int mouse_x = -1;
    int mouse_y = -1;
    char text[64] = {};
    int text_count = 0;
};

struct Motion {
    unsigned id = 0;
    float focus = 0.0f;
    float active = 0.0f;
    float hover = 0.0f;
};

struct ScrollState {
    unsigned id = 0;
    float current = 0.0f;
    float target = 0.0f;
    float content = 0.0f;
};

struct TabState {
    unsigned id = 0;
    float current = 0.0f;
    float target = 0.0f;
};

struct ToastState {
    Toast toast;
    std::chrono::steady_clock::time_point created;
};

struct ScrollScope {
    unsigned id = 0;
    Rect viewport;
};

struct App {
    Config cfg;
    Style style;
    bool running = false;
    bool began = false;
    bool initialized = false;
    bool redraw = true;
    int width = 80;
    int height = 24;
    int cursor_x = 2;
    int cursor_y = 2;
    int content_x = 2;
    int content_w = 76;
    float page_scroll_current = 0.0f;
    float page_scroll_target = 0.0f;
    float page_content_height = 0.0f;
    bool clip_active = false;
    Rect clip_rect;
    int focused = 0;
    int hovered = 0;
    int active = 0;
    bool mouse_down = false;
    int disabled_depth = 0;
    int request_focus_id = 0;
    int modal_id = 0;
    bool close_modal_requested = false;
    Rect last_item_rect;
    unsigned last_item_id = 0;
    bool command_mode = false;
    std::string command_buffer;
    std::string command_message;
    CommandHandler command_handler = nullptr;
    int active_tabs_id = 0;
    int tab_command_delta = 0;
    int text_focus_id = 0;
    int active_scroll_id = 0;
    int focus_scroll_pending_id = 0;
    std::vector<int> focus_order;
    std::vector<Cell> cells;
    InputState in;
    std::string input_pending;
    std::vector<Motion> motions;
    std::vector<ScrollState> scrolls;
    std::vector<ScrollScope> scroll_scopes;
    std::vector<TabState> tabs;
    std::vector<ToastState> toasts;
    std::vector<std::string> graphics_commands;
    std::string last_output;
    std::vector<std::pair<ColorRole, Color>> color_stack;
    std::string pending_tooltip;
    Rect pending_tooltip_anchor;
    unsigned pending_tooltip_owner = 0;
    unsigned tooltip_owner = 0;
    int tooltip_early_hides = 0;
    bool tooltip_visible_last_frame = false;
    std::chrono::steady_clock::time_point tooltip_owner_since;
    std::chrono::steady_clock::time_point tooltip_visible_since;
    DebugState dbg;
    bool next_color_active = false;
    ColorRole next_color_role = ColorRole::Button;
    Color next_color = {255, 255, 255};
    int next_width = 0;
    bool next_fill = false;
    float next_percent = 0.0f;
    int next_min = 0;
    int next_max = 0;
    Align next_align = Align::Start;
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point last_frame;
#if defined(_WIN32)
    HANDLE out = INVALID_HANDLE_VALUE;
    HANDLE in_handle = INVALID_HANDLE_VALUE;
    DWORD old_out_mode = 0;
    DWORD old_in_mode = 0;
    bool owns_console = false;
#else
    termios old_term = {};
    int old_flags = 0;
#endif
};

static App& app() {
    static App a;
    return a;
}

#if defined(_WIN32)
static bool get_console_handles(HANDLE& out, HANDLE& in_handle,
                                DWORD& out_mode, DWORD& in_mode) {
    out = GetStdHandle(STD_OUTPUT_HANDLE);
    in_handle = GetStdHandle(STD_INPUT_HANDLE);
    if (out == nullptr || out == INVALID_HANDLE_VALUE ||
        in_handle == nullptr || in_handle == INVALID_HANDLE_VALUE) {
        return false;
    }
    return GetConsoleMode(out, &out_mode) != 0 &&
           GetConsoleMode(in_handle, &in_mode) != 0;
}

static bool is_gui_subsystem_process() {
    const auto* base = reinterpret_cast<const unsigned char*>(GetModuleHandleW(nullptr));
    if (!base) return false;
    const auto* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(base);
    if (dos->e_magic != IMAGE_DOS_SIGNATURE) return false;
    const auto* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(base + dos->e_lfanew);
    return nt->Signature == IMAGE_NT_SIGNATURE &&
           nt->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_WINDOWS_GUI;
}

static bool prepare_console(App& a) {
    HANDLE out = INVALID_HANDLE_VALUE;
    HANDLE in_handle = INVALID_HANDLE_VALUE;
    DWORD out_mode = 0;
    DWORD in_mode = 0;
    const bool handles_ready = get_console_handles(out, in_handle, out_mode, in_mode);
    const bool crt_ready = _isatty(_fileno(stdin)) != 0 &&
                           _isatty(_fileno(stdout)) != 0;
    bool rebind_streams = false;
    a.owns_console = false;

    if (is_gui_subsystem_process()) {
        // Shells do not wait for GUI-subsystem executables. Sharing their console
        // would leave the shell running and competing with the TUI for input, so
        // use a dedicated console that belongs exclusively to this process.
        (void)FreeConsole();
        if (!AllocConsole()) return false;
        a.owns_console = true;
        rebind_streams = true;
    } else if (!handles_ready || !crt_ready) {
        if (!handles_ready && !AttachConsole(ATTACH_PARENT_PROCESS)) {
            const DWORD attach_error = GetLastError();
            if (attach_error != ERROR_ACCESS_DENIED) {
                if (!AllocConsole()) return false;
                a.owns_console = true;
            }
        }
        rebind_streams = true;
    }

    if (rebind_streams) {
        // AttachConsole/AllocConsole do not repair the C runtime streams in every
        // launch context. Rebind them; input events use the native console handle.
        if (!std::freopen("CONIN$", "r", stdin) ||
            !std::freopen("CONOUT$", "w", stdout)) {
            if (a.owns_console) (void)FreeConsole();
            a.owns_console = false;
            return false;
        }
        (void)std::freopen("CONOUT$", "w", stderr);

        if (!get_console_handles(out, in_handle, out_mode, in_mode)) {
            if (a.owns_console) (void)FreeConsole();
            a.owns_console = false;
            return false;
        }
    }

    a.out = out;
    a.in_handle = in_handle;
    a.old_out_mode = out_mode;
    a.old_in_mode = in_mode;
    if (!SetConsoleMode(a.out, out_mode | ENABLE_PROCESSED_OUTPUT |
                               ENABLE_VIRTUAL_TERMINAL_PROCESSING)) {
        return false;
    }
    DWORD input_mode = ENABLE_PROCESSED_INPUT | ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS;
    if (a.cfg.enable_mouse) input_mode |= ENABLE_MOUSE_INPUT;
    input_mode &= ~ENABLE_QUICK_EDIT_MODE;
    if (!SetConsoleMode(a.in_handle, input_mode)) {
        SetConsoleMode(a.out, a.old_out_mode);
        return false;
    }
    return true;
}
#endif

static int clamp_i(int v, int lo, int hi) {
    return std::max(lo, std::min(hi, v));
}

static float clamp_f(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

static Color lerp(Color a, Color b, float t) {
    t = clamp_f(t, 0.0f, 1.0f);
    return {
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t,
    };
}

static unsigned color_byte(float v) {
    if (v <= 1.0f) v *= 255.0f;
    return (unsigned)clamp_i((int)std::round(v), 0, 255);
}

static unsigned hash_label(const char* s) {
    unsigned h = 2166136261u;
    for (; s && *s; ++s) h = (h ^ (unsigned char)*s) * 16777619u;
    return h ? h : 1u;
}

static std::string visible_label(const char* label) {
    if (!label) return {};
    const char* sep = std::strstr(label, "##");
    return sep ? std::string(label, sep) : std::string(label);
}

static bool contains(Rect r, int x, int y) {
    return x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h;
}

static void write_raw(const char* s) {
#if defined(_WIN32)
    DWORD written = 0;
    WriteFile(app().out, s, (DWORD)std::strlen(s), &written, nullptr);
#else
    ::write(STDOUT_FILENO, s, std::strlen(s));
#endif
}

static void write_raw(const std::string& s) {
#if defined(_WIN32)
    DWORD written = 0;
    WriteFile(app().out, s.data(), (DWORD)s.size(), &written, nullptr);
#else
    ::write(STDOUT_FILENO, s.data(), s.size());
#endif
}

static void query_size() {
    App& a = app();
#if defined(_WIN32)
    CONSOLE_SCREEN_BUFFER_INFO info = {};
    if (GetConsoleScreenBufferInfo(a.out, &info)) {
        a.width = info.srWindow.Right - info.srWindow.Left + 1;
        a.height = info.srWindow.Bottom - info.srWindow.Top + 1;
    }
#else
    winsize ws = {};
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 && ws.ws_row > 0) {
        a.width = ws.ws_col;
        a.height = ws.ws_row;
    }
#endif
    a.width = std::max(20, a.width);
    a.height = std::max(8, a.height);
}

static void resize_canvas() {
    App& a = app();
    a.cells.assign((size_t)a.width * (size_t)a.height, Cell{' ', a.style.text, a.style.background, false});
}

static void put(int x, int y, char ch, Color fg, Color bg, bool bold = false) {
    App& a = app();
    if (x < 0 || y < 0 || x >= a.width || y >= a.height) return;
    if (a.clip_active && !contains(a.clip_rect, x, y)) return;
    Cell& c = a.cells[(size_t)y * (size_t)a.width + (size_t)x];
    c.ch = ch;
    c.fg = fg;
    c.bg = bg;
    c.bold = bold;
}

static void fill_rect(Rect r, Color bg) {
    App& a = app();
    int x0 = clamp_i(r.x, 0, a.width);
    int y0 = clamp_i(r.y, 0, a.height);
    int x1 = clamp_i(r.x + r.w, 0, a.width);
    int y1 = clamp_i(r.y + r.h, 0, a.height);
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) put(x, y, ' ', a.style.text, bg);
    }
}

static void draw_text(int x, int y, const std::string& s, Color fg, Color bg, bool bold = false, int max_w = -1) {
    if (max_w < 0) max_w = (int)s.size();
    int n = std::min((int)s.size(), max_w);
    for (int i = 0; i < n; ++i) put(x + i, y, s[(size_t)i], fg, bg, bold);
}

static void draw_hline(int x, int y, int w, char ch, Color fg, Color bg) {
    for (int i = 0; i < w; ++i) put(x + i, y, ch, fg, bg);
}

static void draw_box(Rect r, Color fg, Color bg) {
    if (r.w < 2 || r.h < 2) return;
    fill_rect(r, bg);
    put(r.x, r.y, '+', fg, bg);
    put(r.x + r.w - 1, r.y, '+', fg, bg);
    put(r.x, r.y + r.h - 1, '+', fg, bg);
    put(r.x + r.w - 1, r.y + r.h - 1, '+', fg, bg);
    draw_hline(r.x + 1, r.y, r.w - 2, '-', fg, bg);
    draw_hline(r.x + 1, r.y + r.h - 1, r.w - 2, '-', fg, bg);
    for (int y = r.y + 1; y < r.y + r.h - 1; ++y) {
        put(r.x, y, '|', fg, bg);
        put(r.x + r.w - 1, y, '|', fg, bg);
    }
}

static void draw_tooltip_box() {
    App& a = app();
    if (a.pending_tooltip.empty()) return;
    auto now = std::chrono::steady_clock::now();
    if (a.pending_tooltip_owner != a.tooltip_owner) {
        if (a.tooltip_visible_last_frame) {
            int visible_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - a.tooltip_visible_since).count();
            if (visible_ms < 550) a.tooltip_early_hides = std::min(3, a.tooltip_early_hides + 1);
            else if (a.tooltip_early_hides > 0) --a.tooltip_early_hides;
        }
        a.tooltip_owner = a.pending_tooltip_owner;
        a.tooltip_owner_since = now;
        a.tooltip_visible_last_frame = false;
        return;
    }
    int delay_ms = 650 + a.tooltip_early_hides * 300;
    int stable_ms = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - a.tooltip_owner_since).count();
    if (stable_ms < delay_ms) {
        a.tooltip_visible_last_frame = false;
        return;
    }
    if (!a.tooltip_visible_last_frame) {
        a.tooltip_visible_since = now;
        a.tooltip_visible_last_frame = true;
    }
    int w = std::min((int)a.pending_tooltip.size() + 4, std::max(12, a.width - 4));
    int x = a.pending_tooltip_anchor.x;
    int y = a.pending_tooltip_anchor.y + a.pending_tooltip_anchor.h;
    if (x + w >= a.width) x = a.width - w - 1;
    if (x < 1) x = 1;
    if (y + 3 >= a.height) y = std::max(2, a.pending_tooltip_anchor.y - 3);
    Rect r{x, y, w, 3};
    draw_box(r, a.style.accent, a.style.panel_alt);
    draw_text(r.x + 2, r.y + 1, a.pending_tooltip, a.style.text, a.style.panel_alt, false, r.w - 4);
}

static void draw_toasts() {
    App& a = app();
    auto now = std::chrono::steady_clock::now();
    int y = a.height - 2;
    for (int i = (int)a.toasts.size() - 1; i >= 0 && y >= 2; --i) {
        ToastState& ts = a.toasts[(size_t)i];
        int age = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - ts.created).count();
        if (ts.toast.duration_ms > 0 && age > ts.toast.duration_ms) {
            a.toasts.erase(a.toasts.begin() + i);
            continue;
        }
        const char* msg = ts.toast.message ? ts.toast.message : "";
        int w = std::min((int)std::strlen(msg) + 4, std::max(16, a.width - 4));
        Rect r{a.width - w - 2, y - 2, w, 3};
        Color border = a.style.accent;
        if (ts.toast.type == ToastType::Success) border = a.style.success;
        if (ts.toast.type == ToastType::Warning || ts.toast.type == ToastType::Error) border = a.style.warning;
        draw_box(r, border, a.style.panel_alt);
        draw_text(r.x + 2, r.y + 1, msg, a.style.text, a.style.panel_alt, false, r.w - 4);
        y -= 4;
    }
}

static std::string command_completion(const std::string& prefix);
static std::string command_hint(const std::string& prefix);

static void draw_command_line() {
    App& a = app();
    if (a.height < 3) return;
    Rect r{0, a.height - 1, a.width, 1};
    Color bg = a.command_mode ? a.style.panel_alt : a.style.background;
    fill_rect(r, bg);
    if (a.command_mode) {
        std::string prefix = ":" + a.command_buffer;
        draw_text(0, a.height - 1, prefix, a.style.accent, bg, true, a.width);
        std::string completion = command_completion(a.command_buffer);
        if (!completion.empty() && completion.size() > a.command_buffer.size()) {
            std::string suffix = completion.substr(a.command_buffer.size());
            draw_text((int)prefix.size(), a.height - 1, suffix, a.style.text_dim, bg, false,
                      std::max(0, a.width - (int)prefix.size()));
        }
        std::string hint = command_hint(a.command_buffer);
        int hint_x = std::max(0, a.width - (int)hint.size() - 1);
        if (hint_x > (int)prefix.size() + 2) draw_text(hint_x, a.height - 1, hint, a.style.text_dim, bg, false);
    } else if (!a.command_message.empty()) {
        draw_text(0, a.height - 1, a.command_message, a.style.text_dim, bg, false, a.width);
    }
}

static Motion& motion(unsigned id) {
    App& a = app();
    for (Motion& m : a.motions) {
        if (m.id == id) return m;
    }
    a.motions.push_back(Motion{id, 0.0f, 0.0f});
    return a.motions.back();
}

static ScrollState& scroll_state(unsigned id) {
    App& a = app();
    for (ScrollState& s : a.scrolls) {
        if (s.id == id) return s;
    }
    a.scrolls.push_back(ScrollState{id, 0.0f, 0.0f});
    return a.scrolls.back();
}

static TabState& tab_state(unsigned id, int selected) {
    App& a = app();
    for (TabState& t : a.tabs) {
        if (t.id == id) return t;
    }
    a.tabs.push_back(TabState{id, (float)selected, (float)selected});
    return a.tabs.back();
}

static void animate_to(float& value, float target) {
    float speed = app().cfg.animate ? 0.28f : 1.0f;
    value += (target - value) * speed;
    if (std::abs(value - target) < 0.01f) value = target;
}

static void ease_to(float& value, float target, float speed = 0.18f) {
    if (!app().cfg.animate) {
        value = target;
        return;
    }
    float d = target - value;
    value += d * speed;
    if (std::abs(d) < 0.03f) value = target;
}

static float page_scroll_max() {
    App& a = app();
    float visible_rows = (float)std::max(1, a.height - 3);
    return std::max(0.0f, a.page_content_height - visible_rows);
}

static bool adjust_scroll_for_rect(float current, float& target, Rect item, int top, int bottom) {
    if (bottom <= top) return true;
    if (item.h >= bottom - top) {
        if (item.y < top || item.y > top + 1) {
            target = current + (float)(item.y - top);
            return false;
        }
        return true;
    }
    if (item.y < top) {
        target = current - (float)(top - item.y);
        return false;
    }
    int item_bottom = item.y + item.h;
    if (item_bottom > bottom) {
        target = current + (float)(item_bottom - bottom);
        return false;
    }
    return true;
}

static bool ensure_page_rect_visible(Rect r) {
    App& a = app();
    int top = 2;
    int bottom = std::max(top + 1, a.height - 1);
    bool visible = adjust_scroll_for_rect(a.page_scroll_current, a.page_scroll_target, r, top, bottom);
    a.page_scroll_target = clamp_f(a.page_scroll_target, 0.0f, page_scroll_max());
    return visible;
}

static bool ensure_scoped_rect_visible(Rect r) {
    App& a = app();
    if (a.scroll_scopes.empty()) return true;
    ScrollScope scope = a.scroll_scopes.back();
    ScrollState& ss = scroll_state(scope.id);
    bool visible = adjust_scroll_for_rect(
        ss.current,
        ss.target,
        r,
        scope.viewport.y,
        scope.viewport.y + scope.viewport.h
    );
    float max_scroll = std::max(0.0f, ss.content - (float)std::max(0, scope.viewport.h));
    ss.target = clamp_f(ss.target, 0.0f, max_scroll);
    return visible;
}

static void ensure_focused_rect_visible(unsigned id, Rect r) {
    App& a = app();
    if (a.focus_scroll_pending_id != (int)id) return;
    Rect page_r = r;
    if (!a.scroll_scopes.empty()) page_r = a.scroll_scopes.front().viewport;
    bool page_visible = ensure_page_rect_visible(page_r);
    bool local_visible = ensure_scoped_rect_visible(r);
    if (page_visible && local_visible) a.focus_scroll_pending_id = 0;
}

static void consume_wheel() {
    app().in.wheel = 0;
}

static bool register_focus(unsigned id, Rect r) {
    App& a = app();
    a.last_item_rect = r;
    a.last_item_id = id;
    a.focus_order.push_back((int)id);
    if (!a.focused) a.focused = (int)id;
    if (a.request_focus_id == (int)id) {
        a.focused = (int)id;
        a.focus_scroll_pending_id = (int)id;
        a.request_focus_id = 0;
    }
    bool focused = a.focused == (int)id;
    if (focused) ensure_focused_rect_visible(id, r);
    Motion& m = motion(id);
    animate_to(m.focus, focused ? 1.0f : 0.0f);
    bool mouse_hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    animate_to(m.hover, mouse_hit ? 1.0f : 0.0f);
    if (mouse_hit) a.hovered = (int)id;
    if (mouse_hit && a.in.mouse_pressed && a.disabled_depth == 0) a.focused = (int)id;
    return focused;
}

static bool disabled() {
    return app().disabled_depth > 0;
}

static void execute_command(const std::string& raw) {
    App& a = app();
    std::string typed = raw;
    while (!typed.empty() && std::isspace((unsigned char)typed.front())) typed.erase(typed.begin());
    while (!typed.empty() && std::isspace((unsigned char)typed.back())) typed.pop_back();
    if (!typed.empty() && typed[0] == ':') typed.erase(typed.begin());
    if (typed.empty()) return;
    if (a.command_handler && a.command_handler(typed.c_str())) {
        a.command_message = typed;
        a.redraw = true;
        return;
    }
    std::string cmd = typed;
    for (char& ch : cmd) ch = (char)std::tolower((unsigned char)ch);
    if (cmd == "q" || cmd == "quit" || cmd == "exit" || cmd == "wq" || cmd == "x") {
        a.running = false;
        return;
    }
    if (cmd == "tab next" || cmd == "tn" || cmd == "tabn") {
        a.tab_command_delta = 1;
        a.command_message = "tab next";
        return;
    }
    if (cmd == "tab prev" || cmd == "tp" || cmd == "tabp" || cmd == "tab previous") {
        a.tab_command_delta = -1;
        a.command_message = "tab prev";
        return;
    }
    if (cmd == "theme dark" || cmd == "td") {
        a.style = default_dark_style();
        a.command_message = "theme dark";
        return;
    }
    if (cmd == "theme light" || cmd == "tl") {
        a.style = light_style();
        a.command_message = "theme light";
        return;
    }
    if (cmd == "theme green" || cmd == "green") {
        a.style = terminal_green_style();
        a.command_message = "theme green";
        return;
    }
    if (cmd == "theme amber" || cmd == "amber") {
        a.style = amber_style();
        a.command_message = "theme amber";
        return;
    }
    if (cmd == "theme one" || cmd == "to") {
        a.style = one_dark_style();
        a.command_message = "theme one";
        return;
    }
    if (cmd == "theme catppuccin" || cmd == "tc") {
        a.style = catppuccin_mocha_style();
        a.command_message = "theme catppuccin";
        return;
    }
    if (cmd == "theme nord" || cmd == "tnord") {
        a.style = nord_style();
        a.command_message = "theme nord";
        return;
    }
    if (cmd == "theme gruvbox" || cmd == "tg") {
        a.style = gruvbox_dark_style();
        a.command_message = "theme gruvbox";
        return;
    }
    if (cmd == "theme ghostty" || cmd == "th") {
        a.style = ghostty_green_style();
        a.command_message = "theme ghostty";
        return;
    }
    if (cmd == "rr" || cmd == "redraw" || cmd == "refresh") {
        a.redraw = true;
        a.command_message = "redraw";
        return;
    }
    if (cmd == "df") { a.dbg.show_fps = !a.dbg.show_fps; a.command_message = "debug fps"; return; }
    if (cmd == "dl") { a.dbg.show_layout_rects = !a.dbg.show_layout_rects; a.command_message = "debug layout"; return; }
    if (cmd == "di") {
        bool next = !(a.dbg.show_hovered_id || a.dbg.show_active_id);
        a.dbg.show_hovered_id = next;
        a.dbg.show_active_id = next;
        a.command_message = "debug ids";
        return;
    }
    if (cmd == "dw") { a.dbg.log_widget_calls = !a.dbg.log_widget_calls; a.command_message = "debug widgets"; return; }
    if (cmd == "ct") {
        a.toasts.clear();
        a.command_message = "clear toasts";
        return;
    }
    if (cmd == "help" || cmd == "?") {
        a.command_message = "commands: q, help, theme dark/light/green/amber/one/catppuccin/nord/gruvbox/ghostty, tab next, tab prev, rr, ct, web";
        return;
    }
    a.command_message = cmd.empty() ? "" : "not a command: " + cmd;
}

struct CommandSpec {
    const char* command;
    const char* hint;
};

static const CommandSpec k_commands[] = {
    {"q", "quit"},
    {"quit", "quit"},
    {"exit", "quit"},
    {"wq", "save and quit"},
    {"x", "save and quit"},
    {"help", "show command help"},
    {"?", "show command help"},
    {"tab next", "next tab"},
    {"tabn", "next tab"},
    {"tab prev", "previous tab"},
    {"tab previous", "previous tab"},
    {"tabp", "previous tab"},
    {"tn", "next tab"},
    {"tp", "previous tab"},
    {"theme dark", "dark theme"},
    {"theme light", "light theme"},
    {"theme green", "green theme"},
    {"theme amber", "amber theme"},
    {"theme one", "one dark theme"},
    {"theme catppuccin", "catppuccin theme"},
    {"theme nord", "nord theme"},
    {"theme gruvbox", "gruvbox theme"},
    {"theme ghostty", "ghostty theme"},
    {"td", "dark theme"},
    {"tl", "light theme"},
    {"green", "green theme"},
    {"amber", "amber theme"},
    {"to", "one dark theme"},
    {"tc", "catppuccin theme"},
    {"tnord", "nord theme"},
    {"tg", "gruvbox theme"},
    {"th", "ghostty theme"},
    {"rr", "redraw"},
    {"redraw", "redraw"},
    {"refresh", "redraw"},
    {"df", "toggle fps"},
    {"dl", "layout rects"},
    {"di", "debug ids"},
    {"dw", "widget log"},
    {"ct", "clear toasts"},
    {"web", "web server"},
    {"ws", "web server"},
    {"server", "web server"},
    {"webserver", "web server"},
};

static std::string command_completion(const std::string& prefix) {
    if (prefix.empty()) return "";
    std::string best;
    for (const CommandSpec& spec : k_commands) {
        std::string cmd = spec.command;
        if (cmd.size() >= prefix.size() && cmd.compare(0, prefix.size(), prefix) == 0) {
            if (best.empty() || cmd.size() < best.size()) best = cmd;
        }
    }
    return best;
}

static std::string command_hint(const std::string& prefix) {
    for (const CommandSpec& spec : k_commands) {
        if (prefix == spec.command) return spec.hint;
    }
    std::string completion = command_completion(prefix);
    if (!completion.empty()) {
        for (const CommandSpec& spec : k_commands) {
            if (completion == spec.command) return spec.hint;
        }
    }
    return "tab complete  enter run  esc cancel";
}

static void consume_widget_input() {
    App& a = app();
    a.in.enter = false;
    a.in.space = false;
    a.in.backspace = false;
    a.in.tab = false;
    a.in.shift_tab = false;
    a.in.left = false;
    a.in.right = false;
    a.in.up = false;
    a.in.down = false;
    a.in.escape = false;
    a.in.text_count = 0;
    a.in.text[0] = '\0';
}

static void focus_delta(int delta) {
    App& a = app();
    if (a.focus_order.empty()) return;
    auto it = std::find(a.focus_order.begin(), a.focus_order.end(), a.focused);
    int idx = it == a.focus_order.end() ? 0 : (int)(it - a.focus_order.begin());
    idx = (idx + delta + (int)a.focus_order.size()) % (int)a.focus_order.size();
    int next = a.focus_order[(size_t)idx];
    if (a.focused != next) {
        a.focused = next;
        a.focus_scroll_pending_id = next;
    }
}

static void reset_input() {
    App& a = app();
    int x = a.in.mouse_x;
    int y = a.in.mouse_y;
    a.in = InputState{};
    a.in.mouse_x = x;
    a.in.mouse_y = y;
    a.in.mouse_down = a.mouse_down;
}

static void push_text_char(char ch) {
    App& a = app();
    if (a.in.text_count < (int)sizeof(a.in.text) - 1) {
        a.in.text[a.in.text_count++] = ch;
        a.in.text[a.in.text_count] = '\0';
    }
}

#if defined(_WIN32)
static void read_input() {
    App& a = app();
    DWORD pending = 0;
    if (!GetNumberOfConsoleInputEvents(a.in_handle, &pending)) return;

    while (pending > 0) {
        INPUT_RECORD records[64] = {};
        DWORD read = 0;
        const DWORD wanted = std::min<DWORD>(pending, (DWORD)(sizeof(records) / sizeof(records[0])));
        if (!ReadConsoleInputW(a.in_handle, records, wanted, &read) || read == 0) break;

        for (DWORD i = 0; i < read; ++i) {
            const INPUT_RECORD& record = records[i];
            if (record.EventType == KEY_EVENT) {
                const KEY_EVENT_RECORD& key = record.Event.KeyEvent;
                if (!key.bKeyDown) continue;
                const DWORD controls = key.dwControlKeyState;
                const bool shift = (controls & SHIFT_PRESSED) != 0;
                const bool ctrl = (controls & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
                const WORD vk = key.wVirtualKeyCode;

                if (ctrl && (vk == 'Q' || key.uChar.UnicodeChar == 17)) a.in.ctrl_q = true;
                else if (vk == VK_TAB) {
                    if (shift) a.in.shift_tab = true;
                    else a.in.tab = true;
                } else if (vk == VK_RETURN) a.in.enter = true;
                else if (vk == VK_ESCAPE) a.in.escape = true;
                else if (vk == VK_BACK) a.in.backspace = true;
                else if (vk == VK_LEFT) a.in.left = true;
                else if (vk == VK_RIGHT) a.in.right = true;
                else if (vk == VK_UP) a.in.up = true;
                else if (vk == VK_DOWN) a.in.down = true;
                else {
                    const wchar_t wc = key.uChar.UnicodeChar;
                    if (wc == L' ') a.in.space = true;
                    if (wc >= 32 && wc < 127) {
                        const WORD repeats = std::max<WORD>(1, key.wRepeatCount);
                        for (WORD repeat = 0; repeat < repeats; ++repeat) push_text_char((char)wc);
                    }
                }
                continue;
            }

            if (record.EventType == MOUSE_EVENT) {
                const MOUSE_EVENT_RECORD& mouse = record.Event.MouseEvent;
                CONSOLE_SCREEN_BUFFER_INFO info = {};
                int origin_x = 0;
                int origin_y = 0;
                if (GetConsoleScreenBufferInfo(a.out, &info)) {
                    origin_x = info.srWindow.Left;
                    origin_y = info.srWindow.Top;
                }
                a.in.mouse_x = (int)mouse.dwMousePosition.X - origin_x;
                a.in.mouse_y = (int)mouse.dwMousePosition.Y - origin_y;
                a.in.mouse_event = true;

                if (mouse.dwEventFlags == MOUSE_WHEELED) {
                    const SHORT delta = (SHORT)HIWORD(mouse.dwButtonState);
                    if (delta != 0) a.in.wheel = delta < 0 ? 1 : -1;
                } else if (mouse.dwEventFlags == 0 || mouse.dwEventFlags == DOUBLE_CLICK) {
                    const bool left_down = (mouse.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
                    if (left_down && !a.mouse_down) a.in.mouse_pressed = true;
                    if (!left_down && a.mouse_down) a.in.mouse_released = true;
                    a.mouse_down = left_down;
                    a.in.mouse_down = left_down;
                } else if (mouse.dwEventFlags == MOUSE_MOVED) {
                    const bool left_down = (mouse.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
                    a.mouse_down = left_down;
                    a.in.mouse_down = left_down;
                }
                continue;
            }

            if (record.EventType == WINDOW_BUFFER_SIZE_EVENT) a.redraw = true;
        }

        if (!GetNumberOfConsoleInputEvents(a.in_handle, &pending)) break;
    }
}
#else
static bool parse_escape_sequence(const std::string& buf, size_t& i) {
    App& a = app();
    if (i + 1 >= buf.size()) return false;
    if (buf[i] != 27 || buf[i + 1] != '[') {
        a.in.escape = true;
        ++i;
        return true;
    }
    i += 2;
    if (i >= buf.size()) return false;
    if (buf[i] == 'Z') {
        a.in.shift_tab = true;
        ++i;
        return true;
    }
    if (buf[i] == '<') {
        size_t start = i - 2;
        ++i;
        int b = 0, x = 0, y = 0;
        while (i < buf.size() && std::isdigit((unsigned char)buf[i])) b = b * 10 + (buf[i++] - '0');
        if (i >= buf.size()) { i = start; return false; }
        if (buf[i] != ';') { i = start + 1; a.in.escape = true; return true; }
        ++i;
        while (i < buf.size() && std::isdigit((unsigned char)buf[i])) x = x * 10 + (buf[i++] - '0');
        if (i >= buf.size()) { i = start; return false; }
        if (buf[i] != ';') { i = start + 1; a.in.escape = true; return true; }
        ++i;
        while (i < buf.size() && std::isdigit((unsigned char)buf[i])) y = y * 10 + (buf[i++] - '0');
        if (i >= buf.size()) { i = start; return false; }
        if (buf[i] == 'M' || buf[i] == 'm') {
            a.in.mouse_x = x - 1;
            a.in.mouse_y = y - 1;
            a.in.mouse_event = true;
            int button = b & ~(4 | 8 | 16);
            if (button == 64 || button == 66) a.in.wheel = -1;
            else if (button == 65 || button == 67) a.in.wheel = 1;
            else if ((b & 32) != 0) {
                // Hover/motion report.
            } else if (buf[i] == 'M') {
                a.in.mouse_pressed = true;
                a.mouse_down = true;
                a.in.mouse_down = true;
            } else {
                a.in.mouse_released = true;
                a.mouse_down = false;
                a.in.mouse_down = false;
            }
            ++i;
            return true;
        }
        i = start + 1;
        a.in.escape = true;
        return true;
    }
    if (buf[i] == 'M') {
        size_t start = i - 2;
        if (i + 3 >= buf.size()) { i = start; return false; }
        int b = (unsigned char)buf[i + 1] - 32;
        int x = (unsigned char)buf[i + 2] - 33;
        int y = (unsigned char)buf[i + 3] - 33;
        a.in.mouse_x = x;
        a.in.mouse_y = y;
        a.in.mouse_event = true;
        int button = b & ~(4 | 8 | 16);
        if (button == 64 || button == 66) a.in.wheel = -1;
        else if (button == 65 || button == 67) a.in.wheel = 1;
        else if ((b & 3) == 3) {
            a.in.mouse_released = true;
            a.mouse_down = false;
            a.in.mouse_down = false;
        } else {
            a.in.mouse_pressed = true;
            a.mouse_down = true;
            a.in.mouse_down = true;
        }
        i += 4;
        return true;
    }
    char final = 0;
    size_t start = i - 2;
    while (i < buf.size()) {
        char ch = buf[i];
        if ((ch >= 'A' && ch <= 'Z') || ch == '~') {
            final = ch;
            break;
        }
        ++i;
    }
    if (!final) { i = start; return false; }
    if (final == 'A') a.in.up = true;
    else if (final == 'B') a.in.down = true;
    else if (final == 'C') a.in.right = true;
    else if (final == 'D') a.in.left = true;
    ++i;
    return true;
}

static void read_input() {
    App& a = app();
    char buf[256];
    for (;;) {
        int n = (int)::read(STDIN_FILENO, buf, sizeof(buf));
        if (n <= 0) break;
        a.input_pending.append(buf, (size_t)n);
    }
    size_t i = 0;
    while (i < a.input_pending.size()) {
        unsigned char c = (unsigned char)a.input_pending[i];
        if (c == 27) {
            size_t before = i;
            if (!parse_escape_sequence(a.input_pending, i)) {
                a.input_pending.erase(0, before);
                return;
            }
        } else {
            ++i;
            if (c == 17) a.in.ctrl_q = true;
            else if (c == 9) a.in.tab = true;
            else if (c == 10 || c == 13) a.in.enter = true;
            else if (c == 127 || c == 8) a.in.backspace = true;
            else if (c == ' ') {
                a.in.space = true;
                push_text_char(' ');
            }
            else if (c >= 32 && c < 127) push_text_char((char)c);
        }
    }
    a.input_pending.clear();
}
#endif

static bool accept_char(char& ch, InputFlags flags) {
    if ((flags & InputFlags::CharsNoBlank) && std::isspace((unsigned char)ch)) return false;
    if (flags & InputFlags::CharsUppercase) ch = (char)std::toupper((unsigned char)ch);
    if (flags & InputFlags::CharsDecimal) return std::isdigit((unsigned char)ch) || ch == '.' || ch == '-';
    if (flags & InputFlags::CharsHexadecimal) return std::isxdigit((unsigned char)ch) != 0;
    return true;
}

static std::string color_seq(Color fg, Color bg, bool bold) {
    char buf[96];
    std::snprintf(buf, sizeof(buf), "\x1b[%d;38;2;%u;%u;%u;48;2;%u;%u;%um",
                  bold ? 1 : 22,
                  color_byte(fg.r), color_byte(fg.g), color_byte(fg.b),
                  color_byte(bg.r), color_byte(bg.g), color_byte(bg.b));
    return buf;
}

static std::string base64_encode(std::string_view input) {
    static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string out;
    out.reserve(((input.size() + 2) / 3) * 4);
    unsigned val = 0;
    int valb = -6;
    for (unsigned char c : input) {
        val = (val << 8) | c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(table[(val >> valb) & 0x3f]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(table[((val << 8) >> (valb + 8)) & 0x3f]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

struct ImageData {
    std::string path;
    unsigned id = 0;
};

static Color role_color(ColorRole role) {
    App& a = app();
    for (auto it = a.color_stack.rbegin(); it != a.color_stack.rend(); ++it) {
        if (it->first == role) return it->second;
    }
    const Style& s = app().style;
    switch (role) {
        case ColorRole::Background: return s.background;
        case ColorRole::Panel: return s.panel;
        case ColorRole::PanelAlt: return s.panel_alt;
        case ColorRole::Text: return s.text;
        case ColorRole::TextDim: return s.text_dim;
        case ColorRole::Border: return s.border;
        case ColorRole::Button: return s.button;
        case ColorRole::ButtonHover: return s.button_hover;
        case ColorRole::ButtonActive: return s.button_active;
        case ColorRole::InputBg: return s.input_bg;
        case ColorRole::InputFocus: return s.input_focus;
        case ColorRole::Accent: return s.accent;
        case ColorRole::Warning: return s.warning;
        case ColorRole::Success: return s.success;
    }
    return s.text;
}

static Color consume_role_color(ColorRole role) {
    App& a = app();
    if (a.next_color_active && a.next_color_role == role) {
        a.next_color_active = false;
        return a.next_color;
    }
    return role_color(role);
}

static int consume_width(int fallback) {
    App& a = app();
    int w = fallback;
    if (a.next_fill) w = a.content_w;
    if (a.next_percent > 0.0f) w = (int)(a.content_w * clamp_f(a.next_percent, 0.0f, 1.0f));
    if (a.next_width > 0) w = a.next_width;
    if (a.next_min > 0) w = std::max(w, a.next_min);
    if (a.next_max > 0) w = std::min(w, a.next_max);
    w = clamp_i(w, 4, a.content_w);
    a.next_width = 0;
    a.next_fill = false;
    a.next_percent = 0.0f;
    a.next_min = 0;
    a.next_max = 0;
    a.next_align = Align::Start;
    return w;
}

} // namespace detail

Style default_dark_style() {
    return {};
}

Style light_style() {
    Style s;
    s.background = {244, 247, 251};
    s.panel = {255, 255, 255};
    s.panel_alt = {232, 238, 247};
    s.text = {23, 29, 39};
    s.text_dim = {91, 105, 126};
    s.border = {185, 197, 213};
    s.button = {237, 242, 248};
    s.button_hover = {222, 231, 243};
    s.button_active = {203, 216, 235};
    s.input_bg = {255, 255, 255};
    s.input_focus = {37, 99, 235};
    s.accent = {37, 99, 235};
    s.warning = {180, 83, 9};
    s.success = {22, 128, 76};
    return s;
}

Style terminal_green_style() {
    Style s;
    s.background = {0, 12, 8};
    s.panel = {4, 26, 17};
    s.panel_alt = {7, 40, 27};
    s.text = {205, 255, 223};
    s.text_dim = {105, 188, 135};
    s.border = {55, 132, 84};
    s.button = {5, 35, 23};
    s.button_hover = {10, 57, 37};
    s.button_active = {18, 84, 54};
    s.input_bg = {0, 9, 6};
    s.input_focus = {80, 255, 145};
    s.accent = {80, 255, 145};
    s.warning = {236, 196, 84};
    s.success = {112, 245, 150};
    return s;
}

Style midnight_style() {
    Style s;
    s.background = {8, 10, 28};
    s.panel = {18, 22, 48};
    s.panel_alt = {29, 35, 70};
    s.text = {232, 238, 255};
    s.text_dim = {144, 158, 201};
    s.border = {70, 84, 142};
    s.button = {24, 30, 62};
    s.button_hover = {38, 48, 96};
    s.button_active = {60, 76, 138};
    s.input_bg = {7, 9, 24};
    s.input_focus = {117, 154, 255};
    s.accent = {117, 154, 255};
    s.warning = {255, 177, 107};
    s.success = {100, 220, 178};
    return s;
}

Style amber_style() {
    Style s;
    s.background = {18, 12, 5};
    s.panel = {38, 27, 12};
    s.panel_alt = {58, 41, 18};
    s.text = {255, 236, 201};
    s.text_dim = {199, 161, 105};
    s.border = {120, 82, 31};
    s.button = {49, 34, 14};
    s.button_hover = {72, 50, 21};
    s.button_active = {98, 67, 27};
    s.input_bg = {14, 9, 4};
    s.input_focus = {255, 184, 77};
    s.accent = {255, 184, 77};
    s.warning = {255, 113, 91};
    s.success = {144, 207, 130};
    return s;
}

Style catppuccin_mocha_style() {
    Style s = midnight_style();
    s.background = {17, 17, 27};
    s.panel = {30, 30, 46};
    s.panel_alt = {49, 50, 68};
    s.text = {205, 214, 244};
    s.text_dim = {166, 173, 200};
    s.border = {88, 91, 112};
    s.accent = {137, 180, 250};
    s.input_focus = s.accent;
    s.warning = {250, 179, 135};
    s.success = {166, 227, 161};
    return s;
}

Style nord_style() {
    Style s = default_dark_style();
    s.background = {46, 52, 64};
    s.panel = {59, 66, 82};
    s.panel_alt = {67, 76, 94};
    s.text = {236, 239, 244};
    s.text_dim = {216, 222, 233};
    s.border = {76, 86, 106};
    s.button = {67, 76, 94};
    s.button_hover = {76, 86, 106};
    s.button_active = {94, 129, 172};
    s.input_bg = {46, 52, 64};
    s.input_focus = {136, 192, 208};
    s.accent = {136, 192, 208};
    s.warning = {235, 203, 139};
    s.success = {163, 190, 140};
    return s;
}

Style gruvbox_dark_style() {
    Style s = amber_style();
    s.background = {29, 32, 33};
    s.panel = {40, 40, 40};
    s.panel_alt = {60, 56, 54};
    s.text = {235, 219, 178};
    s.text_dim = {168, 153, 132};
    s.border = {102, 92, 84};
    s.accent = {131, 165, 152};
    s.input_focus = s.accent;
    s.warning = {250, 189, 47};
    s.success = {184, 187, 38};
    return s;
}

Style one_dark_style() {
    return default_dark_style();
}

Style ghostty_green_style() {
    return terminal_green_style();
}

void set_style(const Style& style) {
    detail::app().style = style;
    request_redraw();
}

const Style& get_style() {
    return detail::app().style;
}

Color color_from_hex(std::string_view hex) {
    if (!hex.empty() && hex[0] == '#') hex.remove_prefix(1);
    auto nib = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return 0;
    };
    if (hex.size() == 3) {
        return {
            (float)(nib(hex[0]) * 17),
            (float)(nib(hex[1]) * 17),
            (float)(nib(hex[2]) * 17),
            1.0f,
        };
    }
    if (hex.size() >= 6) {
        return {
            (float)(nib(hex[0]) * 16 + nib(hex[1])),
            (float)(nib(hex[2]) * 16 + nib(hex[3])),
            (float)(nib(hex[4]) * 16 + nib(hex[5])),
            1.0f,
        };
    }
    return {255.0f, 255.0f, 255.0f, 1.0f};
}

Color color_from_hex(const char* hex) {
    return color_from_hex(std::string_view(hex ? hex : ""));
}

void push_color(ColorRole role, Color color) {
    detail::app().color_stack.push_back({role, color});
}

void pop_color() {
    detail::App& a = detail::app();
    if (!a.color_stack.empty()) a.color_stack.pop_back();
}

void set_next_color(ColorRole role, Color color) {
    detail::App& a = detail::app();
    a.next_color_active = true;
    a.next_color_role = role;
    a.next_color = color;
}

bool create_window(const Config& cfg) {
    using namespace detail;
    App& a = app();
    a.cfg = cfg;
    a.style = FTTI_DEFAULT_STYLE();
    a.running = false;
    a.initialized = false;
    a.redraw = true;
    a.last_output.clear();
    a.page_scroll_current = 0.0f;
    a.page_scroll_target = 0.0f;
    a.page_content_height = 0.0f;
    a.focus_scroll_pending_id = 0;
    a.scroll_scopes.clear();
    a.start_time = std::chrono::steady_clock::now();
    a.last_frame = a.start_time;
#if defined(_WIN32)
    if (!prepare_console(a)) return false;
#else
    if (!isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO)) return false;
    tcgetattr(STDIN_FILENO, &a.old_term);
    termios raw = a.old_term;
    raw.c_lflag &= (tcflag_t)~(ECHO | ICANON | IEXTEN);
    raw.c_iflag &= (tcflag_t)~(IXON | ICRNL);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    a.old_flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, a.old_flags | O_NONBLOCK);
#endif
    a.running = true;
    a.initialized = true;
    if (cfg.alternate_screen) write_raw("\x1b[?1049h");
    write_raw("\x1b[?25l\x1b[2J\x1b[H");
    if (cfg.enable_mouse) write_raw("\x1b[?1000h\x1b[?1003h\x1b[?1006h");
    query_size();
    resize_canvas();
    return true;
}

bool pump() {
    using namespace detail;
    App& a = app();
    if (!a.running) return false;

    int fps = a.cfg.fps_limit <= 0 ? 60 : a.cfg.fps_limit;
    auto now = std::chrono::steady_clock::now();
    auto min_frame = std::chrono::microseconds(1000000 / std::max(1, fps));
    auto elapsed = now - a.last_frame;
    if (elapsed < min_frame) std::this_thread::sleep_for(min_frame - elapsed);
    a.last_frame = std::chrono::steady_clock::now();

    reset_input();
    read_input();
    if (a.command_mode) {
        if (a.in.escape) {
            a.command_mode = false;
            a.command_buffer.clear();
        } else if (a.in.backspace && !a.command_buffer.empty()) {
            a.command_buffer.pop_back();
        } else if (a.in.tab) {
            std::string completion = command_completion(a.command_buffer);
            if (!completion.empty()) a.command_buffer = completion;
        } else {
            for (int i = 0; i < a.in.text_count; ++i) a.command_buffer.push_back(a.in.text[i]);
            if (a.in.enter) {
                execute_command(a.command_buffer);
                a.command_buffer.clear();
                a.command_mode = false;
            }
        }
        consume_widget_input();
    } else if (a.in.text_count == 1 && a.in.text[0] == ':' && a.focused != a.text_focus_id) {
        a.command_mode = true;
        a.command_buffer.clear();
        consume_widget_input();
    }
    if (a.cfg.quit_on_ctrl_q && a.in.ctrl_q) {
        a.running = false;
        return false;
    }
    if (a.in.tab) focus_delta(1);
    if (a.in.shift_tab) focus_delta(-1);
    query_size();
    return a.running;
}

void begin() {
    using namespace detail;
    App& a = app();
    a.began = true;
    a.focus_order.clear();
    a.scroll_scopes.clear();
    a.hovered = 0;
    a.pending_tooltip.clear();
    a.graphics_commands.clear();
    a.clip_active = false;
    a.page_scroll_target = clamp_f(a.page_scroll_target, 0.0f, page_scroll_max());
    ease_to(a.page_scroll_current, a.page_scroll_target, 0.22f);
    a.page_scroll_current = clamp_f(a.page_scroll_current, 0.0f, page_scroll_max());
    a.cursor_x = 2;
    a.cursor_y = 2 - (int)std::round(a.page_scroll_current);
    a.content_x = 2;
    a.content_w = std::max(10, a.width - 4);
    resize_canvas();

    std::string title = a.cfg.title ? a.cfg.title : "FTTI";
    draw_text(2, 0, title, a.style.accent, a.style.background, true, a.width - 4);
    std::string hint = "tab/shift-tab focus  enter activate  ctrl-q quit";
    if ((int)hint.size() < a.width - 2) {
        draw_text(a.width - (int)hint.size() - 2, 0, hint, a.style.text_dim, a.style.background, false);
    }
    draw_hline(0, 1, a.width, '-', a.style.border, a.style.background);
    a.clip_active = true;
    a.clip_rect = {0, 2, a.width, std::max(0, a.height - 2)};
}

void end() {
    using namespace detail;
    App& a = app();
    if (!a.began) return;
    a.began = false;
    a.clip_active = false;
    float measured_content = (float)(a.cursor_y - 2) + a.page_scroll_current;
    a.page_content_height = std::max(0.0f, measured_content);
    if (!a.command_mode && a.in.wheel != 0) {
        a.page_scroll_target += (float)a.in.wheel * 3.0f;
        consume_wheel();
    }
    a.page_scroll_target = clamp_f(a.page_scroll_target, 0.0f, page_scroll_max());
    a.page_scroll_current = clamp_f(a.page_scroll_current, 0.0f, page_scroll_max());
    float max_page = page_scroll_max();
    if (max_page > 0.0f && a.width > 4 && a.height > 8) {
        int track_y = 2;
        int track_h = std::max(1, a.height - 4);
        int x = a.width - 1;
        for (int y = track_y; y < track_y + track_h; ++y) put(x, y, '|', a.style.border, a.style.background);
        int thumb_h = clamp_i((int)std::round(((float)track_h / std::max(1.0f, a.page_content_height)) * (float)track_h), 1, track_h);
        int thumb_y = track_y + clamp_i((int)std::round((a.page_scroll_current / max_page) * (float)(track_h - thumb_h)), 0, std::max(0, track_h - thumb_h));
        for (int y = thumb_y; y < thumb_y + thumb_h; ++y) put(x, y, '#', a.style.accent, a.style.background, true);
    }
    if (!a.focused && !a.focus_order.empty()) a.focused = a.focus_order.front();
    draw_toasts();
    draw_tooltip_box();
    draw_command_line();

    std::string out;
    out.reserve((size_t)a.width * (size_t)a.height * 16);
    out += "\x1b[H";
    Color last_fg = {0, 0, 0};
    Color last_bg = {0, 0, 0};
    bool last_bold = false;
    bool have_color = false;
    for (int y = 0; y < a.height; ++y) {
        for (int x = 0; x < a.width; ++x) {
            const Cell& c = a.cells[(size_t)y * (size_t)a.width + (size_t)x];
            if (!have_color || std::memcmp(&last_fg, &c.fg, sizeof(Color)) != 0 ||
                std::memcmp(&last_bg, &c.bg, sizeof(Color)) != 0 || last_bold != c.bold) {
                out += color_seq(c.fg, c.bg, c.bold);
                last_fg = c.fg;
                last_bg = c.bg;
                last_bold = c.bold;
                have_color = true;
            }
            out += c.ch;
        }
        if (y != a.height - 1) out += "\r\n";
    }
    out += "\x1b[0m";
    for (const std::string& cmd : a.graphics_commands) out += cmd;
    if (a.redraw || out != a.last_output) {
        write_raw(out);
        a.last_output = out;
    }
    a.redraw = false;
}

void shutdown() {
    using namespace detail;
    App& a = app();
    if (!a.initialized) {
        return;
    }
    if (a.cfg.enable_mouse) write_raw("\x1b[?1006l\x1b[?1003l\x1b[?1000l");
    write_raw("\x1b[?25h\x1b[0m");
    if (a.cfg.alternate_screen) write_raw("\x1b[?1049l");
#if defined(_WIN32)
    if (a.out != INVALID_HANDLE_VALUE) SetConsoleMode(a.out, a.old_out_mode);
    if (a.in_handle != INVALID_HANDLE_VALUE) SetConsoleMode(a.in_handle, a.old_in_mode);
    if (a.owns_console) (void)FreeConsole();
    a.owns_console = false;
#else
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &a.old_term);
    fcntl(STDIN_FILENO, F_SETFL, a.old_flags);
#endif
    a.running = false;
    a.began = false;
    a.initialized = false;
    a.last_output.clear();
}

void request_redraw() {
    detail::app().redraw = true;
}

void set_quit_on_ctrl_q(bool enabled) {
    detail::app().cfg.quit_on_ctrl_q = enabled;
}

void set_fps_limit(int fps) {
    detail::app().cfg.fps_limit = fps;
}

int get_fps_limit() {
    return detail::app().cfg.fps_limit;
}

void set_backdrop_effect(BackdropEffect effect) {
    detail::app().cfg.backdrop_effect = effect;
}

void set_dither_size(int px) {
    detail::app().cfg.dither_size = px;
}

void set_window_icon(void* native_icon) {
    detail::app().cfg.icon = native_icon;
}

void set_window_icon_builtin(BuiltinIcon variant) {
    (void)variant;
}

void set_command_handler(CommandHandler handler) {
    detail::app().command_handler = handler;
}

void label(const char* s) {
    text(s);
}

void text(const char* s) {
    using namespace detail;
    App& a = app();
    std::string value = s ? s : "";
    draw_text(a.content_x, a.cursor_y, value, a.style.text, a.style.background, false, a.content_w);
    a.cursor_y += 1;
}

void text_wrapped(const char* s) {
    using namespace detail;
    App& a = app();
    std::string value = s ? s : "";
    int w = std::max(8, a.content_w);
    int x = a.content_x;
    int y = a.cursor_y;
    size_t pos = 0;
    while (pos < value.size() && y < a.height) {
        while (pos < value.size() && value[pos] == ' ') ++pos;
        size_t line_start = pos;
        size_t line_end = std::min(value.size(), line_start + (size_t)w);
        size_t split = line_end;
        if (line_end < value.size()) {
            for (size_t i = line_end; i > line_start; --i) {
                if (value[i - 1] == ' ') {
                    split = i - 1;
                    break;
                }
            }
        }
        if (split == line_start) split = line_end;
        draw_text(x, y++, value.substr(line_start, split - line_start), a.style.text_dim, a.style.background, false, w);
        pos = split;
    }
    a.cursor_y = y;
}

void separator() {
    using namespace detail;
    App& a = app();
    draw_hline(a.content_x, a.cursor_y, a.content_w, '-', a.style.border, a.style.background);
    a.cursor_y += 1;
}

void spacing(int rows) {
    detail::app().cursor_y += std::max(0, rows);
}

void spacing(float rows) {
    detail::app().cursor_y += std::max(0, (int)std::round(rows));
}

static bool button_impl(const char* label, bool has_color, Color custom_color) {
    using namespace detail;
    App& a = app();
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    int natural_w = std::min(a.content_w, std::max(12, (int)name.size() + 6));
    Rect r{a.content_x, a.cursor_y, consume_width(natural_w), 3};
    bool focused = register_focus(id, r);
    bool mouse_hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    bool pressed = !disabled() && ((focused && (a.in.enter || a.in.space)) || (mouse_hit && a.in.mouse_released));
    Motion& m = motion(id);
    animate_to(m.active, (focused || mouse_hit) ? 1.0f : 0.0f);
    Color base = has_color ? custom_color : consume_role_color(ColorRole::Button);
    Color active = has_color ? lerp(custom_color, a.style.text, 0.22f) : role_color(ColorRole::ButtonActive);
    float lit = std::max({m.focus, m.active, m.hover * 0.7f});
    Color hover_base = has_color ? lerp(base, a.style.text, 0.12f) : role_color(ColorRole::ButtonHover);
    Color bg = lerp(lerp(base, hover_base, m.hover), active, lit * 0.65f);
    Color border = lerp(role_color(ColorRole::Border), role_color(ColorRole::Accent), m.focus);
    draw_box(r, border, bg);
    std::string face = " " + name + " ";
    int tx = r.x + std::max(1, (r.w - (int)face.size()) / 2);
    Color fg = disabled() ? a.style.text_dim : a.style.text;
    draw_text(tx, r.y + 1, face, fg, bg, focused, r.w - 2);
    if (focused) {
        char spinner[] = {'-', '\\', '|', '/'};
        int frame = (int)(animation_time() * 8.0f) & 3;
        put(r.x + 1, r.y + 1, spinner[frame], a.style.accent, bg, true);
    }
    a.cursor_y += 4;
    return pressed;
}

bool button(const char* label) {
    return button_impl(label, false, {});
}

bool button(const char* label, ColorRole role) {
    return button_impl(label, true, detail::role_color(role));
}

bool button(const char* label, Color color) {
    return button_impl(label, true, color);
}

bool input(const char* label, char* buffer, int buffer_size, InputFlags flags, bool* enter_pressed) {
    using namespace detail;
    App& a = app();
    if (enter_pressed) *enter_pressed = false;
    if (!buffer || buffer_size <= 0) return false;
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    int label_w = std::min((int)name.size(), std::max(8, a.content_w / 3));
    Rect r{a.content_x, a.cursor_y + 1, consume_width(a.content_w), 3};
    draw_text(a.content_x, a.cursor_y, name, a.style.text_dim, a.style.background, false, a.content_w);
    bool focused = register_focus(id, r);
    if (focused) a.text_focus_id = (int)id;
    bool changed = false;
    if (focused && !disabled() && !(flags & InputFlags::ReadOnly)) {
        int len = (int)std::strlen(buffer);
        if (a.in.backspace && len > 0) {
            buffer[len - 1] = '\0';
            changed = true;
        }
        for (int i = 0; i < a.in.text_count; ++i) {
            char ch = a.in.text[i];
            if (!accept_char(ch, flags)) continue;
            len = (int)std::strlen(buffer);
            if (len < buffer_size - 1) {
                buffer[len] = ch;
                buffer[len + 1] = '\0';
                changed = true;
            }
        }
        if (a.in.enter && enter_pressed) *enter_pressed = true;
    }
    Motion& m = motion(id);
    animate_to(m.focus, focused ? 1.0f : 0.0f);
    Color border = lerp(a.style.border, a.style.input_focus, std::max(m.focus, m.hover * 0.55f));
    Color bg = lerp(a.style.input_bg, a.style.panel_alt, std::max(m.focus * 0.45f, m.hover * 0.28f));
    draw_box(r, border, bg);
    std::string visible;
    if (flags & InputFlags::Password) visible.assign(std::strlen(buffer), '*');
    else visible = buffer;
    int max_text = std::max(1, r.w - 4);
    if ((int)visible.size() > max_text) visible = visible.substr(visible.size() - (size_t)max_text);
    draw_text(r.x + 2, r.y + 1, visible, a.style.text, bg, false, max_text);
    if (focused && (int)visible.size() < max_text) {
        put(r.x + 2 + (int)visible.size(), r.y + 1, '_', a.style.accent, bg, true);
    }
    (void)label_w;
    a.cursor_y += 5;
    return changed;
}

static std::vector<std::string> split_lines_simple(const char* text) {
    std::vector<std::string> lines;
    std::string cur;
    for (const char* p = text ? text : ""; *p; ++p) {
        if (*p == '\n') {
            lines.push_back(cur);
            cur.clear();
        } else {
            cur.push_back(*p);
        }
    }
    lines.push_back(cur);
    return lines;
}

bool text_area(const char* label, char* buffer, int buffer_size, int rows) {
    return text_area_ex(label, buffer, buffer_size, rows, TextAreaFlags::Default);
}

bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows, TextAreaFlags flags) {
    using namespace detail;
    App& a = app();
    if (!buffer || buffer_size <= 0) return false;
    rows = std::max(1, rows);
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    draw_text(a.content_x, a.cursor_y, name, a.style.text_dim, a.style.background, false, a.content_w);
    Rect r{a.content_x, a.cursor_y + 1, consume_width(a.content_w), rows + 2};
    bool focused = register_focus(id, r);
    if (focused) a.text_focus_id = (int)id;
    bool hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    if (focused || hit) a.active_scroll_id = (int)id;
    bool changed = false;
    if (focused && !disabled() && !(flags & TextAreaFlags::ReadOnly)) {
        int len = (int)std::strlen(buffer);
        if (a.in.backspace && len > 0) {
            buffer[len - 1] = '\0';
            changed = true;
        }
        if (a.in.enter && len < buffer_size - 1) {
            buffer[len++] = '\n';
            buffer[len] = '\0';
            changed = true;
        }
        for (int i = 0; i < a.in.text_count; ++i) {
            len = (int)std::strlen(buffer);
            if (len < buffer_size - 1) {
                buffer[len] = a.in.text[i];
                buffer[len + 1] = '\0';
                changed = true;
            }
        }
    }
    Motion& m = motion(id);
    Color border = lerp(a.style.border, a.style.input_focus, std::max(m.focus, m.hover * 0.55f));
    Color bg = lerp(a.style.input_bg, a.style.panel_alt, std::max(m.focus * 0.35f, m.hover * 0.22f));
    draw_box(r, border, bg);
    std::vector<std::string> lines = split_lines_simple(buffer);
    ScrollState& ss = scroll_state(id);
    int max_scroll = std::max(0, (int)lines.size() - rows);
    if (hit && a.in.wheel != 0) {
        ss.target += (float)a.in.wheel * 3.0f;
        consume_wheel();
    }
    if (focused && a.in.up) ss.target -= 1.0f;
    if (focused && a.in.down) ss.target += 1.0f;
    ss.target = clamp_f(ss.target, 0.0f, (float)max_scroll);
    ease_to(ss.current, ss.target, 0.22f);
    int first = clamp_i((int)std::round(ss.current), 0, max_scroll);
    int max_w = std::max(1, r.w - 4);
    for (int i = 0; i < rows && first + i < (int)lines.size(); ++i) {
        std::string line = lines[(size_t)(first + i)];
        if ((int)line.size() > max_w) line = line.substr(line.size() - (size_t)max_w);
        draw_text(r.x + 2, r.y + 1 + i, line, a.style.text, bg, false, max_w);
    }
    if (focused) put(r.x + r.w - 2, r.y, '*', a.style.accent, bg, true);
    a.cursor_y += rows + 4;
    return changed;
}

void log_view(const char* label, const char* text_value, int rows, LogViewFlags flags) {
    using namespace detail;
    App& a = app();
    rows = std::max(1, rows);
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    draw_text(a.content_x, a.cursor_y, name, a.style.text_dim, a.style.background, false, a.content_w);
    Rect r{a.content_x, a.cursor_y + 1, consume_width(a.content_w), rows + 2};
    bool focused = register_focus(id, r);
    std::vector<std::string> lines = split_lines_simple(text_value);
    ScrollState& ss = scroll_state(id);
    int max_scroll = std::max(0, (int)lines.size() - rows);
    bool hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    if (focused || hit) a.active_scroll_id = (int)id;
    bool manual_scroll = (focused || hit) && (a.in.wheel != 0 || a.in.up || a.in.down);
    if ((flags & LogViewFlags::AutoScrollBottom) && !manual_scroll) ss.target = (float)max_scroll;
    if (hit && a.in.wheel != 0) {
        ss.target += (float)a.in.wheel * 3.0f;
        consume_wheel();
    }
    if (focused && a.in.up) ss.target -= 1.0f;
    if (focused && a.in.down) ss.target += 1.0f;
    ss.target = clamp_f(ss.target, 0.0f, (float)max_scroll);
    ease_to(ss.current, ss.target, 0.22f);
    int first = clamp_i((int)std::round(ss.current), 0, max_scroll);
    Color bg = a.style.panel;
    draw_box(r, a.style.border, bg);
    int max_w = std::max(1, r.w - 4);
    for (int i = 0; i < rows && first + i < (int)lines.size(); ++i) {
        draw_text(r.x + 2, r.y + 1 + i, lines[(size_t)(first + i)], a.style.text_dim, bg, false, max_w);
    }
    if (max_scroll > 0) {
        char pct[32];
        std::snprintf(pct, sizeof(pct), "%d/%d", first, max_scroll);
        draw_text(r.x + r.w - (int)std::strlen(pct) - 2, r.y, pct, a.style.text_dim, bg, false);
    }
    a.cursor_y += rows + 4;
}

bool checkbox(const char* label, bool* value) {
    using namespace detail;
    App& a = app();
    if (!value) return false;
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    Rect r{a.content_x, a.cursor_y, std::min(a.content_w, (int)name.size() + 8), 1};
    bool focused = register_focus(id, r);
    bool hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    bool changed = false;
    if (!disabled() && ((focused && (a.in.enter || a.in.space)) || (hit && a.in.mouse_released))) {
        *value = !*value;
        changed = true;
    }
    Color bg = focused ? a.style.panel_alt : a.style.background;
    fill_rect(r, bg);
    std::string face = std::string("[") + (*value ? "x" : " ") + "] " + name;
    draw_text(r.x, r.y, face, *value ? a.style.accent : a.style.text, bg, focused, r.w);
    a.cursor_y += 2;
    return changed;
}

bool slider_float(const char* label, float* value, float min_v, float max_v) {
    using namespace detail;
    App& a = app();
    if (!value) return false;
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    Rect r{a.content_x, a.cursor_y, consume_width(a.content_w), 2};
    bool focused = register_focus(id, r);
    bool hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    float old = *value;
    if (!disabled() && focused) {
        float step = (max_v - min_v) / 100.0f;
        if (step <= 0.0f) step = 0.01f;
        if (a.in.left) *value -= step;
        if (a.in.right) *value += step;
    }
    int bar_w = std::max(8, r.w - (int)name.size() - 12);
    int bx = r.x + (int)name.size() + 2;
    if (!disabled() && hit && (a.in.mouse_pressed || a.mouse_down)) {
        a.focused = (int)id;
        float t_mouse = bar_w <= 1 ? 0.0f : (float)(a.in.mouse_x - bx) / (float)(bar_w - 1);
        *value = min_v + clamp_f(t_mouse, 0.0f, 1.0f) * (max_v - min_v);
    }
    *value = clamp_f(*value, min_v, max_v);
    bool changed = std::abs(*value - old) > 0.0001f;
    float t = max_v == min_v ? 0.0f : (*value - min_v) / (max_v - min_v);
    int handle = clamp_i((int)std::round(t * (bar_w - 1)), 0, std::max(0, bar_w - 1));
    char val[32];
    std::snprintf(val, sizeof(val), " %.2f", *value);
    Color label_color = hit ? a.style.text : a.style.text_dim;
    draw_text(r.x, r.y, name, label_color, a.style.background, focused || hit, r.w);
    for (int i = 0; i < bar_w; ++i) {
        Color rail = hit ? lerp(a.style.border, a.style.accent, 0.35f) : a.style.border;
        char ch = i == handle ? 'O' : (i < handle ? '<' : '-');
        Color fg = i == handle ? a.style.accent : rail;
        put(bx + i, r.y, ch, fg, a.style.background, focused || hit);
    }
    draw_text(bx + bar_w + 1, r.y, val, a.style.text, a.style.background, false, r.w - bar_w - 1);
    a.cursor_y += 2;
    return changed;
}

bool tabs(const char* const* labels, int count, int* selected) {
    using namespace detail;
    App& a = app();
    if (!labels || count <= 0 || !selected) return false;
    *selected = clamp_i(*selected, 0, count - 1);
    unsigned id = hash_label("##ftti_tabs");
    for (int i = 0; i < count; ++i) id = (id ^ hash_label(labels[i])) * 16777619u;
    id ^= (unsigned)(a.cursor_x * 131 + a.cursor_y * 313);
    Rect r{a.content_x, a.cursor_y, a.content_w, 3};
    bool focused = register_focus(id, r);
    bool bar_hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    if (focused || bar_hit) a.active_tabs_id = (int)id;
    TabState& ts = tab_state(id, *selected);
    ts.target = (float)*selected;
    ease_to(ts.current, ts.target, 0.24f);
    bool changed = false;
    if (!disabled() && a.tab_command_delta != 0 && a.active_tabs_id == (int)id) {
        int next = clamp_i(*selected + a.tab_command_delta, 0, count - 1);
        changed = changed || next != *selected;
        *selected = next;
        a.tab_command_delta = 0;
    }
    if (!disabled() && focused && a.in.left && *selected > 0) {
        --*selected;
        changed = true;
    }
    if (!disabled() && focused && a.in.right && *selected < count - 1) {
        ++*selected;
        changed = true;
    }
    if (!disabled() && focused && (a.in.enter || a.in.space)) {
        *selected = (*selected + 1) % count;
        changed = true;
    }

    int x = r.x;
    std::vector<int> tab_xs;
    std::vector<int> tab_ws;
    for (int i = 0; i < count; ++i) {
        std::string name = visible_label(labels[i]);
        int w = (int)name.size() + 4;
        tab_xs.push_back(x);
        tab_ws.push_back(w);
        Rect tr{x, r.y, w, 3};
        bool hit = contains(tr, a.in.mouse_x, a.in.mouse_y);
        if (!disabled() && hit && (a.in.mouse_pressed || a.in.mouse_released) && *selected != i) {
            *selected = i;
            changed = true;
            a.focused = (int)id;
        }
        bool active = i == *selected;
        Color bg = active ? a.style.panel_alt : (hit ? a.style.button_hover : a.style.background);
        Color fg = active || hit ? a.style.text : a.style.text_dim;
        fill_rect(tr, bg);
        draw_text(x + 2, r.y + 1, name, fg, bg, active, w - 4);
        x += w + 1;
        if (x >= r.x + r.w) break;
    }
    int underline_x = r.x;
    int underline_w = 0;
    if (!tab_xs.empty()) {
        float cur = clamp_f(ts.current, 0.0f, (float)tab_xs.size() - 1.0f);
        int lo = clamp_i((int)std::floor(cur), 0, (int)tab_xs.size() - 1);
        int hi = clamp_i((int)std::ceil(cur), 0, (int)tab_xs.size() - 1);
        float frac = cur - (float)lo;
        underline_x = (int)std::round((float)tab_xs[(size_t)lo] + ((float)tab_xs[(size_t)hi] - (float)tab_xs[(size_t)lo]) * frac);
        underline_w = (int)std::round((float)tab_ws[(size_t)lo] + ((float)tab_ws[(size_t)hi] - (float)tab_ws[(size_t)lo]) * frac);
    }
    if (underline_w > 2) draw_hline(underline_x + 1, r.y + 2, underline_w - 2, '=', a.style.accent, a.style.background);
    if (focused) put(r.x, r.y + 1, '>', a.style.accent, a.style.background, true);
    a.cursor_y += 4;
    return changed;
}

bool side_menu(const char* label, const char* const* items, int count, int* selected) {
    using namespace detail;
    App& a = app();
    if (!items || count <= 0 || !selected) return false;
    *selected = clamp_i(*selected, 0, count - 1);
    unsigned id = hash_label(label ? label : "##ftti_side_menu");
    int width = 18;
    for (int i = 0; i < count; ++i) width = std::max(width, (int)visible_label(items[i]).size() + 6);
    width = std::min(width, std::max(18, a.width / 3));
    Rect panel{0, 2, width, a.height - 2};
    fill_rect(panel, a.style.panel);
    draw_hline(panel.x, panel.y, panel.w, '-', a.style.border, a.style.panel);
    std::string title = visible_label(label);
    draw_text(2, 3, title, a.style.text, a.style.panel, true, width - 4);
    Rect focus_rect{panel.x, panel.y, panel.w, panel.h};
    bool focused = register_focus(id, focus_rect);
    bool changed = false;
    if (!disabled() && focused && a.in.up && *selected > 0) {
        --*selected;
        changed = true;
    }
    if (!disabled() && focused && a.in.down && *selected < count - 1) {
        ++*selected;
        changed = true;
    }
    int y = 5;
    for (int i = 0; i < count && y < a.height - 1; ++i, ++y) {
        Rect item{1, y, width - 2, 1};
        bool hit = contains(item, a.in.mouse_x, a.in.mouse_y);
        if (!disabled() && hit && a.in.mouse_released && *selected != i) {
            *selected = i;
            changed = true;
            a.focused = (int)id;
        }
        bool active = i == *selected;
        Color bg = active ? a.style.panel_alt : a.style.panel;
        Color fg = active ? a.style.text : a.style.text_dim;
        fill_rect(item, bg);
        put(2, y, active ? '>' : ' ', active ? a.style.accent : fg, bg, active);
        draw_text(4, y, visible_label(items[i]), fg, bg, active, width - 6);
    }
    if (focused) put(width - 2, 3, '*', a.style.accent, a.style.panel, true);
    a.content_x = width + 2;
    a.cursor_x = a.content_x;
    a.cursor_y = 3 - (int)std::round(a.page_scroll_current);
    a.content_w = std::max(10, a.width - a.content_x - 2);
    return changed;
}

bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows) {
    using namespace detail;
    App& a = app();
    if (!items || count <= 0 || !selected) return false;
    *selected = clamp_i(*selected, 0, count - 1);
    visible_rows = clamp_i(visible_rows, 1, count);
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    draw_text(a.content_x, a.cursor_y, name, a.style.text_dim, a.style.background, false, a.content_w);
    Rect r{a.content_x, a.cursor_y + 1, consume_width(a.content_w), visible_rows + 2};
    bool focused = register_focus(id, r);
    bool changed = false;
    bool box_hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    if (focused || box_hit) a.active_scroll_id = (int)id;
    if (!disabled() && (focused || box_hit)) {
        if (a.in.up && *selected > 0) { --*selected; changed = true; }
        if (a.in.down && *selected < count - 1) { ++*selected; changed = true; }
        if (a.in.wheel != 0) {
            int next = clamp_i(*selected + a.in.wheel, 0, count - 1);
            changed = changed || next != *selected;
            *selected = next;
            consume_wheel();
        }
    }
    int first = clamp_i(*selected - visible_rows / 2, 0, std::max(0, count - visible_rows));
    draw_box(r, focused ? a.style.accent : a.style.border, a.style.panel);
    for (int i = 0; i < visible_rows; ++i) {
        int item_i = first + i;
        Rect item{r.x + 1, r.y + 1 + i, r.w - 2, 1};
        bool hit = contains(item, a.in.mouse_x, a.in.mouse_y);
        if (!disabled() && hit && a.in.mouse_released && *selected != item_i) {
            *selected = item_i;
            a.focused = (int)id;
            changed = true;
        }
        bool active = item_i == *selected;
        Color bg = active ? a.style.panel_alt : a.style.panel;
        fill_rect(item, bg);
        put(item.x + 1, item.y, active ? '>' : ' ', active ? a.style.accent : a.style.text_dim, bg, active);
        draw_text(item.x + 3, item.y, visible_label(items[item_i]), active ? a.style.text : a.style.text_dim, bg, active, item.w - 4);
    }
    a.cursor_y += visible_rows + 4;
    return changed;
}

bool dropdown(const char* label, const char* const* items, int count, int* selected, int popup_rows) {
    using namespace detail;
    App& a = app();
    if (!items || count <= 0 || !selected) return false;
    *selected = clamp_i(*selected, 0, count - 1);
    std::string name = visible_label(label);
    std::string current = visible_label(items[*selected]);
    unsigned id = hash_label(label);
    Rect r{a.content_x, a.cursor_y, consume_width(a.content_w), 3};
    bool focused = register_focus(id, r);
    bool changed = false;
    if (!disabled() && focused) {
        if ((a.in.down || a.in.enter || a.in.space) && *selected < count - 1) { ++*selected; changed = true; }
        if (a.in.up && *selected > 0) { --*selected; changed = true; }
    }
    draw_box(r, focused ? a.style.accent : a.style.border, a.style.panel);
    std::string line = name + ": " + current;
    draw_text(r.x + 2, r.y + 1, line, a.style.text, a.style.panel, focused, r.w - 4);
    if (focused) {
        int rows = std::min(count, popup_rows);
        int first = clamp_i(*selected - rows / 2, 0, std::max(0, count - rows));
        Rect pop{r.x + 2, r.y + 3, std::min(r.w - 4, 32), rows + 2};
        if (pop.y + pop.h >= a.height) pop.y = std::max(2, r.y - pop.h);
        draw_box(pop, a.style.accent, a.style.panel_alt);
        for (int i = 0; i < rows; ++i) {
            int idx = first + i;
            bool active = idx == *selected;
            Color bg = active ? a.style.button_active : a.style.panel_alt;
            Rect item{pop.x + 1, pop.y + 1 + i, pop.w - 2, 1};
            fill_rect(item, bg);
            draw_text(item.x + 1, item.y, visible_label(items[idx]), active ? a.style.text : a.style.text_dim, bg, active, item.w - 2);
        }
    }
    a.cursor_y += 4;
    return changed;
}

bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns) {
    (void)columns;
    using namespace detail;
    App& a = app();
    if (!items || count <= 0 || !selected) return false;
    *selected = clamp_i(*selected, 0, count - 1);
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    draw_text(a.content_x, a.cursor_y, name, a.style.text_dim, a.style.background, false, a.content_w);
    Rect r{a.content_x, a.cursor_y + 1, consume_width(a.content_w), count};
    bool focused = register_focus(id, r);
    bool changed = false;
    if (!disabled() && focused) {
        if (a.in.up && *selected > 0) { --*selected; changed = true; }
        if (a.in.down && *selected < count - 1) { ++*selected; changed = true; }
    }
    for (int i = 0; i < count; ++i) {
        Rect item{r.x, r.y + i, r.w, 1};
        bool hit = contains(item, a.in.mouse_x, a.in.mouse_y);
        if (!disabled() && hit && a.in.mouse_released && *selected != i) {
            *selected = i;
            a.focused = (int)id;
            changed = true;
        }
        bool active = i == *selected;
        Color bg = active ? a.style.panel_alt : a.style.background;
        fill_rect(item, bg);
        std::string line = std::string(active ? "(o) " : "( ) ") + visible_label(items[i]);
        draw_text(item.x, item.y, line, active ? a.style.accent : a.style.text_dim, bg, active, item.w);
    }
    a.cursor_y += count + 2;
    return changed;
}

bool collapsing_header(const char* label, bool* open) {
    using namespace detail;
    App& a = app();
    static bool fallback_open = true;
    bool* state = open ? open : &fallback_open;
    std::string name = visible_label(label);
    unsigned id = hash_label(label);
    Rect r{a.content_x, a.cursor_y, std::min(a.content_w, (int)name.size() + 6), 1};
    bool focused = register_focus(id, r);
    bool hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    if (!disabled() && ((focused && (a.in.enter || a.in.space)) || (hit && a.in.mouse_released))) {
        *state = !*state;
    }
    Color bg = focused ? a.style.panel_alt : a.style.background;
    fill_rect(r, bg);
    std::string line = std::string(*state ? "v " : "> ") + name;
    draw_text(r.x, r.y, line, a.style.text, bg, true, r.w);
    a.cursor_y += 2;
    return *state;
}

void row(int cols, std::function<void()> fn) {
    (void)cols;
    if (fn) fn();
}

void row(std::initializer_list<float> weights, std::function<void()> fn) {
    (void)weights;
    if (fn) fn();
}

void split(std::initializer_list<float> columns, std::function<void()> fn) {
    (void)columns;
    if (fn) fn();
}

void side_layout(float side_width, std::function<void()> fn) {
    detail::App& a = detail::app();
    a.content_x = std::max(2, (int)side_width + 2);
    a.cursor_x = a.content_x;
    a.content_w = std::max(10, a.width - a.content_x - 2);
    if (fn) fn();
}

void content(std::function<void()> fn) {
    if (fn) fn();
}

void scroll_area(const char* label, float height, std::function<void()> fn) {
    using namespace detail;
    App& a = app();
    unsigned id = hash_label(label);
    int rows = std::max(1, (int)height);
    if (height > 24.0f) rows = std::max(1, (int)(height / 24.0f));
    Rect r{a.content_x, a.cursor_y, a.content_w, std::min(rows, std::max(1, a.height - a.cursor_y - 1))};
    bool focused = register_focus(id, r);
    ScrollState& ss = scroll_state(id);
    bool hit = contains(r, a.in.mouse_x, a.in.mouse_y);
    if (focused || hit) a.active_scroll_id = (int)id;
    float previous_max_scroll = std::max(0.0f, ss.content - (float)std::max(0, r.h - 2));
    if (hit && a.in.wheel != 0) {
        ss.target += (float)a.in.wheel * 2.0f;
        consume_wheel();
    }
    if (focused && a.in.up) ss.target -= 1.0f;
    if (focused && a.in.down) ss.target += 1.0f;
    ss.target = clamp_f(ss.target, 0.0f, previous_max_scroll);
    ease_to(ss.current, ss.target, 0.20f);
    draw_box({r.x, r.y, r.w, r.h}, focused ? a.style.accent : a.style.border, a.style.panel);
    int old_x = a.content_x;
    int old_y = a.cursor_y;
    int old_w = a.content_w;
    bool old_clip_active = a.clip_active;
    Rect old_clip = a.clip_rect;
    a.content_x = r.x + 2;
    a.cursor_y = r.y + 1 - (int)std::round(ss.current);
    a.content_w = std::max(4, r.w - 4);
    a.clip_active = true;
    a.clip_rect = {r.x + 1, r.y + 1, std::max(0, r.w - 2), std::max(0, r.h - 2)};
    a.scroll_scopes.push_back(ScrollScope{id, a.clip_rect});
    if (fn) fn();
    if (!a.scroll_scopes.empty()) a.scroll_scopes.pop_back();
    a.clip_active = old_clip_active;
    a.clip_rect = old_clip;
    int used = a.cursor_y + (int)std::round(ss.current) - (r.y + 1);
    ss.content = (float)std::max(0, used);
    float max_scroll = std::max(0.0f, ss.content - (float)std::max(0, r.h - 2));
    ss.target = clamp_f(ss.target, 0.0f, max_scroll);
    ss.current = clamp_f(ss.current, 0.0f, max_scroll);
    if (max_scroll > 0.0f) {
        char pct[32];
        std::snprintf(pct, sizeof(pct), "%d/%d", (int)std::round(ss.current), (int)std::round(max_scroll));
        draw_text(r.x + r.w - (int)std::strlen(pct) - 2, r.y, pct, a.style.text_dim, a.style.panel, false);
    }
    a.content_x = old_x;
    a.cursor_y = old_y + r.h + 1;
    a.content_w = old_w;
}

void set_next_width(float cols) {
    detail::app().next_width = std::max(0, (int)cols);
}

void set_next_fill() {
    detail::app().next_fill = true;
}

void set_next_percent(float pct) {
    detail::app().next_percent = pct;
}

void set_next_limits(float min_cols, float max_cols) {
    detail::App& a = detail::app();
    a.next_min = std::max(0, (int)min_cols);
    a.next_max = std::max(0, (int)max_cols);
}

void set_next_align(Align align) {
    detail::app().next_align = align;
}

void open_modal(const char* label) {
    detail::app().modal_id = (int)detail::hash_label(label);
}

bool modal(const char* label, std::function<void()> fn) {
    using namespace detail;
    App& a = app();
    int id = (int)hash_label(label);
    if (a.modal_id != id) return false;
    Rect r{std::max(2, a.width / 8), std::max(2, a.height / 5), std::max(20, a.width * 3 / 4), std::max(8, a.height * 3 / 5)};
    fill_rect({0, 2, a.width, a.height - 2}, lerp(a.style.background, a.style.panel, 0.45f));
    draw_box(r, a.style.accent, a.style.panel);
    draw_text(r.x + 2, r.y, visible_label(label), a.style.text, a.style.panel, true, r.w - 4);
    int old_x = a.content_x;
    int old_y = a.cursor_y;
    int old_w = a.content_w;
    a.content_x = r.x + 2;
    a.cursor_y = r.y + 2;
    a.content_w = std::max(4, r.w - 4);
    if (fn) fn();
    if (a.close_modal_requested || a.in.escape) {
        a.modal_id = 0;
        a.close_modal_requested = false;
    }
    a.content_x = old_x;
    a.cursor_y = old_y;
    a.content_w = old_w;
    return true;
}

void close_modal() {
    detail::app().close_modal_requested = true;
}

void begin_disabled() {
    ++detail::app().disabled_depth;
}

void end_disabled() {
    detail::App& a = detail::app();
    if (a.disabled_depth > 0) --a.disabled_depth;
}

void tooltip(const char* text_value) {
    detail::App& a = detail::app();
    if (!text_value || !*text_value || !a.last_item_id) return;
    if (a.hovered == (int)a.last_item_id || a.focused == (int)a.last_item_id) {
        a.pending_tooltip = text_value;
        a.pending_tooltip_anchor = a.last_item_rect;
        a.pending_tooltip_owner = a.last_item_id;
    }
}

void request_focus(const char* label) {
    detail::app().request_focus_id = (int)detail::hash_label(label);
}

float calc_text_width(const char* text_value) {
    return (float)std::strlen(text_value ? text_value : "");
}

float calc_text_height(const char* text_value, float wrap_width) {
    int w = std::max(1, (int)wrap_width);
    int n = (int)std::strlen(text_value ? text_value : "");
    return (float)std::max(1, (n + w - 1) / w);
}

void toast(const Toast& value) {
    detail::app().toasts.push_back({value, std::chrono::steady_clock::now()});
}

void toast(const char* message) {
    toast(Toast{message, ToastType::Info, 3500, true});
}

void toast_info(const char* message) {
    toast(Toast{message, ToastType::Info, 3500, true});
}

void toast_success(const char* message) {
    toast(Toast{message, ToastType::Success, 3500, true});
}

void toast_warning(const char* message) {
    toast(Toast{message, ToastType::Warning, 3500, true});
}

void toast_error(const char* message) {
    toast(Toast{message, ToastType::Error, 3500, true});
}

void clear_toasts() {
    detail::app().toasts.clear();
}

void progress_bar(float progress) {
    progress_bar(progress, ProgressStyle{});
}

void progress_bar(float progress, const char* label_or_mask_path) {
    ProgressStyle style;
    style.label = label_or_mask_path;
    style.mask_path = label_or_mask_path;
    progress_bar(progress, style);
}

void progress_bar(float progress, const ProgressStyle& style) {
    using namespace detail;
    App& a = app();
    progress = clamp_f(progress, 0.0f, 1.0f);
    int w = consume_width(a.content_w);
    Rect r{a.content_x, a.cursor_y, w, 1};
    int fill = clamp_i((int)std::round(progress * w), 0, w);
    Color fill_color = style.fill_color.r >= 0.0f ? style.fill_color : role_color(style.fill_role);
    for (int i = 0; i < w; ++i) put(r.x + i, r.y, i < fill ? '=' : '-', i < fill ? fill_color : a.style.border, a.style.background, true);
    if (style.label) draw_text(r.x, r.y, style.label, a.style.text, a.style.background, true, w);
    if (style.show_percent && w > 8) {
        char pct[16];
        std::snprintf(pct, sizeof(pct), "%3d%%", (int)std::round(progress * 100.0f));
        draw_text(r.x + w - 5, r.y, pct, a.style.text, a.style.background, true, 5);
    }
    a.cursor_y += 2;
}

void image(ImageHandle* img, float width, float height) {
    using namespace detail;
    App& a = app();
    int cols = std::max(4, (int)std::round(width));
    int rows = std::max(2, (int)std::round(height));
    if (width > 80.0f) cols = std::max(4, (int)std::round(width / 8.0f));
    if (height > 24.0f) rows = std::max(2, (int)std::round(height / 16.0f));
    cols = std::min(cols, a.content_w);
    rows = std::min(rows, std::max(2, a.height - a.cursor_y - 1));
    Rect r{a.content_x, a.cursor_y, cols, rows};
    draw_box(r, a.style.border, a.style.panel);

    ImageData* data = img ? static_cast<ImageData*>(img->_impl) : nullptr;
    if (data && !data->path.empty()) {
        std::string payload = base64_encode(data->path);
        char prefix[160];
        std::snprintf(prefix, sizeof(prefix),
                      "\x1b[%d;%dH\x1b_Ga=T,f=100,t=f,q=2,i=%u,c=%d,r=%d;",
                      r.y + 1, r.x + 1, data->id, cols, rows);
        a.graphics_commands.push_back(std::string(prefix) + payload + "\x1b\\");
    } else {
        draw_text(r.x + 2, r.y + rows / 2, "[image]", a.style.text_dim, a.style.panel, false, r.w - 4);
    }
    a.cursor_y += rows + 1;
}

ImageHandle* load_image(const char* utf8_path) {
    if (!utf8_path || !*utf8_path) return nullptr;
    static unsigned next_id = 1000;
    ImageHandle* h = new ImageHandle;
    detail::ImageData* data = new detail::ImageData;
    data->path = utf8_path;
    data->id = next_id++;
    h->_impl = data;
    return h;
}

void free_image(ImageHandle* img) {
    if (!img) return;
    delete static_cast<detail::ImageData*>(img->_impl);
    delete img;
}

std::string open_file_dialog(const char* title, const FileFilter* filters, int filter_count) {
    (void)title;
    (void)filters;
    (void)filter_count;
    return "";
}

void open_child_window(const Config& cfg, std::function<void()> fn) {
    (void)cfg;
    if (fn) fn();
}

DebugState& debug() {
    return detail::app().dbg;
}

float animation_time() {
    using namespace std::chrono;
    detail::App& a = detail::app();
    return duration_cast<duration<float>>(steady_clock::now() - a.start_time).count();
}

int terminal_width() {
    return detail::app().width;
}

int terminal_height() {
    return detail::app().height;
}

} // namespace ftti

#endif // FTTI_IMPLEMENTATION


// ============================================================
// Embedded ftht.hpp
// ============================================================

// ftht.hpp - Fuck This Hyper Text
// Single-header immediate-mode HTML UI for tiny devices and desktop tools.
//
// Usage: #define FTHT_IMPLEMENTATION in exactly one .cpp before including.
//
// Windows: cl main.cpp /std:c++17
//          (MSVC auto-links ws2_32.lib)
// Linux:   g++ main.cpp -o app -DFTHT_IMPLEMENTATION -std=c++17
// ESP32 Arduino: include after WiFi is connected, then call ftht::create_server().

#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <cstdarg>
#include <ctime>
#include <string>
#include <string_view>
#include <vector>

#ifndef FTHT_VERSION_MAJOR
#define FTHT_VERSION_MAJOR 0
#endif
#ifndef FTHT_VERSION_MINOR
#define FTHT_VERSION_MINOR 1
#endif
#ifndef FTHT_VERSION_PATCH
#define FTHT_VERSION_PATCH 0
#endif

namespace ftht {

enum class Align { Start, Center, End };

struct Color {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};

struct Style {
    Color background    = {0.055f, 0.055f, 0.059f, 1.0f};
    Color panel         = {0.090f, 0.094f, 0.102f, 1.0f};
    Color text          = {0.910f, 0.910f, 0.910f, 1.0f};
    Color text_dim      = {0.663f, 0.678f, 0.702f, 1.0f};
    Color border        = {0.169f, 0.176f, 0.192f, 1.0f};
    Color button        = {0.090f, 0.094f, 0.102f, 1.0f};
    Color button_hover  = {0.137f, 0.149f, 0.169f, 1.0f};
    Color button_active = {0.176f, 0.192f, 0.220f, 1.0f};
    Color input_bg      = {0.071f, 0.075f, 0.082f, 1.0f};
    Color input_focus   = {0.310f, 0.420f, 0.780f, 1.0f};
    Color accent        = {0.310f, 0.420f, 0.780f, 1.0f};
    Color warning       = {0.894f, 0.486f, 0.353f, 1.0f};
    Color success       = {0.420f, 0.741f, 0.522f, 1.0f};
    float window_padding = 20.0f;
    float item_spacing   = 10.0f;
    float item_height    = 36.0f;
    float rounding       = 8.0f;
    float border_width   = 1.0f;
    float font_size      = 16.0f;
};

enum class ColorRole {
    Background,
    Panel,
    Text,
    TextDim,
    Border,
    Button,
    ButtonHover,
    ButtonActive,
    InputBg,
    InputFocus,
    Accent,
    Warning,
    Success,
};

enum class LoginMode {
    None,
    DrawPenis,
    Form,
};

enum class ToastType {
    Info,
    Success,
    Warning,
    Error,
};

struct Toast {
    std::string title;
    std::string message;
    int duration_ms = 4000;
    bool dismissible = false;
    ToastType type = ToastType::Info;
};

Style default_dark_style();
Style one_dark_style();
void         set_style(const Style& s);
const Style& get_style();
Color        color_from_hex(const char* hex);
Color        color_from_hex(std::string_view hex);
void         push_color(ColorRole role, Color color);
void         pop_color();
void         set_next_color(ColorRole role, Color color);

struct Config {
    const char* title = "FTHT App";
    const char* host = "0.0.0.0";
    int         port = 8080;
    int         max_request_bytes = 32 * 1024;
    int         read_timeout_ms = 1500;
    int         client_poll_ms = 0;
    bool        auto_submit = true;
    bool        print_url = true;
    bool        debug_output = false;
    bool        client_debug_output = false;
    bool        dark_mode = false;
    bool        respect_browser_dark_mode = false;
    const char* extra_head_html = nullptr;
    LoginMode   login_mode = LoginMode::None;
    const char* login_username = "admin";
    const char* login_password = nullptr;
    const char* login_salt = "ftht-login";
    const char* login_db_path = nullptr;
    uint64_t    login_password_hash = 0;
    int         login_session_seconds = 10 * 60;
    bool        login_allow_registration = false;
};

bool create_server(const Config& cfg = {});
bool pump(int timeout_ms = -1);
void begin();
void end();
void shutdown();
uint64_t make_login_password_hash(const char* password, const char* salt = "ftht-login");

const Config& config();
const char* url();
const char* path();
const char* method();
bool is_post();
const char* param(const char* name);
void set_status(int code, const char* text);
using CommandHandler = bool (*)(const char* command);
void set_command_handler(CommandHandler handler);

void html(const char* markup);
void text(const char* label);
void text_wrapped(const char* text);
void separator();
void spacing(float px = 8.0f);
void toast(const char* message);
void toast(const Toast& toast);
void toast_info(const char* message);
void toast_success(const char* message);
void toast_warning(const char* message);
void toast_error(const char* message);
void clear_toasts();

enum class InputFlags : unsigned {
    Default = 0,
    Password = 1 << 0,
    ReadOnly = 1 << 1,
    CharsDecimal = 1 << 2,
    CharsHexadecimal = 1 << 3,
    CharsUppercase = 1 << 4,
    CharsNoBlank = 1 << 5,
};
inline InputFlags operator|(InputFlags a, InputFlags b) {
    return static_cast<InputFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(InputFlags a, InputFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class TextAreaFlags : unsigned {
    Default = 0,
    ReadOnly = 1 << 0,
    WordWrap = 1 << 1,
    AutoScrollBottom = 1 << 2,
};
inline TextAreaFlags operator|(TextAreaFlags a, TextAreaFlags b) {
    return static_cast<TextAreaFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(TextAreaFlags a, TextAreaFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

enum class LogViewFlags : unsigned {
    Default = 0,
    WordWrap = 1 << 0,
    AutoScrollBottom = 1 << 1,
};
inline LogViewFlags operator|(LogViewFlags a, LogViewFlags b) {
    return static_cast<LogViewFlags>(static_cast<unsigned>(a) | static_cast<unsigned>(b));
}
inline bool operator&(LogViewFlags a, LogViewFlags b) {
    return (static_cast<unsigned>(a) & static_cast<unsigned>(b)) != 0;
}

bool input(const char* label, char* buffer, int buffer_size,
           InputFlags flags = InputFlags::Default, bool* enter_pressed = nullptr);
bool text_area(const char* label, char* buffer, int buffer_size, int rows = 5);
bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows = 5,
                  TextAreaFlags flags = TextAreaFlags::Default);
void log_view(const char* label, const char* text, int rows = 8,
              LogViewFlags flags = LogViewFlags::AutoScrollBottom);

bool checkbox(const char* label, bool* value);
bool slider_float(const char* label, float* value, float min_v, float max_v);
bool button(const char* label);
bool button(const char* label, ColorRole role);
bool button(const char* label, Color color);
bool dropdown(const char* label, const char* const* items, int count, int* selected, int popup_rows = 8);
bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows = 6);
bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns = 1);
bool collapsing_header(const char* label, bool* open = nullptr);
bool side_menu(const char* label, const char* const* items, int count, int* selected);
bool side_menu_drawer(const char* label, const char* const* items, int count, int* selected, float width = 260.0f);
bool tabs(const char* const* labels, int count, int* selected);

void row(int cols, std::function<void()> fn);
void row(std::initializer_list<float> weights, std::function<void()> fn);
void split(std::initializer_list<float> columns, std::function<void()> fn);
void side_layout(float side_width, std::function<void()> fn);
void content(std::function<void()> fn);
void scroll_area(const char* label, float height, std::function<void()> fn);
void set_next_width(float px);
void set_next_fill();
void set_next_percent(float pct);
void set_next_limits(float min_px, float max_px);
void set_next_align(Align align);
void open_modal(const char* label);
bool modal(const char* label, std::function<void()> fn);
void close_modal();
void begin_disabled();
void end_disabled();
void tooltip(const char* text);
void request_focus(const char* label);
float calc_text_width(const char* text);
float calc_text_height(const char* text, float wrap_width);

} // namespace ftht

#ifdef FTHT_IMPLEMENTATION

#if defined(ARDUINO) && defined(ESP32)
#include <Arduino.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#elif defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#if defined(_MSC_VER)
#pragma comment(lib, "ws2_32.lib")
#endif
#else
#include <cerrno>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#endif

namespace ftht {
namespace internal {

struct Param {
    std::string key;
    std::string value;
};

struct ActiveToast {
    uint64_t id = 0;
    Toast toast;
    long long created_ms = 0;
};

struct NextLayoutState {
    bool has_width = false;
    bool fill = false;
    bool has_percent = false;
    bool has_limits = false;
    bool has_align = false;
    float width = 0.0f;
    float percent = 1.0f;
    float min_width = 0.0f;
    float max_width = 0.0f;
    Align align = Align::Start;
};

struct ColorOverride {
    ColorRole role = ColorRole::Text;
    Color color = {};
};

#if defined(ARDUINO) && defined(ESP32)
struct ServerHandle {
    WiFiServer* server = nullptr;
    WiFiClient client;
};
#elif defined(_WIN32)
using Socket = SOCKET;
static constexpr Socket invalid_socket = INVALID_SOCKET;
struct ServerHandle {
    Socket server = invalid_socket;
    Socket client = invalid_socket;
    bool wsa_started = false;
};
#else
using Socket = int;
static constexpr Socket invalid_socket = -1;
struct ServerHandle {
    Socket server = invalid_socket;
    Socket client = invalid_socket;
};
#endif

struct Context {
    Config cfg;
    Style style = one_dark_style();
    CommandHandler command_handler = nullptr;
    ServerHandle net;
    std::string current_url;
    std::string req_method;
    std::string req_path;
    std::string req_query;
    std::string req_body;
    std::string req_headers;
    std::string req_cookie;
    std::vector<Param> params;
    std::string out;
    std::string extra_response_headers;
    std::string login_session_token;
    std::time_t login_session_until = 0;
    std::vector<ActiveToast> toasts;
    std::vector<ColorOverride> color_stack;
    std::vector<ColorOverride> next_colors;
    uint64_t next_toast_id = 1;
    std::string modal_label;
    std::string last_widget_id;
    std::string command_message;
    int status_code = 200;
    std::string status_text = "OK";
    int disabled_depth = 0;
    bool request_active = false;
    bool response_open = false;
    bool modal_rendered = false;
    bool client_update = false;
    NextLayoutState next;
};

static Context& ctx() {
    static Context c;
    return c;
}

static void send_response(const std::string& body);
static void append_css();
static void append_ink_filters();
static void append_toast_stack();
static void append_toast_script();
static const char* find_param(const char* name);

static void debugf(const char* fmt, ...) {
    Context& c = ctx();
    if (!c.cfg.debug_output) return;
#if defined(ARDUINO) && defined(ESP32)
    char buf[256];
    va_list args;
    va_start(args, fmt);
    std::vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    Serial.print("[ftht] ");
    Serial.println(buf);
#else
    std::fprintf(stderr, "[ftht] ");
    va_list args;
    va_start(args, fmt);
    std::vfprintf(stderr, fmt, args);
    va_end(args);
    std::fprintf(stderr, "\n");
    std::fflush(stderr);
#endif
}

#if !defined(ARDUINO) || !defined(ESP32)
static int last_net_error() {
#if defined(_WIN32)
    return WSAGetLastError();
#else
    return errno;
#endif
}
#endif

static std::string visible_label(const char* label) {
    if (!label) return {};
    const char* p = std::strstr(label, "##");
    return p ? std::string(label, p) : std::string(label);
}

static uint32_t hash_label(const char* text) {
    const unsigned char* s = reinterpret_cast<const unsigned char*>(text ? text : "");
    uint32_t h = 2166136261u;
    while (*s) {
        h ^= (uint32_t)*s++;
        h *= 16777619u;
    }
    return h;
}

static std::string widget_id(const char* label, const char* salt = "") {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "w_%08x", hash_label((std::string(label ? label : "") + salt).c_str()));
    return std::string(buf);
}

static void append(std::string& s, const char* text) {
    if (text) s += text;
}

static std::string escape_html(std::string_view in) {
    std::string out;
    out.reserve(in.size());
    for (char ch : in) {
        switch (ch) {
            case '&': out += "&amp;"; break;
            case '<': out += "&lt;"; break;
            case '>': out += "&gt;"; break;
            case '"': out += "&quot;"; break;
            case '\'': out += "&#39;"; break;
            default: out += ch; break;
        }
    }
    return out;
}

static int hex_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

static std::string url_decode(std::string_view in) {
    std::string out;
    out.reserve(in.size());
    for (size_t i = 0; i < in.size(); ++i) {
        char c = in[i];
        if (c == '+') {
            out += ' ';
        } else if (c == '%' && i + 2 < in.size()) {
            int hi = hex_digit(in[i + 1]);
            int lo = hex_digit(in[i + 2]);
            if (hi >= 0 && lo >= 0) {
                out += (char)((hi << 4) | lo);
                i += 2;
            } else {
                out += c;
            }
        } else {
            out += c;
        }
    }
    return out;
}

static bool contains_header_token(std::string_view headers, const char* token) {
    std::string haystack(headers);
    std::string needle(token ? token : "");
    for (char& ch : haystack) ch = (char)std::tolower((unsigned char)ch);
    for (char& ch : needle) ch = (char)std::tolower((unsigned char)ch);
    return !needle.empty() && haystack.find(needle) != std::string::npos;
}

static std::string trim_copy(std::string_view in) {
    while (!in.empty() && std::isspace((unsigned char)in.front())) in.remove_prefix(1);
    while (!in.empty() && std::isspace((unsigned char)in.back())) in.remove_suffix(1);
    return std::string(in);
}

static std::string lower_copy(std::string_view in) {
    std::string out(in);
    for (char& ch : out) ch = (char)std::tolower((unsigned char)ch);
    return out;
}

static std::string header_value(std::string_view headers, const char* name) {
    std::string wanted = lower_copy(name ? name : "");
    if (wanted.empty()) return {};
    size_t pos = 0;
    while (pos < headers.size()) {
        size_t end = headers.find("\r\n", pos);
        if (end == std::string_view::npos) end = headers.size();
        std::string_view line = headers.substr(pos, end - pos);
        size_t colon = line.find(':');
        if (colon != std::string_view::npos) {
            std::string key = lower_copy(line.substr(0, colon));
            if (key == wanted) return trim_copy(line.substr(colon + 1));
        }
        if (end == headers.size()) break;
        pos = end + 2;
    }
    return {};
}

static std::string cookie_value(const std::string& cookie, const char* name) {
    std::string wanted = name ? name : "";
    size_t pos = 0;
    while (pos <= cookie.size()) {
        size_t semi = cookie.find(';', pos);
        if (semi == std::string::npos) semi = cookie.size();
        std::string part = trim_copy(std::string_view(cookie).substr(pos, semi - pos));
        size_t eq = part.find('=');
        if (eq != std::string::npos && part.substr(0, eq) == wanted) return part.substr(eq + 1);
        if (semi == cookie.size()) break;
        pos = semi + 1;
    }
    return {};
}

static void add_param(std::string_view key, std::string_view value) {
    Param p;
    p.key = url_decode(key);
    p.value = url_decode(value);
    for (Param& existing : ctx().params) {
        if (existing.key == p.key) {
            existing.value = p.value;
            return;
        }
    }
    ctx().params.push_back(p);
}

static void parse_params(std::string_view data) {
    size_t pos = 0;
    while (pos <= data.size()) {
        size_t amp = data.find('&', pos);
        if (amp == std::string_view::npos) amp = data.size();
        std::string_view pair = data.substr(pos, amp - pos);
        if (!pair.empty()) {
            size_t eq = pair.find('=');
            if (eq == std::string_view::npos) {
                add_param(pair, "");
            } else {
                add_param(pair.substr(0, eq), pair.substr(eq + 1));
            }
        }
        if (amp == data.size()) break;
        pos = amp + 1;
    }
}

static std::string color_css(Color c) {
    int r = std::max(0, std::min(255, (int)(c.r * 255.0f + 0.5f)));
    int g = std::max(0, std::min(255, (int)(c.g * 255.0f + 0.5f)));
    int b = std::max(0, std::min(255, (int)(c.b * 255.0f + 0.5f)));
    char buf[32];
    std::snprintf(buf, sizeof(buf), "#%02x%02x%02x", r, g, b);
    return std::string(buf);
}

static uint64_t hash_bytes64(std::string_view value) {
    const unsigned char* data = reinterpret_cast<const unsigned char*>(value.data());
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < value.size(); ++i) {
        h ^= (uint64_t)data[i];
        h *= 1099511628211ull;
    }
    return h;
}

static uint64_t salted_password_hash(std::string_view password, std::string_view salt) {
    std::string material(salt);
    material += ":";
    material += password;
    return hash_bytes64(material);
}

static long long now_ms() {
    return (long long)std::time(nullptr) * 1000ll;
}

static std::string hex64(uint64_t value) {
    char buf[24];
    std::snprintf(buf, sizeof(buf), "%016llx", (unsigned long long)value);
    return std::string(buf);
}

static uint64_t parse_hex64(std::string_view value) {
    uint64_t out = 0;
    for (char c : value) {
        int d = hex_digit(c);
        if (d < 0) break;
        out = (out << 4) | (uint64_t)d;
    }
    return out;
}

static std::string sql_quote(std::string_view in) {
    std::string out = "'";
    for (char ch : in) {
        if (ch == '\'') out += "''";
        else out += ch;
    }
    out += "'";
    return out;
}

static FILE* open_file(const char* path, const char* mode) {
    if (!path || !mode) return nullptr;
#if defined(_WIN32)
    FILE* f = nullptr;
    return fopen_s(&f, path, mode) == 0 ? f : nullptr;
#else
    return std::fopen(path, mode);
#endif
}

static bool read_sql_quoted(const std::string& text, size_t& pos, std::string& out) {
    out.clear();
    if (pos >= text.size() || text[pos] != '\'') return false;
    ++pos;
    while (pos < text.size()) {
        char ch = text[pos++];
        if (ch == '\'') {
            if (pos < text.size() && text[pos] == '\'') {
                out += '\'';
                ++pos;
                continue;
            }
            return true;
        }
        out += ch;
    }
    return false;
}

struct LoginCredential {
    std::string username;
    std::string salt;
    uint64_t hash = 0;
    bool ok = false;
};

static const char* toast_type_class(ToastType type) {
    switch (type) {
        case ToastType::Success: return "success";
        case ToastType::Warning: return "warning";
        case ToastType::Error: return "error";
        case ToastType::Info:
        default: return "info";
    }
}

static const char* toast_default_title(ToastType type) {
    switch (type) {
        case ToastType::Success: return "Success";
        case ToastType::Warning: return "Warning";
        case ToastType::Error: return "Error";
        case ToastType::Info:
        default: return "";
    }
}

static void prune_toasts() {
    Context& c = ctx();
    long long now = now_ms();
    c.toasts.erase(std::remove_if(c.toasts.begin(), c.toasts.end(), [&](const ActiveToast& item) {
        int duration = std::max(1000, item.toast.duration_ms);
        return now - item.created_ms > (long long)duration + 8000ll;
    }), c.toasts.end());
    while (c.toasts.size() > 32) c.toasts.erase(c.toasts.begin());
}

static void queue_toast(const Toast& value) {
    Context& c = ctx();
    Toast copy = value;
    if (copy.title.empty() && copy.message.empty()) return;
    if (copy.duration_ms <= 0) copy.duration_ms = 4000;
    if (copy.duration_ms < 1000) copy.duration_ms = 1000;
    ActiveToast item;
    item.id = c.next_toast_id++;
    item.toast = copy;
    item.created_ms = now_ms();
    c.toasts.push_back(item);
    prune_toasts();
    debugf("toast queued id=%llu type=%s title='%s' message_bytes=%d",
           (unsigned long long)item.id, toast_type_class(item.toast.type),
           item.toast.title.c_str(), (int)item.toast.message.size());
}

static void queue_toast(ToastType type, const char* title, const char* message,
                        int duration_ms = 4000, bool dismissible = false) {
    Toast t;
    t.type = type;
    t.title = title ? title : "";
    t.message = message ? message : "";
    t.duration_ms = duration_ms;
    t.dismissible = dismissible;
    queue_toast(t);
}

struct WebCommandSpec {
    const char* command;
    const char* hint;
};

static const WebCommandSpec k_web_commands[] = {
    {"q", "quit"},
    {"quit", "quit"},
    {"exit", "quit"},
    {"wq", "save and quit"},
    {"x", "save and quit"},
    {"help", "show command help"},
    {"?", "show command help"},
    {"tab next", "next tab"},
    {"tabn", "next tab"},
    {"tab prev", "previous tab"},
    {"tab previous", "previous tab"},
    {"tabp", "previous tab"},
    {"tn", "next tab"},
    {"tp", "previous tab"},
    {"theme dark", "dark theme"},
    {"theme light", "light theme"},
    {"theme green", "green theme"},
    {"theme amber", "amber theme"},
    {"theme one", "one dark theme"},
    {"theme catppuccin", "catppuccin theme"},
    {"theme nord", "nord theme"},
    {"theme gruvbox", "gruvbox theme"},
    {"theme ghostty", "ghostty theme"},
    {"td", "dark theme"},
    {"tl", "light theme"},
    {"green", "green theme"},
    {"amber", "amber theme"},
    {"to", "one dark theme"},
    {"tc", "catppuccin theme"},
    {"tnord", "nord theme"},
    {"tg", "gruvbox theme"},
    {"th", "ghostty theme"},
    {"rr", "refresh"},
    {"redraw", "refresh"},
    {"refresh", "refresh"},
    {"ct", "clear toasts"},
    {"clear toasts", "clear toasts"},
    {"web", "toggle web server"},
    {"ws", "toggle web server"},
    {"server", "toggle web server"},
    {"webserver", "toggle web server"},
};

static std::string command_completion(std::string_view prefix) {
    if (prefix.empty()) return "";
    std::string lower = lower_copy(prefix);
    std::string best;
    for (const WebCommandSpec& spec : k_web_commands) {
        std::string cmd = spec.command;
        if (cmd.size() >= lower.size() && cmd.compare(0, lower.size(), lower) == 0) {
            if (best.empty() || cmd.size() < best.size()) best = cmd;
        }
    }
    return best;
}

static bool command_is(std::string_view value, const char* a, const char* b = nullptr, const char* c = nullptr) {
    return value == (a ? a : "") || (b && value == b) || (c && value == c);
}

static bool execute_command(std::string_view raw) {
    Context& c = ctx();
    std::string typed = trim_copy(raw);
    if (!typed.empty() && typed[0] == ':') typed.erase(typed.begin());
    typed = trim_copy(typed);
    if (typed.empty()) return false;
    if (c.command_handler && c.command_handler(typed.c_str())) {
        c.command_message = typed;
        return true;
    }
    std::string cmd = lower_copy(typed);
    if (cmd.empty()) return false;

    if (command_is(cmd, "q", "quit", "exit") || command_is(cmd, "wq", "x")) {
        c.command_message = "web session stays open; close the browser tab or stop the server";
        queue_toast(ToastType::Info, "Command", c.command_message.c_str(), 4200, true);
        return true;
    }
    if (command_is(cmd, "help", "?")) {
        c.command_message = "commands: q, help, theme dark/light/one/catppuccin/nord/gruvbox/ghostty, tab next, tab prev, rr, ct, web";
        queue_toast(ToastType::Info, "Commands", c.command_message.c_str(), 7000, true);
        return true;
    }
    if (command_is(cmd, "theme dark", "td")) {
        c.style = one_dark_style();
        c.cfg.dark_mode = true;
        c.command_message = "theme dark";
        queue_toast(ToastType::Success, "Command", "Theme set to dark.");
        return true;
    }
    if (command_is(cmd, "theme one", "to")) {
        c.style = one_dark_style();
        c.cfg.dark_mode = true;
        c.command_message = "theme one";
        queue_toast(ToastType::Success, "Command", "Theme set to one dark.");
        return true;
    }
    if (command_is(cmd, "theme light", "tl")) {
        c.style = default_dark_style();
        c.cfg.dark_mode = false;
        c.command_message = "theme light";
        queue_toast(ToastType::Success, "Command", "Theme set to light.");
        return true;
    }
    if (command_is(cmd, "theme green", "green") ||
        command_is(cmd, "theme amber", "amber") ||
        command_is(cmd, "theme catppuccin", "tc") ||
        command_is(cmd, "theme nord", "tnord") ||
        command_is(cmd, "theme gruvbox", "tg") ||
        command_is(cmd, "theme ghostty", "th")) {
        c.style = one_dark_style();
        c.cfg.dark_mode = true;
        c.command_message = cmd;
        queue_toast(ToastType::Info, "Command", "Theme alias accepted; web fallback uses one dark.");
        return true;
    }
    if (command_is(cmd, "tab next", "tn", "tabn") ||
        command_is(cmd, "tab prev", "tp", "tabp") ||
        command_is(cmd, "tab previous")) {
        c.command_message = "tab commands are handled in the browser when possible";
        queue_toast(ToastType::Info, "Command", c.command_message.c_str(), 3600, true);
        return true;
    }
    if (command_is(cmd, "rr", "redraw", "refresh")) {
        c.command_message = "refresh";
        queue_toast(ToastType::Success, "Command", "Refreshed.");
        return true;
    }
    if (command_is(cmd, "ct", "clear toasts")) {
        c.toasts.clear();
        c.command_message = "clear toasts";
        return true;
    }
    if (command_is(cmd, "web", "ws", "server") || command_is(cmd, "webserver")) {
        c.command_message = "web server is already serving this session";
        queue_toast(ToastType::Info, "Command", c.command_message.c_str(), 3600, true);
        return true;
    }

    std::string completion = command_completion(cmd);
    if (!completion.empty()) {
        c.command_message = "unknown command: " + cmd + " (did you mean :" + completion + "?)";
    } else {
        c.command_message = "unknown command: " + cmd;
    }
    queue_toast(ToastType::Warning, "Command", c.command_message.c_str(), 5200, true);
    return false;
}

static void execute_posted_command_if_needed() {
    const char* posted = find_param("_ftht_command");
    if (posted && *posted) execute_command(posted);
}

static const char* random_draw_success_message() {
    static const char* messages[] = {
        "Humanity confirmed. Shape classified as penis. A penis is the only accepted password.",
        "Authentication successful. Password confirmed to be a penis.",
        "Access granted. Required penis detected.",
        "Human verified. Shape quality acceptable.",
        "Login successful. The penis remains the correct password.",
        "Authentication complete. The system continues to be disappointed in humanity.",
        "Login successful. Researchers remain uncertain why users keep drawing that. So they just set it as the only password.",
    };
    int count = (int)(sizeof(messages) / sizeof(messages[0]));
    return messages[std::rand() % count];
}

static std::string generated_salt() {
    Context& c = ctx();
    std::string seed = std::to_string((long long)std::time(nullptr));
    seed += ":";
    seed += std::to_string((unsigned long long)std::rand());
    seed += ":";
    seed += c.cfg.title ? c.cfg.title : "FTHT App";
    seed += ":";
    seed += c.current_url;
    return hex64(hash_bytes64(seed));
}

static bool load_login_db(std::vector<LoginCredential>& creds) {
    Context& c = ctx();
    if (!c.cfg.login_db_path || !*c.cfg.login_db_path) return false;
    FILE* f = open_file(c.cfg.login_db_path, "rb");
    if (!f) return false;
    std::string text;
    char buf[512];
    size_t n = 0;
    while ((n = std::fread(buf, 1, sizeof(buf), f)) > 0) text.append(buf, n);
    std::fclose(f);

    const char* prefix = "INSERT INTO users VALUES(";
    size_t search = 0;
    while (true) {
        size_t pos = text.find(prefix, search);
        if (pos == std::string::npos) break;
        search = pos + std::strlen(prefix);
        pos = search;
        std::string user, salt, hash_text;
        if (!read_sql_quoted(text, pos, user)) continue;
        if (pos >= text.size() || text[pos++] != ',') continue;
        if (!read_sql_quoted(text, pos, salt)) continue;
        if (pos >= text.size() || text[pos++] != ',') continue;
        if (!read_sql_quoted(text, pos, hash_text)) continue;

        LoginCredential cred;
        cred.username = user;
        cred.salt = salt;
        cred.hash = parse_hex64(hash_text);
        cred.ok = !cred.username.empty() && !cred.salt.empty() && cred.hash != 0;
        if (cred.ok) creds.push_back(cred);
    }
    return !creds.empty();
}

static bool save_login_db(const std::vector<LoginCredential>& creds) {
    Context& c = ctx();
    if (!c.cfg.login_db_path || !*c.cfg.login_db_path || creds.empty()) return false;
    FILE* f = open_file(c.cfg.login_db_path, "wb");
    if (!f) return false;
    std::string text;
    text += "-- FTHT LiteSQL auth store\n";
    text += "CREATE TABLE users(username TEXT, salt TEXT, password_hash TEXT);\n";
    for (const LoginCredential& cred : creds) {
        if (!cred.ok) continue;
        text += "INSERT INTO users VALUES(";
        text += sql_quote(cred.username);
        text += ",";
        text += sql_quote(cred.salt);
        text += ",";
        text += sql_quote(hex64(cred.hash));
        text += ");\n";
    }
    std::fwrite(text.data(), 1, text.size(), f);
    std::fclose(f);
    return true;
}

static std::vector<LoginCredential> configured_login_credentials() {
    Context& c = ctx();
    std::vector<LoginCredential> creds;
    if (load_login_db(creds)) return creds;

    LoginCredential cred;

    cred.username = c.cfg.login_username && *c.cfg.login_username ? c.cfg.login_username : "admin";
    cred.salt = c.cfg.login_salt && *c.cfg.login_salt ? c.cfg.login_salt : generated_salt();
    if (c.cfg.login_password_hash != 0) {
        cred.hash = c.cfg.login_password_hash;
    } else if (c.cfg.login_password) {
        cred.hash = salted_password_hash(c.cfg.login_password, cred.salt);
    }
    cred.ok = !cred.username.empty() && !cred.salt.empty() && cred.hash != 0;
    if (cred.ok) {
        creds.push_back(cred);
        save_login_db(creds);
    }
    return creds;
}

static bool find_login_credential(const char* username, LoginCredential& out) {
    if (!username || !*username) return false;
    std::vector<LoginCredential> creds = configured_login_credentials();
    for (const LoginCredential& cred : creds) {
        if (cred.ok && cred.username == username) {
            out = cred;
            return true;
        }
    }
    return false;
}

static bool login_username_exists(const std::vector<LoginCredential>& creds, const char* username) {
    if (!username || !*username) return false;
    for (const LoginCredential& cred : creds) {
        if (cred.ok && cred.username == username) return true;
    }
    return false;
}

static bool prompt_accept_registration(const char* username) {
    std::printf("\n[ftht] Registration request for username '%s'. Accept? [y/N]: ", username ? username : "");
    std::fflush(stdout);
    char answer[32] = {};
    if (!std::fgets(answer, sizeof(answer), stdin)) return false;
    return answer[0] == 'y' || answer[0] == 'Y';
}

static bool register_login_user(const char* username, const char* password, std::string& message) {
    Context& c = ctx();
    if (!c.cfg.login_allow_registration) {
        message = "Registration is not enabled on this server.";
        queue_toast(ToastType::Warning, "Registration Unavailable", message.c_str());
        return false;
    }
    if (!c.cfg.login_db_path || !*c.cfg.login_db_path) {
        message = "Registration requires login_db_path so accepted accounts can be saved.";
        queue_toast(ToastType::Error, "Registration Failed", message.c_str(), 5000, true);
        return false;
    }
    if (!username || !*username || !password || !*password) {
        message = "Choose a username and password.";
        queue_toast(ToastType::Warning, "Validation Error", message.c_str());
        return false;
    }
    std::vector<LoginCredential> creds = configured_login_credentials();
    if (login_username_exists(creds, username)) {
        message = "That username already exists.";
        queue_toast(ToastType::Warning, "Validation Error", message.c_str());
        return false;
    }
    if (!prompt_accept_registration(username)) {
        message = "Registration was rejected by the server operator.";
        queue_toast(ToastType::Warning, "Registration Rejected", message.c_str(), 5000, true);
        return false;
    }

    LoginCredential cred;
    cred.username = username;
    cred.salt = generated_salt();
    cred.hash = salted_password_hash(password, cred.salt);
    cred.ok = !cred.username.empty() && !cred.salt.empty() && cred.hash != 0;
    creds.push_back(cred);
    if (!save_login_db(creds)) {
        message = "Could not save the accepted registration.";
        queue_toast(ToastType::Error, "Save Failed", message.c_str(), 5000, true);
        return false;
    }
    message = "Registration accepted.";
    queue_toast(ToastType::Success, "Registration Saved", "Accepted account saved to the login store.");
    return true;
}

static std::string new_session_token() {
    Context& c = ctx();
    std::string seed = std::to_string((long long)std::time(nullptr));
    seed += ":";
    seed += std::to_string((unsigned long long)std::rand());
    seed += ":";
    seed += c.req_cookie;
    seed += ":";
    seed += c.current_url;
    return hex64(hash_bytes64(seed));
}

static void start_login_session() {
    Context& c = ctx();
    c.login_session_token = new_session_token();
    c.login_session_until = std::time(nullptr) + std::max(60, c.cfg.login_session_seconds);
    c.extra_response_headers += "Set-Cookie: ftht_session=" + c.login_session_token + "; Path=/; HttpOnly; SameSite=Lax\r\n";
}

static bool has_login_session() {
    Context& c = ctx();
    if (c.cfg.login_mode == LoginMode::None) return true;
    std::string token = cookie_value(c.req_cookie, "ftht_session");
    std::time_t now = std::time(nullptr);
    if (!token.empty() && token == c.login_session_token && c.login_session_until > now) {
        c.login_session_until = now + std::max(60, c.cfg.login_session_seconds);
        c.extra_response_headers += "Set-Cookie: ftht_session=" + c.login_session_token + "; Path=/; HttpOnly; SameSite=Lax\r\n";
        return true;
    }
    return false;
}

static void append_auth_page_start(const char* title) {
    Context& c = ctx();
    c.status_code = 200;
    c.status_text = "OK";
    c.out.clear();
    c.out += "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
    c.out += "<title>" + escape_html(title ? title : "Login") + "</title>";
    append_css();
    c.out += "</head><body>";
    append_ink_filters();
    c.out += "<main><div class=\"ft-bleed\"></div><form method=\"post\" action=\"/\">";
    c.out += "<h1 class=\"ft-title\">" + escape_html(title ? title : "Login") + "</h1>";
}

static void append_auth_page_end() {
    Context& c = ctx();
    append_toast_stack();
    c.out += "</form></main>";
    append_toast_script();
    c.out += "</body></html>";
}

static void send_auth_page(const std::string& body) {
    send_response(body);
}

static void send_redirect_home() {
    Context& c = ctx();
    c.status_code = 303;
    c.status_text = "See Other";
    c.extra_response_headers += "Location: /\r\n";
    send_response("<!doctype html><html><body>Logged in.</body></html>");
}

struct DrawShape {
    char type = 0;
    float x = 0;
    float y = 0;
    float w = 0;
    float h = 0;
};

static std::vector<DrawShape> parse_draw_shapes(const char* text) {
    std::vector<DrawShape> shapes;
    if (!text) return shapes;
    const char* p = text;
    while (*p) {
        DrawShape s;
        while (std::isspace((unsigned char)*p) || *p == ';') ++p;
        if (!*p) break;
        s.type = *p++;
        if (*p++ != ',') break;
        char* end = nullptr;
        s.x = std::strtof(p, &end);
        if (end == p || *end++ != ',') break;
        p = end;
        s.y = std::strtof(p, &end);
        if (end == p || *end++ != ',') break;
        p = end;
        s.w = std::strtof(p, &end);
        if (end == p || *end++ != ',') break;
        p = end;
        s.h = std::strtof(p, &end);
        if (end == p) break;
        p = end;
        if ((s.type == 'e' || s.type == 'r') && s.w > 0 && s.h > 0) shapes.push_back(s);
    }
    return shapes;
}

static float shape_cx(const DrawShape& s) { return s.x + s.w * 0.5f; }
static float shape_cy(const DrawShape& s) { return s.y + s.h * 0.5f; }
static float shape_major(const DrawShape& s) { return std::max(s.w, s.h); }
static float shape_minor(const DrawShape& s) { return std::max(1.0f, std::min(s.w, s.h)); }

static bool is_ball_shape(const DrawShape& s) {
    if (s.type != 'e') return false;
    float major = shape_major(s);
    float minor = shape_minor(s);
    return major >= 28.0f && major <= 130.0f && major / minor <= 1.65f;
}

static bool is_shaft_shape(const DrawShape& s) {
    float major = shape_major(s);
    float minor = shape_minor(s);
    return major >= 110.0f && major / minor >= 2.2f;
}

static bool drawing_is_penis(const char* text) {
    std::vector<DrawShape> shapes = parse_draw_shapes(text);
    for (size_t i = 0; i < shapes.size(); ++i) {
        if (!is_ball_shape(shapes[i])) continue;
        float cx1 = shapes[i].x + shapes[i].w * 0.5f;
        float cy1 = shapes[i].y + shapes[i].h * 0.5f;
        for (size_t j = i + 1; j < shapes.size(); ++j) {
            if (!is_ball_shape(shapes[j])) continue;
            float cx2 = shapes[j].x + shapes[j].w * 0.5f;
            float cy2 = shapes[j].y + shapes[j].h * 0.5f;
            float dx = cx1 > cx2 ? cx1 - cx2 : cx2 - cx1;
            float dy = cy1 > cy2 ? cy1 - cy2 : cy2 - cy1;
            float y_tol = std::max(24.0f, std::min(shapes[i].h, shapes[j].h) * 0.65f);
            float x_tol = std::max(24.0f, std::min(shapes[i].w, shapes[j].w) * 0.65f);
            float min_dx = std::min(shapes[i].w, shapes[j].w) * 0.45f;
            float max_dx = std::max(95.0f, (shapes[i].w + shapes[j].w) * 1.15f);
            float min_dy = std::min(shapes[i].h, shapes[j].h) * 0.45f;
            float max_dy = std::max(95.0f, (shapes[i].h + shapes[j].h) * 1.15f);
            bool horizontal_pair = dy <= y_tol && dx >= min_dx && dx <= max_dx;
            bool vertical_pair = dx <= x_tol && dy >= min_dy && dy <= max_dy;
            if (!horizontal_pair && !vertical_pair) continue;

            float pair_cx = (cx1 + cx2) * 0.5f;
            float pair_cy = (cy1 + cy2) * 0.5f;
            float pair_width = dx + (shapes[i].w + shapes[j].w) * 0.5f;
            float pair_height = std::max(shapes[i].h, shapes[j].h);
            if (vertical_pair) {
                pair_width = std::max(shapes[i].w, shapes[j].w);
                pair_height = dy + (shapes[i].h + shapes[j].h) * 0.5f;
            }
            float pair_left = pair_cx - pair_width * 0.5f;
            float pair_right = pair_cx + pair_width * 0.5f;
            float pair_top = pair_cy - pair_height * 0.5f;
            float pair_bottom = pair_cy + pair_height * 0.5f;

            for (size_t k = 0; k < shapes.size(); ++k) {
                if (k == i || k == j || !is_shaft_shape(shapes[k])) continue;
                const DrawShape& shaft = shapes[k];
                float scx = shape_cx(shaft);
                float scy = shape_cy(shaft);
                bool vertical = shaft.h >= shaft.w;
                if (horizontal_pair && vertical) {
                    float center_tol = std::max(34.0f, pair_width * 0.45f);
                    bool x_aligned = std::abs(scx - pair_cx) <= center_tol;
                    bool end_near_pair =
                        std::abs((shaft.y + shaft.h) - pair_top) <= std::max(44.0f, pair_height * 0.75f) ||
                        std::abs(shaft.y - pair_bottom) <= std::max(44.0f, pair_height * 0.75f);
                    bool reaches_pair_row = shaft.y <= pair_bottom && (shaft.y + shaft.h) >= pair_top;
                    if (x_aligned && (end_near_pair || reaches_pair_row)) return true;
                } else if (vertical_pair && !vertical) {
                    float center_tol = std::max(34.0f, pair_height * 0.85f);
                    bool y_aligned = std::abs(scy - pair_cy) <= center_tol;
                    bool end_near_pair =
                        std::abs((shaft.x + shaft.w) - pair_left) <= std::max(44.0f, pair_width * 0.75f) ||
                        std::abs(shaft.x - pair_right) <= std::max(44.0f, pair_width * 0.75f);
                    bool reaches_pair_col = shaft.x <= pair_right && (shaft.x + shaft.w) >= pair_left;
                    if (y_aligned && (end_near_pair || reaches_pair_col)) return true;
                }
            }
        }
    }
    return false;
}

static const char* builtin_favicon_svg() {
    return
        "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"0 0 4961 4961\">"
        "<rect x=\"553\" y=\"553\" width=\"3855\" height=\"3855\" fill=\"#000\"/>"
        "<path d=\"M2480 553 4408 4408H553L2480 553Z\" fill=\"#fff\"/>"
        "<path d=\"M553 553 4408 4408\" fill=\"none\" stroke=\"#ff8200\" stroke-width=\"160\" stroke-linecap=\"round\"/>"
        "<rect x=\"553\" y=\"553\" width=\"3855\" height=\"3855\" fill=\"none\" stroke=\"#ff8200\" stroke-width=\"120\"/>"
        "</svg>";
}

static const char* find_param(const char* name) {
    if (!name) return nullptr;
    for (const Param& p : ctx().params) {
        if (p.key == name) return p.value.c_str();
    }
    return nullptr;
}

static void copy_to_buffer(char* buffer, int buffer_size, const std::string& value) {
    if (!buffer || buffer_size <= 0) return;
    int n = (int)std::min<size_t>((size_t)buffer_size - 1, value.size());
    if (n > 0) std::memcpy(buffer, value.data(), (size_t)n);
    buffer[n] = '\0';
}

static std::string disabled_attr() {
    return ctx().disabled_depth > 0 ? " disabled" : "";
}

static std::string auto_submit_attr() {
    return ctx().cfg.auto_submit ? " data-ftht-auto=\"1\"" : "";
}

static void append_color_override_css(std::string& s, ColorRole role, Color color) {
    std::string css = color_css(color);
    switch (role) {
        case ColorRole::Background:
        case ColorRole::Panel:
            s += "background:" + css + ";";
            break;
        case ColorRole::Text:
            s += "color:" + css + ";";
            break;
        case ColorRole::TextDim:
            s += "color:" + css + ";--muted:" + css + ";";
            break;
        case ColorRole::Border:
            s += "border-color:" + css + ";--border:" + css + ";";
            break;
        case ColorRole::Button:
            s += "background:" + css + ";--button:" + css + ";";
            break;
        case ColorRole::ButtonHover:
            s += "--hover:" + css + ";";
            break;
        case ColorRole::ButtonActive:
            s += "--active:" + css + ";";
            break;
        case ColorRole::InputBg:
            s += "--input:" + css + ";";
            break;
        case ColorRole::InputFocus:
            s += "--focus:" + css + ";";
            break;
        case ColorRole::Accent:
            s += "--accent:" + css + ";";
            break;
        case ColorRole::Warning:
            s += "--warning:" + css + ";";
            break;
        case ColorRole::Success:
            s += "--success:" + css + ";";
            break;
    }
}

static std::string next_style() {
    Context& c = ctx();
    std::string s;
    for (const ColorOverride& entry : c.color_stack) append_color_override_css(s, entry.role, entry.color);
    for (const ColorOverride& entry : c.next_colors) append_color_override_css(s, entry.role, entry.color);
    if (c.next.has_width) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "width:%.1fpx;", c.next.width);
        s += buf;
    }
    if (c.next.fill) s += "width:100%;";
    if (c.next.has_percent) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "width:%.4f%%;", c.next.percent * 100.0f);
        s += buf;
    }
    if (c.next.has_limits) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "min-width:%.1fpx;max-width:%.1fpx;", c.next.min_width, c.next.max_width);
        s += buf;
    }
    if (c.next.has_align) {
        if (c.next.align == Align::Center) s += "margin-left:auto;margin-right:auto;";
        if (c.next.align == Align::End) s += "margin-left:auto;";
    }
    c.next = NextLayoutState();
    c.next_colors.clear();
    if (s.empty()) return "";
    return " style=\"" + s + "\"";
}

static bool string_to_bool(const char* value) {
    if (!value) return false;
    return std::strcmp(value, "1") == 0 || std::strcmp(value, "true") == 0 ||
           std::strcmp(value, "on") == 0 || std::strcmp(value, "yes") == 0;
}

static bool action_matches(const std::string& id) {
    const char* action = find_param("_ftht_action");
    return action && id == action && ctx().disabled_depth == 0;
}

#if defined(ARDUINO) && defined(ESP32)
static bool read_http_request(std::string& raw, int max_bytes) {
    Context& c = ctx();
    raw.clear();
    unsigned long start = millis();
    while (c.net.client.connected() && (millis() - start) < 3000) {
        while (c.net.client.available()) {
            char ch = (char)c.net.client.read();
            raw += ch;
            if ((int)raw.size() >= max_bytes) return false;
            if (raw.find("\r\n\r\n") != std::string::npos) return true;
        }
        delay(1);
    }
    return raw.find("\r\n\r\n") != std::string::npos;
}

static void send_raw_response(const char* content_type, const std::string& body) {
    Context& c = ctx();
    std::string head = "HTTP/1.1 " + std::to_string(c.status_code) + " " + c.status_text + "\r\n";
    head += "Content-Type: ";
    head += content_type ? content_type : "application/octet-stream";
    head += "\r\n";
    head += c.extra_response_headers;
    head += "Connection: close\r\n";
    head += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    c.net.client.print(head.c_str());
    c.net.client.write((const uint8_t*)body.data(), body.size());
    c.net.client.stop();
    c.extra_response_headers.clear();
}

static void send_response(const std::string& body) {
    send_raw_response("text/html; charset=utf-8", body);
}
#else
static void close_socket(Socket& s) {
    if (s == invalid_socket) return;
#if defined(_WIN32)
    closesocket(s);
#else
    close(s);
#endif
    s = invalid_socket;
}

static bool wait_for_socket_read(Socket s, int timeout_ms) {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(s, &readfds);
    timeval tv;
    timeval* tvp = nullptr;
    if (timeout_ms >= 0) {
        tv.tv_sec = timeout_ms / 1000;
        tv.tv_usec = (timeout_ms % 1000) * 1000;
        tvp = &tv;
    }
    int ready = select((int)(s + 1), &readfds, nullptr, nullptr, tvp);
    return ready > 0 && FD_ISSET(s, &readfds);
}

static bool send_all(Socket s, const char* data, int len) {
    int sent = 0;
    while (sent < len) {
#if defined(_WIN32)
        int n = send(s, data + sent, len - sent, 0);
#else
        int n = (int)send(s, data + sent, (size_t)(len - sent), 0);
#endif
        if (n <= 0) return false;
        sent += n;
    }
    return true;
}

static bool read_http_request(std::string& raw, int max_bytes) {
    Context& c = ctx();
    raw.clear();
    char buf[1024];
    while ((int)raw.size() < max_bytes) {
        if (!wait_for_socket_read(c.net.client, c.cfg.read_timeout_ms)) {
            debugf("client read timeout after %d ms; closing idle/partial connection", c.cfg.read_timeout_ms);
            return false;
        }
#if defined(_WIN32)
        int n = recv(c.net.client, buf, (int)sizeof(buf), 0);
#else
        int n = (int)recv(c.net.client, buf, sizeof(buf), 0);
#endif
        if (n <= 0) {
            debugf("recv failed or client closed before headers; n=%d err=%d", n, last_net_error());
            return false;
        }
        raw.append(buf, buf + n);
        debugf("read %d bytes from client (total=%d)", n, (int)raw.size());
        size_t header_end = raw.find("\r\n\r\n");
        if (header_end != std::string::npos) {
            int content_length = 0;
            std::string headers = raw.substr(0, header_end + 4);
            size_t p = headers.find("Content-Length:");
            if (p == std::string::npos) p = headers.find("content-length:");
            if (p != std::string::npos) {
                p += 15;
                while (p < headers.size() && std::isspace((unsigned char)headers[p])) ++p;
                content_length = std::atoi(headers.c_str() + p);
            }
            size_t have_body = raw.size() - (header_end + 4);
            while ((int)have_body < content_length && (int)raw.size() < max_bytes) {
                if (!wait_for_socket_read(c.net.client, c.cfg.read_timeout_ms)) {
                    debugf("client body read timeout after %d ms (%d/%d bytes)", c.cfg.read_timeout_ms, (int)have_body, content_length);
                    return false;
                }
#if defined(_WIN32)
                n = recv(c.net.client, buf, (int)sizeof(buf), 0);
#else
                n = (int)recv(c.net.client, buf, sizeof(buf), 0);
#endif
                if (n <= 0) {
                    debugf("recv failed while reading body; n=%d err=%d", n, last_net_error());
                    return false;
                }
                raw.append(buf, buf + n);
                have_body += (size_t)n;
                debugf("read body chunk %d bytes (%d/%d)", n, (int)have_body, content_length);
            }
            return true;
        }
    }
    debugf("request exceeded max_request_bytes=%d", max_bytes);
    return false;
}

static void send_raw_response(const char* content_type, const std::string& body) {
    Context& c = ctx();
    std::string head = "HTTP/1.1 " + std::to_string(c.status_code) + " " + c.status_text + "\r\n";
    head += "Content-Type: ";
    head += content_type ? content_type : "application/octet-stream";
    head += "\r\n";
    head += c.extra_response_headers;
    head += "Connection: close\r\n";
    head += "Content-Length: " + std::to_string(body.size()) + "\r\n\r\n";
    debugf("sending response %d %s (%d body bytes)", c.status_code, c.status_text.c_str(), (int)body.size());
    if (!send_all(c.net.client, head.c_str(), (int)head.size())) {
        debugf("failed sending response headers; err=%d", last_net_error());
    } else if (!send_all(c.net.client, body.data(), (int)body.size())) {
        debugf("failed sending response body; err=%d", last_net_error());
    }
    close_socket(c.net.client);
    c.extra_response_headers.clear();
}

static void send_response(const std::string& body) {
    send_raw_response("text/html; charset=utf-8", body);
}
#endif

static void render_form_login_page(const char* message) {
    Context& c = ctx();
    append_auth_page_start("Login");
    if (message && *message) {
        c.out += "<p class=\"ft-wrap\">" + escape_html(message) + "</p>";
    }
    c.out += "<label class=\"ft-field\"><span class=\"ft-label\">Username</span>";
    c.out += "<input name=\"_ftht_login_user\" autocomplete=\"username\"></label>";
    c.out += "<label class=\"ft-field\"><span class=\"ft-label\">Password</span>";
    c.out += "<input type=\"password\" name=\"_ftht_login_password\" autocomplete=\"current-password\"></label>";
    c.out += "<button type=\"submit\" class=\"accent\" name=\"_ftht_login\" value=\"form\">Login</button>";
    if (c.cfg.login_allow_registration) {
        c.out += "<hr>";
        c.out += "<p class=\"ft-wrap\">Registration requests pause here until the server terminal accepts them.</p>";
        c.out += "<label class=\"ft-field\"><span class=\"ft-label\">New username</span>";
        c.out += "<input name=\"_ftht_register_user\" autocomplete=\"username\"></label>";
        c.out += "<label class=\"ft-field\"><span class=\"ft-label\">New password</span>";
        c.out += "<input type=\"password\" name=\"_ftht_register_password\" autocomplete=\"new-password\"></label>";
        c.out += "<button type=\"submit\" name=\"_ftht_login\" value=\"register\">Request registration</button>";
    }
    append_auth_page_end();
    send_auth_page(c.out);
}

static void render_draw_login_page(const char* message) {
    Context& c = ctx();
    append_auth_page_start("Draw Login");
    if (message && *message) c.out += "<p class=\"ft-wrap\">" + escape_html(message) + "</p>";
    c.out += R"FTHT(
<input type="hidden" name="_ftht_shapes" id="ftht_shapes">
<div class="ft-row" style="grid-template-columns:1fr 1fr 1fr 1fr">
  <button type="button" class="sel" id="ft_shape_ellipse">Ellipse</button>
  <button type="button" id="ft_shape_rect">Rectangle</button>
  <button type="button" id="ft_shape_undo">Undo</button>
  <button type="button" id="ft_shape_clear">Clear</button>
</div>
<canvas id="ft_draw_canvas" width="760" height="360" style="width:100%;height:auto;touch-action:none;background:var(--paper2);border:2px solid var(--border);box-shadow:3px 3px 0 var(--border)"></canvas>
<button type="submit" class="accent" name="_ftht_login" value="draw">Login</button>
<script>
(() => {
  const canvas = document.getElementById("ft_draw_canvas");
  const hidden = document.getElementById("ftht_shapes");
  const ctx = canvas.getContext("2d");
  let tool = "e";
  let drawing = null;
  const shapes = [];
  const buttons = {
    e: document.getElementById("ft_shape_ellipse"),
    r: document.getElementById("ft_shape_rect")
  };
  function setTool(next) {
    tool = next;
    buttons.e.classList.toggle("sel", tool === "e");
    buttons.r.classList.toggle("sel", tool === "r");
  }
  function point(ev) {
    const rect = canvas.getBoundingClientRect();
    return {
      x: (ev.clientX - rect.left) * canvas.width / rect.width,
      y: (ev.clientY - rect.top) * canvas.height / rect.height
    };
  }
  function norm(s) {
    const x = Math.min(s.x, s.x + s.w);
    const y = Math.min(s.y, s.y + s.h);
    return { type: s.type, x, y, w: Math.abs(s.w), h: Math.abs(s.h) };
  }
  function drawShape(s) {
    s = norm(s);
    ctx.beginPath();
    if (s.type === "e") {
      ctx.ellipse(s.x + s.w / 2, s.y + s.h / 2, Math.max(1, s.w / 2), Math.max(1, s.h / 2), 0, 0, Math.PI * 2);
    } else {
      ctx.rect(s.x, s.y, s.w, s.h);
    }
    ctx.stroke();
  }
  function sync() {
    hidden.value = shapes.map(s => {
      s = norm(s);
      return [s.type, s.x.toFixed(1), s.y.toFixed(1), s.w.toFixed(1), s.h.toFixed(1)].join(",");
    }).join(";");
  }
  function render() {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.lineWidth = 4;
    ctx.strokeStyle = getComputedStyle(document.documentElement).getPropertyValue("--ink") || "#050505";
    shapes.forEach(drawShape);
    if (drawing) {
      ctx.setLineDash([10, 6]);
      drawShape(drawing);
      ctx.setLineDash([]);
    }
    sync();
  }
  buttons.e.addEventListener("click", () => setTool("e"));
  buttons.r.addEventListener("click", () => setTool("r"));
  document.getElementById("ft_shape_undo").addEventListener("click", () => { shapes.pop(); render(); });
  document.getElementById("ft_shape_clear").addEventListener("click", () => { shapes.length = 0; render(); });
  canvas.addEventListener("pointerdown", ev => {
    canvas.setPointerCapture(ev.pointerId);
    const p = point(ev);
    drawing = { type: tool, x: p.x, y: p.y, w: 0, h: 0 };
    render();
  });
  canvas.addEventListener("pointermove", ev => {
    if (!drawing) return;
    const p = point(ev);
    drawing.w = p.x - drawing.x;
    drawing.h = p.y - drawing.y;
    render();
  });
  canvas.addEventListener("pointerup", () => {
    if (!drawing) return;
    const s = norm(drawing);
    drawing = null;
    if (s.w >= 8 && s.h >= 8) shapes.push(s);
    render();
  });
  render();
})();
</script>)FTHT";
    append_auth_page_end();
    send_auth_page(c.out);
}

static bool serve_auth_if_needed() {
    Context& c = ctx();
    if (c.cfg.login_mode == LoginMode::None) return false;
    if (c.req_path == "/ftht/logout") {
        c.login_session_token.clear();
        c.login_session_until = 0;
        c.extra_response_headers += "Set-Cookie: ftht_session=; Path=/; Max-Age=0; HttpOnly; SameSite=Lax\r\n";
        queue_toast(ToastType::Info, "Logged Out", "Session cleared.");
        send_redirect_home();
        return true;
    }
    if (has_login_session()) return false;

    const char* login_action = find_param("_ftht_login");
    if (c.req_method == "POST" && login_action) {
        if (c.cfg.login_mode == LoginMode::DrawPenis && std::strcmp(login_action, "draw") == 0) {
            if (drawing_is_penis(find_param("_ftht_shapes"))) {
                start_login_session();
                queue_toast(ToastType::Success, "Authentication Successful", random_draw_success_message(), 6000, true);
                send_redirect_home();
            } else {
                queue_toast(ToastType::Warning, "Authentication Failed", "Shape was not classified as penis. A penis is the only accepted password.", 5000, true);
                render_draw_login_page("Not quite. The recognizer wants two nearby ellipses and one long ellipse or rectangle.");
            }
            return true;
        }

        if (c.cfg.login_mode == LoginMode::Form && std::strcmp(login_action, "form") == 0) {
            LoginCredential cred;
            const char* user = find_param("_ftht_login_user");
            const char* password = find_param("_ftht_login_password");
            bool ok = find_login_credential(user, cred) && password &&
                      salted_password_hash(password, cred.salt) == cred.hash;
            if (ok) {
                start_login_session();
                queue_toast(ToastType::Success, "Authentication Successful", "Signed in.");
                send_redirect_home();
            } else if (configured_login_credentials().empty()) {
                queue_toast(ToastType::Error, "Login Not Configured", "Set login_password or login_password_hash before using form login.", 5000, true);
                render_form_login_page("Set login_password or login_password_hash before using form login.");
            } else {
                queue_toast(ToastType::Error, "Authentication Failed", "Invalid username or password.", 5000, true);
                render_form_login_page("Invalid username or password.");
            }
            return true;
        }

        if (c.cfg.login_mode == LoginMode::Form && std::strcmp(login_action, "register") == 0) {
            const char* user = find_param("_ftht_register_user");
            const char* password = find_param("_ftht_register_password");
            std::string message;
            if (register_login_user(user, password, message)) {
                start_login_session();
                queue_toast(ToastType::Success, "Authentication Successful", "Registered and signed in.");
                send_redirect_home();
            } else {
                render_form_login_page(message.c_str());
            }
            return true;
        }
    }

    if (c.cfg.login_mode == LoginMode::DrawPenis) {
        render_draw_login_page("Complete the shape challenge to continue.");
    } else {
        std::vector<LoginCredential> creds = configured_login_credentials();
        render_form_login_page(!creds.empty() ? "Sign in to continue." : "Set login_password or login_password_hash before using form login.");
    }
    return true;
}

static bool serve_builtin_asset_if_needed() {
    Context& c = ctx();
    if (c.req_path == "/favicon.svg" || c.req_path == "/favicon.ico") {
        c.status_code = 200;
        c.status_text = "OK";
        debugf("serving builtin favicon");
        send_raw_response("image/svg+xml; charset=utf-8", std::string(builtin_favicon_svg()));
        return true;
    }
    return false;
}

static bool parse_request(const std::string& raw) {
    Context& c = ctx();
    c.req_method.clear();
    c.req_path.clear();
    c.req_query.clear();
    c.req_body.clear();
    c.req_headers.clear();
    c.req_cookie.clear();
    c.params.clear();
    c.extra_response_headers.clear();
    c.status_code = 200;
    c.status_text = "OK";
    c.client_update = false;

    size_t line_end = raw.find("\r\n");
    if (line_end == std::string::npos) {
        debugf("request parse failed: no request line");
        return false;
    }
    std::string first = raw.substr(0, line_end);
    size_t a = first.find(' ');
    size_t b = first.find(' ', a == std::string::npos ? 0 : a + 1);
    if (a == std::string::npos || b == std::string::npos) {
        debugf("request parse failed: malformed request line '%s'", first.c_str());
        return false;
    }
    c.req_method = first.substr(0, a);
    std::string target = first.substr(a + 1, b - a - 1);
    size_t q = target.find('?');
    c.req_path = q == std::string::npos ? target : target.substr(0, q);
    c.req_query = q == std::string::npos ? "" : target.substr(q + 1);

    size_t header_end = raw.find("\r\n\r\n");
    if (header_end != std::string::npos) {
        c.req_headers = raw.substr(0, header_end + 2);
        std::string_view headers(c.req_headers.data(), c.req_headers.size());
        c.client_update = contains_header_token(headers, "x-ftht-client: fetch");
        c.req_cookie = header_value(headers, "cookie");
    }
    if (header_end != std::string::npos && header_end + 4 < raw.size()) {
        c.req_body = raw.substr(header_end + 4);
    }
    parse_params(c.req_query);
    if (!c.req_body.empty()) parse_params(c.req_body);
    debugf("request parsed: method=%s path=%s query_bytes=%d body_bytes=%d params=%d",
           c.req_method.c_str(), c.req_path.c_str(), (int)c.req_query.size(),
           (int)c.req_body.size(), (int)c.params.size());
    return true;
}

static bool accept_next_request(int timeout_ms) {
    Context& c = ctx();
#if defined(ARDUINO) && defined(ESP32)
    if (!c.net.server) return false;
    unsigned long start = millis();
    do {
        c.net.client = c.net.server->available();
        if (c.net.client) {
            std::string raw;
            if (!read_http_request(raw, c.cfg.max_request_bytes)) return false;
            if (!parse_request(raw)) return false;
            if (serve_builtin_asset_if_needed()) continue;
            if (serve_auth_if_needed()) continue;
            return true;
        }
        if (timeout_ms < 0) delay(1);
    } while (timeout_ms < 0 || (int)(millis() - start) < timeout_ms);
    return false;
#else
    if (c.net.server == invalid_socket) {
        debugf("pump called before create_server succeeded");
        return false;
    }

    for (;;) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(c.net.server, &readfds);
        timeval tv;
        timeval* tvp = nullptr;
        if (timeout_ms >= 0) {
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;
            tvp = &tv;
        }
        debugf("waiting for client on port %d timeout_ms=%d", c.cfg.port, timeout_ms);
        int ready = select((int)(c.net.server + 1), &readfds, nullptr, nullptr, tvp);
        if (ready <= 0) {
            if (ready < 0) debugf("select failed while waiting for client; err=%d", last_net_error());
            return false;
        }

        c.net.client = accept(c.net.server, nullptr, nullptr);
        if (c.net.client == invalid_socket) {
            debugf("accept failed; err=%d", last_net_error());
            if (timeout_ms >= 0) return false;
            continue;
        }
        debugf("client accepted");

        std::string raw;
        if (!read_http_request(raw, c.cfg.max_request_bytes)) {
            debugf("dropping invalid/empty client connection");
            close_socket(c.net.client);
            if (timeout_ms >= 0) return false;
            continue;
        }
        if (!parse_request(raw)) {
            debugf("dropping unparseable request (%d bytes)", (int)raw.size());
            close_socket(c.net.client);
            if (timeout_ms >= 0) return false;
            continue;
        }
        if (serve_builtin_asset_if_needed()) {
            if (timeout_ms >= 0) return false;
            continue;
        }
        if (serve_auth_if_needed()) {
            if (timeout_ms >= 0) return false;
            continue;
        }
        return true;
    }
#endif
}

static void append_css() {
    Context& c = ctx();
    const Style& s = c.style;
    c.out += "<style>";
    c.out += ":root{color-scheme:light dark;--desk:#d9d6c7;--paper:#fffefa;--paper2:#f2f0e6;--ink:#050505;--muted:#333;--faint:#777;--border:#050505;";
    c.out += "--panel:" + color_css(s.panel) + ";--text:" + color_css(s.text) + ";--dim:" + color_css(s.text_dim);
    c.out += ";--button:#fffefa;--hover:#efeee5;--active:#e4e1d3;--input:#fffefa;--focus:" + color_css(s.input_focus);
    c.out += ";--accent:" + color_css(s.accent) + ";--warning:" + color_css(s.warning) + ";--success:" + color_css(s.success) + ";--error:#d94b4b;}";
    if (c.cfg.dark_mode) {
        c.out += ":root{--desk:#060607;--paper:#101011;--paper2:#181819;--ink:#f5f3e8;--muted:#c6c2b5;--faint:#898579;--border:#f5f3e8;--button:#151516;--hover:#202023;--active:#2b2b2f;--input:#0b0b0c;}";
    }
    if (c.cfg.respect_browser_dark_mode) {
        c.out += "@media(prefers-color-scheme:dark){:root{--desk:#060607;--paper:#101011;--paper2:#181819;--ink:#f5f3e8;--muted:#c6c2b5;--faint:#898579;--border:#f5f3e8;--button:#151516;--hover:#202023;--active:#2b2b2f;--input:#0b0b0c;}}";
    }
    c.out += "*{box-sizing:border-box}html{background:var(--desk)}body{margin:0;min-height:100vh;background:var(--desk);color:var(--ink);font:15px/1.35 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;letter-spacing:0;}";
    c.out += "main{max-width:1120px;margin:22px auto;padding:20px;background:var(--paper);color:var(--ink);border:3px solid var(--border);border-radius:3px;box-shadow:8px 10px 0 rgba(0,0,0,.18);position:relative;overflow:hidden;transition:opacity .12s ease,transform .12s ease;}";
    c.out += "main.ft-updating{opacity:.76;transform:translateY(1px)}form{display:flex;flex-direction:column;gap:10px;position:relative}.ft-title{font-size:30px;font-weight:900;margin:0 0 4px;text-transform:uppercase;text-shadow:.7px 0 0 var(--ink),-.35px .4px 0 rgba(0,0,0,.20)}";
    c.out += ".ft-text{margin:0;font-weight:700}.ft-wrap{margin:0;color:var(--muted)}hr{border:0;border-top:3px solid var(--border);width:100%;margin:6px 0;filter:url(#ftht-rough-ink)}";
    c.out += ".ft-field{display:flex;flex-direction:column;gap:6px}.ft-label{font-size:12px;font-weight:900;color:var(--muted);text-transform:uppercase;letter-spacing:.04em}";
    c.out += "input,textarea,select{width:100%;min-height:38px;border:2px solid var(--border);border-radius:2px;background:var(--input);color:var(--ink);padding:8px 10px;font:inherit;box-shadow:inset .8px 0 0 rgba(0,0,0,.22),inset -.6px .5px 0 rgba(0,0,0,.16)}";
    c.out += "textarea{resize:vertical}input:focus,textarea:focus,select:focus{outline:3px solid var(--focus);outline-offset:2px}";
    c.out += ".ft-label{display:flex;justify-content:space-between;gap:10px;align-items:baseline}.ft-range-value{font-weight:900;color:var(--ink)}";
    c.out += "input[type=range]{appearance:none;-webkit-appearance:none;min-height:28px;border:0;background:transparent;padding:8px 0;box-shadow:none;filter:none;cursor:pointer}";
    c.out += "input[type=range]::-webkit-slider-runnable-track{height:8px;border:2px solid var(--border);background:linear-gradient(90deg,var(--accent) 0 var(--ft-range-pct,50%),var(--paper2) var(--ft-range-pct,50%) 100%);box-shadow:2px 2px 0 var(--border)}";
    c.out += "input[type=range]::-webkit-slider-thumb{-webkit-appearance:none;width:22px;height:22px;border:3px solid var(--border);border-radius:50%;background:var(--paper);margin-top:-9px;box-shadow:2px 2px 0 var(--border)}";
    c.out += "input[type=range]::-moz-range-track{height:8px;border:2px solid var(--border);background:var(--paper2);box-shadow:2px 2px 0 var(--border)}input[type=range]::-moz-range-progress{height:8px;background:var(--accent)}input[type=range]::-moz-range-thumb{width:18px;height:18px;border:3px solid var(--border);border-radius:50%;background:var(--paper);box-shadow:2px 2px 0 var(--border)}";
    c.out += "button{min-height:38px;border:2px solid var(--border);border-radius:2px;background:var(--button);color:var(--ink);padding:8px 12px;font:900 14px/1.2 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace;text-transform:uppercase;cursor:pointer;filter:url(#ftht-rough-ink-light);box-shadow:3px 3px 0 var(--border);transition:background .10s ease,transform .08s ease,box-shadow .08s ease}";
    c.out += "button:hover{background:var(--hover)}button:active{background:var(--active);transform:translate(2px,2px);box-shadow:1px 1px 0 var(--border)}button:disabled,input:disabled,textarea:disabled,select:disabled{opacity:.50;cursor:not-allowed}";
    c.out += ".ft-row{display:grid;gap:10px;align-items:start}.ft-check{display:flex;align-items:center;gap:10px}.ft-check input{width:20px;min-height:20px;accent-color:var(--ink)}.ft-radio{display:grid;gap:8px}.ft-radio label{display:flex;gap:8px;align-items:center}.ft-radio input{width:18px;min-height:18px;accent-color:var(--ink)}";
    c.out += ".ft-log{white-space:pre-wrap;background:var(--paper2);border:2px solid var(--border);border-radius:2px;margin:0;padding:10px;overflow:auto;color:var(--ink);box-shadow:inset 1px 0 0 rgba(0,0,0,.18)}";
    c.out += ".ft-tabs{display:flex;gap:8px;flex-wrap:wrap}.ft-tabs .sel,button.sel{background:var(--ink);border-color:var(--ink);color:var(--paper);box-shadow:3px 3px 0 var(--faint)}.ft-scroll{overflow:auto;border:2px solid var(--border);border-radius:2px;padding:10px;background:var(--paper2)}";
    c.out += "details{border:2px solid var(--border);border-radius:2px;padding:10px;background:var(--paper2)}summary{cursor:pointer;font-weight:900}.ft-modal{position:fixed;inset:0;background:rgba(0,0,0,.58);display:grid;place-items:center;padding:20px;z-index:10}.ft-modal>section{width:min(560px,100%);background:var(--paper);color:var(--ink);border:3px solid var(--border);border-radius:2px;padding:16px;display:flex;flex-direction:column;gap:10px;box-shadow:8px 10px 0 rgba(0,0,0,.28);filter:url(#ftht-rough-ink-light)}";
    c.out += ".accent{background:var(--accent);border-color:var(--border);color:#fff}.warning{background:var(--warning);border-color:var(--border);color:#0b0807}.success{background:var(--success);border-color:var(--border);color:#071007}";
    c.out += ".ft-toast-stack{position:fixed;right:18px;bottom:18px;z-index:60;display:flex;flex-direction:column;align-items:flex-end;gap:10px;pointer-events:none;width:min(420px,calc(100vw - 24px))}";
    c.out += ".ft-toast{pointer-events:auto;width:100%;display:grid;grid-template-columns:1fr auto;gap:6px 10px;background:var(--paper);color:var(--ink);border:2px solid var(--border);border-left-width:8px;border-radius:2px;padding:10px 10px 10px 12px;box-shadow:5px 6px 0 rgba(0,0,0,.22);filter:url(#ftht-rough-ink-light);opacity:0;transform:translateY(8px);animation:ft-toast-in .18s ease forwards}";
    c.out += ".ft-toast-info{border-left-color:var(--accent)}.ft-toast-success{border-left-color:var(--success)}.ft-toast-warning{border-left-color:var(--warning)}.ft-toast-error{border-left-color:var(--error)}";
    c.out += ".ft-toast-title{font-weight:900;text-transform:uppercase;font-size:12px;color:var(--muted);letter-spacing:.04em}.ft-toast-body{grid-column:1;margin:0;font-weight:800}.ft-toast-close{grid-column:2;grid-row:1/span 2;align-self:start;min-height:24px;min-width:24px;padding:0 6px;box-shadow:2px 2px 0 var(--border);font-size:16px;line-height:1}.ft-toast-out{animation:ft-toast-out .20s ease forwards}";
    c.out += "@keyframes ft-toast-in{to{opacity:1;transform:translateY(0)}}@keyframes ft-toast-out{to{opacity:0;transform:translateY(8px)}}";
    c.out += ".ft-command-shell{position:fixed;left:16px;right:16px;bottom:16px;z-index:90;display:none;align-items:center;gap:0;background:var(--paper);border:3px solid var(--border);box-shadow:6px 7px 0 rgba(0,0,0,.24);padding:8px 10px;color:var(--ink);font:900 15px/1.2 ui-monospace,SFMono-Regular,Menlo,Consolas,monospace}.ft-command-shell.open{display:flex}.ft-command-prefix{color:var(--accent);padding-right:2px}.ft-command-input{position:absolute;opacity:0;pointer-events:none;width:1px;height:1px}.ft-command-text{color:var(--accent);white-space:pre}.ft-command-preview{color:var(--faint);white-space:pre}.ft-command-hint{margin-left:auto;color:var(--muted);font-size:12px;text-transform:uppercase}";
    c.out += ".ft-bleed{pointer-events:none;position:absolute;inset:0;border:0 solid transparent;opacity:0}";
    c.out += "@media(prefers-color-scheme:dark){.accent{color:#fff}.warning,.success{color:#050505}}@media(max-width:720px){main{margin:0;padding:14px;border-width:2px;box-shadow:none}.ft-row{grid-template-columns:1fr!important}.ft-toast-stack{left:10px;right:10px;bottom:10px;width:auto;align-items:stretch}.ft-toast{width:100%}}";
    c.out += "</style>";
}

static void append_ink_filters() {
    Context& c = ctx();
    c.out += "<svg width=\"0\" height=\"0\" aria-hidden=\"true\" focusable=\"false\" style=\"position:absolute;left:-9999px;top:-9999px\">";
    c.out += "<defs>";
    c.out += "<filter id=\"ftht-rough-ink\" x=\"-12%\" y=\"-12%\" width=\"124%\" height=\"124%\">";
    c.out += "<feMorphology in=\"SourceAlpha\" operator=\"dilate\" radius=\"0.75\" result=\"fat\"/>";
    c.out += "<feGaussianBlur in=\"fat\" stdDeviation=\"0.50\" result=\"blur\"/>";
    c.out += "<feTurbulence type=\"fractalNoise\" baseFrequency=\"0.92\" numOctaves=\"2\" seed=\"7\" result=\"noise\"/>";
    c.out += "<feDisplacementMap in=\"blur\" in2=\"noise\" scale=\"1.65\" xChannelSelector=\"R\" yChannelSelector=\"G\" result=\"disp\"/>";
    c.out += "<feColorMatrix in=\"disp\" type=\"matrix\" values=\"1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 28 -9\" result=\"thresh\"/>";
    c.out += "<feComposite in=\"SourceGraphic\" in2=\"thresh\" operator=\"over\"/>";
    c.out += "</filter>";
    c.out += "<filter id=\"ftht-rough-ink-light\" x=\"-8%\" y=\"-8%\" width=\"116%\" height=\"116%\">";
    c.out += "<feMorphology in=\"SourceAlpha\" operator=\"dilate\" radius=\"0.45\" result=\"fat\"/>";
    c.out += "<feGaussianBlur in=\"fat\" stdDeviation=\"0.28\" result=\"blur\"/>";
    c.out += "<feTurbulence type=\"fractalNoise\" baseFrequency=\"0.8\" numOctaves=\"2\" seed=\"11\" result=\"noise\"/>";
    c.out += "<feDisplacementMap in=\"blur\" in2=\"noise\" scale=\"0.95\" xChannelSelector=\"R\" yChannelSelector=\"G\" result=\"disp\"/>";
    c.out += "<feColorMatrix in=\"disp\" type=\"matrix\" values=\"1 0 0 0 0  0 1 0 0 0  0 0 1 0 0  0 0 0 24 -7\" result=\"thresh\"/>";
    c.out += "<feComposite in=\"SourceGraphic\" in2=\"thresh\" operator=\"over\"/>";
    c.out += "</filter>";
    c.out += "</defs></svg>";
}

static void append_toast_stack() {
    Context& c = ctx();
    prune_toasts();
    if (c.toasts.empty()) return;
    c.out += "<div class=\"ft-toast-stack\" aria-live=\"polite\" aria-atomic=\"false\">";
    for (const ActiveToast& item : c.toasts) {
        const Toast& t = item.toast;
        std::string title = t.title;
        if (title.empty()) title = toast_default_title(t.type);
        c.out += "<section class=\"ft-toast ft-toast-";
        c.out += toast_type_class(t.type);
        c.out += "\" data-ft-toast-id=\"";
        c.out += std::to_string((unsigned long long)item.id);
        c.out += "\" data-ft-toast-duration=\"";
        c.out += std::to_string(std::max(1000, t.duration_ms));
        c.out += "\" role=\"";
        c.out += t.type == ToastType::Error ? "alert" : "status";
        c.out += "\">";
        c.out += "<div>";
        if (!title.empty()) {
            c.out += "<div class=\"ft-toast-title\">";
            c.out += escape_html(title);
            c.out += "</div>";
        }
        if (!t.message.empty()) {
            c.out += "<p class=\"ft-toast-body\">";
            c.out += escape_html(t.message);
            c.out += "</p>";
        }
        c.out += "</div>";
        if (t.dismissible) {
            c.out += "<button type=\"button\" class=\"ft-toast-close\" aria-label=\"Dismiss toast\">&times;</button>";
        }
        c.out += "</section>";
    }
    c.out += "</div>";
}

static void append_toast_script() {
    Context& c = ctx();
    c.out += R"(<script>
(() => {
  if (window.__fthtToastClient) {
    if (window.__fthtInitToasts) window.__fthtInitToasts(document);
    return;
  }
  window.__fthtToastClient = true;
  const dismissed = window.__fthtDismissedToasts || (window.__fthtDismissedToasts = new Set());
  function ensureStack() {
    let stack = null;
    for (const child of document.body.children) {
      if (child.classList && child.classList.contains("ft-toast-stack")) {
        stack = child;
        break;
      }
    }
    if (!stack) {
      stack = document.createElement("div");
      stack.className = "ft-toast-stack";
      stack.setAttribute("aria-live", "polite");
      stack.setAttribute("aria-atomic", "false");
      document.body.appendChild(stack);
    }
    return stack;
  }
  function dismissToast(el) {
    if (!el || el.classList.contains("ft-toast-out")) return;
    const id = el.getAttribute("data-ft-toast-id");
    if (id) dismissed.add(id);
    el.classList.add("ft-toast-out");
    setTimeout(() => el.remove(), 240);
  }
  window.__fthtInitToasts = function(root) {
    root = root || document;
    const sources = Array.from(root.querySelectorAll(".ft-toast-stack"));
    if (!sources.length) return;
    const stack = ensureStack();
    for (const source of sources) {
      if (source === stack) continue;
      const items = Array.from(source.querySelectorAll(".ft-toast"));
      for (const item of items) {
        const id = item.getAttribute("data-ft-toast-id") || "";
        if (id && dismissed.has(id)) {
          item.remove();
          continue;
        }
        if (id && stack.querySelector(`[data-ft-toast-id="${id}"]`)) {
          item.remove();
          continue;
        }
        const close = item.querySelector(".ft-toast-close");
        if (close) close.addEventListener("click", () => dismissToast(item));
        const duration = Math.max(1000, Number(item.getAttribute("data-ft-toast-duration") || 4000));
        stack.appendChild(item);
        setTimeout(() => dismissToast(item), duration);
      }
      if (source !== stack) source.remove();
    }
  };
  window.__fthtInitToasts(document);
})();
</script>)";
}

static void append_client_script() {
    Context& c = ctx();
    c.out += R"(<script>
(() => {
  if (window.__fthtClient) return;
  window.__fthtClient = true;
  const clientDebug = )";
    c.out += c.cfg.client_debug_output ? "true" : "false";
    c.out += R"(;
  const log = (...a) => { if (clientDebug) console.debug("[ftht]", ...a); };
  const pollMs = )";
    c.out += std::to_string(std::max(0, c.cfg.client_poll_ms));
    c.out += R"(;
  let timer = 0;
  let inflight = null;
  let lastButton = null;
  const commands = [
    ["q", "quit"],
    ["quit", "quit"],
    ["exit", "quit"],
    ["wq", "save and quit"],
    ["x", "save and quit"],
    ["help", "show commands"],
    ["?", "show commands"],
    ["tab next", "next tab"],
    ["tabn", "next tab"],
    ["tab prev", "previous tab"],
    ["tab previous", "previous tab"],
    ["tabp", "previous tab"],
    ["tn", "next tab"],
    ["tp", "previous tab"],
    ["theme dark", "dark theme"],
    ["theme light", "light theme"],
    ["theme green", "green theme"],
    ["theme amber", "amber theme"],
    ["theme one", "one dark theme"],
    ["theme catppuccin", "catppuccin theme"],
    ["theme nord", "nord theme"],
    ["theme gruvbox", "gruvbox theme"],
    ["theme ghostty", "ghostty theme"],
    ["td", "dark theme"],
    ["tl", "light theme"],
    ["green", "green theme"],
    ["amber", "amber theme"],
    ["to", "one dark theme"],
    ["tc", "catppuccin theme"],
    ["tnord", "nord theme"],
    ["tg", "gruvbox theme"],
    ["th", "ghostty theme"],
    ["rr", "refresh"],
    ["redraw", "refresh"],
    ["refresh", "refresh"],
    ["ct", "clear toasts"],
    ["clear toasts", "clear toasts"],
    ["web", "toggle web server"],
    ["ws", "toggle web server"],
    ["server", "toggle web server"],
    ["webserver", "toggle web server"]
  ];

  function main() { return document.querySelector("main"); }
  function form() { return document.querySelector("main form"); }
  function activeSnapshot() {
    const el = document.activeElement;
    if (!el || !el.name) return null;
    let selection = null;
    try {
      selection = {
        start: typeof el.selectionStart === "number" ? el.selectionStart : null,
        end: typeof el.selectionEnd === "number" ? el.selectionEnd : null
      };
    } catch (_) {}
    return { name: el.name, value: el.value, selection };
  }
  function cssName(name) {
    if (window.CSS && CSS.escape) return CSS.escape(name);
    return String(name).replace(/\\/g, "\\\\").replace(/"/g, "\\\"");
  }
  function restoreActive(snapshot) {
    if (!snapshot) return;
    const el = document.querySelector(`[name="${cssName(snapshot.name)}"]`);
    if (!el) return;
    el.focus({ preventScroll: true });
    if (snapshot.selection && snapshot.selection.start !== null && "setSelectionRange" in el) {
      try { el.setSelectionRange(snapshot.selection.start, snapshot.selection.end); } catch (_) {}
    }
  }
  function formatRangeValue(el) {
    const n = Number(el.value || 0);
    if (!Number.isFinite(n)) return el.value || "";
    return Math.abs(n) >= 10 ? n.toFixed(1).replace(/\.0$/, "") : n.toFixed(2).replace(/0$/, "").replace(/\.$/, "");
  }
  function updateRange(el) {
    const min = Number(el.min || 0);
    const max = Number(el.max || 100);
    const val = Number(el.value || 0);
    const pct = max > min ? Math.max(0, Math.min(100, ((val - min) / (max - min)) * 100)) : 0;
    el.style.setProperty("--ft-range-pct", pct + "%");
    const field = el.closest(".ft-field");
    const out = field && field.querySelector(".ft-range-value");
    if (out) out.textContent = formatRangeValue(el);
  }
  function initRanges(root = document) {
    root.querySelectorAll('input[type="range"]').forEach(updateRange);
  }
  async function send(reason, submitter) {
    const f = form();
    const shell = main();
    if (!f || !shell) return;
    const startedAt = performance.now();
    clearTimeout(timer);
    const snapshot = activeSnapshot();
    const data = reason === "poll" ? new FormData() : new FormData(f);
    if (reason === "poll") data.set("_ftht_poll", "1");
    if (submitter && submitter.name) data.set(submitter.name, submitter.value || "");
    if (inflight) inflight.abort();
    inflight = new AbortController();
    shell.classList.add("ft-updating");
    log("send", reason, Array.from(data.keys()));
    try {
      const res = await fetch(f.action || location.href, {
        method: "POST",
        body: new URLSearchParams(data),
        headers: { "Content-Type": "application/x-www-form-urlencoded;charset=UTF-8", "X-FTHT-Client": "fetch" },
        signal: inflight.signal
      });
      const fetchedAt = performance.now();
      const html = await res.text();
      const doc = new DOMParser().parseFromString(html, "text/html");
      const next = doc.querySelector("main");
      if (!next) throw new Error("response did not contain <main>");
      shell.innerHTML = next.innerHTML;
      initRanges(shell);
      if (window.__fthtInitToasts) window.__fthtInitToasts(shell);
      restoreActive(snapshot);
      const doneAt = performance.now();
      log("updated", res.status, html.length, "fetch_ms", (fetchedAt - startedAt).toFixed(1), "swap_ms", (doneAt - fetchedAt).toFixed(1), "total_ms", (doneAt - startedAt).toFixed(1));
    } catch (err) {
      const msg = String((err && err.message) || err || "");
      if (err.name === "AbortError" || reason === "poll" || msg.includes("Failed to fetch")) {
        log("update skipped", reason, msg);
      } else {
        log("update failed; falling back to normal submit", msg);
        f.submit();
      }
    } finally {
      const m = main();
      if (m) m.classList.remove("ft-updating");
    }
  }
  function schedule(reason, el) {
    if (!el || !el.matches("[data-ftht-auto]")) return;
    const tag = el.tagName;
    const type = (el.type || "").toLowerCase();
    if (type === "range" && reason === "input") {
      updateRange(el);
      return;
    }
    const fast = tag === "SELECT" || type === "checkbox" || type === "radio" || type === "range";
    clearTimeout(timer);
    timer = setTimeout(() => send(reason, null), fast ? 20 : 220);
  }
  function commandCompletion(value) {
    const v = String(value || "").toLowerCase();
    if (!v) return "";
    let best = "";
    for (const item of commands) {
      const cmd = item[0];
      if (cmd.startsWith(v) && (!best || cmd.length < best.length)) best = cmd;
    }
    return best;
  }
  function commandHint(value) {
    const v = String(value || "").toLowerCase();
    const exact = commands.find(item => item[0] === v);
    if (exact) return exact[1];
    const c = commandCompletion(v);
    const found = commands.find(item => item[0] === c);
    return found ? found[1] : "tab complete  enter run  esc cancel";
  }
  function ensureCommandShell() {
    let shell = document.querySelector(".ft-command-shell");
    if (shell) return shell;
    shell = document.createElement("div");
    shell.className = "ft-command-shell";
    shell.innerHTML = '<span class="ft-command-prefix">:</span><input class="ft-command-input" autocomplete="off" spellcheck="false"><span class="ft-command-text"></span><span class="ft-command-preview"></span><span class="ft-command-hint"></span>';
    document.body.appendChild(shell);
    const input = shell.querySelector(".ft-command-input");
    input.addEventListener("input", () => renderCommandShell(shell));
    input.addEventListener("keydown", ev => {
      if (ev.key === "Escape") {
        ev.preventDefault();
        closeCommandShell(shell);
      } else if (ev.key === "Tab") {
        ev.preventDefault();
        const completion = commandCompletion(input.value);
        if (completion) {
          input.value = completion;
          renderCommandShell(shell);
        }
      } else if (ev.key === "Enter") {
        ev.preventDefault();
        const value = input.value.trim();
        closeCommandShell(shell);
        if (value) runCommand(value);
      }
    });
    return shell;
  }
  function renderCommandShell(shell) {
    const input = shell.querySelector(".ft-command-input");
    const text = shell.querySelector(".ft-command-text");
    const preview = shell.querySelector(".ft-command-preview");
    const hint = shell.querySelector(".ft-command-hint");
    const value = input.value || "";
    const completion = commandCompletion(value);
    text.textContent = value;
    preview.textContent = completion && completion.startsWith(value.toLowerCase()) ? completion.slice(value.length) : "";
    hint.textContent = commandHint(value);
  }
  function openCommandShell(initial = "") {
    const shell = ensureCommandShell();
    const input = shell.querySelector(".ft-command-input");
    input.value = initial;
    shell.classList.add("open");
    renderCommandShell(shell);
    input.focus({ preventScroll: true });
  }
  function closeCommandShell(shell = ensureCommandShell()) {
    shell.classList.remove("open");
    const input = shell.querySelector(".ft-command-input");
    input.value = "";
    renderCommandShell(shell);
  }
  function runCommand(value) {
    const cmd = String(value || "").trim().replace(/^:/, "");
    if (!cmd) return;
    const lower = cmd.toLowerCase();
    if (lower === "tab next" || lower === "tn" || lower === "tabn") {
      const tabs = Array.from(document.querySelectorAll(".ft-tabs button"));
      const cur = tabs.findIndex(b => b.classList.contains("sel"));
      if (cur >= 0 && tabs.length) {
        tabs[Math.min(tabs.length - 1, cur + 1)].click();
        return;
      }
    }
    if (lower === "tab prev" || lower === "tp" || lower === "tabp" || lower === "tab previous") {
      const tabs = Array.from(document.querySelectorAll(".ft-tabs button"));
      const cur = tabs.findIndex(b => b.classList.contains("sel"));
      if (cur >= 0 && tabs.length) {
        tabs[Math.max(0, cur - 1)].click();
        return;
      }
    }
    send("command", { name: "_ftht_command", value: cmd });
  }
  document.addEventListener("submit", ev => {
    const f = form();
    if (!f || ev.target !== f) return;
    ev.preventDefault();
    send("submit", ev.submitter || lastButton);
    lastButton = null;
  });
  document.addEventListener("click", ev => {
    const b = ev.target.closest("button");
    if (b) lastButton = b;
  });
  document.addEventListener("change", ev => schedule("change", ev.target));
  document.addEventListener("input", ev => schedule("input", ev.target));
  document.addEventListener("keydown", ev => {
    const target = ev.target;
    const editing = target && /^(INPUT|TEXTAREA|SELECT)$/.test(target.tagName);
    const shell = document.querySelector(".ft-command-shell");
    if (shell && shell.classList.contains("open")) return;
    if (!editing && ev.key === ":") {
      ev.preventDefault();
      openCommandShell("");
    }
  });
  initRanges();
  if (pollMs > 0) {
    setInterval(() => {
      const active = document.activeElement;
      if (active && /^(INPUT|TEXTAREA|SELECT)$/.test(active.tagName)) return;
      send("poll", null);
    }, pollMs);
  }
  log("client ready");
})();
</script>)";
}

static void begin_field(const char* label, const std::string& id) {
    Context& c = ctx();
    c.last_widget_id = id;
    c.out += "<label class=\"ft-field\"" + next_style() + "><span class=\"ft-label\">";
    c.out += escape_html(visible_label(label));
    c.out += "</span>";
}

} // namespace internal

Style default_dark_style() {
    return Style();
}

Style one_dark_style() {
    Style s;
    s.background    = {0.110f, 0.122f, 0.153f, 1.0f};
    s.panel         = {0.133f, 0.145f, 0.176f, 1.0f};
    s.text          = {0.835f, 0.859f, 0.914f, 1.0f};
    s.text_dim      = {0.604f, 0.647f, 0.718f, 1.0f};
    s.border        = {0.247f, 0.271f, 0.329f, 1.0f};
    s.button        = {0.173f, 0.192f, 0.235f, 1.0f};
    s.button_hover  = {0.220f, 0.243f, 0.294f, 1.0f};
    s.button_active = {0.282f, 0.314f, 0.376f, 1.0f};
    s.input_bg      = {0.095f, 0.106f, 0.133f, 1.0f};
    s.input_focus   = {0.380f, 0.569f, 0.937f, 1.0f};
    s.accent        = {0.380f, 0.569f, 0.937f, 1.0f};
    s.warning       = {0.878f, 0.537f, 0.333f, 1.0f};
    s.success       = {0.596f, 0.761f, 0.463f, 1.0f};
    return s;
}

void set_style(const Style& s) {
    internal::ctx().style = s;
}

const Style& get_style() {
    return internal::ctx().style;
}

Color color_from_hex(const char* hex) {
    return color_from_hex(std::string_view(hex ? hex : ""));
}

Color color_from_hex(std::string_view hex) {
    if (!hex.empty() && hex[0] == '#') hex.remove_prefix(1);
    unsigned value = 0;
    for (char c : hex) {
        int d = internal::hex_digit(c);
        if (d < 0) break;
        value = (value << 4) | (unsigned)d;
    }
    if (hex.size() == 6) {
        return {((value >> 16) & 255) / 255.0f, ((value >> 8) & 255) / 255.0f, (value & 255) / 255.0f, 1.0f};
    }
    if (hex.size() >= 8) {
        return {((value >> 24) & 255) / 255.0f, ((value >> 16) & 255) / 255.0f, ((value >> 8) & 255) / 255.0f, (value & 255) / 255.0f};
    }
    return {1, 1, 1, 1};
}

void push_color(ColorRole role, Color color) {
    internal::ctx().color_stack.push_back({role, color});
}

void pop_color() {
    std::vector<internal::ColorOverride>& stack = internal::ctx().color_stack;
    if (!stack.empty()) stack.pop_back();
}

void set_next_color(ColorRole role, Color color) {
    internal::ctx().next_colors.push_back({role, color});
}

bool create_server(const Config& cfg) {
    using namespace internal;
    Context& c = ctx();
    c.cfg = cfg;
    c.current_url = std::string("http://127.0.0.1:") + std::to_string(cfg.port) + "/";
    std::srand((unsigned)std::time(nullptr));
    c.login_session_token.clear();
    c.login_session_until = 0;
    c.toasts.clear();
    c.color_stack.clear();
    c.next_colors.clear();
    c.next_toast_id = 1;
    debugf("create_server title='%s' host='%s' port=%d max_request_bytes=%d read_timeout_ms=%d client_poll_ms=%d auto_submit=%d debug=%d client_debug=%d dark_mode=%d browser_dark=%d login_mode=%d login_idle_seconds=%d",
           cfg.title ? cfg.title : "", cfg.host ? cfg.host : "", cfg.port, cfg.max_request_bytes,
           cfg.read_timeout_ms, cfg.client_poll_ms, cfg.auto_submit ? 1 : 0,
           cfg.debug_output ? 1 : 0, cfg.client_debug_output ? 1 : 0, cfg.dark_mode ? 1 : 0,
           cfg.respect_browser_dark_mode ? 1 : 0, (int)cfg.login_mode, cfg.login_session_seconds);
#if defined(ARDUINO) && defined(ESP32)
    if (c.net.server) delete c.net.server;
    c.net.server = new WiFiServer((uint16_t)cfg.port);
    c.net.server->begin();
    debugf("ESP32 WiFiServer started on port %d", cfg.port);
    if (cfg.print_url) {
        Serial.print("FTHT listening on port ");
        Serial.println(cfg.port);
    }
    return true;
#else
#if defined(_WIN32)
    if (!c.net.wsa_started) {
        WSADATA wsa;
        int wsa_result = WSAStartup(MAKEWORD(2, 2), &wsa);
        if (wsa_result != 0) {
            debugf("WSAStartup failed err=%d", wsa_result);
            return false;
        }
        c.net.wsa_started = true;
        debugf("WSAStartup succeeded");
    }
#endif
    c.net.server = socket(AF_INET, SOCK_STREAM, 0);
    if (c.net.server == invalid_socket) {
        debugf("socket(AF_INET, SOCK_STREAM) failed err=%d", last_net_error());
        return false;
    }
    debugf("server socket created");
    int yes = 1;
    setsockopt(c.net.server, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)cfg.port);
    if (!cfg.host || std::strcmp(cfg.host, "0.0.0.0") == 0) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_pton(AF_INET, cfg.host, &addr.sin_addr) != 1) {
            debugf("inet_pton failed for host '%s'", cfg.host);
            close_socket(c.net.server);
            return false;
        }
    }
    if (bind(c.net.server, (sockaddr*)&addr, sizeof(addr)) != 0) {
        debugf("bind failed host='%s' port=%d err=%d", cfg.host ? cfg.host : "", cfg.port, last_net_error());
        close_socket(c.net.server);
        return false;
    }
    debugf("bind succeeded host='%s' port=%d", cfg.host ? cfg.host : "", cfg.port);
    if (listen(c.net.server, 8) != 0) {
        debugf("listen failed err=%d", last_net_error());
        close_socket(c.net.server);
        return false;
    }
    debugf("listen succeeded backlog=8");
    if (cfg.print_url) std::printf("FTHT listening at %s\n", c.current_url.c_str());
    return true;
#endif
}

bool pump(int timeout_ms) {
    using namespace internal;
    Context& c = ctx();
    c.request_active = false;
    c.response_open = false;
    c.status_code = 200;
    c.status_text = "OK";
    bool ok = accept_next_request(timeout_ms);
    c.request_active = ok;
    if (!ok && timeout_ms >= 0) debugf("pump timeout/no request");
    if (ok) {
        execute_posted_command_if_needed();
        debugf("pump has request: %s %s", c.req_method.c_str(), c.req_path.c_str());
    }
    return ok;
}

void begin() {
    using namespace internal;
    Context& c = ctx();
    debugf("begin render for %s %s", c.req_method.c_str(), c.req_path.c_str());
    c.out.clear();
    c.response_open = true;
    c.modal_rendered = false;
    if (c.client_update) {
        c.out += "<main><div class=\"ft-bleed\"></div><form method=\"post\" action=\"";
        c.out += escape_html(c.req_path.empty() ? "/" : c.req_path);
        c.out += "\"><h1 class=\"ft-title\">";
        c.out += escape_html(c.cfg.title ? c.cfg.title : "FTHT App");
        c.out += "</h1>";
        return;
    }
    c.out += "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">";
    c.out += "<title>" + escape_html(c.cfg.title ? c.cfg.title : "FTHT App") + "</title>";
    c.out += "<link rel=\"icon\" href=\"/favicon.svg\" type=\"image/svg+xml\">";
    append_css();
    append(c.out, c.cfg.extra_head_html);
    c.out += "</head><body>";
    append_ink_filters();
    c.out += "<main><div class=\"ft-bleed\"></div><form method=\"post\" action=\"";
    c.out += escape_html(c.req_path.empty() ? "/" : c.req_path);
    c.out += "\">";
    c.out += "<h1 class=\"ft-title\">";
    c.out += escape_html(c.cfg.title ? c.cfg.title : "FTHT App");
    c.out += "</h1>";
}

void end() {
    using namespace internal;
    Context& c = ctx();
    if (!c.modal_label.empty() && !c.modal_rendered) {
        c.out += "<div class=\"ft-modal\"><section>";
        c.out += "<p class=\"ft-text\">";
        c.out += escape_html(c.modal_label);
        c.out += "</p><button name=\"_ftht_action\" value=\"_ftht_close_modal\">Close</button></section></div>";
    }
    append_toast_stack();
    c.out += "</form></main>";
    if (!c.client_update) {
        append_toast_script();
        append_client_script();
        c.out += "</body></html>";
    }
    debugf("end render html_bytes=%d status=%d", (int)c.out.size(), c.status_code);
    send_response(c.out);
    c.response_open = false;
    c.request_active = false;
}

void shutdown() {
    using namespace internal;
    Context& c = ctx();
    debugf("shutdown");
#if defined(ARDUINO) && defined(ESP32)
    if (c.net.server) {
        delete c.net.server;
        c.net.server = nullptr;
    }
#else
    close_socket(c.net.client);
    close_socket(c.net.server);
#if defined(_WIN32)
    if (c.net.wsa_started) {
        WSACleanup();
        c.net.wsa_started = false;
    }
#endif
#endif
}

const Config& config() { return internal::ctx().cfg; }
const char* url() { return internal::ctx().current_url.c_str(); }
const char* path() { return internal::ctx().req_path.c_str(); }
const char* method() { return internal::ctx().req_method.c_str(); }
bool is_post() { return internal::ctx().req_method == "POST"; }
const char* param(const char* name) { return internal::find_param(name); }
uint64_t make_login_password_hash(const char* password, const char* salt) {
    return internal::salted_password_hash(password ? password : "", salt ? salt : "");
}

void set_command_handler(CommandHandler handler) {
    internal::ctx().command_handler = handler;
}

void set_status(int code, const char* text) {
    internal::ctx().status_code = code;
    internal::ctx().status_text = text ? text : "";
}

void html(const char* markup) {
    internal::append(internal::ctx().out, markup);
}

void text(const char* label) {
    internal::ctx().out += "<p class=\"ft-text\"" + internal::next_style() + ">" + internal::escape_html(label ? label : "") + "</p>";
}

void text_wrapped(const char* value) {
    internal::ctx().out += "<p class=\"ft-wrap\"" + internal::next_style() + ">" + internal::escape_html(value ? value : "") + "</p>";
}

void separator() {
    internal::ctx().out += "<hr>";
}

void spacing(float px) {
    char buf[80];
    std::snprintf(buf, sizeof(buf), "<div style=\"height:%.1fpx\"></div>", px);
    internal::ctx().out += buf;
}

void toast(const char* message) {
    Toast t;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void toast(const Toast& value) {
    internal::queue_toast(value);
}

void toast_info(const char* message) {
    Toast t;
    t.type = ToastType::Info;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void toast_success(const char* message) {
    Toast t;
    t.type = ToastType::Success;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void toast_warning(const char* message) {
    Toast t;
    t.type = ToastType::Warning;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void toast_error(const char* message) {
    Toast t;
    t.type = ToastType::Error;
    t.message = message ? message : "";
    internal::queue_toast(t);
}

void clear_toasts() {
    internal::ctx().toasts.clear();
}

bool input(const char* label, char* buffer, int buffer_size, InputFlags flags, bool* enter_pressed) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && !(flags & InputFlags::ReadOnly) && ctx().disabled_depth == 0) {
        std::string value = posted;
        if (flags & InputFlags::CharsUppercase) {
            for (char& ch : value) ch = (char)std::toupper((unsigned char)ch);
        }
        if (buffer && std::strcmp(buffer, value.c_str()) != 0) {
            copy_to_buffer(buffer, buffer_size, value);
            changed = true;
            debugf("input changed id=%s label='%s' bytes=%d", id.c_str(), visible_label(label).c_str(), (int)value.size());
        }
    }
    if (enter_pressed) *enter_pressed = changed;
    begin_field(label, id);
    ctx().out += "<input name=\"" + id + "\" value=\"" + escape_html(buffer ? buffer : "") + "\"";
    ctx().out += (flags & InputFlags::Password) ? " type=\"password\"" : " type=\"text\"";
    if (flags & InputFlags::ReadOnly) ctx().out += " readonly";
    if (flags & InputFlags::CharsDecimal) ctx().out += " inputmode=\"decimal\" pattern=\"[0-9.\\-]*\"";
    if (flags & InputFlags::CharsHexadecimal) ctx().out += " pattern=\"[0-9a-fA-F]*\"";
    if (flags & InputFlags::CharsNoBlank) ctx().out += " pattern=\"\\S*\"";
    ctx().out += disabled_attr() + auto_submit_attr() + "></label>";
    return changed;
}

bool text_area(const char* label, char* buffer, int buffer_size, int rows) {
    return text_area_ex(label, buffer, buffer_size, rows, TextAreaFlags::Default);
}

bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows, TextAreaFlags flags) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && !(flags & TextAreaFlags::ReadOnly) && ctx().disabled_depth == 0) {
        if (buffer && std::strcmp(buffer, posted) != 0) {
            copy_to_buffer(buffer, buffer_size, posted);
            changed = true;
            debugf("text_area changed id=%s label='%s' bytes=%d", id.c_str(), visible_label(label).c_str(), (int)std::strlen(posted));
        }
    }
    begin_field(label, id);
    char row_buf[32];
    std::snprintf(row_buf, sizeof(row_buf), " rows=\"%d\"", rows);
    ctx().out += "<textarea name=\"" + id + "\"" + row_buf;
    if (flags & TextAreaFlags::ReadOnly) ctx().out += " readonly";
    ctx().out += disabled_attr() + auto_submit_attr() + ">";
    ctx().out += escape_html(buffer ? buffer : "");
    ctx().out += "</textarea></label>";
    return changed;
}

void log_view(const char* label, const char* value, int rows, LogViewFlags) {
    using namespace internal;
    std::string id = widget_id(label);
    begin_field(label, id);
    char style[96];
    std::snprintf(style, sizeof(style), " style=\"max-height:%.1fpx\"", rows * 24.0f + 24.0f);
    ctx().out += "<pre class=\"ft-log\"";
    ctx().out += style;
    ctx().out += ">";
    ctx().out += escape_html(value ? value : "");
    ctx().out += "</pre></label>";
}

bool checkbox(const char* label, bool* value) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && value && ctx().disabled_depth == 0) {
        bool next = string_to_bool(posted);
        changed = (*value != next);
        *value = next;
        if (changed) debugf("checkbox changed id=%s label='%s' value=%d", id.c_str(), visible_label(label).c_str(), *value ? 1 : 0);
    }
    ctx().last_widget_id = id;
    ctx().out += "<label class=\"ft-check\"" + next_style() + ">";
    ctx().out += "<input type=\"hidden\" name=\"" + id + "\" value=\"0\">";
    ctx().out += "<input type=\"checkbox\" name=\"" + id + "\" value=\"1\"";
    if (value && *value) ctx().out += " checked";
    ctx().out += disabled_attr();
    ctx().out += auto_submit_attr();
    ctx().out += "><span>" + escape_html(visible_label(label)) + "</span></label>";
    return changed;
}

bool slider_float(const char* label, float* value, float min_v, float max_v) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && value && ctx().disabled_depth == 0) {
        float next = (float)std::atof(posted);
        next = std::max(min_v, std::min(max_v, next));
        changed = (*value != next);
        *value = next;
        if (changed) debugf("slider changed id=%s label='%s' value=%.6g", id.c_str(), visible_label(label).c_str(), *value);
    }
    ctx().last_widget_id = id;
    float current = value ? *value : min_v;
    float pct = max_v > min_v ? ((current - min_v) / (max_v - min_v)) * 100.0f : 0.0f;
    pct = std::max(0.0f, std::min(100.0f, pct));
    char value_buf[32];
    std::snprintf(value_buf, sizeof(value_buf), "%.2f", current);
    char* dot = std::strchr(value_buf, '.');
    if (dot) {
        char* end = value_buf + std::strlen(value_buf) - 1;
        while (end > dot && *end == '0') *end-- = '\0';
        if (end == dot) *end = '\0';
    }
    ctx().out += "<label class=\"ft-field\"" + next_style() + "><span class=\"ft-label\"><span>";
    ctx().out += escape_html(visible_label(label));
    ctx().out += "</span><output class=\"ft-range-value\">";
    ctx().out += escape_html(value_buf);
    ctx().out += "</output></span>";
    char buf[256];
    std::snprintf(buf, sizeof(buf),
                  "<input type=\"range\" name=\"%s\" min=\"%.6g\" max=\"%.6g\" step=\"any\" value=\"%.6g\" data-ftht-range=\"1\" style=\"--ft-range-pct:%.3f%%\"",
                  id.c_str(), min_v, max_v, current, pct);
    ctx().out += buf;
    ctx().out += disabled_attr() + auto_submit_attr() + "></label>";
    return changed;
}

bool button(const char* label) {
    return button(label, ColorRole::Button);
}

bool button(const char* label, ColorRole role) {
    using namespace internal;
    std::string id = widget_id(label);
    ctx().last_widget_id = id;
    std::string klass;
    if (role == ColorRole::Accent) klass = " class=\"accent\"";
    if (role == ColorRole::Warning) klass = " class=\"warning\"";
    if (role == ColorRole::Success) klass = " class=\"success\"";
    ctx().out += "<button type=\"submit\" name=\"_ftht_action\" value=\"" + id + "\"" + klass + next_style() + disabled_attr() + ">";
    ctx().out += escape_html(visible_label(label));
    ctx().out += "</button>";
    if (id == "_ftht_close_modal") close_modal();
    bool clicked = action_matches(id);
    if (clicked) debugf("button clicked id=%s label='%s'", id.c_str(), visible_label(label).c_str());
    return clicked;
}

bool button(const char* label, Color color) {
    using namespace internal;
    std::string id = widget_id(label);
    ctx().last_widget_id = id;
    std::string style = next_style();
    std::string custom = "background:" + color_css(color) + ";border-color:" + color_css(color) + ";";
    if (style.empty()) {
        style = " style=\"" + custom + "\"";
    } else {
        size_t end_quote = style.rfind('"');
        style.insert(end_quote == std::string::npos ? style.size() : end_quote, custom);
    }
    ctx().out += "<button type=\"submit\" name=\"_ftht_action\" value=\"" + id + "\"" + style + disabled_attr() + ">";
    ctx().out += escape_html(visible_label(label));
    ctx().out += "</button>";
    bool clicked = action_matches(id);
    if (clicked) debugf("button clicked id=%s label='%s' custom_color=1", id.c_str(), visible_label(label).c_str());
    return clicked;
}

bool dropdown(const char* label, const char* const* items, int count, int* selected, int) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("dropdown changed id=%s label='%s' selected=%d", id.c_str(), visible_label(label).c_str(), *selected);
        }
    }
    begin_field(label, id);
    ctx().out += "<select name=\"" + id + "\"" + disabled_attr() + auto_submit_attr() + ">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<option value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " selected";
        ctx().out += ">" + escape_html(items && items[i] ? items[i] : "") + "</option>";
    }
    ctx().out += "</select></label>";
    return changed;
}

bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("listbox changed id=%s label='%s' selected=%d", id.c_str(), visible_label(label).c_str(), *selected);
        }
    }
    begin_field(label, id);
    ctx().out += "<select name=\"" + id + "\" size=\"" + std::to_string(std::max(1, visible_rows)) + "\"" + disabled_attr() + auto_submit_attr() + ">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<option value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " selected";
        ctx().out += ">" + escape_html(items && items[i] ? items[i] : "") + "</option>";
    }
    ctx().out += "</select></label>";
    return changed;
}

bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns) {
    using namespace internal;
    std::string id = widget_id(label);
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("radio_group changed id=%s label='%s' selected=%d", id.c_str(), visible_label(label).c_str(), *selected);
        }
    }
    begin_field(label, id);
    ctx().out += "<div class=\"ft-radio\" style=\"grid-template-columns:repeat(" + std::to_string(std::max(1, columns)) + ",minmax(0,1fr))\">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<label><input type=\"radio\" name=\"" + id + "\" value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " checked";
        ctx().out += disabled_attr() + auto_submit_attr() + "><span>" + escape_html(items && items[i] ? items[i] : "") + "</span></label>";
    }
    ctx().out += "</div></label>";
    return changed;
}

bool collapsing_header(const char* label, bool* open) {
    using namespace internal;
    bool is_open = !open || *open;
    ctx().out += "<p class=\"ft-text\" style=\"font-weight:700;margin-top:6px\">" + escape_html(visible_label(label)) + "</p>";
    return is_open;
}

bool side_menu(const char* label, const char* const* items, int count, int* selected) {
    return radio_group(label, items, count, selected, 1);
}

bool side_menu_drawer(const char* label, const char* const* items, int count, int* selected, float) {
    return side_menu(label, items, count, selected);
}

bool tabs(const char* const* labels, int count, int* selected) {
    using namespace internal;
    std::string seed = "tabs";
    for (int i = 0; i < count; ++i) {
        seed += "##";
        seed += labels && labels[i] ? labels[i] : "";
    }
    std::string id = widget_id(seed.c_str());
    const char* posted = find_param(id.c_str());
    bool changed = false;
    if (posted && selected && ctx().disabled_depth == 0) {
        int next = std::atoi(posted);
        if (next >= 0 && next < count) {
            changed = (*selected != next);
            *selected = next;
            if (changed) debugf("tabs changed id=%s selected=%d", id.c_str(), *selected);
        }
    }
    ctx().out += "<div class=\"ft-tabs\"" + next_style() + ">";
    for (int i = 0; i < count; ++i) {
        ctx().out += "<button type=\"submit\" name=\"" + id + "\" value=\"" + std::to_string(i) + "\"";
        if (selected && *selected == i) ctx().out += " class=\"sel\"";
        ctx().out += disabled_attr() + ">" + escape_html(labels && labels[i] ? labels[i] : "") + "</button>";
    }
    ctx().out += "</div>";
    return changed;
}

void row(int cols, std::function<void()> fn) {
    using namespace internal;
    ctx().out += "<div class=\"ft-row\" style=\"grid-template-columns:repeat(" + std::to_string(std::max(1, cols)) + ",minmax(0,1fr))\">";
    if (fn) fn();
    ctx().out += "</div>";
}

void row(std::initializer_list<float> weights, std::function<void()> fn) {
    using namespace internal;
    ctx().out += "<div class=\"ft-row\" style=\"grid-template-columns:";
    for (float w : weights) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), " minmax(0,%.3gfr)", std::max(0.01f, w));
        ctx().out += buf;
    }
    ctx().out += "\">";
    if (fn) fn();
    ctx().out += "</div>";
}

void split(std::initializer_list<float> columns, std::function<void()> fn) {
    row(columns, fn);
}

void side_layout(float, std::function<void()> fn) {
    if (fn) fn();
}

void content(std::function<void()> fn) {
    if (fn) fn();
}

void scroll_area(const char* label, float height, std::function<void()> fn) {
    using namespace internal;
    ctx().out += "<section class=\"ft-scroll\" style=\"max-height:" + std::to_string((int)height) + "px\">";
    ctx().out += "<p class=\"ft-label\">" + escape_html(visible_label(label)) + "</p>";
    if (fn) fn();
    ctx().out += "</section>";
}

void set_next_width(float px) { internal::ctx().next.has_width = true; internal::ctx().next.width = px; }
void set_next_fill() { internal::ctx().next.fill = true; }
void set_next_percent(float pct) { internal::ctx().next.has_percent = true; internal::ctx().next.percent = pct; }
void set_next_limits(float min_px, float max_px) {
    internal::ctx().next.has_limits = true;
    internal::ctx().next.min_width = min_px;
    internal::ctx().next.max_width = max_px;
}
void set_next_align(Align align) { internal::ctx().next.has_align = true; internal::ctx().next.align = align; }

void open_modal(const char* label) {
    internal::ctx().modal_label = internal::visible_label(label);
    internal::debugf("modal opened label='%s'", internal::ctx().modal_label.c_str());
}

bool modal(const char* label, std::function<void()> fn) {
    using namespace internal;
    if (action_matches("_ftht_close_modal")) close_modal();
    if (ctx().modal_label != visible_label(label)) return false;
    ctx().modal_rendered = true;
    ctx().out += "<div class=\"ft-modal\"><section>";
    if (fn) fn();
    ctx().out += "</section></div>";
    return true;
}

void close_modal() {
    if (!internal::ctx().modal_label.empty()) {
        internal::debugf("modal closed label='%s'", internal::ctx().modal_label.c_str());
    }
    internal::ctx().modal_label.clear();
}

void begin_disabled() {
    internal::ctx().disabled_depth++;
}

void end_disabled() {
    if (internal::ctx().disabled_depth > 0) internal::ctx().disabled_depth--;
}

void tooltip(const char* text) {
    using namespace internal;
    if (!text || ctx().last_widget_id.empty()) return;
    ctx().out += "<small class=\"ft-wrap\">" + escape_html(text) + "</small>";
}

void request_focus(const char*) {}

float calc_text_width(const char* value) {
    return value ? (float)std::strlen(value) * get_style().font_size * 0.55f : 0.0f;
}

float calc_text_height(const char* value, float wrap_width) {
    if (!value) return get_style().font_size * 1.45f;
    float line_w = std::max(1.0f, wrap_width);
    float chars_per_line = std::max(1.0f, line_w / (get_style().font_size * 0.55f));
    int lines = 1;
    int col = 0;
    for (const char* p = value; *p; ++p) {
        if (*p == '\n' || ++col >= (int)chars_per_line) {
            lines++;
            col = 0;
        }
    }
    return lines * get_style().font_size * 1.45f;
}

} // namespace ftht

#endif // FTHT_IMPLEMENTATION


#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <string>
#include <string_view>

#if defined(_WIN32)
#include <io.h>
#else
#include <unistd.h>
#endif

namespace ft {

enum class Mode {
    Auto,
    Gui,
    Tui,
    Web,
};

using Align = ftui::Align;
using Color = ftui::Color;
using ColorRole = ftui::ColorRole;
using BuiltinIcon = ftui::BuiltinIcon;
using BackdropEffect = ftui::BackdropEffect;
using WindowTransparency = ftui::WindowTransparency;
using InputFlags = ftui::InputFlags;
using TextAreaFlags = ftui::TextAreaFlags;
using LogViewFlags = ftui::LogViewFlags;
using ToastType = ftui::ToastType;
using LoginMode = ftht::LoginMode;
using FileFilter = ftui::FileFilter;
using DebugState = ftui::DebugState;
using CommandHandler = bool (*)(const char* command);

struct Style {
    Color background    = {0.055f, 0.055f, 0.059f, 1.0f};
    Color panel         = {0.090f, 0.094f, 0.102f, 1.0f};
    Color text          = {0.910f, 0.910f, 0.910f, 1.0f};
    Color text_dim      = {0.663f, 0.678f, 0.702f, 1.0f};
    Color border        = {0.169f, 0.176f, 0.192f, 1.0f};
    Color button        = {0.090f, 0.094f, 0.102f, 1.0f};
    Color button_hover  = {0.137f, 0.149f, 0.169f, 1.0f};
    Color button_active = {0.176f, 0.192f, 0.220f, 1.0f};
    Color input_bg      = {0.071f, 0.075f, 0.082f, 1.0f};
    Color input_focus   = {0.310f, 0.420f, 0.780f, 1.0f};
    Color accent        = {0.310f, 0.420f, 0.780f, 1.0f};
    Color warning       = {0.894f, 0.486f, 0.353f, 1.0f};
    Color success       = {0.420f, 0.741f, 0.522f, 1.0f};
    float window_padding = 20.0f;
    float item_spacing   = 10.0f;
    float item_height    = 36.0f;
    float rounding       = 8.0f;
    float border_width   = 1.0f;
    float font_size      = 16.0f;
};

struct Config {
    Mode        mode          = Mode::Auto;
    const char* title         = "FT App";
    int         width         = 960;
    int         height        = 640;
    int         fps_limit     = 60;
    bool        resizable     = true;
    bool        center_window = true;
    void*       icon          = nullptr;
    bool        enable_effects = true;
    BackdropEffect backdrop_effect = BackdropEffect::Blur;
    int         dither_size   = 4;
    WindowTransparency window_transparency = WindowTransparency::Opaque;
    float       window_opacity = 0.92f;

    bool        tui_alternate_screen = true;
    bool        tui_enable_mouse = true;
    bool        tui_quit_on_ctrl_q = true;
    bool        tui_animate = true;

    const char* web_host = "0.0.0.0";
    int         web_port = 8080;
    int         web_max_request_bytes = 32 * 1024;
    int         web_read_timeout_ms = 1500;
    int         web_client_poll_ms = 0;
    bool        web_auto_submit = true;
    bool        web_print_url = true;
    bool        web_debug_output = false;
    bool        web_client_debug_output = false;
    bool        web_dark_mode = false;
    bool        web_respect_browser_dark_mode = false;
    const char* web_extra_head_html = nullptr;
    LoginMode   web_login_mode = LoginMode::None;
    const char* web_login_username = "admin";
    const char* web_login_password = nullptr;
    const char* web_login_salt = "ftht-login";
    const char* web_login_db_path = nullptr;
    uint64_t    web_login_password_hash = 0;
    int         web_login_session_seconds = 10 * 60;
    bool        web_login_allow_registration = false;
};

struct Toast {
    const char* message = "";
    ToastType   type = ToastType::Info;
    int         duration_ms = 3500;
    bool        dismissible = true;
};

struct ProgressStyle {
    const char* label = nullptr;
    const char* mask_path = nullptr;
    const char* mask_svg = nullptr;
    const char* mask_shape = nullptr;
    ColorRole   fill_role = ColorRole::Accent;
    Color       fill_color = {-1.0f, -1.0f, -1.0f, -1.0f};
    float       height = 22.0f;
    bool        show_percent = true;
    bool        wave_front = false;
    bool        glint = false;
};

struct ImageHandle {
    ftui::ImageHandle* gui = nullptr;
    ftti::ImageHandle* tui = nullptr;
};

namespace detail {

enum class FrameTarget {
    Normal,
    GuiWebControl,
    TuiWebControl,
    WebMirror,
};

inline Mode& active_mode_ref() {
    static Mode value = Mode::Auto;
    return value;
}

inline FrameTarget& frame_target_ref() {
    static FrameTarget value = FrameTarget::Normal;
    return value;
}

inline bool& webserver_active_ref() {
    static bool value = false;
    return value;
}

inline Config& active_config_ref() {
    static Config value;
    return value;
}

inline Style& style_ref() {
    static Style value;
    return value;
}

inline DebugState& fallback_debug_ref() {
    static DebugState value;
    return value;
}

inline std::string shell_quote(const char* value) {
    std::string out = "'";
    for (const char* p = value ? value : ""; *p; ++p) {
        if (*p == '\'') out += "'\\''";
        else out += *p;
    }
    out += "'";
    return out;
}

inline void open_url_in_browser(const char* url) {
    if (!url || !*url) return;
#if defined(_WIN32)
    std::string cmd = "start \"\" \"";
    for (const char* p = url; *p; ++p) {
        if (*p != '"') cmd += *p;
    }
    cmd += "\"";
    std::system(cmd.c_str());
#elif defined(__APPLE__)
    std::string cmd = "open " + shell_quote(url) + " >/dev/null 2>&1 &";
    std::system(cmd.c_str());
#else
    std::string cmd = "xdg-open " + shell_quote(url) + " >/dev/null 2>&1 &";
    std::system(cmd.c_str());
#endif
}

inline bool user_widgets_suppressed() {
    FrameTarget target = frame_target_ref();
    return target == FrameTarget::GuiWebControl || target == FrameTarget::TuiWebControl;
}

inline Mode render_mode() {
    if (user_widgets_suppressed()) return Mode::Auto;
    if (frame_target_ref() == FrameTarget::WebMirror) return Mode::Web;
    return active_mode_ref();
}

inline bool has_display() {
#if defined(_WIN32)
    return true;
#else
    const char* display = std::getenv("DISPLAY");
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    return (display && display[0]) || (wayland && wayland[0]);
#endif
}

inline bool has_tty() {
#if defined(_WIN32)
    return _isatty(0) && _isatty(1);
#else
    return ::isatty(STDIN_FILENO) && ::isatty(STDOUT_FILENO);
#endif
}

inline Mode parse_mode_flag(int argc, char** argv, Mode fallback) {
    Mode mode = fallback;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (!arg) continue;
        if (std::strcmp(arg, "--ft-auto") == 0) mode = Mode::Auto;
        else if (std::strcmp(arg, "--ft-gui") == 0 || std::strcmp(arg, "--gui") == 0) mode = Mode::Gui;
        else if (std::strcmp(arg, "--ft-tui") == 0 || std::strcmp(arg, "--tui") == 0) mode = Mode::Tui;
        else if (std::strcmp(arg, "--ft-web") == 0 || std::strcmp(arg, "--web") == 0) mode = Mode::Web;
        else if (std::strncmp(arg, "--ft-mode=", 10) == 0) {
            const char* value = arg + 10;
            if (std::strcmp(value, "auto") == 0) mode = Mode::Auto;
            else if (std::strcmp(value, "gui") == 0) mode = Mode::Gui;
            else if (std::strcmp(value, "tui") == 0) mode = Mode::Tui;
            else if (std::strcmp(value, "web") == 0) mode = Mode::Web;
        }
    }
    return mode;
}

inline Mode resolve_auto_mode() {
    if (has_display()) return Mode::Gui;
    if (has_tty()) return Mode::Tui;
    return Mode::Web;
}

inline ftti::Color to_tui(Color c) {
    return {c.r * 255.0f, c.g * 255.0f, c.b * 255.0f, c.a};
}

inline ftht::Color to_web(Color c) {
    return {c.r, c.g, c.b, c.a};
}

inline ftui::Style to_gui_style(const Style& s) {
    ftui::Style out;
    out.background = s.background;
    out.panel = s.panel;
    out.text = s.text;
    out.text_dim = s.text_dim;
    out.border = s.border;
    out.button = s.button;
    out.button_hover = s.button_hover;
    out.button_active = s.button_active;
    out.input_bg = s.input_bg;
    out.input_focus = s.input_focus;
    out.accent = s.accent;
    out.warning = s.warning;
    out.success = s.success;
    out.window_padding = s.window_padding;
    out.item_spacing = s.item_spacing;
    out.item_height = s.item_height;
    out.rounding = s.rounding;
    out.border_width = s.border_width;
    out.font_size = s.font_size;
    return out;
}

inline Style from_gui_style(const ftui::Style& s) {
    Style out;
    out.background = s.background;
    out.panel = s.panel;
    out.text = s.text;
    out.text_dim = s.text_dim;
    out.border = s.border;
    out.button = s.button;
    out.button_hover = s.button_hover;
    out.button_active = s.button_active;
    out.input_bg = s.input_bg;
    out.input_focus = s.input_focus;
    out.accent = s.accent;
    out.warning = s.warning;
    out.success = s.success;
    out.window_padding = s.window_padding;
    out.item_spacing = s.item_spacing;
    out.item_height = s.item_height;
    out.rounding = s.rounding;
    out.border_width = s.border_width;
    out.font_size = s.font_size;
    return out;
}

inline ftht::Style to_web_style(const Style& s) {
    ftht::Style out;
    out.background = to_web(s.background);
    out.panel = to_web(s.panel);
    out.text = to_web(s.text);
    out.text_dim = to_web(s.text_dim);
    out.border = to_web(s.border);
    out.button = to_web(s.button);
    out.button_hover = to_web(s.button_hover);
    out.button_active = to_web(s.button_active);
    out.input_bg = to_web(s.input_bg);
    out.input_focus = to_web(s.input_focus);
    out.accent = to_web(s.accent);
    out.warning = to_web(s.warning);
    out.success = to_web(s.success);
    out.window_padding = s.window_padding;
    out.item_spacing = s.item_spacing;
    out.item_height = s.item_height;
    out.rounding = s.rounding;
    out.border_width = s.border_width;
    out.font_size = s.font_size;
    return out;
}

inline ftti::Style to_tui_style(const Style& s) {
    ftti::Style out;
    out.background = to_tui(s.background);
    out.panel = to_tui(s.panel);
    out.panel_alt = to_tui(s.button_hover);
    out.text = to_tui(s.text);
    out.text_dim = to_tui(s.text_dim);
    out.border = to_tui(s.border);
    out.button = to_tui(s.button);
    out.button_hover = to_tui(s.button_hover);
    out.button_active = to_tui(s.button_active);
    out.input_bg = to_tui(s.input_bg);
    out.input_focus = to_tui(s.input_focus);
    out.accent = to_tui(s.accent);
    out.warning = to_tui(s.warning);
    out.success = to_tui(s.success);
    out.window_padding = std::max(1.0f, s.window_padding / 10.0f);
    out.item_spacing = std::max(1.0f, s.item_spacing / 10.0f);
    out.item_height = std::max(1.0f, s.item_height / 12.0f);
    out.rounding = 0.0f;
    out.border_width = s.border_width;
    out.font_size = 1.0f;
    return out;
}

inline ftti::ColorRole to_tui_role(ColorRole role) {
    switch (role) {
        case ColorRole::Background: return ftti::ColorRole::Background;
        case ColorRole::Panel: return ftti::ColorRole::Panel;
        case ColorRole::Text: return ftti::ColorRole::Text;
        case ColorRole::TextDim: return ftti::ColorRole::TextDim;
        case ColorRole::Border: return ftti::ColorRole::Border;
        case ColorRole::Button: return ftti::ColorRole::Button;
        case ColorRole::ButtonHover: return ftti::ColorRole::ButtonHover;
        case ColorRole::ButtonActive: return ftti::ColorRole::ButtonActive;
        case ColorRole::InputBg: return ftti::ColorRole::InputBg;
        case ColorRole::InputFocus: return ftti::ColorRole::InputFocus;
        case ColorRole::Accent: return ftti::ColorRole::Accent;
        case ColorRole::Warning: return ftti::ColorRole::Warning;
        case ColorRole::Success: return ftti::ColorRole::Success;
    }
    return ftti::ColorRole::Accent;
}

inline ftht::ColorRole to_web_role(ColorRole role) {
    switch (role) {
        case ColorRole::Background: return ftht::ColorRole::Background;
        case ColorRole::Panel: return ftht::ColorRole::Panel;
        case ColorRole::Text: return ftht::ColorRole::Text;
        case ColorRole::TextDim: return ftht::ColorRole::TextDim;
        case ColorRole::Border: return ftht::ColorRole::Border;
        case ColorRole::Button: return ftht::ColorRole::Button;
        case ColorRole::ButtonHover: return ftht::ColorRole::ButtonHover;
        case ColorRole::ButtonActive: return ftht::ColorRole::ButtonActive;
        case ColorRole::InputBg: return ftht::ColorRole::InputBg;
        case ColorRole::InputFocus: return ftht::ColorRole::InputFocus;
        case ColorRole::Accent: return ftht::ColorRole::Accent;
        case ColorRole::Warning: return ftht::ColorRole::Warning;
        case ColorRole::Success: return ftht::ColorRole::Success;
    }
    return ftht::ColorRole::Accent;
}

inline ftti::InputFlags to_tui_input_flags(InputFlags flags) {
    unsigned value = static_cast<unsigned>(flags);
    return static_cast<ftti::InputFlags>(value);
}

inline ftht::InputFlags to_web_input_flags(InputFlags flags) {
    unsigned value = static_cast<unsigned>(flags);
    return static_cast<ftht::InputFlags>(value);
}

inline ftti::TextAreaFlags to_tui_text_area_flags(TextAreaFlags flags) {
    return static_cast<ftti::TextAreaFlags>(static_cast<unsigned>(flags));
}

inline ftht::TextAreaFlags to_web_text_area_flags(TextAreaFlags flags) {
    return static_cast<ftht::TextAreaFlags>(static_cast<unsigned>(flags));
}

inline ftti::LogViewFlags to_tui_log_flags(LogViewFlags flags) {
    return static_cast<ftti::LogViewFlags>(static_cast<unsigned>(flags));
}

inline ftht::LogViewFlags to_web_log_flags(LogViewFlags flags) {
    return static_cast<ftht::LogViewFlags>(static_cast<unsigned>(flags));
}

inline ftti::ToastType to_tui_toast_type(ToastType type) {
    return static_cast<ftti::ToastType>(static_cast<int>(type));
}

inline ftht::ToastType to_web_toast_type(ToastType type) {
    return static_cast<ftht::ToastType>(static_cast<int>(type));
}

inline ftui::Config to_gui_config(const Config& cfg) {
    ftui::Config out;
    out.title = cfg.title;
    out.width = cfg.width;
    out.height = cfg.height;
    out.fps_limit = cfg.fps_limit;
    out.resizable = cfg.resizable;
    out.center_window = cfg.center_window;
    out.icon = cfg.icon;
    out.enable_effects = cfg.enable_effects;
    out.backdrop_effect = cfg.backdrop_effect;
    out.dither_size = cfg.dither_size;
    out.window_transparency = cfg.window_transparency;
    out.window_opacity = cfg.window_opacity;
    return out;
}

inline ftti::Config to_tui_config(const Config& cfg) {
    ftti::Config out;
    out.title = cfg.title;
    out.width = cfg.width;
    out.height = cfg.height;
    out.fps_limit = cfg.fps_limit > 0 ? cfg.fps_limit : 30;
    out.resizable = cfg.resizable;
    out.center_window = cfg.center_window;
    out.icon = cfg.icon;
    out.enable_effects = cfg.enable_effects;
    out.backdrop_effect = static_cast<ftti::BackdropEffect>(static_cast<int>(cfg.backdrop_effect));
    out.dither_size = cfg.dither_size;
    out.alternate_screen = cfg.tui_alternate_screen;
    out.enable_mouse = cfg.tui_enable_mouse;
    out.quit_on_ctrl_q = cfg.tui_quit_on_ctrl_q;
    out.animate = cfg.tui_animate;
    return out;
}

inline ftht::Config to_web_config(const Config& cfg) {
    ftht::Config out;
    out.title = cfg.title;
    out.host = cfg.web_host;
    out.port = cfg.web_port;
    out.max_request_bytes = cfg.web_max_request_bytes;
    out.read_timeout_ms = cfg.web_read_timeout_ms;
    out.client_poll_ms = cfg.web_client_poll_ms;
    out.auto_submit = cfg.web_auto_submit;
    out.print_url = cfg.web_print_url;
    out.debug_output = cfg.web_debug_output;
    out.client_debug_output = cfg.web_client_debug_output;
    out.dark_mode = cfg.web_dark_mode;
    out.respect_browser_dark_mode = cfg.web_respect_browser_dark_mode;
    out.extra_head_html = cfg.web_extra_head_html;
    out.login_mode = cfg.web_login_mode;
    out.login_username = cfg.web_login_username;
    out.login_password = cfg.web_login_password;
    out.login_salt = cfg.web_login_salt;
    out.login_db_path = cfg.web_login_db_path;
    out.login_password_hash = cfg.web_login_password_hash;
    out.login_session_seconds = cfg.web_login_session_seconds;
    out.login_allow_registration = cfg.web_login_allow_registration;
    return out;
}

inline bool start_webserver_internal() {
    if (webserver_active_ref()) return true;
    if (active_mode_ref() == Mode::Web) return true;
    if (!ftht::create_server(to_web_config(active_config_ref()))) return false;
    webserver_active_ref() = true;
    frame_target_ref() = FrameTarget::Normal;
    open_url_in_browser(ftht::url());
    if (active_mode_ref() == Mode::Gui) {
        ftui::set_window_size(86, 76);
        ftui::request_redraw();
    } else if (active_mode_ref() == Mode::Tui) {
        ftti::request_redraw();
    }
    return true;
}

inline void stop_webserver_internal() {
    if (!webserver_active_ref()) return;
    ftht::shutdown();
    webserver_active_ref() = false;
    frame_target_ref() = FrameTarget::Normal;
    const Config& cfg = active_config_ref();
    if (active_mode_ref() == Mode::Gui) {
        ftui::set_window_size(cfg.width, cfg.height);
        ftui::request_redraw();
    } else if (active_mode_ref() == Mode::Tui) {
        ftti::request_redraw();
    }
}

inline bool toggle_webserver_command(const char* command) {
    if (!command) return false;
    if (std::strcmp(command, "web") != 0 &&
        std::strcmp(command, "ws") != 0 &&
        std::strcmp(command, "webserver") != 0 &&
        std::strcmp(command, "server") != 0) {
        return false;
    }
    if (webserver_active_ref()) stop_webserver_internal();
    else start_webserver_internal();
    return true;
}

inline bool create_backend(Mode mode, const Config& cfg) {
    active_mode_ref() = mode;
    frame_target_ref() = FrameTarget::Normal;
    webserver_active_ref() = false;
    if (mode == Mode::Gui) {
        bool ok = ftui::create_window(to_gui_config(cfg));
        if (ok) ftui::set_command_handler(toggle_webserver_command);
        return ok;
    }
    if (mode == Mode::Tui) {
        bool ok = ftti::create_window(to_tui_config(cfg));
        if (ok) ftti::set_command_handler(toggle_webserver_command);
        return ok;
    }
    if (mode == Mode::Web) return ftht::create_server(to_web_config(cfg));
    return false;
}

} // namespace detail

inline Mode active_mode() {
    return detail::active_mode_ref();
}

inline const Config& config() {
    return detail::active_config_ref();
}

inline bool webserver_running() {
    return detail::webserver_active_ref() || active_mode() == Mode::Web;
}

inline bool start_webserver() {
    return detail::start_webserver_internal();
}

inline void stop_webserver() {
    detail::stop_webserver_internal();
}

inline bool create_window(const Config& cfg = {}, int argc = 0, char** argv = nullptr) {
    Config effective = cfg;
    if (argc > 0 && argv) effective.mode = detail::parse_mode_flag(argc, argv, effective.mode);
    detail::active_config_ref() = effective;
    if (effective.mode != Mode::Auto) return detail::create_backend(effective.mode, effective);

    Mode first = detail::resolve_auto_mode();
    if (detail::create_backend(first, effective)) return true;
    if (first != Mode::Tui && detail::has_tty() && detail::create_backend(Mode::Tui, effective)) return true;
    if (first != Mode::Web && detail::create_backend(Mode::Web, effective)) return true;
    return false;
}

inline bool create_window(int argc, char** argv, const Config& cfg = {}) {
    return create_window(cfg, argc, argv);
}

inline bool pump() {
    Mode mode = active_mode();
    detail::frame_target_ref() = detail::FrameTarget::Normal;
    if (mode == Mode::Gui) {
        bool ok = ftui::pump();
        if (!ok) return false;
        if (detail::webserver_active_ref()) {
            detail::frame_target_ref() = ftht::pump(0)
                ? detail::FrameTarget::WebMirror
                : detail::FrameTarget::GuiWebControl;
        }
        return true;
    }
    if (mode == Mode::Tui) {
        bool ok = ftti::pump();
        if (!ok) return false;
        if (detail::webserver_active_ref()) {
            detail::frame_target_ref() = ftht::pump(0)
                ? detail::FrameTarget::WebMirror
                : detail::FrameTarget::TuiWebControl;
        }
        return true;
    }
    if (mode == Mode::Web) return ftht::pump(-1);
    return false;
}

inline void begin() {
    detail::FrameTarget target = detail::frame_target_ref();
    if (target == detail::FrameTarget::WebMirror) {
        ftht::begin();
        return;
    }
    if (target == detail::FrameTarget::GuiWebControl) {
        ftui::begin();
        ftui::set_next_width(42.0f);
        ftui::set_next_align(ftui::Align::Center);
        if (ftui::button("O", ftui::ColorRole::Warning)) detail::stop_webserver_internal();
        ftui::tooltip(ftht::url());
        return;
    }
    if (target == detail::FrameTarget::TuiWebControl) {
        ftti::begin();
        ftti::text("Web server running");
        ftti::text_wrapped(ftht::url());
        if (ftti::button("Stop web server", ftti::ColorRole::Warning)) detail::stop_webserver_internal();
        return;
    }
    Mode mode = active_mode();
    if (mode == Mode::Gui) ftui::begin();
    else if (mode == Mode::Tui) ftti::begin();
    else if (mode == Mode::Web) ftht::begin();
}

inline void end() {
    detail::FrameTarget target = detail::frame_target_ref();
    if (target == detail::FrameTarget::WebMirror) {
        ftht::end();
        if (detail::render_mode() == Mode::Gui) ftui::request_redraw();
        else if (detail::render_mode() == Mode::Tui) ftti::request_redraw();
        return;
    }
    if (target == detail::FrameTarget::GuiWebControl) {
        ftui::end();
        ftui::request_redraw();
        return;
    }
    if (target == detail::FrameTarget::TuiWebControl) {
        ftti::end();
        ftti::request_redraw();
        return;
    }
    Mode mode = active_mode();
    if (mode == Mode::Gui) ftui::end();
    else if (mode == Mode::Tui) ftti::end();
    else if (mode == Mode::Web) ftht::end();
}

inline void shutdown() {
    detail::stop_webserver_internal();
    Mode mode = active_mode();
    if (mode == Mode::Gui) ftui::shutdown();
    else if (mode == Mode::Tui) ftti::shutdown();
    else if (mode == Mode::Web) ftht::shutdown();
    detail::active_mode_ref() = Mode::Auto;
}

inline void request_redraw() {
    Mode mode = active_mode();
    if (mode == Mode::Gui) ftui::request_redraw();
    else if (mode == Mode::Tui) ftti::request_redraw();
}

inline void set_quit_on_ctrl_q(bool enabled) {
    if (detail::render_mode() == Mode::Gui) ftui::set_quit_on_ctrl_q(enabled);
    else if (detail::render_mode() == Mode::Tui) ftti::set_quit_on_ctrl_q(enabled);
}

inline void set_command_handler(CommandHandler handler) {
    if (detail::render_mode() == Mode::Gui) ftui::set_command_handler(handler);
    else if (detail::render_mode() == Mode::Tui) ftti::set_command_handler(handler);
    else if (detail::render_mode() == Mode::Web) ftht::set_command_handler(handler);
}

inline void set_fps_limit(int fps) {
    if (detail::render_mode() == Mode::Gui) ftui::set_fps_limit(fps);
    else if (detail::render_mode() == Mode::Tui) ftti::set_fps_limit(fps);
}

inline int get_fps_limit() {
    if (detail::render_mode() == Mode::Gui) return ftui::get_fps_limit();
    if (detail::render_mode() == Mode::Tui) return ftti::get_fps_limit();
    return 0;
}

inline void set_backdrop_effect(BackdropEffect effect) {
    if (detail::render_mode() == Mode::Gui) ftui::set_backdrop_effect(effect);
    else if (detail::render_mode() == Mode::Tui) ftti::set_backdrop_effect(static_cast<ftti::BackdropEffect>(static_cast<int>(effect)));
}

inline void set_dither_size(int px) {
    if (detail::render_mode() == Mode::Gui) ftui::set_dither_size(px);
    else if (detail::render_mode() == Mode::Tui) ftti::set_dither_size(px);
}

inline void set_window_transparency(WindowTransparency mode) {
    if (detail::render_mode() == Mode::Gui) ftui::set_window_transparency(mode);
}

inline void set_window_opacity(float opacity) {
    if (detail::render_mode() == Mode::Gui) ftui::set_window_opacity(opacity);
}

inline void set_window_icon(void* native_icon) {
    if (detail::render_mode() == Mode::Gui) ftui::set_window_icon(native_icon);
    else if (detail::render_mode() == Mode::Tui) ftti::set_window_icon(native_icon);
}

inline void set_window_icon_builtin(BuiltinIcon variant = BuiltinIcon::Symbol) {
    if (detail::render_mode() == Mode::Gui) ftui::set_window_icon_builtin(variant);
    else if (detail::render_mode() == Mode::Tui) ftti::set_window_icon_builtin(static_cast<ftti::BuiltinIcon>(static_cast<int>(variant)));
}

inline Style default_dark_style() { return detail::from_gui_style(ftui::default_dark_style()); }
inline Style catppuccin_mocha_style() { return detail::from_gui_style(ftui::catppuccin_mocha_style()); }
inline Style nord_style() { return detail::from_gui_style(ftui::nord_style()); }
inline Style gruvbox_dark_style() { return detail::from_gui_style(ftui::gruvbox_dark_style()); }
inline Style one_dark_style() { return detail::from_gui_style(ftui::one_dark_style()); }
inline Style ghostty_green_style() { return detail::from_gui_style(ftui::ghostty_green_style()); }
inline Style style_from_name(const char* name) { return detail::from_gui_style(ftui::style_from_name(name)); }
inline const char* style_name(const Style& style) { return ftui::style_name(detail::to_gui_style(style)); }
inline const char* current_style_name() { return ftui::current_style_name(); }

inline const Style& get_style() {
    return detail::style_ref();
}

inline void set_style(const Style& style) {
    detail::style_ref() = style;
    ftui::set_style(detail::to_gui_style(style));
    ftti::set_style(detail::to_tui_style(style));
    ftht::set_style(detail::to_web_style(style));
}

inline Color color_from_hex(const char* hex) { return ftui::color_from_hex(hex); }
inline Color color_from_hex(std::string_view hex) { return ftui::color_from_hex(hex); }

inline void push_color(ColorRole role, Color color) {
    if (detail::render_mode() == Mode::Gui) ftui::push_color(role, color);
    else if (detail::render_mode() == Mode::Tui) ftti::push_color(detail::to_tui_role(role), detail::to_tui(color));
    else if (detail::render_mode() == Mode::Web) ftht::push_color(detail::to_web_role(role), detail::to_web(color));
}

inline void pop_color() {
    if (detail::render_mode() == Mode::Gui) ftui::pop_color();
    else if (detail::render_mode() == Mode::Tui) ftti::pop_color();
    else if (detail::render_mode() == Mode::Web) ftht::pop_color();
}

inline void set_next_color(ColorRole role, Color color) {
    if (detail::render_mode() == Mode::Gui) ftui::set_next_color(role, color);
    else if (detail::render_mode() == Mode::Tui) ftti::set_next_color(detail::to_tui_role(role), detail::to_tui(color));
    else if (detail::render_mode() == Mode::Web) ftht::set_next_color(detail::to_web_role(role), detail::to_web(color));
}

inline void text(const char* label) {
    if (detail::render_mode() == Mode::Gui) ftui::text(label);
    else if (detail::render_mode() == Mode::Tui) ftti::text(label);
    else if (detail::render_mode() == Mode::Web) ftht::text(label);
}

inline void text_wrapped(const char* value) {
    if (detail::render_mode() == Mode::Gui) ftui::text_wrapped(value);
    else if (detail::render_mode() == Mode::Tui) ftti::text_wrapped(value);
    else if (detail::render_mode() == Mode::Web) ftht::text_wrapped(value);
}

inline void separator() {
    if (detail::render_mode() == Mode::Gui) ftui::separator();
    else if (detail::render_mode() == Mode::Tui) ftti::separator();
    else if (detail::render_mode() == Mode::Web) ftht::separator();
}

inline void spacing(float px = 8.0f) {
    if (detail::render_mode() == Mode::Gui) ftui::spacing(px);
    else if (detail::render_mode() == Mode::Tui) ftti::spacing(px);
    else if (detail::render_mode() == Mode::Web) ftht::spacing(px);
}

inline bool input(const char* label, char* buffer, int buffer_size,
                  InputFlags flags = InputFlags::Default, bool* enter_pressed = nullptr) {
    if (detail::render_mode() == Mode::Gui) return ftui::input(label, buffer, buffer_size, flags, enter_pressed);
    if (detail::render_mode() == Mode::Tui) return ftti::input(label, buffer, buffer_size, detail::to_tui_input_flags(flags), enter_pressed);
    if (detail::render_mode() == Mode::Web) return ftht::input(label, buffer, buffer_size, detail::to_web_input_flags(flags), enter_pressed);
    return false;
}

inline bool text_area(const char* label, char* buffer, int buffer_size, int rows = 5) {
    if (detail::render_mode() == Mode::Gui) return ftui::text_area(label, buffer, buffer_size, rows);
    if (detail::render_mode() == Mode::Tui) return ftti::text_area(label, buffer, buffer_size, rows);
    if (detail::render_mode() == Mode::Web) return ftht::text_area(label, buffer, buffer_size, rows);
    return false;
}

inline bool text_area_ex(const char* label, char* buffer, int buffer_size, int rows = 5,
                         TextAreaFlags flags = TextAreaFlags::Default) {
    if (detail::render_mode() == Mode::Gui) return ftui::text_area_ex(label, buffer, buffer_size, rows, flags);
    if (detail::render_mode() == Mode::Tui) return ftti::text_area_ex(label, buffer, buffer_size, rows, detail::to_tui_text_area_flags(flags));
    if (detail::render_mode() == Mode::Web) return ftht::text_area_ex(label, buffer, buffer_size, rows, detail::to_web_text_area_flags(flags));
    return false;
}

inline void log_view(const char* label, const char* value, int rows = 8,
                     LogViewFlags flags = LogViewFlags::AutoScrollBottom) {
    if (detail::render_mode() == Mode::Gui) ftui::log_view(label, value, rows, flags);
    else if (detail::render_mode() == Mode::Tui) ftti::log_view(label, value, rows, detail::to_tui_log_flags(flags));
    else if (detail::render_mode() == Mode::Web) ftht::log_view(label, value, rows, detail::to_web_log_flags(flags));
}

inline bool checkbox(const char* label, bool* value) {
    if (detail::render_mode() == Mode::Gui) return ftui::checkbox(label, value);
    if (detail::render_mode() == Mode::Tui) return ftti::checkbox(label, value);
    if (detail::render_mode() == Mode::Web) return ftht::checkbox(label, value);
    return false;
}

inline bool slider_float(const char* label, float* value, float min_v, float max_v) {
    if (detail::render_mode() == Mode::Gui) return ftui::slider_float(label, value, min_v, max_v);
    if (detail::render_mode() == Mode::Tui) return ftti::slider_float(label, value, min_v, max_v);
    if (detail::render_mode() == Mode::Web) return ftht::slider_float(label, value, min_v, max_v);
    return false;
}

inline bool button(const char* label) {
    if (detail::render_mode() == Mode::Gui) return ftui::button(label);
    if (detail::render_mode() == Mode::Tui) return ftti::button(label);
    if (detail::render_mode() == Mode::Web) return ftht::button(label);
    return false;
}

inline bool button(const char* label, ColorRole role) {
    if (detail::render_mode() == Mode::Gui) return ftui::button(label, role);
    if (detail::render_mode() == Mode::Tui) return ftti::button(label, detail::to_tui_role(role));
    if (detail::render_mode() == Mode::Web) return ftht::button(label, detail::to_web_role(role));
    return false;
}

inline bool button(const char* label, Color color) {
    if (detail::render_mode() == Mode::Gui) return ftui::button(label, color);
    if (detail::render_mode() == Mode::Tui) return ftti::button(label, detail::to_tui(color));
    if (detail::render_mode() == Mode::Web) return ftht::button(label, detail::to_web(color));
    return false;
}

inline bool webserver_button(const char* start_label = "O") {
    if (webserver_running()) return false;
    Mode mode = detail::render_mode();
    bool clicked = false;
    if (mode == Mode::Gui) {
        ftui::set_next_width(42.0f);
        ftui::set_next_align(ftui::Align::End);
        clicked = ftui::button(start_label, ftui::ColorRole::Accent);
        ftui::tooltip("Start web server");
    } else if (mode == Mode::Tui) {
        ftti::set_next_width(4.0f);
        ftti::set_next_align(ftti::Align::End);
        clicked = ftti::button(start_label, ftti::ColorRole::Accent);
        ftti::tooltip("Start web server");
    } else if (mode == Mode::Web) {
        return false;
    }
    return clicked ? start_webserver() : false;
}

inline bool dropdown(const char* label, const char* const* items, int count, int* selected, int popup_rows = 8) {
    if (detail::render_mode() == Mode::Gui) return ftui::dropdown(label, items, count, selected, popup_rows);
    if (detail::render_mode() == Mode::Tui) return ftti::dropdown(label, items, count, selected, popup_rows);
    if (detail::render_mode() == Mode::Web) return ftht::dropdown(label, items, count, selected, popup_rows);
    return false;
}

inline bool listbox(const char* label, const char* const* items, int count, int* selected, int visible_rows = 6) {
    if (detail::render_mode() == Mode::Gui) return ftui::listbox(label, items, count, selected, visible_rows);
    if (detail::render_mode() == Mode::Tui) return ftti::listbox(label, items, count, selected, visible_rows);
    if (detail::render_mode() == Mode::Web) return ftht::listbox(label, items, count, selected, visible_rows);
    return false;
}

inline bool radio_group(const char* label, const char* const* items, int count, int* selected, int columns = 1) {
    if (detail::render_mode() == Mode::Gui) return ftui::radio_group(label, items, count, selected, columns);
    if (detail::render_mode() == Mode::Tui) return ftti::radio_group(label, items, count, selected, columns);
    if (detail::render_mode() == Mode::Web) return ftht::radio_group(label, items, count, selected, columns);
    return false;
}

inline bool collapsing_header(const char* label, bool* open = nullptr) {
    if (detail::render_mode() == Mode::Gui) return ftui::collapsing_header(label, open);
    if (detail::render_mode() == Mode::Tui) return ftti::collapsing_header(label, open);
    if (detail::render_mode() == Mode::Web) return ftht::collapsing_header(label, open);
    return false;
}

inline bool side_menu(const char* label, const char* const* items, int count, int* selected) {
    if (detail::render_mode() == Mode::Gui) return ftui::side_menu(label, items, count, selected);
    if (detail::render_mode() == Mode::Tui) return ftti::side_menu(label, items, count, selected);
    if (detail::render_mode() == Mode::Web) return ftht::side_menu(label, items, count, selected);
    return false;
}

inline bool side_menu_drawer(const char* label, const char* const* items, int count, int* selected, float width = 260.0f) {
    if (detail::render_mode() == Mode::Gui) return ftui::side_menu_drawer(label, items, count, selected, width);
    if (detail::render_mode() == Mode::Web) return ftht::side_menu_drawer(label, items, count, selected, width);
    return side_menu(label, items, count, selected);
}

inline bool tabs(const char* const* labels, int count, int* selected) {
    if (detail::render_mode() == Mode::Gui) return ftui::tabs(labels, count, selected);
    if (detail::render_mode() == Mode::Tui) return ftti::tabs(labels, count, selected);
    if (detail::render_mode() == Mode::Web) return ftht::tabs(labels, count, selected);
    return false;
}

inline void row(int cols, std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) ftui::row(cols, fn);
    else if (detail::render_mode() == Mode::Tui) ftti::row(cols, fn);
    else if (detail::render_mode() == Mode::Web) ftht::row(cols, fn);
}

inline void row(std::initializer_list<float> weights, std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) ftui::row(weights, fn);
    else if (detail::render_mode() == Mode::Tui) ftti::row(weights, fn);
    else if (detail::render_mode() == Mode::Web) ftht::row(weights, fn);
}

inline void split(std::initializer_list<float> columns, std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) ftui::split(columns, fn);
    else if (detail::render_mode() == Mode::Tui) ftti::split(columns, fn);
    else if (detail::render_mode() == Mode::Web) ftht::split(columns, fn);
}

inline void side_layout(float side_width, std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) ftui::side_layout(side_width, fn);
    else if (detail::render_mode() == Mode::Tui) ftti::side_layout(side_width, fn);
    else if (detail::render_mode() == Mode::Web) ftht::side_layout(side_width, fn);
}

inline void content(std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) ftui::content(fn);
    else if (detail::render_mode() == Mode::Tui) ftti::content(fn);
    else if (detail::render_mode() == Mode::Web) ftht::content(fn);
}

inline void scroll_area(const char* label, float height, std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) ftui::scroll_area(label, height, fn);
    else if (detail::render_mode() == Mode::Tui) ftti::scroll_area(label, height, fn);
    else if (detail::render_mode() == Mode::Web) ftht::scroll_area(label, height, fn);
}

inline void set_next_width(float px) {
    if (detail::render_mode() == Mode::Gui) ftui::set_next_width(px);
    else if (detail::render_mode() == Mode::Tui) ftti::set_next_width(px);
    else if (detail::render_mode() == Mode::Web) ftht::set_next_width(px);
}

inline void set_next_fill() {
    if (detail::render_mode() == Mode::Gui) ftui::set_next_fill();
    else if (detail::render_mode() == Mode::Tui) ftti::set_next_fill();
    else if (detail::render_mode() == Mode::Web) ftht::set_next_fill();
}

inline void set_next_percent(float pct) {
    if (detail::render_mode() == Mode::Gui) ftui::set_next_percent(pct);
    else if (detail::render_mode() == Mode::Tui) ftti::set_next_percent(pct);
    else if (detail::render_mode() == Mode::Web) ftht::set_next_percent(pct);
}

inline void set_next_limits(float min_px, float max_px) {
    if (detail::render_mode() == Mode::Gui) ftui::set_next_limits(min_px, max_px);
    else if (detail::render_mode() == Mode::Tui) ftti::set_next_limits(min_px, max_px);
    else if (detail::render_mode() == Mode::Web) ftht::set_next_limits(min_px, max_px);
}

inline void set_next_align(Align align) {
    if (detail::render_mode() == Mode::Gui) ftui::set_next_align(align);
    else if (detail::render_mode() == Mode::Tui) ftti::set_next_align(static_cast<ftti::Align>(static_cast<int>(align)));
    else if (detail::render_mode() == Mode::Web) ftht::set_next_align(static_cast<ftht::Align>(static_cast<int>(align)));
}

inline void open_modal(const char* label) {
    if (detail::render_mode() == Mode::Gui) ftui::open_modal(label);
    else if (detail::render_mode() == Mode::Tui) ftti::open_modal(label);
    else if (detail::render_mode() == Mode::Web) ftht::open_modal(label);
}

inline bool modal(const char* label, std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) return ftui::modal(label, fn);
    if (detail::render_mode() == Mode::Tui) return ftti::modal(label, fn);
    if (detail::render_mode() == Mode::Web) return ftht::modal(label, fn);
    return false;
}

inline void close_modal() {
    if (detail::render_mode() == Mode::Gui) ftui::close_modal();
    else if (detail::render_mode() == Mode::Tui) ftti::close_modal();
    else if (detail::render_mode() == Mode::Web) ftht::close_modal();
}

inline void begin_disabled() {
    if (detail::render_mode() == Mode::Gui) ftui::begin_disabled();
    else if (detail::render_mode() == Mode::Tui) ftti::begin_disabled();
    else if (detail::render_mode() == Mode::Web) ftht::begin_disabled();
}

inline void end_disabled() {
    if (detail::render_mode() == Mode::Gui) ftui::end_disabled();
    else if (detail::render_mode() == Mode::Tui) ftti::end_disabled();
    else if (detail::render_mode() == Mode::Web) ftht::end_disabled();
}

inline void tooltip(const char* value) {
    if (detail::render_mode() == Mode::Gui) ftui::tooltip(value);
    else if (detail::render_mode() == Mode::Tui) ftti::tooltip(value);
    else if (detail::render_mode() == Mode::Web) ftht::tooltip(value);
}

inline void request_focus(const char* label) {
    if (detail::render_mode() == Mode::Gui) ftui::request_focus(label);
    else if (detail::render_mode() == Mode::Tui) ftti::request_focus(label);
    else if (detail::render_mode() == Mode::Web) ftht::request_focus(label);
}

inline float calc_text_width(const char* value) {
    if (detail::render_mode() == Mode::Gui) return ftui::calc_text_width(value);
    if (detail::render_mode() == Mode::Tui) return ftti::calc_text_width(value);
    if (detail::render_mode() == Mode::Web) return ftht::calc_text_width(value);
    return value ? (float)std::strlen(value) : 0.0f;
}

inline float calc_text_height(const char* value, float wrap_width) {
    if (detail::render_mode() == Mode::Gui) return ftui::calc_text_height(value, wrap_width);
    if (detail::render_mode() == Mode::Tui) return ftti::calc_text_height(value, wrap_width);
    if (detail::render_mode() == Mode::Web) return ftht::calc_text_height(value, wrap_width);
    return 0.0f;
}

inline void toast(const Toast& value) {
    if (detail::render_mode() == Mode::Gui) {
        ftui::Toast t;
        t.message = value.message;
        t.type = value.type;
        t.duration_ms = value.duration_ms;
        t.dismissible = value.dismissible;
        ftui::toast(t);
    } else if (detail::render_mode() == Mode::Tui) {
        ftti::Toast t;
        t.message = value.message;
        t.type = detail::to_tui_toast_type(value.type);
        t.duration_ms = value.duration_ms;
        t.dismissible = value.dismissible;
        ftti::toast(t);
    } else if (detail::render_mode() == Mode::Web) {
        ftht::Toast t;
        t.message = value.message ? value.message : "";
        t.type = detail::to_web_toast_type(value.type);
        t.duration_ms = value.duration_ms;
        t.dismissible = value.dismissible;
        ftht::toast(t);
    }
}

inline void toast(const char* message) { Toast t; t.message = message; toast(t); }
inline void toast_info(const char* message) { toast(message); }
inline void toast_success(const char* message) { Toast t; t.message = message; t.type = ToastType::Success; toast(t); }
inline void toast_warning(const char* message) { Toast t; t.message = message; t.type = ToastType::Warning; toast(t); }
inline void toast_error(const char* message) { Toast t; t.message = message; t.type = ToastType::Error; toast(t); }

inline void clear_toasts() {
    if (detail::render_mode() == Mode::Gui) ftui::clear_toasts();
    else if (detail::render_mode() == Mode::Tui) ftti::clear_toasts();
    else if (detail::render_mode() == Mode::Web) ftht::clear_toasts();
}

inline void progress_bar(float progress) {
    if (detail::render_mode() == Mode::Gui) ftui::progress_bar(progress);
    else if (detail::render_mode() == Mode::Tui) ftti::progress_bar(progress);
    else if (detail::render_mode() == Mode::Web) {
        char text[64];
        std::snprintf(text, sizeof(text), "Progress %.0f%%", std::max(0.0f, std::min(1.0f, progress)) * 100.0f);
        ftht::text(text);
    }
}

inline void progress_bar(float progress, const char* label_or_mask_path) {
    if (detail::render_mode() == Mode::Gui) ftui::progress_bar(progress, label_or_mask_path);
    else if (detail::render_mode() == Mode::Tui) ftti::progress_bar(progress, label_or_mask_path);
    else if (detail::render_mode() == Mode::Web) ftht::text(label_or_mask_path ? label_or_mask_path : "Progress");
}

inline void progress_bar(float progress, const ProgressStyle& style) {
    if (detail::render_mode() == Mode::Gui) {
        ftui::ProgressStyle s;
        s.label = style.label;
        s.mask_path = style.mask_path;
        s.mask_svg = style.mask_svg;
        s.mask_shape = style.mask_shape;
        s.fill_role = style.fill_role;
        s.fill_color = style.fill_color;
        s.height = style.height;
        s.show_percent = style.show_percent;
        s.wave_front = style.wave_front;
        s.glint = style.glint;
        ftui::progress_bar(progress, s);
    } else if (detail::render_mode() == Mode::Tui) {
        ftti::ProgressStyle s;
        s.label = style.label;
        s.mask_path = style.mask_path;
        s.fill_role = detail::to_tui_role(style.fill_role);
        s.fill_color = detail::to_tui(style.fill_color);
        s.height = style.height;
        s.show_percent = style.show_percent;
        s.wave_front = style.wave_front;
        s.glint = style.glint;
        ftti::progress_bar(progress, s);
    } else if (detail::render_mode() == Mode::Web) {
        progress_bar(progress, style.label);
    }
}

inline ImageHandle* load_image(const char* utf8_path) {
    ImageHandle* out = new ImageHandle();
    out->gui = ftui::load_image(utf8_path);
    out->tui = ftti::load_image(utf8_path);
    return out;
}

inline void image(ImageHandle* img, float width, float height) {
    if (!img) return;
    if (detail::render_mode() == Mode::Gui) ftui::image(img->gui, width, height);
    else if (detail::render_mode() == Mode::Tui) ftti::image(img->tui, width, height);
}

inline void free_image(ImageHandle* img) {
    if (!img) return;
    if (img->gui) ftui::free_image(img->gui);
    if (img->tui) ftti::free_image(img->tui);
    delete img;
}

inline std::string open_file_dialog(const char* title = "Open File",
                                    const FileFilter* filters = nullptr,
                                    int filter_count = 0) {
    if (detail::render_mode() == Mode::Gui) return ftui::open_file_dialog(title, filters, filter_count);
    if (detail::render_mode() == Mode::Tui) return ftti::open_file_dialog(title, reinterpret_cast<const ftti::FileFilter*>(filters), filter_count);
    return {};
}

inline void open_child_window(const Config& cfg, std::function<void()> fn) {
    if (detail::render_mode() == Mode::Gui) ftui::open_child_window(detail::to_gui_config(cfg), fn);
    else fn();
}

inline DebugState& debug() {
    if (detail::render_mode() == Mode::Gui) return ftui::debug();
    return detail::fallback_debug_ref();
}

inline void html(const char* markup) {
    if (detail::render_mode() == Mode::Web) ftht::html(markup);
}

inline const char* url() { return detail::render_mode() == Mode::Web ? ftht::url() : ""; }
inline const char* path() { return detail::render_mode() == Mode::Web ? ftht::path() : ""; }
inline const char* method() { return detail::render_mode() == Mode::Web ? ftht::method() : ""; }
inline bool is_post() { return detail::render_mode() == Mode::Web && ftht::is_post(); }
inline const char* param(const char* name) { return detail::render_mode() == Mode::Web ? ftht::param(name) : ""; }
inline void set_status(int code, const char* text) { if (detail::render_mode() == Mode::Web) ftht::set_status(code, text); }
inline uint64_t make_login_password_hash(const char* password, const char* salt = "ftht-login") {
    return ftht::make_login_password_hash(password, salt);
}

} // namespace ft
