"""Export honest shaded CAD reference views; do not modify or save geometry."""
import adsk.core, adsk.fusion, os, json, traceback, time
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT=os.path.join(ROOT,'output','NanoBanana_References')
P=adsk.core.Point3D.create

def run(context):
    global OUT
    settings=context if isinstance(context,dict) else {}
    if settings.get('output'):OUT=os.path.join(ROOT,'output',settings['output'])
    app=adsk.core.Application.get();os.makedirs(OUT,exist_ok=True)
    vp=app.activeViewport;oldcam=vp.camera;style=vp.visualStyle;saved=[];selectable=[]
    report={'images':[]}
    try:
        doc=app.activeDocument
        expected=json.load(open(os.path.join(ROOT,'output','revision2','master_save_record.json')))['data_file_id']
        assert (doc.dataFile and doc.dataFile.id==expected) or (settings.get('allow_trial') and doc.name.startswith('Ergonomic trial'))
        d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent
        for occ in root.occurrences:
            for body in occ.component.bRepBodies:
                selectable.append((body,body.isSelectable));body.isSelectable=False
        for occ in root.occurrences:saved.append((occ,occ.isLightBulbOn))
        for n in (0,2,3,4,5,6,7):root.occurrences.item(n).isLightBulbOn=True
        root.occurrences.item(1).isLightBulbOn=False
        vp.visualStyle=adsk.core.VisualStyles.ShadedVisualStyle
        # Allow the operator to move the pointer off geometry after Run closes.
        deadline=time.time()+15
        while time.time()<deadline:
            adsk.doEvents();time.sleep(.05)
        views=[
            ('01_Front_wrist_edge',(0,-40,3.5),(0,0,1)),
            ('02_Rear_USB_edge',(0,40,3.5),(0,0,1)),
            ('03_Left_joystick_end',(-40,0,3.5),(0,0,1)),
            ('04_Right_palm_end',(40,0,3.5),(0,0,1)),
            ('05_Top',(0,0,40),(0,1,0)),
            ('06_Bottom',(0,0,-40),(0,-1,0)),
            ('07_Front_left',(-25,-32,25),(0,0,1)),
            ('08_Front_right',(25,-32,25),(0,0,1)),
            ('09_Rear_right',(25,32,25),(0,0,1)),
        ]
        def shot(name,eye,up):
            app.userInterface.activeSelections.clear()
            cam=vp.camera;cam.isSmoothTransition=False;cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType
            cam.eye=P(*eye);cam.target=P(0,0,3.5);cam.upVector=adsk.core.Vector3D.create(*up)
            vp.camera=cam;vp.fit();vp.refresh();adsk.doEvents()
            op=adsk.core.SaveImageFileOptions.create(os.path.join(OUT,name+'.png'))
            op.width=2400;op.height=1800;op.isBackgroundTransparent=True
            assert vp.saveAsImageFileWithOptions(op)
            report['images'].append(name+'.png')
        for name,eye,up in views:shot(name,eye,up)
        for n in (3,4,5,6,7):root.occurrences.item(n).isLightBulbOn=False
        for n in (0,2,4,6,7):
            name,eye,up=views[n];shot('Body_'+name,eye,up)
        report.update(complete=True,source_file=doc.dataFile.name if doc.dataFile else doc.name,source_id=doc.dataFile.id if doc.dataFile else None,
                      note=settings.get('note','Current rejected dome shape, unmodified. Shaded CAD viewport images, not photorealistic renders. Joystick/boot are simplified engineering proxies.'))
    except Exception:report['error']=traceback.format_exc()
    finally:
        for body,on in selectable:body.isSelectable=on
        for occ,on in saved:occ.isLightBulbOn=on
        vp.visualStyle=style;vp.camera=oldcam;vp.refresh()
        with open(os.path.join(OUT,'export_record.json'),'w') as f:json.dump(report,f,indent=2)
