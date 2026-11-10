#!/usr/bin/env python3
"""Run production Windows/Linux trigger methods with deterministic input/timers."""
from pathlib import Path
import os
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
windows = (root / 'src/main.cpp').read_text()
linux = (root / 'linux/main.cpp').read_text()

def method(source, signature):
    start = source.index(signature)
    opening = source.index('{', start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]

code = r'''
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
using UINT = unsigned;
constexpr int kShowTimer = 42;
void SetTimer(int, int, UINT, void*) {}
int GetForegroundWindow() { return 0; }
struct Config { bool enabled=true, toggle_trigger=false; float delay_ms=250; };
const char* trigger_button_name(const Config&) { return "test button"; }
struct Windows {
    Config active_config_;
    bool armed_=true, trigger_was_down_=false, pending_show_=false, visible_=false, down=false;
    int last_foreground_hwnd_=0, aims=0;
    std::atomic<int> control_hwnd_{0};
    bool trigger_button_down() const { return down; }
    void refresh_static_overlay_placement() {}
    void reassert_static_overlay() {}
    void set_status(const std::string&) {}
    void hide_overlay() { pending_show_=false; visible_=false; }
    void show_overlay(bool aim) { visible_=true; pending_show_=false; aims+=aim; }
'''
code += '\n'.join(method(windows, signature) for signature in (
    '    void toggle_mouse_trigger()', '    void poll_trigger_button()', '    void begin_trigger_delay()'))
timer = windows[windows.index('                if (!self->pending_show_)'):windows.index('            } else if (wp == kMousePollTimer)')]
code += '\nvoid expire() { auto self=this; int hwnd=0;\n' + timer.replace('return 0;', 'return;').replace('KillTimer(hwnd, kShowTimer);', '(void)hwnd;') + '\n}\n};\n'
code += r'''
struct Check { bool value=false; bool isChecked() const { return value; } };
struct Timer { bool active=false; int starts=0; void start(){active=true;++starts;} void stop(){active=false;} bool isActive() const{return active;} };
struct Image { bool visible=false; void hide(){visible=false;} bool isVisible()const{return visible;} };
struct Linux {
    Check mode, arm{true}, previewState; Check *toggleTrigger=&mode, *armed=&arm, *preview=&previewState;
    Timer delay; Image overlay;
    bool down=false, wasDown=false, awaitRelease=false;
    bool anyDown() const{return down;}
'''
code += method(linux, '    void changed()') + method(linux, '    void triggerChanged(bool down)')
condition = linux.split('connect(&delay,&QTimer::timeout,this,[&]{if(', 1)[1].split('){overlay.place();', 1)[0]
code += '\nvoid expire() { if (!delay.active) return; delay.stop(); if (' + condition + ') overlay.visible=true; }\n};\n'
code += r'''
int main() {
    for (bool toggle : {false,true}) {
        Windows w; w.active_config_.toggle_trigger=toggle;
        w.down=true; w.poll_trigger_button(); assert(w.pending_show_);
        // A held button never repeats/restarts a toggle.
        w.poll_trigger_button(); assert(w.pending_show_);
        w.down=false; w.poll_trigger_button(); assert(w.pending_show_==toggle);
        w.expire(); assert(w.visible_==toggle && w.aims==int(toggle));
        w.down=true; w.poll_trigger_button();
        if(toggle) { assert(!w.visible_ && !w.pending_show_); }
        else { w.expire(); assert(w.visible_ && w.aims==1); }
        w.down=false; w.poll_trigger_button(); assert(!w.visible_);
        // Second press during a delay cancels it, even after a timer is queued.
        w.down=true; w.poll_trigger_button();
        w.down=false; w.poll_trigger_button();
        if(toggle) { w.down=true; w.poll_trigger_button(); }
        assert(!w.pending_show_); w.expire(); assert(!w.visible_);
        // Disarm cancels both pending and visible state.
        w.down=false; w.poll_trigger_button(); w.down=true; w.poll_trigger_button();
        w.toggle_mouse_trigger(); assert(!w.pending_show_ && !w.visible_ && !w.armed_);
        // Toggle mode requires a fresh press when arming with a held button.
        w.toggle_mouse_trigger(); assert(w.pending_show_==!toggle);
        w.hide_overlay(); w.down=false; w.poll_trigger_button();
        w.active_config_.delay_ms=0;
        w.down=true; w.poll_trigger_button(); assert(w.visible_);
        w.poll_trigger_button(); assert(w.visible_);
        w.down=false; w.poll_trigger_button(); assert(w.visible_==toggle);
        if(toggle) { w.down=true; w.poll_trigger_button(); assert(!w.visible_); }

        Linux l; l.mode.value=toggle;
        l.down=true; l.changed(); assert(l.delay.active && l.delay.starts==1);
        l.changed(); assert(l.delay.starts==1);
        l.down=false; l.changed(); assert(l.delay.active==toggle);
        l.expire(); assert(l.overlay.visible==toggle);
        if(toggle) { l.down=true; l.changed(); assert(!l.delay.active && !l.overlay.visible); }
        // Toggle-off during the pending delay prevents later display.
        l.down=false; l.changed(); l.down=true; l.changed();
        l.down=false; l.changed();
        if(toggle) { l.down=true; l.changed(); }
        l.expire(); assert(!l.overlay.visible);
        // A visible toggle survives release, then hides on the next press.
        l.down=false; l.changed(); l.overlay.visible=true;
        l.changed(); assert(l.overlay.visible==toggle);
        l.down=true; l.changed(); assert(!l.overlay.visible);
        // Rebinding/arming blocks a held trigger until released.
        l.delay.stop(); l.wasDown=true; l.awaitRelease=true;
        l.changed(); assert(!l.delay.active);
        l.down=false; l.changed(); assert(!l.awaitRelease);
        l.down=true; l.changed(); assert(l.delay.active);
        l.delay.stop(); l.arm.value=false;
        l.down=false; l.changed(); l.down=true; l.changed(); assert(!l.delay.active);
    }
    std::cout << "PASS: Windows/Linux hold and toggle edges, release, cancellation, repeat suppression, arming, zero delay\n";
}
'''
with tempfile.TemporaryDirectory(prefix='soyscope-trigger-test-') as directory:
    path = Path(directory)
    (path / 'test.cpp').write_text(code)
    subprocess.run([os.environ.get('CXX', 'g++'), '-std=c++17', '-Wall', '-Wextra', '-Werror', str(path / 'test.cpp'), '-o', str(path / 'test')], check=True)
    subprocess.run([str(path / 'test')], check=True)
