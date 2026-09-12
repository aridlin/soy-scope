#include <windows.h>
#include <wincodec.h>
#include <d2d1_1.h>
#include <d3d11.h>
#include <dcomp.h>
#include <dxgi1_2.h>

#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <future>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#define FT_IMPLEMENTATION
#include "ft.hpp"

#include "../resources/resource.h"

namespace {

constexpr UINT kMsgApply = WM_APP + 1;
constexpr UINT kMsgShowNow = WM_APP + 2;
constexpr UINT kMsgHide = WM_APP + 3;
constexpr UINT_PTR kShowTimer = 42;
constexpr UINT_PTR kMousePollTimer = 43;
constexpr int kHotkeyId = 1001;
constexpr const char* kEmbeddedImagePath = "embedded://two-soyjaks-pointing";

struct Hotkey {
    UINT modifiers = MOD_NOREPEAT;
    UINT vk = VK_F8;
    bool valid = true;
};

constexpr UINT kAlternateButtons[] = {VK_LBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2};
constexpr const char* kAlternateButtonNames[] = {"Left mouse button", "Middle mouse button", "Mouse button 4 (X1)", "Mouse button 5 (X2)"};

struct AppConfig {
    std::string image_path;
    std::string hotkey_text = "F8";
    float delay_ms = 250.0f;
    float width_px = 1200.0f;
    float opacity = 0.72f;
    float anchor_x_pct = 57.5f;
    float anchor_y_pct = 37.5f;
    bool enabled = true;
    bool custom_mouse_button = false;
    int mouse_button = 0;
};

UINT trigger_button(const AppConfig& cfg) {
    return cfg.custom_mouse_button ? kAlternateButtons[std::clamp(cfg.mouse_button, 0, 3)] : VK_RBUTTON;
}

const char* trigger_button_name(const AppConfig& cfg) {
    return cfg.custom_mouse_button ? kAlternateButtonNames[std::clamp(cfg.mouse_button, 0, 3)] : "Right mouse button";
}

bool nearly_equal(float a, float b) {
    return std::fabs(a - b) < 0.01f;
}

bool same_config(const AppConfig& a, const AppConfig& b) {
    return a.image_path == b.image_path &&
           a.hotkey_text == b.hotkey_text &&
           nearly_equal(a.delay_ms, b.delay_ms) &&
           nearly_equal(a.width_px, b.width_px) &&
           nearly_equal(a.opacity, b.opacity) &&
           nearly_equal(a.anchor_x_pct, b.anchor_x_pct) &&
           nearly_equal(a.anchor_y_pct, b.anchor_y_pct) &&
           a.enabled == b.enabled &&
           a.custom_mouse_button == b.custom_mouse_button &&
           a.mouse_button == b.mouse_button;
}

std::string trim(std::string s) {
    auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](char c) { return !is_space((unsigned char)c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [&](char c) { return !is_space((unsigned char)c); }).base(), s.end());
    return s;
}

std::string upper(std::string s) {
    for (char& c : s) c = (char)std::toupper((unsigned char)c);
    return s;
}

std::vector<std::string> split_plus(const std::string& input) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : input) {
        if (c == '+') {
            out.push_back(trim(cur));
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    out.push_back(trim(cur));
    return out;
}

bool parse_key_token(const std::string& token, UINT* vk) {
    if (token.size() == 1) {
        char c = token[0];
        if ((c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9')) {
            *vk = (UINT)c;
            return true;
        }
    }
    if (token.size() >= 2 && token[0] == 'F') {
        int n = std::atoi(token.c_str() + 1);
        if (n >= 1 && n <= 24) {
            *vk = VK_F1 + (UINT)n - 1;
            return true;
        }
    }
    if (token == "ESC" || token == "ESCAPE") *vk = VK_ESCAPE;
    else if (token == "SPACE") *vk = VK_SPACE;
    else if (token == "TAB") *vk = VK_TAB;
    else if (token == "INSERT" || token == "INS") *vk = VK_INSERT;
    else if (token == "DELETE" || token == "DEL") *vk = VK_DELETE;
    else if (token == "HOME") *vk = VK_HOME;
    else if (token == "END") *vk = VK_END;
    else if (token == "PAGEUP" || token == "PGUP") *vk = VK_PRIOR;
    else if (token == "PAGEDOWN" || token == "PGDN") *vk = VK_NEXT;
    else if (token == "UP") *vk = VK_UP;
    else if (token == "DOWN") *vk = VK_DOWN;
    else if (token == "LEFT") *vk = VK_LEFT;
    else if (token == "RIGHT") *vk = VK_RIGHT;
    else if (token == "CAPSLOCK") *vk = VK_CAPITAL;
    else if (token.size() == 4 && token.rfind("NUM", 0) == 0 && token[3] >= '0' && token[3] <= '9') {
        *vk = VK_NUMPAD0 + (UINT)(token[3] - '0');
    } else {
        return false;
    }
    return true;
}

Hotkey parse_hotkey(const std::string& text) {
    Hotkey result;
    result.modifiers = MOD_NOREPEAT;
    result.valid = false;

    bool saw_key = false;
    for (std::string token : split_plus(upper(text))) {
        if (token.empty()) continue;
        if (token == "CTRL" || token == "CONTROL") {
            result.modifiers |= MOD_CONTROL;
        } else if (token == "ALT") {
            result.modifiers |= MOD_ALT;
        } else if (token == "SHIFT") {
            result.modifiers |= MOD_SHIFT;
        } else if (token == "WIN" || token == "WINDOWS") {
            result.modifiers |= MOD_WIN;
        } else {
            UINT vk = 0;
            if (!parse_key_token(token, &vk) || saw_key) return result;
            result.vk = vk;
            saw_key = true;
        }
    }

    result.valid = saw_key;
    return result;
}

std::wstring utf8_to_wide(const std::string& s) {
    if (s.empty()) return {};
    int n = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (n <= 0) return {};
    std::wstring w((size_t)n - 1, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, w.data(), n);
    return w;
}

std::string wide_to_utf8(const std::wstring& w) {
    if (w.empty()) return {};
    int n = WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (n <= 0) return {};
    std::string s((size_t)n - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, w.c_str(), -1, s.data(), n, nullptr, nullptr);
    return s;
}

std::wstring exe_dir_wide() {
    wchar_t path[MAX_PATH]{};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring s(path);
    size_t slash = s.find_last_of(L"\\/");
    if (slash != std::wstring::npos) s.resize(slash);
    return s;
}

std::string exe_dir() {
    return wide_to_utf8(exe_dir_wide());
}

std::string default_asset_path() {
    return kEmbeddedImagePath;
}

std::string config_path() {
    return exe_dir() + "\\soy-scope.ini";
}

void read_config(AppConfig* cfg) {
    std::ifstream in(config_path());
    if (!in) return;

    std::string line;
    while (std::getline(in, line)) {
        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));
        try {
            if (key == "image_path") cfg->image_path = val;
            else if (key == "hotkey") cfg->hotkey_text = val;
            else if (key == "delay_ms") cfg->delay_ms = std::stof(val);
            else if (key == "width_px") cfg->width_px = std::stof(val);
            else if (key == "opacity") cfg->opacity = std::stof(val);
            else if (key == "anchor_x_pct") cfg->anchor_x_pct = std::stof(val);
            else if (key == "anchor_y_pct") cfg->anchor_y_pct = std::stof(val);
            else if (key == "custom_mouse_button") cfg->custom_mouse_button = (val == "1" || upper(val) == "TRUE");
            else if (key == "mouse_button") cfg->mouse_button = std::clamp(std::stoi(val), 0, 3);
            else if (key == "enabled") cfg->enabled = (val == "1" || upper(val) == "TRUE");
        } catch (...) {
        }
    }
}

void write_config(const AppConfig& cfg) {
    std::ofstream out(config_path(), std::ios::trunc);
    if (!out) return;
    out << "image_path=" << cfg.image_path << "\n";
    out << "hotkey=" << cfg.hotkey_text << "\n";
    out << "delay_ms=" << (int)std::round(cfg.delay_ms) << "\n";
    out << "width_px=" << (int)std::round(cfg.width_px) << "\n";
    out << "opacity=" << cfg.opacity << "\n";
    out << "anchor_x_pct=" << cfg.anchor_x_pct << "\n";
    out << "anchor_y_pct=" << cfg.anchor_y_pct << "\n";
    out << "custom_mouse_button=" << (cfg.custom_mouse_button ? 1 : 0) << "\n";
    out << "mouse_button=" << cfg.mouse_button << "\n";
    out << "enabled=" << (cfg.enabled ? 1 : 0) << "\n";
}

struct DecodedImage {
    UINT width = 0;
    UINT height = 0;
    std::vector<std::uint8_t> bgra;
};

struct ResourceBytes {
    const void* data = nullptr;
    DWORD size = 0;
};

struct RenderedBitmap {
    HBITMAP bitmap = nullptr;
    int width = 0;
    int height = 0;

    ~RenderedBitmap() {
        if (bitmap) DeleteObject(bitmap);
    }
};

class OverlayController {
public:
    OverlayController() = default;
    ~OverlayController() { stop(); }

    bool start(const AppConfig& cfg) {
        pending_config_ = cfg;
        std::promise<bool> ready;
        auto future = ready.get_future();
        thread_ = std::thread([this, promise = std::move(ready)]() mutable {
            thread_main(std::move(promise));
        });
        return future.get();
    }

    void stop() {
        HWND hwnd = control_hwnd_.load();
        if (hwnd) PostMessageW(hwnd, WM_CLOSE, 0, 0);
        if (thread_.joinable()) thread_.join();
        control_hwnd_.store(nullptr);
    }

    void apply(const AppConfig& cfg) {
        {
            std::lock_guard<std::mutex> lock(config_mutex_);
            pending_config_ = cfg;
        }
        HWND hwnd = control_hwnd_.load();
        if (hwnd) PostMessageW(hwnd, kMsgApply, 0, 0);
    }

    void show_now() {
        HWND hwnd = control_hwnd_.load();
        if (hwnd) PostMessageW(hwnd, kMsgShowNow, 0, 0);
    }

    void hide() {
        HWND hwnd = control_hwnd_.load();
        if (hwnd) PostMessageW(hwnd, kMsgHide, 0, 0);
    }

    std::string status() const {
        std::lock_guard<std::mutex> lock(status_mutex_);
        return status_;
    }

private:
    static LRESULT CALLBACK control_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        auto* self = reinterpret_cast<OverlayController*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (msg == WM_NCCREATE) {
            auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
            self = reinterpret_cast<OverlayController*>(cs->lpCreateParams);
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
        if (!self) return DefWindowProcW(hwnd, msg, wp, lp);

        switch (msg) {
        case kMsgApply:
            self->apply_on_thread();
            return 0;
        case kMsgShowNow:
            self->show_overlay();
            return 0;
        case kMsgHide:
            self->hide_overlay();
            return 0;
        case WM_HOTKEY:
            if ((int)wp == kHotkeyId) self->toggle_mouse_trigger();
            return 0;
        case WM_TIMER:
            if (wp == kShowTimer) {
                if (!self->pending_show_) return 0;
                KillTimer(hwnd, kShowTimer);
                self->pending_show_ = false;
                if (self->armed_ && self->trigger_button_down()) self->show_overlay();
            } else if (wp == kMousePollTimer) {
                self->poll_trigger_button();
            }
            return 0;
        case WM_CLOSE:
            self->unregister_hotkey();
            KillTimer(hwnd, kShowTimer);
            KillTimer(hwnd, kMousePollTimer);
            if (self->overlay_hwnd_) DestroyWindow(self->overlay_hwnd_);
            DestroyWindow(hwnd);
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        default:
            return DefWindowProcW(hwnd, msg, wp, lp);
        }
    }

    static LRESULT CALLBACK overlay_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
        if (msg == WM_NCHITTEST) return HTTRANSPARENT;
        if (msg == WM_MOUSEACTIVATE) return MA_NOACTIVATEANDEAT;
        return DefWindowProcW(hwnd, msg, wp, lp);
    }

    void thread_main(std::promise<bool> ready) {
        CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        HINSTANCE inst = GetModuleHandleW(nullptr);

        WNDCLASSW control_class{};
        control_class.lpfnWndProc = control_proc;
        control_class.hInstance = inst;
        control_class.lpszClassName = L"SoyScopeControl";
        RegisterClassW(&control_class);

        WNDCLASSW overlay_class{};
        overlay_class.lpfnWndProc = overlay_proc;
        overlay_class.hInstance = inst;
        overlay_class.lpszClassName = L"SoyScopeOverlay";
        RegisterClassW(&overlay_class);

        overlay_hwnd_ = CreateWindowExW(
            WS_EX_TRANSPARENT | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_NOREDIRECTIONBITMAP,
            overlay_class.lpszClassName, L"Soy Scope Overlay", WS_POPUP,
            0, 0, 1, 1, nullptr, nullptr, inst, nullptr);

        HWND control = CreateWindowExW(0, control_class.lpszClassName, L"Soy Scope Control",
                                      0, 0, 0, 0, 0, HWND_MESSAGE, nullptr, inst, this);
        control_hwnd_.store(control);

        bool ok = overlay_hwnd_ != nullptr && control != nullptr && initialize_composition();
        if (ok) {
            LONG_PTR ex_style = GetWindowLongPtrW(overlay_hwnd_, GWL_EXSTYLE);
            SetWindowLongPtrW(overlay_hwnd_, GWL_EXSTYLE, ex_style | WS_EX_LAYERED);
            ok = SetLayeredWindowAttributes(overlay_hwnd_, 0, 255, LWA_ALPHA) != FALSE;
            SetWindowPos(overlay_hwnd_, nullptr, 0, 0, 0, 0,
                         SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE |
                         SWP_NOZORDER | SWP_NOACTIVATE);
            if (!ok) set_status("Could not enable cross-process mouse pass-through.");
        }
        ready.set_value(ok);
        if (!ok) {
            release_composition();
            if (overlay_hwnd_) DestroyWindow(overlay_hwnd_);
            if (control) DestroyWindow(control);
            CoUninitialize();
            return;
        }

        apply_on_thread();

        MSG msg{};
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        release_composition();
        if (rendered_.bitmap) {
            DeleteObject(rendered_.bitmap);
            rendered_.bitmap = nullptr;
        }
        CoUninitialize();
    }

    void set_status(const std::string& s) {
        std::lock_guard<std::mutex> lock(status_mutex_);
        status_ = s;
    }

    bool decode_from_decoder(IWICImagingFactory* factory, IWICBitmapDecoder* decoder, DecodedImage* out) {
        IWICBitmapFrameDecode* frame = nullptr;
        HRESULT hr = decoder->GetFrame(0, &frame);
        if (FAILED(hr)) {
            set_status("Image has no readable frame.");
            return false;
        }

        IWICFormatConverter* converter = nullptr;
        hr = factory->CreateFormatConverter(&converter);
        if (SUCCEEDED(hr)) {
            hr = converter->Initialize(frame, GUID_WICPixelFormat32bppBGRA,
                                       WICBitmapDitherTypeNone, nullptr, 0.0,
                                       WICBitmapPaletteTypeCustom);
        }

        UINT w = 0, h = 0;
        if (SUCCEEDED(hr)) hr = converter->GetSize(&w, &h);
        DecodedImage next;
        if (SUCCEEDED(hr) && w > 0 && h > 0) {
            next.width = w;
            next.height = h;
            next.bgra.resize((size_t)w * h * 4);
            hr = converter->CopyPixels(nullptr, w * 4, (UINT)next.bgra.size(), next.bgra.data());
        }

        if (converter) converter->Release();
        frame->Release();

        if (FAILED(hr)) {
            set_status("Image conversion failed.");
            return false;
        }

        *out = std::move(next);
        return true;
    }

    bool embedded_resource(ResourceBytes* bytes) {
        HINSTANCE inst = GetModuleHandleW(nullptr);
        HRSRC res = FindResourceW(inst, MAKEINTRESOURCEW(IDR_SOYJAK_WEBP), RT_RCDATA);
        if (!res) return false;
        HGLOBAL handle = LoadResource(inst, res);
        if (!handle) return false;
        bytes->data = LockResource(handle);
        bytes->size = SizeofResource(inst, res);
        return bytes->data != nullptr && bytes->size > 0;
    }

    bool load_embedded_image(IWICImagingFactory* factory, DecodedImage* out) {
        ResourceBytes bytes;
        if (!embedded_resource(&bytes)) {
            set_status("Embedded image resource is missing.");
            return false;
        }

        HGLOBAL global = GlobalAlloc(GMEM_MOVEABLE, bytes.size);
        if (!global) {
            set_status("Could not allocate embedded image stream.");
            return false;
        }

        void* dest = GlobalLock(global);
        if (!dest) {
            GlobalFree(global);
            set_status("Could not lock embedded image stream.");
            return false;
        }
        std::memcpy(dest, bytes.data, bytes.size);
        GlobalUnlock(global);

        IStream* stream = nullptr;
        HRESULT hr = CreateStreamOnHGlobal(global, TRUE, &stream);
        if (FAILED(hr)) {
            GlobalFree(global);
            set_status("Could not open embedded image stream.");
            return false;
        }

        IWICBitmapDecoder* decoder = nullptr;
        hr = factory->CreateDecoderFromStream(stream, nullptr, WICDecodeMetadataCacheOnLoad, &decoder);
        if (FAILED(hr)) {
            stream->Release();
            set_status("Embedded image decode failed.");
            return false;
        }

        bool ok = decode_from_decoder(factory, decoder, out);
        decoder->Release();
        stream->Release();
        return ok;
    }

    bool load_file_image(IWICImagingFactory* factory, const std::string& path, DecodedImage* out) {
        IWICBitmapDecoder* decoder = nullptr;
        std::wstring wide = utf8_to_wide(path);
        HRESULT hr = factory->CreateDecoderFromFilename(wide.c_str(), nullptr, GENERIC_READ,
                                                        WICDecodeMetadataCacheOnLoad, &decoder);
        if (FAILED(hr)) {
            set_status("Image decode failed. Use embedded image or choose a PNG/JPG/WebP.");
            return false;
        }

        bool ok = decode_from_decoder(factory, decoder, out);
        decoder->Release();
        return ok;
    }

    bool load_image(const std::string& path) {
        IWICImagingFactory* factory = nullptr;
        HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER,
                                      IID_PPV_ARGS(&factory));
        if (FAILED(hr)) {
            set_status("WIC unavailable.");
            return false;
        }

        DecodedImage next;
        bool ok = (path == kEmbeddedImagePath)
            ? load_embedded_image(factory, &next)
            : load_file_image(factory, path, &next);
        factory->Release();

        if (!ok) return false;

        source_ = std::move(next);
        loaded_path_ = path;
        return true;
    }

    bool render_bitmap() {
        if (source_.bgra.empty() || source_.width == 0 || source_.height == 0) return false;

        int out_w = std::max(64, (int)std::round(active_config_.width_px));
        int out_h = std::max(64, (int)std::round((double)out_w * source_.height / source_.width));

        BITMAPINFO bmi{};
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = out_w;
        bmi.bmiHeader.biHeight = -out_h;
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;

        void* bits = nullptr;
        HBITMAP bmp = CreateDIBSection(nullptr, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
        if (!bmp || !bits) {
            if (bmp) DeleteObject(bmp);
            set_status("Could not allocate overlay bitmap.");
            return false;
        }

        auto* dst = static_cast<std::uint8_t*>(bits);
        double sx_scale = (double)source_.width / out_w;
        double sy_scale = (double)source_.height / out_h;
        for (int y = 0; y < out_h; ++y) {
            int sy = std::min((int)source_.height - 1, (int)std::floor((y + 0.5) * sy_scale));
            for (int x = 0; x < out_w; ++x) {
                int sx = std::min((int)source_.width - 1, (int)std::floor((x + 0.5) * sx_scale));
                const std::uint8_t* sp = &source_.bgra[((size_t)sy * source_.width + sx) * 4];
                double b = sp[0], g = sp[1], r = sp[2], a = sp[3] / 255.0;
                double luma = 0.2126 * r + 0.7152 * g + 0.0722 * b;
                double ink = std::pow(std::clamp((255.0 - luma) / 255.0, 0.0, 1.0), 1.15);
                std::uint8_t alpha = (std::uint8_t)std::round(255.0 * ink * a);
                if (alpha < 8) alpha = 0;

                std::uint8_t* dp = &dst[((size_t)y * out_w + x) * 4];
                dp[0] = 0;
                dp[1] = 0;
                dp[2] = 0;
                dp[3] = alpha;
            }
        }

        if (rendered_.bitmap) DeleteObject(rendered_.bitmap);
        rendered_.bitmap = bmp;
        rendered_.width = out_w;
        rendered_.height = out_h;
        bitmap_dirty_ = false;
        surface_dirty_ = true;
        return true;
    }

    void apply_on_thread() {
        AppConfig next;
        {
            std::lock_guard<std::mutex> lock(config_mutex_);
            next = pending_config_;
        }

        bool bitmap_settings_changed =
            !nearly_equal(active_config_.width_px, next.width_px);
        const bool trigger_changed = trigger_button(active_config_) != trigger_button(next);
        active_config_ = next;
        if (trigger_changed) {
            hide_overlay();
            // Require a fresh press after rebinding, including clicks in setup.
            trigger_was_down_ = trigger_button_down();
        }
        Hotkey hk = parse_hotkey(active_config_.hotkey_text);
        if (active_config_.enabled && hk.valid) {
            register_hotkey(hk);
            SetTimer(control_hwnd_.load(), kMousePollTimer, 10, nullptr);
        } else {
            unregister_hotkey();
            KillTimer(control_hwnd_.load(), kMousePollTimer);
            armed_ = false;
            trigger_was_down_ = false;
            hide_overlay();
        }

        bool source_changed = loaded_path_ != active_config_.image_path;
        if (source_changed) {
            hide_overlay();
            if (!load_image(active_config_.image_path)) {
                bitmap_dirty_ = true;
                return;
            }
            bitmap_settings_changed = true;
        }

        if (bitmap_settings_changed || !rendered_.bitmap) bitmap_dirty_ = true;
        if (bitmap_dirty_ && !render_bitmap()) return;

        // Render, upload, position, and establish z-order during configuration.
        // Scope-in only changes the compositor's global alpha value.
        if (!prime_overlay_surface()) return;
        std::ostringstream ss;
        ss << "Ready. " << active_config_.hotkey_text
           << " toggles trigger " << (armed_ ? "on" : "off")
           << ". Hold: " << trigger_button_name(active_config_) << ".";
        set_status(ss.str());
    }

    void register_hotkey(const Hotkey& hk) {
        if (hotkey_registered_ && hk.modifiers == registered_hotkey_.modifiers && hk.vk == registered_hotkey_.vk) {
            return;
        }
        unregister_hotkey();
        if (RegisterHotKey(control_hwnd_.load(), kHotkeyId, hk.modifiers, hk.vk)) {
            hotkey_registered_ = true;
            registered_hotkey_ = hk;
        } else {
            set_status("Hotkey registration failed. Pick another key.");
        }
    }

    void unregister_hotkey() {
        if (hotkey_registered_) {
            UnregisterHotKey(control_hwnd_.load(), kHotkeyId);
            hotkey_registered_ = false;
        }
    }

    void toggle_mouse_trigger() {
        if (!active_config_.enabled) return;
        armed_ = !armed_;
        if (!armed_) {
            trigger_was_down_ = trigger_button_down();
            hide_overlay();
            set_status("Mouse trigger off. Overlay will not show.");
            return;
        }
        trigger_was_down_ = trigger_button_down();
        last_foreground_hwnd_ = GetForegroundWindow();
        reassert_static_overlay();
        set_status(std::string("Trigger on. Hold ") + trigger_button_name(active_config_) + " to show after delay.");
        if (trigger_was_down_) begin_trigger_delay();
    }

    bool trigger_button_down() const {
        return (GetAsyncKeyState(trigger_button(active_config_)) & 0x8000) != 0;
    }

    void poll_trigger_button() {
        refresh_static_overlay_placement();
        bool down = trigger_button_down();
        if (!active_config_.enabled || !armed_) {
            trigger_was_down_ = down;
            if (!armed_ && (visible_ || pending_show_)) hide_overlay();
            return;
        }

        if (down && !trigger_was_down_) {
            trigger_was_down_ = true;
            begin_trigger_delay();
        } else if (!down && trigger_was_down_) {
            trigger_was_down_ = false;
            hide_overlay();
        }
    }

    void begin_trigger_delay() {
        if (visible_ || pending_show_) return;
        int delay = std::max(0, (int)std::round(active_config_.delay_ms));
        if (delay == 0) {
            show_overlay();
        } else {
            pending_show_ = true;
            SetTimer(control_hwnd_.load(), kShowTimer, (UINT)delay, nullptr);
            set_status(std::string(trigger_button_name(active_config_)) + " held. Waiting for delay...");
        }
    }

    POINT overlay_position() const {
        int screen_w = GetSystemMetrics(SM_CXSCREEN);
        int screen_h = GetSystemMetrics(SM_CYSCREEN);
        int center_x = screen_w / 2;
        int center_y = screen_h / 2;

        double ax = std::clamp((double)active_config_.anchor_x_pct / 100.0, 0.0, 1.0);
        double ay = std::clamp((double)active_config_.anchor_y_pct / 100.0, 0.0, 1.0);
        return POINT{
            center_x - (int)std::round(rendered_.width * ax),
            center_y - (int)std::round(rendered_.height * ay)
        };
    }

    template <typename T>
    static void release_com(T*& value) {
        if (value) {
            value->Release();
            value = nullptr;
        }
    }

    void release_composition() {
        release_com(scope_surface_);
        release_com(sentinel_surface_);
        release_com(scope_effect_);
        release_com(scope_visual_);
        release_com(sentinel_visual_);
        release_com(root_visual_);
        release_com(composition_target_);
        release_com(composition_device_);
        release_com(d2d_device_);
        release_com(d2d_factory_);
        release_com(d3d_device_);
        composition_ready_ = false;
        overlay_primed_ = false;
        current_scope_opacity_ = -1.0f;
    }

    bool draw_sentinel_surface() {
        POINT offset{};
        ID2D1DeviceContext* context = nullptr;
        HRESULT hr = sentinel_surface_->BeginDraw(
            nullptr, IID_PPV_ARGS(&context), &offset);
        if (FAILED(hr)) return false;

        D2D1_MATRIX_3X2_F transform{1.0f, 0.0f, 0.0f, 1.0f,
                                   (FLOAT)offset.x, (FLOAT)offset.y};
        context->SetTransform(transform);
        D2D1_COLOR_F opaque_black{0.0f, 0.0f, 0.0f, 1.0f};
        context->Clear(&opaque_black);
        context->Release();
        return SUCCEEDED(sentinel_surface_->EndDraw());
    }

    bool initialize_composition() {
        UINT device_flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        D3D_FEATURE_LEVEL feature_level{};
        ID3D11DeviceContext* immediate_context = nullptr;
        HRESULT hr = D3D11CreateDevice(
            nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, device_flags,
            nullptr, 0, D3D11_SDK_VERSION, &d3d_device_,
            &feature_level, &immediate_context);
        if (FAILED(hr)) {
            hr = D3D11CreateDevice(
                nullptr, D3D_DRIVER_TYPE_WARP, nullptr, device_flags,
                nullptr, 0, D3D11_SDK_VERSION, &d3d_device_,
                &feature_level, &immediate_context);
        }
        release_com(immediate_context);
        if (FAILED(hr)) {
            set_status("Could not create the Direct3D overlay device.");
            return false;
        }

        IDXGIDevice* dxgi_device = nullptr;
        hr = d3d_device_->QueryInterface(IID_PPV_ARGS(&dxgi_device));
        if (FAILED(hr)) {
            set_status("Could not access the DirectX overlay device.");
            release_composition();
            return false;
        }

        D2D1_FACTORY_OPTIONS factory_options{};
        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory1),
            &factory_options,
            reinterpret_cast<void**>(&d2d_factory_));
        if (SUCCEEDED(hr)) hr = d2d_factory_->CreateDevice(dxgi_device, &d2d_device_);
        release_com(dxgi_device);
        if (FAILED(hr)) {
            set_status("Could not create the Direct2D overlay device.");
            release_composition();
            return false;
        }

        hr = DCompositionCreateDevice2(d2d_device_, IID_PPV_ARGS(&composition_device_));
        if (SUCCEEDED(hr)) {
            hr = composition_device_->CreateTargetForHwnd(
                overlay_hwnd_, TRUE, &composition_target_);
        }
        if (SUCCEEDED(hr)) hr = composition_device_->CreateVisual(&root_visual_);
        if (SUCCEEDED(hr)) hr = composition_device_->CreateVisual(&scope_visual_);
        if (SUCCEEDED(hr)) hr = composition_device_->CreateVisual(&sentinel_visual_);
        if (SUCCEEDED(hr)) hr = composition_device_->CreateEffectGroup(&scope_effect_);
        if (SUCCEEDED(hr)) hr = scope_visual_->SetEffect(scope_effect_);
        if (SUCCEEDED(hr)) hr = composition_target_->SetRoot(root_visual_);
        if (SUCCEEDED(hr)) hr = root_visual_->AddVisual(scope_visual_, FALSE, nullptr);
        if (SUCCEEDED(hr)) hr = root_visual_->AddVisual(sentinel_visual_, TRUE, scope_visual_);
        if (SUCCEEDED(hr)) {
            hr = composition_device_->CreateSurface(
                1, 1, DXGI_FORMAT_B8G8R8A8_UNORM,
                DXGI_ALPHA_MODE_PREMULTIPLIED, &sentinel_surface_);
        }
        if (SUCCEEDED(hr) && !draw_sentinel_surface()) hr = E_FAIL;
        if (SUCCEEDED(hr)) hr = sentinel_visual_->SetContent(sentinel_surface_);
        if (SUCCEEDED(hr)) hr = scope_effect_->SetOpacity(0.0f);
        if (SUCCEEDED(hr)) hr = composition_device_->Commit();
        if (FAILED(hr)) {
            set_status("Could not initialize the GPU overlay compositor.");
            release_composition();
            return false;
        }

        composition_ready_ = true;
        current_scope_opacity_ = 0.0f;
        return true;
    }

    bool upload_scope_surface() {
        if (!composition_ready_ || !rendered_.bitmap) return false;

        release_com(scope_surface_);
        HRESULT hr = composition_device_->CreateSurface(
            (UINT)rendered_.width, (UINT)rendered_.height,
            DXGI_FORMAT_B8G8R8A8_UNORM,
            DXGI_ALPHA_MODE_PREMULTIPLIED, &scope_surface_);
        if (FAILED(hr)) {
            set_status("Could not allocate the GPU overlay surface.");
            return false;
        }

        POINT offset{};
        ID2D1DeviceContext* context = nullptr;
        hr = scope_surface_->BeginDraw(nullptr, IID_PPV_ARGS(&context), &offset);
        if (FAILED(hr)) {
            set_status("Could not begin drawing the GPU overlay surface.");
            release_com(scope_surface_);
            return false;
        }

        DIBSECTION dib{};
        bool got_bitmap = GetObjectW(rendered_.bitmap, sizeof(dib), &dib) == sizeof(dib);
        ID2D1Bitmap1* bitmap = nullptr;
        if (got_bitmap && dib.dsBm.bmBits) {
            D2D1_BITMAP_PROPERTIES1 properties{};
            properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
            properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
            properties.dpiX = 96.0f;
            properties.dpiY = 96.0f;
            properties.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
            D2D1_SIZE_U size{(UINT32)rendered_.width, (UINT32)rendered_.height};
            hr = context->CreateBitmap(
                size, dib.dsBm.bmBits, (UINT32)dib.dsBm.bmWidthBytes,
                &properties, &bitmap);
        } else {
            hr = E_FAIL;
        }

        if (SUCCEEDED(hr)) {
            D2D1_MATRIX_3X2_F transform{1.0f, 0.0f, 0.0f, 1.0f,
                                       (FLOAT)offset.x, (FLOAT)offset.y};
            context->SetTransform(transform);
            D2D1_COLOR_F transparent{0.0f, 0.0f, 0.0f, 0.0f};
            context->Clear(&transparent);
            D2D1_RECT_F destination{
                0.0f, 0.0f, (FLOAT)rendered_.width, (FLOAT)rendered_.height};
            context->DrawBitmap(
                bitmap, &destination, 1.0f,
                D2D1_INTERPOLATION_MODE_NEAREST_NEIGHBOR);
        }

        release_com(bitmap);
        context->Release();
        HRESULT end_hr = scope_surface_->EndDraw();
        if (FAILED(hr) || FAILED(end_hr)) {
            set_status("Could not upload the scope image to the GPU.");
            release_com(scope_surface_);
            return false;
        }

        hr = scope_visual_->SetContent(scope_surface_);
        if (FAILED(hr)) {
            set_status("Could not attach the GPU overlay surface.");
            release_com(scope_surface_);
            return false;
        }

        surface_dirty_ = false;
        return true;
    }

    float configured_overlay_opacity() const {
        return std::clamp(active_config_.opacity, 0.0f, 1.0f);
    }

    bool set_scope_opacity(float opacity) {
        if (!overlay_primed_ || !composition_ready_) return false;
        opacity = std::clamp(opacity, 0.0f, 1.0f);
        if (std::fabs(current_scope_opacity_ - opacity) < 0.001f) return true;

        HRESULT hr = scope_effect_->SetOpacity(opacity);
        if (SUCCEEDED(hr)) hr = composition_device_->Commit();
        if (FAILED(hr)) {
            set_status("Could not commit the GPU overlay opacity.");
            return false;
        }
        current_scope_opacity_ = opacity;
        return true;
    }

    bool prime_overlay_surface() {
        if (bitmap_dirty_ && !render_bitmap()) return false;
        if (!composition_ready_) return false;
        if ((surface_dirty_ || !scope_surface_) && !upload_scope_surface()) return false;

        POINT pos = overlay_position();
        float desired_opacity = visible_ ? configured_overlay_opacity() : 0.0f;
        HRESULT hr = scope_effect_->SetOpacity(desired_opacity);
        if (SUCCEEDED(hr)) {
            hr = sentinel_visual_->SetOffsetX((float)std::max(0, rendered_.width - 1));
        }
        if (SUCCEEDED(hr)) {
            hr = sentinel_visual_->SetOffsetY((float)std::max(0, rendered_.height - 1));
        }
        if (FAILED(hr)) {
            set_status("Could not prepare the GPU overlay visuals.");
            return false;
        }

        SetWindowPos(overlay_hwnd_, HWND_TOPMOST, pos.x, pos.y,
                     rendered_.width, rendered_.height,
                     SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOSENDCHANGING);
        hr = composition_device_->Commit();
        if (FAILED(hr)) {
            set_status("Could not commit the GPU overlay surface.");
            return false;
        }

        overlay_primed_ = true;
        current_scope_opacity_ = desired_opacity;
        last_screen_width_ = GetSystemMetrics(SM_CXSCREEN);
        last_screen_height_ = GetSystemMetrics(SM_CYSCREEN);
        return true;
    }

    void reassert_static_overlay() {
        if (!overlay_primed_) return;
        POINT pos = overlay_position();
        SetWindowPos(overlay_hwnd_, HWND_TOPMOST, pos.x, pos.y, 0, 0,
                     SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW |
                     SWP_NOSENDCHANGING);
        last_screen_width_ = GetSystemMetrics(SM_CXSCREEN);
        last_screen_height_ = GetSystemMetrics(SM_CYSCREEN);
    }

    void refresh_static_overlay_placement() {
        HWND foreground = GetForegroundWindow();
        int screen_width = GetSystemMetrics(SM_CXSCREEN);
        int screen_height = GetSystemMetrics(SM_CYSCREEN);
        bool placement_changed =
            foreground != last_foreground_hwnd_ ||
            screen_width != last_screen_width_ ||
            screen_height != last_screen_height_;
        if (!placement_changed) return;

        last_foreground_hwnd_ = foreground;
        if (armed_) reassert_static_overlay();
    }

    void hide_overlay() {
        if (pending_show_) {
            KillTimer(control_hwnd_.load(), kShowTimer);
            pending_show_ = false;
        }
        if (overlay_primed_) set_scope_opacity(0.0f);
        visible_ = false;
    }

    void show_overlay() {
        if (!overlay_hwnd_ || !active_config_.enabled || !armed_) return;
        if (bitmap_dirty_ || surface_dirty_ || !overlay_primed_) {
            if (!prime_overlay_surface()) return;
        }
        if (!set_scope_opacity(configured_overlay_opacity())) return;
        visible_ = true;
        pending_show_ = false;
    }
    mutable std::mutex config_mutex_;
    AppConfig pending_config_;
    AppConfig active_config_;

    mutable std::mutex status_mutex_;
    std::string status_ = "Starting overlay thread...";

    std::thread thread_;
    std::atomic<HWND> control_hwnd_{nullptr};
    HWND overlay_hwnd_ = nullptr;

    DecodedImage source_;
    RenderedBitmap rendered_;
    std::string loaded_path_;
    bool bitmap_dirty_ = true;
    bool surface_dirty_ = true;
    bool overlay_primed_ = false;
    bool composition_ready_ = false;
    ID3D11Device* d3d_device_ = nullptr;
    ID2D1Factory1* d2d_factory_ = nullptr;
    ID2D1Device* d2d_device_ = nullptr;
    IDCompositionDesktopDevice* composition_device_ = nullptr;
    IDCompositionTarget* composition_target_ = nullptr;
    IDCompositionVisual2* root_visual_ = nullptr;
    IDCompositionEffectGroup* scope_effect_ = nullptr;
    IDCompositionVisual2* scope_visual_ = nullptr;
    IDCompositionVisual2* sentinel_visual_ = nullptr;
    IDCompositionSurface* scope_surface_ = nullptr;
    IDCompositionSurface* sentinel_surface_ = nullptr;
    float current_scope_opacity_ = -1.0f;
    int last_screen_width_ = 0;
    int last_screen_height_ = 0;
    HWND last_foreground_hwnd_ = nullptr;
    bool visible_ = false;
    bool pending_show_ = false;
    bool armed_ = false;
    bool trigger_was_down_ = false;
    bool hotkey_registered_ = false;
    Hotkey registered_hotkey_;
};

void copy_to_buffer(char* dst, size_t size, const std::string& src) {
    if (size == 0) return;
    std::snprintf(dst, size, "%s", src.c_str());
}

ft::Mode requested_mode(int argc, char** argv) {
    ft::Mode mode = ft::Mode::Auto;
    for (int i = 1; i < argc; ++i) {
        const char* arg = argv[i];
        if (!arg) continue;
        if (std::strcmp(arg, "--ft-auto") == 0) mode = ft::Mode::Auto;
        else if (std::strcmp(arg, "--ft-gui") == 0 || std::strcmp(arg, "--gui") == 0) mode = ft::Mode::Gui;
        else if (std::strcmp(arg, "--ft-tui") == 0 || std::strcmp(arg, "--tui") == 0) mode = ft::Mode::Tui;
        else if (std::strcmp(arg, "--ft-web") == 0 || std::strcmp(arg, "--web") == 0) mode = ft::Mode::Web;
        else if (std::strncmp(arg, "--ft-mode=", 10) == 0) {
            const char* value = arg + 10;
            if (std::strcmp(value, "auto") == 0) mode = ft::Mode::Auto;
            else if (std::strcmp(value, "gui") == 0) mode = ft::Mode::Gui;
            else if (std::strcmp(value, "tui") == 0) mode = ft::Mode::Tui;
            else if (std::strcmp(value, "web") == 0) mode = ft::Mode::Web;
        }
    }
    return mode;
}

void detach_console_for_windowed_mode() {
    DWORD console_processes[2] = {};
    const DWORD process_count = GetConsoleProcessList(console_processes, 2);
    HWND console_window = GetConsoleWindow();
    if (process_count == 1 && console_window) ShowWindow(console_window, SW_HIDE);
    if (process_count > 0) (void)FreeConsole();
}

} // namespace

int main(int argc, char** argv) {
    const bool tui_mode = requested_mode(argc, argv) == ft::Mode::Tui;
    if (!tui_mode) detach_console_for_windowed_mode();

    AppConfig cfg;
    cfg.image_path = default_asset_path();
    read_config(&cfg);

    OverlayController overlay;
    overlay.start(cfg);

    ft::Config window{};
    window.title = "Soy Scope";
    window.width = 760;
    window.height = 680;
    window.resizable = false;
    window.center_window = true;
    if (tui_mode) {
        window.fps_limit = 15;
        window.tui_animate = false;
    }

    if (!ft::create_window(window, argc, argv)) {

        return 1;
    }

    HICON app_icon = static_cast<HICON>(LoadImageW(
        GetModuleHandleW(nullptr), MAKEINTRESOURCEW(IDI_SOYSCOPE_ICON),
        IMAGE_ICON, 256, 256, LR_DEFAULTCOLOR));
    if (app_icon) ft::set_window_icon(app_icon);

    char image_path[1024]{};
    char hotkey[96]{};
    copy_to_buffer(image_path, sizeof(image_path), cfg.image_path);
    copy_to_buffer(hotkey, sizeof(hotkey), cfg.hotkey_text);

    AppConfig last_sent = cfg;
    std::string last_saved_status;

    while (ft::pump()) {
        ft::begin();

        ft::text("Soy Scope");
        ft::text_wrapped("Transparent click-through overlay. Press the hotkey to arm it, then hold the selected mouse button to show it after the configured delay.");
        ft::separator();

        bool dirty = false;
        dirty |= ft::checkbox("Enable hotkey/mouse trigger", &cfg.enabled);
        dirty |= ft::checkbox("Use a different mouse button", &cfg.custom_mouse_button);
        if (cfg.custom_mouse_button) {
            dirty |= ft::dropdown("Trigger button", kAlternateButtonNames, 4, &cfg.mouse_button);
        } else {
            ft::text("Trigger button: Right mouse button");
        }

        ft::set_next_fill();
        if (ft::input("Image path", image_path, sizeof(image_path))) {
            cfg.image_path = image_path;
            dirty = true;
        }

        ft::row({1.0f, 1.0f, 1.0f, 1.0f}, [&]() {
            if (ft::button("Browse image")) {
                ft::FileFilter filters[] = {
                    {"Images", "*.webp;*.png;*.jpg;*.jpeg;*.bmp"},
                    {"All files", "*.*"},
                };
                std::string picked = ft::open_file_dialog("Choose scope image", filters, 2);
                if (!picked.empty()) {
                    cfg.image_path = picked;
                    copy_to_buffer(image_path, sizeof(image_path), cfg.image_path);
                    dirty = true;
                }
            }
            if (ft::button("Use embedded")) {
                cfg.image_path = kEmbeddedImagePath;
                copy_to_buffer(image_path, sizeof(image_path), cfg.image_path);
                dirty = true;
            }
            if (ft::button("Show if armed", ft::ColorRole::Accent)) {
                overlay.show_now();
            }
            if (ft::button("Hide")) {
                overlay.hide();
            }
        });

        ft::spacing(4.0f);
        if (ft::input("Hotkey", hotkey, sizeof(hotkey), ft::InputFlags::CharsNoBlank)) {
            cfg.hotkey_text = hotkey;
            dirty = true;
        }

        dirty |= ft::slider_float("Delay before show (ms)", &cfg.delay_ms, 0.0f, 3000.0f);
        dirty |= ft::slider_float("Overlay width (px)", &cfg.width_px, 200.0f, 2200.0f);
        dirty |= ft::slider_float("Line opacity", &cfg.opacity, 0.05f, 1.0f);
        dirty |= ft::slider_float("Fingertip anchor X (%)", &cfg.anchor_x_pct, 0.0f, 100.0f);
        dirty |= ft::slider_float("Fingertip anchor Y (%)", &cfg.anchor_y_pct, 0.0f, 100.0f);

        ft::separator();
        ft::row({1.0f, 1.0f}, [&]() {
            if (ft::button("Save config", ft::ColorRole::Success)) {
                write_config(cfg);
                last_saved_status = "Saved to " + config_path();
            }
            if (ft::button("Reset anchor")) {
                cfg.anchor_x_pct = 57.5f;
                cfg.anchor_y_pct = 37.5f;
                dirty = true;
            }
        });

        Hotkey parsed = parse_hotkey(cfg.hotkey_text);
        if (!parsed.valid) {
            ft::set_next_color(ft::ColorRole::Text, ft::get_style().warning);
            ft::text("Hotkey format examples: F8, Ctrl+Alt+S, Shift+F9.");
        }

        std::string status = overlay.status();
        ft::text_wrapped(status.c_str());
        if (!last_saved_status.empty()) {
            ft::set_next_color(ft::ColorRole::Text, ft::get_style().success);
            ft::text_wrapped(last_saved_status.c_str());
        }

        if (dirty || !same_config(cfg, last_sent)) {
            overlay.apply(cfg);
            last_sent = cfg;
        }

        ft::end();
    }

    write_config(cfg);
    overlay.stop();
    ft::shutdown();
    if (app_icon) DestroyIcon(app_icon);
    return 0;
}
