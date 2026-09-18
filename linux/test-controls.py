import json,os,pathlib,subprocess,sys,tempfile,urllib.request,urllib.error
binary=sys.argv[1]
with tempfile.TemporaryDirectory(prefix='soy-controls-') as directory:
    env=dict(os.environ,XDG_CONFIG_HOME=directory,DBUS_SESSION_BUS_ADDRESS='unix:path=/nonexistent-soy-test-bus')
    result=subprocess.run([binary,'--test-controls','--ft-tui'],input='delay 59\nwidth 1158\nopacity 36.4785\nanchorX 57.1617\nanchorY 36.6997\nsave\nquit\n',text=True,capture_output=True,env=env,timeout=15)
    assert result.returncode==0,result.stderr
    ini=pathlib.Path(directory,'soy-scope.ini').read_text()
    for value in ['delay_ms=59','width_px=1158','opacity=0.364785','anchor_x_pct=57.1617','anchor_y_pct=36.6997']:assert value in ini,(value,ini)
    process=subprocess.Popen([binary,'--test-controls','--ft-web'],stdout=subprocess.PIPE,stderr=subprocess.DEVNULL,text=True,env=env)
    try:
        url=process.stdout.readline().strip();base,token=url.split('/#');base+='/'
        def request(path,command=None,auth=True,origin=True):
            headers={'X-Soy-Token':token} if auth else {}
            data=None
            if command is not None:
                data=json.dumps(command).encode();headers['Content-Type']='application/json'
                if origin:headers['Origin']=base.rstrip('/')
            try:
                with urllib.request.urlopen(urllib.request.Request(base+path,data=data,headers=headers),timeout=4) as r:return r.status,r.read()
            except urllib.error.HTTPError as e:return e.code,e.read()
        assert request('state',auth=False)[0]==403
        code,body=request('state');state=json.loads(body);assert code==200 and state['delay']==59 and state['width']==1158
        assert request('command',{'command':'width','value':'1234'},origin=False)[0]==403
        code,body=request('command',{'command':'width','value':'1234'});assert code==200 and json.loads(body)['width']==1234
        assert request('command',{'command':'unknown','value':''})[0]==400
        assert request('command',{'command':'save','value':''})[0]==200
        assert 'width_px=1234' in pathlib.Path(directory,'soy-scope.ini').read_text()
    finally:
        process.terminate();process.wait(timeout=5)
print('PASS: batched TUI commands, Windows-format persistence, web settings, token and origin protection')
