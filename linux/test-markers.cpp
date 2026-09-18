#include "markers.h"
#include <QCoreApplication>
#ifdef NDEBUG
#undef NDEBUG
#endif
#include <cassert>
#include <iostream>
int main(int argc,char **argv){QCoreApplication app(argc,argv);
 assert(shortcut("F8").key==KEY_F8);assert(shortcut("Ctrl+Alt+M").key==KEY_M);assert(shortcut("Ctrl+Alt+M").modifiers.size()==2);assert(!shortcut("nonsense").valid());assert(!shortcut("F8, F9").valid());
 std::vector<std::pair<int,bool>> events;bool held=false;MarkerQueue q;q.target=shortcut("Ctrl+F10");q.held=[&](int){return held;};q.output=[&](int k,bool d){events.emplace_back(k,d);return true;};
 held=true;q.request(0);q.request(1000);assert(q.deadline==2000);q.tick(1999);assert(events.empty());q.tick(2000);assert(!q.pending&&events.empty());
 held=false;q.request(3000);q.tick(3000);assert(q.active&&events.size()==2);q.request(3020);q.tick(3049);assert(events.size()==2);q.tick(3050);assert(!q.active&&events.size()==4);assert(events.back()==std::make_pair(KEY_LEFTCTRL,false));
 q.request(4000);q.tick(4000);q.cancel();assert(events.size()==8&&!q.active&&!q.pending);
 q.request(5000);q.cancel();q.tick(5000);assert(events.size()==8);
 std::cout<<"PASS: shortcuts, held keys, timeout, coalescing, 50ms release and cancellation\n";
}
