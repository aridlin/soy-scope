#!/usr/bin/env python3
"""Exercise the production Medal state machine with deterministic Win32 input.
Run: python3 tests/test_medal.py (requires a C++17 compiler).
"""
from pathlib import Path
import os
import subprocess
import tempfile

source = (Path(__file__).resolve().parents[1] / 'src/main.cpp').read_text()
logic = source[source.index('struct Hotkey {'):source.index('std::wstring utf8_to_wide')]
config_io = source[source.index('void read_config('):source.index('struct DecodedImage')]
stubs = r'''
#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdlib>
#include <string>
#include <vector>
#include <fstream>
#include <cassert>
#include <iostream>
#include <limits>
using UINT = unsigned; using WORD = unsigned short; using ULONGLONG = unsigned long long;
constexpr UINT MOD_ALT=1, MOD_CONTROL=2, MOD_SHIFT=4, MOD_WIN=8, MOD_NOREPEAT=0x4000;
constexpr UINT VK_LBUTTON=1, VK_RBUTTON=2, VK_MBUTTON=4, VK_XBUTTON1=5, VK_XBUTTON2=6,
 VK_TAB=9, VK_CONTROL=17, VK_MENU=18, VK_SHIFT=16, VK_CAPITAL=20, VK_ESCAPE=27, VK_SPACE=32,
 VK_PRIOR=33, VK_NEXT=34, VK_END=35, VK_HOME=36, VK_LEFT=37, VK_UP=38, VK_RIGHT=39,
 VK_DOWN=40, VK_INSERT=45, VK_DELETE=46, VK_LWIN=91, VK_RWIN=92, VK_NUMPAD0=96,
 VK_F1=112, VK_F8=119;
constexpr UINT INPUT_KEYBOARD=1, KEYEVENTF_KEYUP=2, KEYEVENTF_EXTENDEDKEY=1;
struct INPUT { UINT type=0; struct { WORD wVk=0; UINT dwFlags=0; } ki; };
bool keys[256]{};
ULONGLONG clock_ms=100;
std::vector<INPUT> events;
UINT send_limit=std::numeric_limits<UINT>::max();
short GetAsyncKeyState(UINT vk) { return keys[vk] ? (short)0x8000 : 0; }
ULONGLONG GetTickCount64() { return clock_ms; }
UINT SendInput(UINT count, INPUT* inputs, int) {
 UINT accepted=std::min(count, send_limit);
 for(UINT i=0;i<accepted;++i) { events.push_back(inputs[i]); keys[inputs[i].ki.wVk]=!(inputs[i].ki.dwFlags & KEYEVENTF_KEYUP); }
 return accepted;
}
std::string config_path() { return "test.ini"; }
void reset() { std::fill(std::begin(keys),std::end(keys),false); events.clear(); clock_ms=100; send_limit=std::numeric_limits<UINT>::max(); }
'''
tests = r'''
void finish(MedalMarkers& m) { clock_ms += 60; m.tick(); }
int main() {
 const UINT f9=VK_F1+8, f10=VK_F1+9;
 AppConfig cfg;
 // Disabled defaults and old INI files never send input.
 { MedalMarkers m; m.configure(cfg); m.aim(); m.tick(); assert(events.empty()); }
 { std::ofstream f(config_path()); f << "hotkey=F8\n"; }
 AppConfig old; read_config(&old); assert(!old.medal_enabled);
 // Persist both independent modes and custom shortcuts.
 cfg.medal_enabled=true; cfg.medal_separate_key=true;
 cfg.medal_marker_key="Ctrl+F9"; cfg.medal_target_key="Alt+F10";
 write_config(cfg); AppConfig loaded; read_config(&loaded); assert(same_config(cfg,loaded));
 cfg=AppConfig{}; cfg.medal_enabled=true;
 // Automatic request: one keydown, held for 50 ms, then keyup; no repeats.
 { MedalMarkers m; m.configure(cfg); m.aim(); m.tick(); assert(events.size()==1 && events[0].ki.wVk==f10);
   m.tick(); assert(events.size()==1); finish(m); assert(events.size()==2 && !keys[f10]);
   finish(m); assert(events.size()==2); m.aim(); m.tick(); finish(m); assert(events.size()==4); }
 reset();
 // Manual works independently of overlay arming/enabling and once per press.
 cfg.enabled=false; cfg.medal_every_aim=false; cfg.medal_separate_key=true;
 { MedalMarkers m; m.configure(cfg); m.aim(); m.tick(); assert(events.empty());
   keys[f9]=true; m.tick(); finish(m); finish(m); assert(events.size()==2);
   keys[f9]=false; m.tick(); keys[f9]=true; m.tick(); finish(m); assert(events.size()==4); }
 reset();
 // Both modes and live cancellation when disabling while output is held.
 cfg.medal_every_aim=true;
 { MedalMarkers m; m.configure(cfg); m.aim(); m.tick(); finish(m);
   keys[f9]=true; m.tick(); assert(events.size()==3);
   cfg.medal_enabled=false; m.configure(cfg); assert(!keys[f10] && events.size()==4);
   keys[f9]=false; m.tick(); keys[f9]=true; m.aim(); m.tick(); assert(events.size()==4); }
 reset(); cfg.medal_enabled=true;
 // Block shortcut recursion and arming collisions, including modified variants.
 for (const char* bad : {"F8", "Ctrl+F8", "F9", "garbage", "F10junk"}) {
   AppConfig c=cfg; c.medal_target_key=bad; MedalMarkers m; m.configure(c); m.aim(); m.tick(); assert(events.empty());
 }
 { AppConfig c=cfg; c.medal_marker_key="F8"; MedalMarkers m; m.configure(c); m.aim(); m.tick(); assert(events.empty()); }
 // Do not synthesize releases of the user's held modifiers. Wait, then expire.
 { MedalMarkers m; m.configure(cfg); keys[VK_SHIFT]=true; m.aim(); m.tick(); assert(events.empty());
   clock_ms+=2100; m.tick(); keys[VK_SHIFT]=false; m.tick(); assert(events.empty());
   m.aim(); m.tick(); finish(m); assert(events.size()==2); }
 reset();
 // Modified target: release only modifiers injected by Soyscope.
 { AppConfig c=cfg; c.medal_target_key="Ctrl+F10"; MedalMarkers m; m.configure(c);
   keys[VK_CONTROL]=true; m.aim(); m.tick(); finish(m); assert(events.size()==2 && keys[VK_CONTROL]);
   keys[VK_CONTROL]=false; m.aim(); m.tick(); finish(m); assert(events.size()==6 && !keys[VK_CONTROL]); }
 reset();
 // Manual combo waits for unrelated modifiers to be released before forwarding.
 { AppConfig c=cfg; c.medal_marker_key="Ctrl+F9"; MedalMarkers m; m.configure(c);
   keys[VK_CONTROL]=true; keys[f9]=true; m.tick(); assert(events.empty());
   keys[f9]=false; keys[VK_CONTROL]=false; m.tick(); finish(m); assert(events.size()==2); }
 reset();
 // Binding changes discard pending events and require a fresh manual press.
 { MedalMarkers m; m.configure(cfg); keys[VK_SHIFT]=true; m.aim(); m.tick();
   AppConfig c=cfg; c.medal_target_key="F11"; keys[f9]=true; m.configure(c);
   keys[VK_SHIFT]=false; m.tick(); assert(events.empty()); keys[f9]=false; m.tick();
   keys[f9]=true; m.tick(); finish(m); assert(events.size()==2 && events[0].ki.wVk==VK_F1+10); }
 reset();
 // Partial send cleans up keys that actually went down; shutdown releases too.
 { AppConfig c=cfg; c.medal_target_key="Ctrl+F10"; MedalMarkers m; m.configure(c);
   send_limit=1; m.aim(); m.tick(); assert(!keys[VK_CONTROL]); assert(m.status().find("blocked")!=std::string::npos); }
 reset();
 { MedalMarkers m; m.configure(cfg); m.aim(); m.tick(); m.stop(); assert(!keys[f10]); }
 reset();
 // Failed key-up is retried on the next tick.
 { MedalMarkers m; m.configure(cfg); m.aim(); m.tick(); send_limit=0; finish(m); assert(keys[f10]);
   send_limit=100; m.tick(); assert(!keys[f10]); }
 std::cout << "PASS: Medal modes, persistence, press/release timing, conflicts, held modifiers, cancellation and input failures\n";
}
'''
with tempfile.TemporaryDirectory(prefix='soyscope-medal-test-') as directory:
    root = Path(directory)
    (root / 'test.cpp').write_text(stubs + logic + config_io + tests)
    subprocess.run([os.environ.get('CXX', 'g++'), '-std=c++17', '-Wall', '-Wextra', '-Werror', str(root / 'test.cpp'), '-o', str(root / 'test')], check=True)
    subprocess.run([str(root / 'test')], cwd=root, check=True)
