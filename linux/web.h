#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUuid>
#include <functional>

class WebSetup : public QTcpServer {
    QByteArray token=QUuid::createUuid().toByteArray(QUuid::WithoutBraces);
public:
    std::function<QJsonObject()> state;
    std::function<void(QString)> command;
    explicit WebSetup(QObject *parent=nullptr):QTcpServer(parent){
        connect(this,&QTcpServer::newConnection,this,[this]{
            while(hasPendingConnections()){
                auto socket=nextPendingConnection();auto buffer=std::make_shared<QByteArray>();
                connect(socket,&QTcpSocket::disconnected,socket,&QObject::deleteLater);
                QTimer::singleShot(5000,socket,[socket]{socket->disconnectFromHost();});
                connect(socket,&QTcpSocket::readyRead,socket,[this,socket,buffer]{
                    buffer->append(socket->readAll());
                    auto reply=[socket](int code,const QByteArray &body,const QByteArray &type="application/json"){
                        socket->write("HTTP/1.1 "+QByteArray::number(code)+" Response\r\nContent-Type: "+type+"\r\nContent-Length: "+QByteArray::number(body.size())+"\r\nCache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nContent-Security-Policy: default-src 'self'; script-src 'unsafe-inline'; style-src 'unsafe-inline'; frame-ancestors 'none'; object-src 'none'\r\nConnection: close\r\n\r\n"+body);
                        socket->disconnectFromHost();
                    };
                    if(buffer->size()>65536){reply(413,"{}");return;}
                    int end=buffer->indexOf("\r\n\r\n");if(end<0)return;
                    auto lines=buffer->left(end).split('\n');auto request=lines.takeFirst().trimmed().split(' ');
                    if(request.size()!=3){reply(400,"{}");return;}
                    QMap<QByteArray,QByteArray> headers;
                    for(auto line:lines){int colon=line.indexOf(':');if(colon>0)headers[line.left(colon).trimmed().toLower()]=line.mid(colon+1).trimmed();}
                    QByteArray host="127.0.0.1:"+QByteArray::number(serverPort());
                    if(headers.value("host")!=host){reply(403,"{}");return;}
                    if(request[0]=="GET"&&request[1]=="/"){reply(200,page(),"text/html; charset=utf-8");return;}
                    if(headers.value("x-soy-token")!=token){reply(403,"{}");return;}
                    if(request[0]=="GET"&&request[1]=="/state"){reply(200,QJsonDocument(state()).toJson(QJsonDocument::Compact));return;}
                    if(request[0]!="POST"||request[1]!="/command"){reply(405,"{}");return;}
                    if(headers.value("origin")!="http://"+host||headers.value("content-type")!="application/json"||headers.contains("transfer-encoding")){reply(403,"{}");return;}
                    bool ok=false;int length=headers.value("content-length").toInt(&ok);
                    if(!ok||length<0||length>8192){reply(400,"{}");return;}
                    if(buffer->size()<end+4+length)return;
                    QJsonParseError error;auto doc=QJsonDocument::fromJson(buffer->mid(end+4,length),&error);
                    if(error.error!=QJsonParseError::NoError||!doc.isObject()){reply(400,"{}");return;}
                    auto obj=doc.object();QString name=obj.value("command").toString(),value=obj.value("value").toString();
                    static const QStringList allowed={"arm","show","hide","save","reset-anchor","image","browse","delay","width","opacity","anchorX","anchorY","hotkey","custom_mouse_button","mouse_button","medal_enabled","medal_every_aim","medal_separate_key","medal_marker_key","medal_target_key"};
                    if(!allowed.contains(name)||value.contains('\n')||value.contains('\r')){reply(400,"{}");return;}
                    command(name+" "+value);reply(200,QJsonDocument(state()).toJson(QJsonDocument::Compact));
                });
            }
        });
    }
    QString address() const {return "http://127.0.0.1:"+QString::number(serverPort())+"/#"+QString::fromLatin1(token);}
    static QByteArray page(){return R"HTML(<!doctype html><meta charset="utf-8"><meta name="viewport" content="width=device-width"><title>Soy Scope</title>
<style>body{font:16px system-ui;max-width:650px;margin:40px auto;padding:20px;background:#24282d;color:#f2f3f5}label{display:flex;justify-content:space-between;gap:20px;margin:14px 0}input,select,button{font:inherit;padding:7px;border-radius:5px;border:1px solid #818993;background:#363c44;color:inherit}input[type=number]{width:110px}button{margin:4px;cursor:pointer}#status{color:#b8d6ee;white-space:pre-wrap}</style>
<h1>Soy Scope</h1><p>Settings apply to the overlay on your desktop. Hold the selected mouse button to show it.</p><div id="fields"></div>
<div><button data-c="show">Show if armed</button><button data-c="hide">Hide</button><button data-c="browse">Choose image…</button><button data-c="image" data-v="embedded">Original image</button><button data-c="reset-anchor">Reset anchor</button><button data-c="save">Save settings</button></div><p id="status"></p>
<script>
const token=location.hash.slice(1);history.replaceState(null,'','/');
const definitions=[['arm','Armed','checkbox'],['delay','Delay (ms)','number',5000],['width','Width (physical pixels)','number',5000],['opacity','Opacity (%)','number',100],['anchorX','Anchor X (%)','number',100],['anchorY','Anchor Y (%)','number',100],['hotkey','Toggle shortcut','text'],['custom_mouse_button','Use another mouse button','checkbox'],['mouse_button','Alternate button','select'],['medal_enabled','Enable recorder markers','checkbox'],['medal_every_aim','Mark every aim','checkbox'],['medal_separate_key','Use a separate marker key','checkbox'],['medal_marker_key','Manual marker shortcut','text'],['medal_target_key','Recorder shortcut','text']];
const controls={};
async function refresh(command,value=''){try{const response=await fetch(command?'/command':'/state',{method:command?'POST':'GET',headers:{'X-Soy-Token':token,...(command?{'Content-Type':'application/json'}:{})},body:command?JSON.stringify({command,value}):undefined});if(!response.ok)throw Error('Request failed ('+response.status+'). Reopen web setup if the app restarted.');const s=await response.json();for(const [name,input] of Object.entries(controls)){if(input.type==='checkbox')input.checked=!!s[name];else input.value=s[name];}document.querySelector('#status').textContent=s.status;}catch(e){document.querySelector('#status').textContent=e.message;}}
for(const [name,label,type,max] of definitions){let row=document.createElement('label');row.append(document.createTextNode(label));let input=document.createElement(type==='select'?'select':'input');if(type==='select'){['Left','Middle','Side X1','Side X2'].forEach((v,i)=>input.add(new Option(v,i)));}else{input.type=type;if(type==='number'){input.min=0;input.max=max;input.step='any';}}input.onchange=()=>refresh(name,type==='checkbox'?(input.checked?'1':'0'):input.value);controls[name]=input;row.append(input);document.querySelector('#fields').append(row);}
document.querySelectorAll('button').forEach(b=>b.onclick=()=>refresh(b.dataset.c,b.dataset.v||''));refresh();
</script>)HTML";}
};
