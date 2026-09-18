#include "markers.h"
#include <QComboBox>
#include <QLineEdit>
#include <QIcon>
#include <QTextStream>
#include <QCloseEvent>
#include <QSet>
#include <QApplication>
#include <QAction>
#include <QCheckBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QSettings>
#include <QSocketNotifier>
#include <QTimer>
#include <QStandardPaths>
#include <KGlobalAccel>
#include <KWindowSystem>
#include <LayerShellQt/Window>
#include <QDebug>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <map>
#include <memory>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <QDesktopServices>
#include "web.h"

// Same luminance-to-alpha treatment as the original Windows renderer.
QImage inkMask(const QImage &source, double opacity) {
    auto pixels=source.convertToFormat(QImage::Format_ARGB32);
    QImage result(pixels.size(),QImage::Format_ARGB32_Premultiplied);
    for(int y=0;y<pixels.height();++y) {
        auto input=reinterpret_cast<const QRgb*>(pixels.constScanLine(y));
        auto output=reinterpret_cast<QRgb*>(result.scanLine(y));
        for(int x=0;x<pixels.width();++x) {
            double luma=.2126*qRed(input[x])+.7152*qGreen(input[x])+.0722*qBlue(input[x]);
            double ink=std::pow(std::clamp((255.-luma)/255.,0.,1.),1.15);
            int alpha=qRound(ink*std::clamp(opacity,0.,1.)*qAlpha(input[x]));
            output[x]=qRgba(0,0,0,alpha<8?0:alpha);
        }
    }
    return result;
}

class Overlay : public QWidget {
public:
    QImage image{":/two-soyjaks-pointing.webp"};
    QImage rendered;
    QString screenName;
    LayerShellQt::Window *layer=nullptr;
    double alpha=.72, width=1200, anchorX=.575, anchorY=.375;
    Overlay() : QWidget(nullptr,Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint|Qt::WindowTransparentForInput|Qt::WindowDoesNotAcceptFocus) {
        setAttribute(Qt::WA_TranslucentBackground);setAttribute(Qt::WA_ShowWithoutActivating);
        if(QGuiApplication::platformName()=="wayland") {
            winId();
            layer=LayerShellQt::Window::get(windowHandle());
            layer->setLayer(LayerShellQt::Window::LayerOverlay);
            layer->setAnchors(LayerShellQt::Window::Anchors(LayerShellQt::Window::AnchorTop) | LayerShellQt::Window::AnchorLeft);
            layer->setExclusiveZone(-1);
            layer->setKeyboardInteractivity(LayerShellQt::Window::KeyboardInteractivityNone);
            layer->setScope("soy-scope-overlay");
        }
        rebuild();
    }
    void rebuild() {rendered=inkMask(image,alpha);update();}
    void place() {
        auto screen=QGuiApplication::primaryScreen();
        for(auto candidate:QGuiApplication::screens()) if(candidate->name()==screenName) {screen=candidate;break;}
        if(!screen || image.isNull()) return;
        // Match Windows physical-pixel sizing while Qt places windows in logical pixels.
        double logicalWidth=width/screen->devicePixelRatio();
        QSize size(qRound(logicalWidth),qRound(logicalWidth*image.height()/image.width()));
        if(layer) {
            layer->setScreen(screen);
            layer->setDesiredSize(size);
            layer->setMargins(QMargins(qRound(screen->geometry().width()/2.-size.width()*anchorX),qRound(screen->geometry().height()/2.-size.height()*anchorY),0,0));
            resize(size);update();return;
        }
        QPoint center=screen->geometry().center();
        setGeometry(center.x()-qRound(size.width()*anchorX),center.y()-qRound(size.height()*anchorY),size.width(),size.height());
        update();
    }
    void paintEvent(QPaintEvent*) override {QPainter p(this);p.setCompositionMode(QPainter::CompositionMode_Source);p.fillRect(rect(),Qt::transparent);p.setCompositionMode(QPainter::CompositionMode_SourceOver);p.setRenderHint(QPainter::SmoothPixmapTransform);p.drawImage(rect(),rendered);}
};

class Settings : public QWidget {
    Overlay overlay;
    QSettings settings;
    QTimer delay, scan, markerTimer;
    MarkerOutput markerOutput;
    MarkerQueue marker;
    QElapsedTimer clock;
    QCheckBox *alternate, *markerEnabled, *everyAim, *manualEnabled;
    QComboBox *buttonChoice;
    QLineEdit *hotkeyEdit, *manualEdit, *targetEdit;
    QAction *toggleAction, *manualAction;
    QLabel *markerStatus;
    bool hotkeysValid=false, awaitRelease=false, manualLatch=false, toggleLatch=false;
    int trigger=BTN_RIGHT;
    QDoubleSpinBox *ax=nullptr,*ay=nullptr;
    QTimer previewTimer;
    QCheckBox *armed;
    QLabel *status;
    struct Device {int fd; QSocketNotifier *notifier; QSet<int> down;};
    std::map<QString,Device> devices;
    bool wasDown=false;
public:
    Settings() {
        setWindowTitle("Soy Scope — Linux");setWindowIcon(QIcon(":/soyscope-icon.png"));resize(520,680);clock.start();
        auto form=new QFormLayout(this);
        auto intro=new QLabel("Hold right click to show the scope on your primary screen.\nUse the configurable toggle hotkey to arm it. The image is click-through.");
        intro->setWordWrap(true);form->addRow(intro);
        armed=new QCheckBox("Armed");armed->setObjectName("armed");armed->setChecked(settings.value("armed",true).toBool());form->addRow(armed);
        auto spin=[&](const QString &label,const QString &key,double value,double maximum,auto changed){
            auto box=new QDoubleSpinBox;box->setObjectName(key);box->setDecimals((key=="anchorX"||key=="anchorY"||key=="opacity")?4:0);box->setRange(0,maximum);box->setValue(settings.value(key,value).toDouble());
            changed(box->value());form->addRow(label,box);if(key=="anchorX")ax=box;if(key=="anchorY")ay=box;
            connect(box,&QDoubleSpinBox::valueChanged,this,[this,key,changed](double v){settings.setValue(key,v);changed(v);overlay.place();});
        };
        spin("Delay (ms)","delay",250,5000,[&](double v){delay.setInterval(qRound(v));});
        spin("Width (screen pixels)","width",1200,5000,[&](double v){overlay.width=qMax(1.,v);});
        spin("Opacity (%)","opacity",72,100,[&](double v){overlay.alpha=v/100.;overlay.rebuild();});
        spin("Finger anchor X (%)","anchorX",57.5,100,[&](double v){overlay.anchorX=v/100.;});
        spin("Finger anchor Y (%)","anchorY",37.5,100,[&](double v){overlay.anchorY=v/100.;});
        QString path=settings.value("image").toString();if(!path.isEmpty()){QImage custom(path);if(!custom.isNull()){overlay.image=custom;overlay.rebuild();}}
        auto image=new QPushButton("Choose image…");image->setObjectName("browse-image");form->addRow(image);
        connect(image,&QPushButton::clicked,this,[&]{auto path=QFileDialog::getOpenFileName(this,"Scope image",{},"Images (*.png *.webp *.jpg *.jpeg *.bmp)");if(path.isEmpty())return;QImage custom(path);if(custom.isNull())return;overlay.image=custom;overlay.rebuild();settings.setValue("image",path);overlay.place();});
        auto reset=new QPushButton("Use original soyjak image");form->addRow(reset);
        connect(reset,&QPushButton::clicked,this,[&]{settings.remove("image");overlay.image=QImage(":/two-soyjaks-pointing.webp");overlay.rebuild();overlay.place();});
        auto monitorChoice=new QComboBox;
        for(auto screen:QGuiApplication::screens()) monitorChoice->addItem(screen->name());
        overlay.screenName=settings.value("screen", "DP-2").toString();
        monitorChoice->setCurrentText(overlay.screenName);
        form->addRow("Overlay monitor",monitorChoice);
        connect(monitorChoice,&QComboBox::currentTextChanged,this,[&](const QString &name){overlay.screenName=name;settings.setValue("screen",name);overlay.hide();overlay.place();});
        auto preview=new QPushButton("Show if armed");form->addRow(preview);
        connect(preview,&QPushButton::clicked,this,[&]{if(armed->isChecked()){overlay.place();overlay.show();}});
        auto hide=new QPushButton("Hide");form->addRow(hide);connect(hide,&QPushButton::clicked,this,[&]{delay.stop();overlay.hide();awaitRelease=anyDown();});
        auto resetAnchor=new QPushButton("Reset anchor");form->addRow(resetAnchor);connect(resetAnchor,&QPushButton::clicked,this,[&]{ax->setValue(57.5);ay->setValue(37.5);});
        alternate=new QCheckBox("Use a different mouse button");alternate->setObjectName("custom_mouse_button");alternate->setChecked(settings.value("custom_mouse_button",false).toBool());form->addRow(alternate);
        buttonChoice=new QComboBox;buttonChoice->setObjectName("mouse_button");buttonChoice->addItems({"Left","Middle","Side X1","Side X2"});buttonChoice->setCurrentIndex(settings.value("mouse_button",0).toInt());form->addRow("Alternate button",buttonChoice);
        auto updateTrigger=[&]{static int codes[]={BTN_LEFT,BTN_MIDDLE,BTN_SIDE,BTN_EXTRA};trigger=alternate->isChecked()?codes[qBound(0,buttonChoice->currentIndex(),3)]:BTN_RIGHT;buttonChoice->setEnabled(alternate->isChecked());settings.setValue("custom_mouse_button",alternate->isChecked());settings.setValue("mouse_button",buttonChoice->currentIndex());delay.stop();overlay.hide();wasDown=anyDown();awaitRelease=wasDown;};
        connect(alternate,&QCheckBox::toggled,this,updateTrigger);connect(buttonChoice,&QComboBox::currentIndexChanged,this,updateTrigger);updateTrigger();
        auto textSetting=[&](const QString& name,const QString& label,const QString& value){auto edit=new QLineEdit(settings.value(name,value).toString());edit->setObjectName(name);form->addRow(label,edit);return edit;};
        hotkeyEdit=textSetting("hotkey","Toggle shortcut","F8");
        markerEnabled=new QCheckBox("Enable Medal / recorder markers");markerEnabled->setObjectName("medal_enabled");markerEnabled->setChecked(settings.value("medal_enabled",false).toBool());form->addRow(markerEnabled);
        everyAim=new QCheckBox("Mark every aim");everyAim->setObjectName("medal_every_aim");everyAim->setChecked(settings.value("medal_every_aim",true).toBool());form->addRow(everyAim);
        manualEnabled=new QCheckBox("Mark with a separate key");manualEnabled->setObjectName("medal_separate_key");manualEnabled->setChecked(settings.value("medal_separate_key",false).toBool());form->addRow(manualEnabled);
        manualEdit=textSetting("medal_marker_key","Manual marker shortcut","F9");targetEdit=textSetting("medal_target_key","Recorder clip/bookmark shortcut","F10");
        auto hint=new QLabel("Set the same shortcut in your recorder. Forwarding works on Linux; actual Medal receipt is unverified. Markers reach the foreground application too.");hint->setWordWrap(true);form->addRow(hint);
        markerStatus=new QLabel;markerStatus->setWordWrap(true);form->addRow(markerStatus);
        auto save=new QPushButton("Save config");form->addRow(save);connect(save,&QPushButton::clicked,this,[&]{settings.sync();exportIni();});
        status=new QLabel;status->setWordWrap(true);form->addRow(status);
        delay.setSingleShot(true);
        connect(&delay,&QTimer::timeout,this,[&]{if(armed->isChecked()&&anyDown()&&!awaitRelease){overlay.place();overlay.show();if(markerEnabled->isChecked()&&everyAim->isChecked()&&hotkeysValid)marker.request(clock.elapsed());}});
        connect(armed,&QCheckBox::toggled,this,[&](bool enabled){settings.setValue("armed",enabled);delay.stop();overlay.hide();wasDown=false;});
        toggleAction=new QAction(this);toggleAction->setObjectName("toggle-soycope");toggleAction->setText("Arm/disarm Soy Scope");toggleAction->setAutoRepeat(false);
        manualAction=new QAction(this);manualAction->setObjectName("marker-soycope");manualAction->setText("Soy Scope recorder marker");manualAction->setAutoRepeat(false);
        connect(toggleAction,&QAction::triggered,this,[&]{if(!toggleLatch){toggleLatch=true;armed->toggle();}});
        connect(manualAction,&QAction::triggered,this,[&]{if(!manualLatch&&markerEnabled->isChecked()&&manualEnabled->isChecked()&&hotkeysValid){manualLatch=true;marker.request(clock.elapsed());}});
        for(auto edit:{hotkeyEdit,manualEdit,targetEdit})connect(edit,&QLineEdit::editingFinished,this,[this,edit]{settings.setValue(edit->objectName(),edit->text());configureShortcuts();});
        for(auto box:{markerEnabled,everyAim,manualEnabled})connect(box,&QCheckBox::toggled,this,[this,box]{settings.setValue(box->objectName(),box->isChecked());configureShortcuts();});
        marker.held=[this](int code){return physicalHeld(code);};marker.output=[this](int code,bool down){return markerOutput.key(code,down);};
        connect(&markerTimer,&QTimer::timeout,this,[&]{if(!physicalHeld(shortcut(manualEdit->text()).key))manualLatch=false;if(!physicalHeld(shortcut(hotkeyEdit->text()).key))toggleLatch=false;marker.tick(clock.elapsed());markerStatus->setText(marker.status);});markerTimer.start(10);configureShortcuts();
        connect(&scan,&QTimer::timeout,this,[&]{scanDevices();});scan.start(2000);scanDevices();
        connect(qApp,&QGuiApplication::primaryScreenChanged,this,[&]{overlay.place();});
    }
    ~Settings() override {marker.cancel();settings.sync();exportIni();for(auto &[name,d]:devices){delete d.notifier;::close(d.fd);}}
    bool anyDown() const {for(const auto &[name,d]:devices)if(d.down.contains(trigger))return true;return false;}
    void changed() {
        bool down=anyDown();
        if(!down)awaitRelease=false;
        if(down && !wasDown && !awaitRelease && armed->isChecked())delay.start();
        if(!down){delay.stop();overlay.hide();}
        wasDown=down;
    }
    void scanDevices() {
        if(qApp->arguments().contains("--test-controls"))return;
        for(auto it=devices.begin();it!=devices.end();) {
            if(!QFileInfo::exists(it->first)){delete it->second.notifier;::close(it->second.fd);it=devices.erase(it);changed();}
            else ++it;
        }
        QDir directory("/dev/input");
        for(const auto &name:directory.entryList({"event*"},QDir::System|QDir::Files)) {
            auto path=directory.filePath(name);if(devices.contains(path))continue;
            int fd=::open(path.toLocal8Bit().constData(),O_RDONLY|O_NONBLOCK|O_CLOEXEC);if(fd<0)continue;
            unsigned long buttons[(KEY_MAX+8*sizeof(long))/(8*sizeof(long))]{};
            char deviceName[256]{};ioctl(fd,EVIOCGNAME(sizeof(deviceName)),deviceName);
            if(QString::fromLocal8Bit(deviceName)=="Soy Scope Marker"){::close(fd);continue;}
            if(ioctl(fd,EVIOCGBIT(EV_KEY,sizeof(buttons)),buttons)<0){::close(fd);continue;}
            auto has=[&](int key){return buttons[key/(8*sizeof(long))]&(1UL<<(key%(8*sizeof(long))));};
            if(!has(BTN_RIGHT)&&!has(KEY_LEFTCTRL)&&!has(KEY_F8)){::close(fd);continue;}
            auto notifier=new QSocketNotifier(fd,QSocketNotifier::Read,this);
            devices.emplace(path,Device{fd,notifier,{}});
            connect(notifier,&QSocketNotifier::activated,this,[this,path]{
                auto it=devices.find(path);if(it==devices.end())return;
                input_event event{};
                while(::read(it->second.fd,&event,sizeof(event))==sizeof(event)) {
                    if(event.type==EV_KEY && event.code>=BTN_LEFT && event.code<=BTN_EXTRA){if(event.value)it->second.down.insert(event.code);else it->second.down.remove(event.code);changed();}
                    if(event.type==EV_SYN && event.code==SYN_DROPPED){it->second.down.clear();changed();}
                }
            });
        }
        status->setText(devices.empty()?"Mouse access is not ready. Preview still works; input permission setup is pending.":QString("Ready · %1 input device(s)").arg(devices.size()));
    }
    bool physicalHeld(int code) {
        for(const auto &[name,d]:devices){unsigned long keys[(KEY_MAX+8*sizeof(long))/(8*sizeof(long))]{};if(ioctl(d.fd,EVIOCGKEY(sizeof(keys)),keys)>=0 && (keys[code/(8*sizeof(long))]&(1UL<<(code%(8*sizeof(long))))))return true;}return false;
    }
    void configureShortcuts() {
        marker.cancel();auto toggle=shortcut(hotkeyEdit->text()),manual=shortcut(manualEdit->text()),target=shortcut(targetEdit->text());
        hotkeysValid=toggle.valid()&&target.valid()&&(!manualEnabled->isChecked()||manual.valid());
        if(markerEnabled->isChecked()&&(toggle.key==target.key||(manualEnabled->isChecked()&&(manual.key==target.key||manual.key==toggle.key))))hotkeysValid=false;
        if(!hotkeysValid){marker.status="Invalid or conflicting shortcut base keys; markers disabled";return;}
        if(qApp->arguments().contains("--test-controls")){marker.status="Input disabled for interface test";return;}
        bool registered=KGlobalAccel::self()->setShortcut(toggleAction,{QKeySequence(hotkeyEdit->text())},KGlobalAccel::NoAutoloading);
        bool useManual=markerEnabled->isChecked()&&manualEnabled->isChecked();
        registered=KGlobalAccel::self()->setShortcut(manualAction,useManual?QList<QKeySequence>{QKeySequence(manualEdit->text())}:QList<QKeySequence>{},KGlobalAccel::NoAutoloading)&&registered;
        hotkeysValid=registered;marker.target=target;if(markerEnabled->isChecked()&&!markerOutput.openDevice()){marker.status="Marker input unavailable (/dev/uinput)";return;}
        marker.status=!registered?"Shortcut unavailable; choose another binding":markerEnabled->isChecked()?"Ready to forward recorder shortcuts":"Markers off";
    }
    void exportIni() {
        QFile file(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/soy-scope.ini");if(!file.open(QIODevice::WriteOnly|QIODevice::Truncate))return;QTextStream out(&file);
        out<<"image_path="<<(settings.value("image").toString().isEmpty()?QString("embedded://two-soyjaks-pointing"):settings.value("image").toString())<<"\nhotkey="<<hotkeyEdit->text()<<"\ndelay_ms="<<delay.interval()<<"\nwidth_px="<<overlay.width<<"\nopacity="<<overlay.alpha<<"\nanchor_x_pct="<<overlay.anchorX*100<<"\nanchor_y_pct="<<overlay.anchorY*100<<"\nenabled="<<(armed->isChecked()?1:0)<<"\n";
        for(auto box:{alternate,markerEnabled,everyAim,manualEnabled})out<<box->objectName()<<"="<<(box->isChecked()?1:0)<<"\n";
        out<<"mouse_button="<<buttonChoice->currentIndex()<<"\nmedal_marker_key="<<manualEdit->text()<<"\nmedal_target_key="<<targetEdit->text()<<"\n";
    }
    void terminalCommand(QString line) {
        auto words=line.trimmed().split(' ');QString cmd=words.takeFirst().toLower(),value=words.join(' ');if(cmd=="anchorx")cmd="anchorX";if(cmd=="anchory")cmd="anchorY";
        if(cmd=="quit")qApp->quit();
        else if(cmd=="arm")armed->setChecked(value!="0");
        else if(cmd=="show"){if(armed->isChecked()){overlay.place();overlay.show();}}
        else if(cmd=="hide"){overlay.hide();delay.stop();awaitRelease=anyDown();}
        else if(cmd=="save"){settings.sync();exportIni();}
        else if(cmd=="browse")findChild<QPushButton*>("browse-image")->click();
        else if(cmd=="reset-anchor"){ax->setValue(57.5);ay->setValue(37.5);}
        else if(cmd=="image"){QImage img(value=="embedded"?":/two-soyjaks-pointing.webp":value);if(!img.isNull()){overlay.image=img;overlay.rebuild();overlay.place();settings.setValue("image",value=="embedded"?"":value);}}
        else if(auto box=findChild<QCheckBox*>(cmd))box->setChecked(value=="1"||value=="true");
        else if(auto box=findChild<QDoubleSpinBox*>(cmd))box->setValue(value.toDouble());
        else if(cmd=="mouse_button")buttonChoice->setCurrentIndex(qBound(0,value.toInt(),3));
        else if(auto edit=findChild<QLineEdit*>(cmd)){edit->setText(value);settings.setValue(cmd,value);configureShortcuts();}
        else std::cout<<"Commands: arm 0/1, show, hide, image PATH/embedded, reset-anchor, save, quit; or SETTING VALUE. Settings: delay width opacity anchorX anchorY hotkey custom_mouse_button mouse_button medal_enabled medal_every_aim medal_separate_key medal_marker_key medal_target_key\n";
        std::cout<<"Armed="<<armed->isChecked()<<" | "<<marker.status.toStdString()<<"\n> "<<std::flush;
    }
    QJsonObject webState() const {
        QJsonObject state{{"arm",armed->isChecked()},{"mouse_button",buttonChoice->currentIndex()},{"status",marker.status}};
        for(auto box:findChildren<QDoubleSpinBox*>())state[box->objectName()]=box->value();
        for(auto box:findChildren<QCheckBox*>())state[box->objectName()]=box->isChecked();
        for(auto edit:{hotkeyEdit,manualEdit,targetEdit})state[edit->objectName()]=edit->text();
        return state;
    }

};
int main(int argc,char **argv) {
    if(argc==2 && QString::fromLocal8Bit(argv[1])=="--test-mask") {
        QCoreApplication app(argc,argv);
        QImage sample(4,1,QImage::Format_ARGB32);
        sample.setPixel(0,0,qRgba(255,255,255,255));sample.setPixel(1,0,qRgba(0,0,0,255));
        sample.setPixel(2,0,qRgba(128,128,128,255));sample.setPixel(3,0,qRgba(0,0,0,0));
        auto mask=inkMask(sample,.72);
        if(qAlpha(mask.pixel(0,0))!=0 || qAlpha(mask.pixel(1,0))!=184 || qAlpha(mask.pixel(2,0))<=0 || qAlpha(mask.pixel(2,0))>=184 || qAlpha(mask.pixel(3,0))!=0)return 1;
        QImage original(":/two-soyjaks-pointing.webp");if(original.isNull())return 2;
        auto real=inkMask(original,.72);qsizetype transparent=0,ink=0;
        for(int y=0;y<real.height();++y)for(int x=0;x<real.width();++x){if(qAlpha(real.pixel(x,y)))++ink;else ++transparent;}
        if(!ink || transparent<=ink)return 3;
        std::cout<<"PASS: white/transparent pixels disappear; black/grey line alpha preserved; embedded image background removed\n";
        return 0;
    }
    // XWayland supports precise desktop placement and a click-through input shape.
    bool testing=false;for(int i=1;i<argc;++i)if(QString::fromLocal8Bit(argv[i])=="--test-controls")testing=true;
    qputenv("QT_QPA_PLATFORM",testing?"offscreen":qEnvironmentVariableIsSet("WAYLAND_DISPLAY")?"wayland":"xcb");
    QApplication app(argc,argv);app.setOrganizationName("aridlin");app.setApplicationName("Soycope");
    app.setWindowIcon(QIcon(":/soyscope-icon.png"));
    QString ini=QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)+"/soy-scope.ini";
    int configArg=app.arguments().indexOf("--config");if(configArg>=0&&configArg+1<app.arguments().size())ini=app.arguments()[configArg+1];
    if(QFileInfo::exists(ini)){
        QSettings input(ini,QSettings::IniFormat),dest;
        const std::map<QString,QString> names={{"image_path","image"},{"hotkey","hotkey"},{"delay_ms","delay"},{"width_px","width"},{"anchor_x_pct","anchorX"},{"anchor_y_pct","anchorY"},{"enabled","armed"},{"custom_mouse_button","custom_mouse_button"},{"mouse_button","mouse_button"},{"medal_enabled","medal_enabled"},{"medal_every_aim","medal_every_aim"},{"medal_separate_key","medal_separate_key"},{"medal_marker_key","medal_marker_key"},{"medal_target_key","medal_target_key"}};
        for(const auto &[from,to]:names)if(input.contains(from))dest.setValue(to,input.value(from));
        if(input.contains("opacity"))dest.setValue("opacity",input.value("opacity").toDouble()*100);
        if(dest.value("image").toString().startsWith("embedded://"))dest.remove("image");
    }
    Settings window;
    WebSetup web;
    if(app.arguments().contains("--ft-web")||app.arguments().contains("--web")) {
        web.state=[&]{return window.webState();};web.command=[&](QString command){window.terminalCommand(command);};
        if(!web.listen(QHostAddress::LocalHost,0)){std::cerr<<"Cannot start local web setup\n";return 1;}
        if(testing)std::cout<<web.address().toStdString()<<std::endl;
        else QDesktopServices::openUrl(QUrl(web.address()));
        return app.exec();
    }
    if(app.arguments().contains("--ft-tui")) {
        window.terminalCommand("help");auto input=new QSocketNotifier(STDIN_FILENO,QSocketNotifier::Read,&app);
        auto pending=std::make_shared<QByteArray>();
        QObject::connect(input,&QSocketNotifier::activated,&app,[&,pending,input]{
            char bytes[4096];auto count=::read(STDIN_FILENO,bytes,sizeof(bytes));
            if(count<=0){if(count==0){input->setEnabled(false);if(!pending->isEmpty())window.terminalCommand(QString::fromUtf8(*pending));qApp->quit();}return;}
            pending->append(bytes,count);
            int newline;
            while((newline=pending->indexOf('\n'))>=0){auto line=pending->left(newline);pending->remove(0,newline+1);window.terminalCommand(QString::fromUtf8(line));}
        });
    } else window.show();return app.exec();
}
