"""Four corner-region fasteners, clear of joystick and TPU feet."""
import adsk.core,adsk.fusion,os,json,math,traceback,importlib.util
ROOT=r'D:\github\fusion360\space-mouse';OUT=os.path.join(ROOT,'output','two-piece')
V=adsk.core.ValueInput.createByString;P=adsk.core.Point3D.create
NEW=adsk.fusion.FeatureOperations.NewBodyFeatureOperation;JOIN=adsk.fusion.FeatureOperations.JoinFeatureOperation;CUT=adsk.fusion.FeatureOperations.CutFeatureOperation
MOUNTS=[dict(x=-28,y=-26,seat=26,length=30),dict(x=-23.5,y=41,seat=31,length=35),dict(x=37,y=30,seat=21,length=25),dict(x=37,y=-10,seat=21,length=25)]
def coll(items):
 c=adsk.core.ObjectCollection.create()
 for x in items:c.add(x)
 return c
def relieve_feet(d,root,tray):
 base=root.occurrences.item(2).component
 for s in list(tray.sketches):
  if s.name.startswith('Retain TPU pocket clearance') and s.sketchCurves.count==0:s.deleteMe()
 for i in (1,4):
  src=next(s for s in base.sketches if s.name.startswith(f'TPU perimeter pocket {i} -'))
  sk=tray.sketches.add(tray.xYConstructionPlane);sk.name=f'Retain TPU pocket clearance near new fixing {i}'
  for curve in src.sketchCurves:sk.project(curve.createForAssemblyContext(root.occurrences.item(2)))
  ei=tray.features.extrudeFeatures.createInput(coll(list(sk.profiles)),CUT);ei.setDistanceExtent(False,V('tpu_pocket_depth'));ei.participantBodies=[tray.bRepBodies.item(0)]
  tray.features.extrudeFeatures.add(ei).name=f'Restore TPU pocket {i} through screw-column edge';sk.isVisible=False
def run(req):
 r={};stage='start'
 def status(s):
  with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':s},f)
 try:
  app=adsk.core.Application.get();doc=app.activeDocument
  assert doc.dataFile.id=='urn:adsk.wipprod:dm.lineage:Oa1GpTwQR8iVvi6D4SRQIg'
  d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent;d.timeline.moveToEnd();tmp=adsk.fusion.TemporaryBRepManager.get();ps=d.userParameters
  spec=importlib.util.spec_from_file_location('two_piece',os.path.join(ROOT,'scripts','ConceptC_Complete','two_piece.py'));module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
  module.POINTS=[(m['x'],m['y']) for m in MOUNTS]
  req.update(mounts=MOUNTS,post_clearance_mm=3.0)
  if req.get('finish'):
   tray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray');cap=next(o.component for o in root.occurrences if o.component.name=='02 Registered curved palm cap')
   if req.get('adjust_rear'):
    for c in (tray,cap):
     for sk in c.sketches:
      for dm in sk.sketchDimensions:
       if dm.parameter.expression=='43 mm':dm.parameter.expression='40.5 mm'
    for o in list(root.occurrences):
     if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print'):o.deleteMe()
   if req.get('foot_relief'):relieve_feet(d,root,tray)
   if req.get('adjust_rear_final'):
    for c in (tray,cap):
     for sk in c.sketches:
      if sk.name.endswith('2 sketch'):
       for dm in sk.sketchDimensions:
        if dm.parameter.expression=='30 mm':dm.parameter.expression='23.5 mm'
        elif dm.parameter.expression=='40.5 mm':dm.parameter.expression='41 mm'
    for o in list(root.occurrences):
     if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print'):o.deleteMe()
   module.finish(app,doc,d,root,tray,cap,r,req)
  else:
   assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'Before_distribution.f3d')))
   source=root.occurrences.item(0).component;base=root.occurrences.item(2).component
   oldtray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray');oldcap=next(o.component for o in root.occurrences if o.component.name=='02 Registered curved palm cap')
   appearance=oldtray.bRepBodies.item(0).appearance
   stage='derive clean cap without old fastening pockets';status(stage)
   first=min((f for f in oldcap.features.extrudeFeatures if f.name.startswith('Flat hard-stop shoulder 1')),key=lambda f:f.timelineObject.index)
   d.timeline.markerPosition=first.timelineObject.index;capcopy=tmp.copy(oldcap.bRepBodies.item(0));d.timeline.moveToEnd()
   first=max((f for f in source.features.extrudeFeatures if f.name.startswith('R3 cover column 0')),key=lambda f:f.timelineObject.index)
   d.timeline.markerPosition=first.timelineObject.index;wall=tmp.copy(min(list(source.bRepBodies),key=lambda b:b.volume));d.timeline.moveToEnd()
   mx=adsk.core.Matrix3D.create();mx.setCell(0,0,-1);tmp.transform(wall,mx)
   oldtray.name='SOURCE - clustered tray before distribution';oldcap.name='SOURCE - clustered cap before distribution'
   for o in root.occurrences:
    if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print'):o.component.name='SOURCE - old '+o.component.name
   tray=root.occurrences.addNewComponent(adsk.core.Matrix3D.create()).component;tray.name='01 Integrated base tray'
   cap=root.occurrences.addNewComponent(adsk.core.Matrix3D.create()).component;cap.name='02 Registered curved palm cap'
   for c,b in [(tray,wall),(cap,capcopy)]:
    bf=c.features.baseFeatures.add();bf.startEdit();c.bRepBodies.add(b,bf);bf.finishEdit()
   def sk(c,z,name):
    pi=c.constructionPlanes.createInput();pi.setByOffset(c.xYConstructionPlane,V(z));pl=c.constructionPlanes.add(pi);pl.isLightBulbOn=False
    s=c.sketches.add(pl);s.name=name;return s
   def ext(c,s,depth,op,name):
    ei=c.features.extrudeFeatures.createInput(coll(list(s.profiles)),op);ei.setDistanceExtent(False,V(depth));ei.participantBodies=[c.bRepBodies.item(0)]
    f=c.features.extrudeFeatures.add(ei);f.name=name;s.isVisible=False
   def circle(c,z,x,y,diam,name):
    s=sk(c,z,name+' sketch');ci=s.sketchCurves.sketchCircles.addByCenterRadius(P(x/10,y/10,0),d.unitsManager.evaluateExpression(diam,'mm')/2)
    s.sketchDimensions.addDiameterDimension(ci,P(x/10+.6,y/10,0)).parameter.expression=diam
    for ori,v in [(adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation,x),(adsk.fusion.DimensionOrientations.VerticalDimensionOrientation,y)]:s.sketchDimensions.addDistanceDimension(s.originPoint,ci.centerSketchPoint,ori,P(x/10+.6,y/10+.6,0)).parameter.expression=f'{abs(v)} mm'
    return s
   def disk(c,z,x,y,diam,depth,op,name):ext(c,circle(c,z,x,y,diam,name),depth,op,name)
   stage='join integrated tray';status(stage)
   s=tray.sketches.add(tray.xYConstructionPlane);s.is3D=True;pts=[s.sketchPoints.add(P(x,y,2.2-.14*x+.04*y)) for x,y in [(0,0),(10,0),(0,10)]]
   pi=tray.constructionPlanes.createInput();pi.setByThreePoints(*pts);pl=tray.constructionPlanes.add(pi);pl.isLightBulbOn=False;s.isVisible=False
   si=tray.features.splitBodyFeatures.createInput(tray.bRepBodies.item(0),pl,True);tray.features.splitBodyFeatures.add(si)
   for b in list(tray.bRepBodies):
    pt=b.physicalProperties.centerOfMass
    if pt.z+.14*pt.x-.04*pt.y>2.2:tray.features.removeFeatures.add(b)
   bf=tray.features.baseFeatures.add();bf.startEdit();tray.bRepBodies.add(tmp.copy(base.bRepBodies.item(0)),bf);bf.finishEdit()
   bs=list(tray.bRepBodies);target=max(bs,key=lambda b:b.volume);ci=tray.features.combineFeatures.createInput(target,coll([b for b in bs if b!=target]));ci.operation=JOIN;tray.features.combineFeatures.add(ci).name='Integrated tray - no separate floor'
   for i,(x,y) in enumerate([(-74,-12),(22,39),(60,28),(66,-10)]):disk(tray,'0 mm',x,y,'8 mm','3 mm',JOIN,'Close obsolete floor hole '+str(i))
   def val(n):return ps.itemByName(n).value
   s=sk(tray,'usb_z-usb_height/2','Rear USB opening');y=val('pcb_y')+val('pcb_length')/2
   s.sketchCurves.sketchLines.addTwoPointRectangle(P(-val('usb_width')/2,y-.6,0),P(val('usb_width')/2,val('body_depth')/2+1,0));ext(tray,s,'usb_height',CUT,'Rear USB access')
   s=sk(tray,'cover_thickness+lip_height','Rear PCB clearance');s.sketchCurves.sketchLines.addTwoPointRectangle(P(-1.35,y-.7,0),P(1.35,y+.07,0));ext(tray,s,'pcb_z+pcb_thickness+0.7 mm-(cover_thickness+lip_height)',CUT,'Rear PCB edge clearance')
   for i,m in enumerate(MOUNTS,1):
    stage=f'build distributed fixing {i}';status(stage);x=m['x'];y=m['y'];seat=f'dist_seat_{i}';face=f'dist_insert_face_{i}'
    for n,e in [(seat,f"{m['seat']} mm"),(face,seat+'+0.8 mm'),(f'dist_screw_{i}',f"{m['length']} mm")]:ps.add(n,V(e),'mm','Distributed fixing '+str(i))
    disk(tray,'0 mm',x,y,'case_post_diameter',seat,JOIN,f'Distributed column {i}')
    disk(tray,seat,x,y,'case_locator_diameter','case_locator_height',JOIN,f'Distributed locator {i}')
    disk(tray,'0 mm',x,y,'case_screw_clearance',seat+'+1 mm',CUT,f'Distributed M3 clearance {i}')
    low=circle(tray,'0 mm',x,y,'case_csk_diameter',f'CSK mouth {i}');high=circle(tray,'(case_csk_diameter-case_screw_clearance)/2',x,y,'case_screw_clearance',f'CSK throat {i}')
    li=tray.features.loftFeatures.createInput(CUT);li.loftSections.add(low.profiles.item(0));li.loftSections.add(high.profiles.item(0));li.participantBodies=[tray.bRepBodies.item(0)];tray.features.loftFeatures.add(li).name=f'Distributed 90 degree countersink {i}';low.isVisible=False;high.isVisible=False
    disk(cap,'0 mm',x,y,'case_post_diameter+0.6 mm',seat,CUT,f'Distributed hard stop {i}')
    disk(cap,seat,x,y,'case_locator_diameter+0.6 mm','0.8 mm',CUT,f'Distributed receiver {i}')
    disk(cap,face,x,y,'case_insert_bore','case_insert_depth',CUT,f'Distributed insert pilot {i}')
   for c in (tray,cap):c.bRepBodies.item(0).appearance=appearance
   relieve_feet(d,root,tray)
   stage='validate distributed assembly';status(stage);module.finish(app,doc,d,root,tray,cap,r,req)
  r['distribution']='Two joystick-side fixings, two palm-side fixings; front and rear widely separated'
 except Exception:r.update(error=traceback.format_exc(),stage=stage);status('failed')
 with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
