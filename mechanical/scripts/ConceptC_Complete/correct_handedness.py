"""Reflect the complete current assembly into the specified left-hand layout."""
import adsk.core,adsk.fusion,os,json,traceback,importlib.util
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'));OUT=os.path.join(ROOT,'output','two-piece')
def run(req):
 r={}
 try:
  app=adsk.core.Application.get();doc=app.activeDocument
  assert doc.dataFile.id=='urn:adsk.wipprod:dm.lineage:Oa1GpTwQR8iVvi6D4SRQIg'
  d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent;d.timeline.moveToEnd()
  assert not d.attributes.itemByName('SpaceMouse','left_hand_corrected'),'Already corrected'
  assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'Before_handedness_fix.f3d')))
  selected=[o for o in root.occurrences if o.isLightBulbOn and not o.component.name.startswith('SOURCE')]
  r['mirrored_components']=[]
  for o in ([] if req.get("resume") else selected):
   c=o.component;originals=list(c.bRepBodies)
   if not originals:continue
   col=adsk.core.ObjectCollection.create()
   for b in originals:col.add(b)
   mi=c.features.mirrorFeatures.createInput(col,c.yZConstructionPlane)
   f=c.features.mirrorFeatures.add(mi);f.name='Left-hand correction - reflect across centre YZ'
   for b in originals:c.features.removeFeatures.add(b).name='Hide replaced right-hand body'
   r['mirrored_components'].append(c.name)
  d.computeAll()
  joystick=root.occurrences.item(5);bb=joystick.component.bRepBodies.item(0).boundingBox
  r["joystick_bodies"]=[dict(name=b.name,min=[b.boundingBox.minPoint.x,b.boundingBox.minPoint.y,b.boundingBox.minPoint.z],max=[b.boundingBox.maxPoint.x,b.boundingBox.maxPoint.y,b.boundingBox.maxPoint.z]) for b in joystick.component.bRepBodies]
  x=(bb.minPoint.x+bb.maxPoint.x)*5
  assert x>0,('Joystick must be RIGHT from wrist edge',x)
  r.update(handedness='LEFT hand: wrist at -Y, USB at +Y, joystick at +X',joystick_centre_x_mm=x,joint_normal=[-.14,-.04,1])
  path=os.path.join(ROOT,'scripts','ConceptC_Complete','two_piece.py')
  spec=importlib.util.spec_from_file_location('two_piece',path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
  mounts=[dict(x=28,y=-26,seat=26,length=30),dict(x=23.5,y=41,seat=31,length=35),dict(x=-37,y=30,seat=21,length=25),dict(x=-37,y=-10,seat=21,length=25)]
  m.POINTS=[(p['x'],p['y']) for p in mounts]
  tray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray');cap=next(o.component for o in root.occurrences if o.component.name=='02 Registered curved palm cap')
  m.finish(app,doc,d,root,tray,cap,r,dict(mounts=mounts,post_clearance_mm=3,save_cloud=False))
  vp=app.activeViewport;cam=vp.camera;cam.isSmoothTransition=False;cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType
  cam.eye=adsk.core.Point3D.create(0,0,50);cam.target=adsk.core.Point3D.create(0,0,0);cam.upVector=adsk.core.Vector3D.create(0,1,0);vp.camera=cam;vp.fit();vp.refresh();adsk.doEvents()
  assert vp.saveAsImageFile(os.path.join(OUT,'Left_hand_top.png'),2200,1600)
  d.attributes.add('SpaceMouse','left_hand_corrected','Joystick +X; wrist -Y; USB +Y')
  assert doc.save('Correct left-hand layout: joystick RIGHT from wrist entry; complete assembly mirrored')
  assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_TwoPiece.f3d')))
  r.update(cloud_saved=True,cloud_version=doc.dataFile.versionNumber)
 except Exception:r['error']=traceback.format_exc();r['complete']=False
 with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
