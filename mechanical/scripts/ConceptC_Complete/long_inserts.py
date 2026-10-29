"""Adapt only the corrected palm cap to user-owned M3 x 5.7 x 4.6 inserts."""
import adsk.core,adsk.fusion,os,json,traceback,importlib.util,math
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'));OUT=os.path.join(ROOT,'output','two-piece')
def run(req):
 r=json.load(open(os.path.join(OUT,'report.json')));r.pop('error',None)
 try:
  app=adsk.core.Application.get();doc=app.activeDocument
  assert doc.dataFile.id==r['cloud_id']
  d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent;d.timeline.moveToEnd()
  assert d.attributes.itemByName('SpaceMouse','left_hand_corrected')
  tray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray');cap=next(o.component for o in root.occurrences if o.component.name=='02 Registered curved palm cap')
  if req.get('restore'):
   d.userParameters.itemByName('case_insert_depth').expression='4.8 mm';d.computeAll()
   r.pop('error',None);r.pop('roof_samples',None);r['complete']=True
   r['long_insert_assessment']='5.7 mm inserts break through the low palm-side exterior; retain 4 mm inserts unless cap exterior is thickened'
   assert doc.save('Restore safe short-insert pockets after checking long insert clearance')
   assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_TwoPiece.f3d')))
   r['cloud_version']=doc.dataFile.versionNumber
   with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
   return
  before=tray.bRepBodies.item(0).volume
  d.userParameters.itemByName('case_insert_depth').expression='6.0 mm'
  d.computeAll();body=cap.bRepBodies.item(0)
  roofs=[]
  for m in r['mounts']:
   tip=m['seat']+.8+6.0;minimum=100
   for i in range(17):
    a=2*math.pi*i/16;rad=0 if i==16 else 1.9
    x=m['x']+rad*math.cos(a);y=m['y']+rad*math.sin(a)
    assert body.pointContainment(adsk.core.Point3D.create(x/10,y/10,(tip+.02)/10))==adsk.fusion.PointContainment.PointInsidePointContainment, ('Pilot containment',m,x,y,tip,[(round(tip+k*.1,2),body.pointContainment(adsk.core.Point3D.create(x/10,y/10,(tip+k*.1)/10))) for k in range(-5,31)])
    for step in range(1,301):
     height=.02+step*.05
     if body.pointContainment(adsk.core.Point3D.create(x/10,y/10,(tip+height)/10))!=adsk.fusion.PointContainment.PointInsidePointContainment:
      minimum=min(minimum,height-.05);break

   roofs.append(dict(x=m['x'],y=m['y'],minimum_sampled_vertical_roof_mm=round(minimum,2)))
  r['roof_samples']=roofs
  assert min(v['minimum_sampled_vertical_roof_mm'] for v in roofs)>=1.8,roofs
  assert abs(tray.bRepBodies.item(0).volume-before)<1e-8,'Printed tray must remain unchanged'
  for o in list(root.occurrences):
   if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print'):o.deleteMe()
  spec=importlib.util.spec_from_file_location('two_piece',os.path.join(ROOT,'scripts','ConceptC_Complete','two_piece.py'));mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
  mod.POINTS=[(m['x'],m['y']) for m in r['mounts']]
  mod.finish(app,doc,d,root,tray,cap,r,dict(mounts=r['mounts'],post_clearance_mm=3,insert_length_mm=5.7,save_cloud=True))
  r.update(inserts='4 x user-owned M3, OD 4.6 mm, L 5.7 mm',insert_pilot_depth_mm=6.0,insert_end_clearance_mm=.3,roof_samples=roofs,printed_base_unchanged=True)
 except Exception:r['error']=traceback.format_exc();r['complete']=False
 with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
