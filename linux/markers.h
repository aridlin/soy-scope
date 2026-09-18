#pragma once
#include <QKeySequence>
#include <QString>
#include <QElapsedTimer>
#include <linux/uinput.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <functional>
#include <vector>
#include <algorithm>

struct Shortcut {
 int key=0; std::vector<int> modifiers;
 bool valid() const {return key!=0;}
};
inline Shortcut shortcut(const QString &text) {
 QKeySequence seq=QKeySequence::fromString(text,QKeySequence::PortableText);
 Shortcut result;if(seq.count()!=1 || seq.isEmpty())return result;
 auto k=seq[0];int key=k.key();
 if(key>=Qt::Key_F1 && key<=Qt::Key_F24){static int f[]={KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,KEY_F13,KEY_F14,KEY_F15,KEY_F16,KEY_F17,KEY_F18,KEY_F19,KEY_F20,KEY_F21,KEY_F22,KEY_F23,KEY_F24};result.key=f[key-Qt::Key_F1];}
 else if(key>=Qt::Key_A && key<=Qt::Key_Z){static int letters[]={KEY_A,KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,KEY_U,KEY_V,KEY_W,KEY_X,KEY_Y,KEY_Z};result.key=letters[key-Qt::Key_A];}
 else if(key>=Qt::Key_0 && key<=Qt::Key_9){static int digits[]={KEY_0,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9};result.key=digits[key-Qt::Key_0];}
 else {switch(key){case Qt::Key_Space:result.key=KEY_SPACE;break;case Qt::Key_Insert:result.key=KEY_INSERT;break;case Qt::Key_Delete:result.key=KEY_DELETE;break;case Qt::Key_Home:result.key=KEY_HOME;break;case Qt::Key_End:result.key=KEY_END;break;case Qt::Key_PageUp:result.key=KEY_PAGEUP;break;case Qt::Key_PageDown:result.key=KEY_PAGEDOWN;break;case Qt::Key_Pause:result.key=KEY_PAUSE;break;case Qt::Key_Escape:result.key=KEY_ESC;break;case Qt::Key_Left:result.key=KEY_LEFT;break;case Qt::Key_Right:result.key=KEY_RIGHT;break;case Qt::Key_Up:result.key=KEY_UP;break;case Qt::Key_Down:result.key=KEY_DOWN;break;default:break;}}
 auto mods=k.keyboardModifiers();
 if(mods&Qt::ControlModifier)result.modifiers.push_back(KEY_LEFTCTRL);
 if(mods&Qt::AltModifier)result.modifiers.push_back(KEY_LEFTALT);
 if(mods&Qt::ShiftModifier)result.modifiers.push_back(KEY_LEFTSHIFT);
 if(mods&Qt::MetaModifier)result.modifiers.push_back(KEY_LEFTMETA);
 return result;
}
class MarkerOutput {
 int fd=-1;
public:
 ~MarkerOutput(){if(fd>=0){ioctl(fd,UI_DEV_DESTROY);close(fd);}}
 bool openDevice(){
  if(fd>=0)return true;fd=::open("/dev/uinput",O_WRONLY|O_NONBLOCK|O_CLOEXEC);if(fd<0)return false;
  bool ok=ioctl(fd,UI_SET_EVBIT,EV_KEY)>=0;
  for(int key=1;key<=KEY_MAX;++key)ok=(ioctl(fd,UI_SET_KEYBIT,key)>=0)&&ok;
  uinput_setup setup{};std::strncpy(setup.name,"Soy Scope Marker",UINPUT_MAX_NAME_SIZE-1);setup.id.bustype=BUS_VIRTUAL;
  ok=ok&&ioctl(fd,UI_DEV_SETUP,&setup)>=0&&ioctl(fd,UI_DEV_CREATE)>=0;
  if(!ok){close(fd);fd=-1;}return ok;
 }
 bool key(int code,bool down){if(!openDevice())return false;input_event ev[2]{};ev[0].type=EV_KEY;ev[0].code=code;ev[0].value=down;ev[1].type=EV_SYN;ev[1].code=SYN_REPORT;return write(fd,ev,sizeof(ev))==sizeof(ev);}
};
class MarkerQueue {
public:
 Shortcut target;bool pending=false,active=false; qint64 deadline=0,releaseAt=0;
 QString status="Markers off";
 std::vector<int> injected;
 std::function<bool(int)> held=[](int){return false;};
 std::function<bool(int,bool)> output=[](int,bool){return false;};
 void cancel(){for(auto i=injected.rbegin();i!=injected.rend();++i)output(*i,false);injected.clear();pending=false;active=false;}
 ~MarkerQueue(){cancel();}
 void request(qint64 now){if(pending||active||!target.valid())return;pending=true;deadline=now+2000;status="Marker queued";}
 void tick(qint64 now){
  if(active){if(now>=releaseAt){cancel();status="Shortcut sent; recorder receipt is unverified";}return;}
  if(!pending)return;
  if(now>=deadline){pending=false;status="Marker skipped: key or modifier remained held";return;}
  static int mods[]={KEY_LEFTCTRL,KEY_RIGHTCTRL,KEY_LEFTALT,KEY_RIGHTALT,KEY_LEFTSHIFT,KEY_RIGHTSHIFT,KEY_LEFTMETA,KEY_RIGHTMETA};
  if(held(target.key)||std::any_of(std::begin(mods),std::end(mods),held))return;
  std::vector<int> keys=target.modifiers;keys.push_back(target.key);
  for(int key:keys){injected.push_back(key);if(!output(key,true)){cancel();status="Marker input unavailable (/dev/uinput)";return;}}
  pending=false;active=true;releaseAt=now+50;
 }
};
