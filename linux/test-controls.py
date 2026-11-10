import json,os,pathlib,subprocess,sys,tempfile,urllib.request,urllib.error
binary=sys.argv[1]
with tempfile.TemporaryDirectory(prefix='soy-controls-') as directory:
    env=dict(os.environ,XDG_CONFIG_HOME=directory,DBUS_SESSION_BUS_ADDRESS='unix:path=/nonexistent-soy-test-bus')
    preview=subprocess.run([binary,'--test-controls','--test-preview'],capture_output=True,text=True,env=env,timeout=15)
    assert preview.returncode==0,(preview.returncode,preview.stdout,preview.stderr)
    result=subprocess.run([binary,'--test-controls','--ft-tui'],input='toggle_trigger 1\ndelay 59\nwidth 1158\nopacity 36.4785\nanchorX 57.1617\nanchorY 36.6997\nsave\nquit\n',text=True,capture_output=True,env=env,timeout=15)
    assert result.returncode==0,(result.returncode,result.stdout,result.stderr)
    ini=pathlib.Path(directory,'soy-scope.ini').read_text()
    for value in ['toggle_trigger=1','delay_ms=59','width_px=1158','opacity=0.364785','anchor_x_pct=57.1617','anchor_y_pct=36.6997']:assert value in ini,(value,ini)
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
        code,body=request('state');state=json.loads(body);assert code==200 and state['delay']==59 and state['width']==1158 and state['toggle_trigger'] is True
        assert request('command',{'command':'width','value':'1234'},origin=False)[0]==403
        code,body=request('command',{'command':'width','value':'1234'});assert code==200 and json.loads(body)['width']==1234
        code,body=request('command',{'command':'toggle_trigger','value':'0'});assert code==200 and json.loads(body)['toggle_trigger'] is False
        for command,value,expected in [('arm','1',False),('preview','1',True),('show','',False),('show','',True),('hide','',False),('preview','1',True),('arm','0',False),('show','',False)]:
            code,body=request('command',{'command':command,'value':value})
            assert code==200 and json.loads(body)['preview']==expected,(command,body)
        assert request('command',{'command':'unknown','value':''})[0]==400
        assert request('command',{'command':'save','value':''})[0]==200
        saved=pathlib.Path(directory,'soy-scope.ini').read_text()
        assert 'width_px=1234' in saved and 'toggle_trigger=0' in saved
    finally:
        process.terminate();process.wait(timeout=5)
print('PASS: batched TUI commands, Windows-format persistence, web settings, token and origin protection')
