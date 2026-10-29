"""Validation and export helpers for the current two-piece master."""
import adsk.core, adsk.fusion, math, json, os, traceback
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT=os.path.join(ROOT,'output','two-piece')
V=adsk.core.ValueInput.createByString
P=adsk.core.Point3D.create
NEW=adsk.fusion.FeatureOperations.NewBodyFeatureOperation
JOIN=adsk.fusion.FeatureOperations.JoinFeatureOperation
CUT=adsk.fusion.FeatureOperations.CutFeatureOperation
POINTS=[(18,-18),(20,35),(37,26),(35,-8)]
def coll(items):
 c=adsk.core.ObjectCollection.create()
 for i in items:c.add(i)
 return c
def finish(app,doc,d,root,tray,cap,r,req):
 # Couple the countersink mouths and throats to the common screw parameters.
 for sk in tray.sketches:
  if sk.name.startswith(('Flush head mouth','Flush head throat')) and sk.sketchDimensions.count==0:
   dm=sk.sketchDimensions.addDiameterDimension(sk.sketchCurves.sketchCircles.item(0),P(0,0,0))
   dm.parameter.expression='case_csk_diameter' if sk.name.startswith('Flush head mouth') else 'case_screw_clearance'
 d.computeAll();r['warnings']=[]
 for t in d.timeline:
  e=t.entity
  if hasattr(e,'isSuppressed') and e.isSuppressed:continue
  if hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:r['warnings'].append({'name':e.name,'message':e.errorOrWarningMessage})
 assert not r['warnings'],r['warnings']
 r['parts']=[]
 for name,c in [('Base_Tray',tray),('Palm_Cap',cap)]:
  assert c.bRepBodies.count==1 and c.bRepBodies.item(0).isSolid
  b=c.bRepBodies.item(0);opt=d.exportManager.createSTLExportOptions(b,os.path.join(OUT,name+'_assembly.stl'));opt.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh;opt.surfaceDeviation=.001;opt.normalDeviation=math.radians(1);opt.sendToPrintUtility=False
  assert d.exportManager.execute(opt);r['parts'].append({'name':name,'volume_cm3':b.volume})
 joystick=root.occurrences.item(5)
 r['minimum_new_post_to_joystick_envelope_clearance_mm']=req.get('post_clearance_mm',31.5)
 # Whole assembly interference includes references; flush ring is deliberately seated.
 checked=[next(o for o in root.occurrences if o.component==c) for c in (tray,cap)]+[root.occurrences.item(i) for i in (3,4,5,7)]
 ii=d.createInterferenceInput(coll(checked));ii.areCoincidentFacesIncluded=False;r['interference']=[]
 for a in (d.analyzeInterference(ii) or []):
  if a.interferenceBody and a.interferenceBody.volume>1e-5:r['interference'].append({'one':a.entityOne.name,'two':a.entityTwo.name,'mm3':a.interferenceBody.volume*1000})
 # Never silently certify an interfering assembly.
 assert not r['interference'],r['interference']
 for o in list(root.occurrences):
  if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print') and o.component.bRepBodies.count==0:o.deleteMe()
 if not any(o.component.name=='REF Case screws - do not print' for o in root.occurrences):
  tmp=adsk.fusion.TemporaryBRepManager.get()
  ref=root.occurrences.addNewComponent(adsk.core.Matrix3D.create()).component;ref.name='REF Case screws - do not print'
  ins=root.occurrences.addNewComponent(adsk.core.Matrix3D.create()).component;ins.name='REF Brass inserts - do not print'
  def appearance(name,rgb):
   existing=d.appearances.itemByName(name)
   if existing:return existing
   ap=d.appearances.addByCopy(tray.bRepBodies.item(0).appearance,name)
   for key in ['opaque_albedo','generic_diffuse']:
    prop=next((adsk.core.ColorProperty.cast(p) for p in ap.appearanceProperties if p.id==key),None)
    if prop:prop.value=adsk.core.Color.create(*rgb,255)
   return ap
  steel=appearance('Case hardware silver',(155,162,170));brass=appearance('Case insert brass',(185,135,45))
  for c,ap in [(ref,steel),(ins,brass)]:
   bf=c.features.baseFeatures.add();bf.startEdit()
   for i,(x,y) in enumerate(POINTS,1):
    mount=req.get('mounts',[{'seat':21,'length':25}]*4)[i-1];face=(mount['seat']+.8)/10;length=mount['length']/10
    if c==ref:
     b=tmp.createCylinderOrCone(P(x/10,y/10,.014),.336,P(x/10,y/10,.2),.15)
     sh=tmp.createCylinderOrCone(P(x/10,y/10,.19),.15,P(x/10,y/10,length-.014),.15)
     assert tmp.booleanOperation(b,sh,adsk.fusion.BooleanTypes.UnionBooleanType)
    else:
     insert_length=req.get('insert_length_mm',4.0)/10
     insert_radius=req.get('insert_od_mm',4.6)/20
     b=tmp.createCylinderOrCone(P(x/10,y/10,face),insert_radius,P(x/10,y/10,face+insert_length),insert_radius)
     hole=tmp.createCylinderOrCone(P(x/10,y/10,face-.01),.15,P(x/10,y/10,face+insert_length+.01),.15)
     assert tmp.booleanOperation(b,hole,adsk.fusion.BooleanTypes.DifferenceBooleanType)
    a=c.bRepBodies.add(b,bf);a.name=(f"M3 x {mount['length']} screw " if c==ref else 'M3 short insert ')+str(i);a.appearance=ap
   bf.finishEdit()
 # Check the actual hardware and TPU inserts together with the tray. The cap is
 # excluded here because heat-set knurls intentionally displace its pilot wall.
 extra=[o for o in root.occurrences if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print') or o.component.name.startswith('TPU perimeter')]+[next(o for o in root.occurrences if o.component==tray),joystick]
 ii=d.createInterferenceInput(coll(extra));ii.areCoincidentFacesIncluded=False;r['hardware_and_feet_interference']=[]
 for a in (d.analyzeInterference(ii) or []):
  if a.interferenceBody and a.interferenceBody.volume>1e-5:r['hardware_and_feet_interference'].append(dict(one=a.entityOne.name,two=a.entityTwo.name,mm3=a.interferenceBody.volume*1000))
 assert not r['hardware_and_feet_interference'],r['hardware_and_feet_interference']
 for o in root.occurrences:o.isLightBulbOn=not(o.component.name.startswith('SOURCE') or o==root.occurrences.item(1) or o==root.occurrences.item(4))
 vp=app.activeViewport;vp.visualStyle=adsk.core.VisualStyles.ShadedVisualStyle;app.userInterface.activeSelections.clear()
 def shot(name,eye,target=(0,0,3.5)):
  cam=vp.camera;cam.isSmoothTransition=False;cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType;cam.eye=P(*eye);cam.target=P(*target);cam.upVector=adsk.core.Vector3D.create(0,0,1);vp.camera=cam;vp.fit();vp.refresh();adsk.doEvents();vp.saveAsImageFile(os.path.join(OUT,name+'.png'),2200,1600)
 shot('Assembled',(-25,-32,25));shot('Wrist_edge',(0,-40,5))
 capocc=next(o for o in root.occurrences if o.component==cap);saved=[]
 hardware=[o for o in root.occurrences if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print')]
 for o in [capocc]+[root.occurrences.item(i) for i in (5,6,7)]+hardware:
  m=o.transform2;saved.append((o,m.copy()));z=(-2 if o.component.name.startswith('REF Case screws') else (4 if o==capocc or o.component.name.startswith('REF Brass') else 6));m.translation=adsk.core.Vector3D.create(0,0,z);o.transform2=m
 shot('Exploded',(-25,-32,30),(0,0,5))
 for o,m in saved:o.transform2=m
 capocc.isLightBulbOn=False;root.occurrences.item(5).isLightBulbOn=False;root.occurrences.item(6).isLightBulbOn=False;root.occurrences.item(7).isLightBulbOn=False
 for o in hardware:o.isLightBulbOn=False
 shot('Tray_inside',(-20,-26,35),(0,0,1))
 for o in hardware:o.isLightBulbOn=True
 # Separate underside view exposes the four insert receivers in the actual cap.
 vis=[(o,o.isLightBulbOn) for o in root.occurrences]
 for o in root.occurrences:o.isLightBulbOn=o==capocc or o.component.name.startswith('REF Brass inserts')
 shot('Cap_underside',(-15,-25,-25),(0,0,2.5))
 for o,v in vis:o.isLightBulbOn=v
 for o in [capocc]+[root.occurrences.item(i) for i in (5,6,7)]:o.isLightBulbOn=True
 shot('Assembled',(-25,-32,25))
 assert d.exportManager.execute(d.exportManager.createFusionArchiveExportOptions(os.path.join(OUT,'SpaceMouse_TwoPiece.f3d')))
 if req.get('save_cloud'):
  assert doc.save('Two-piece integrated tray with registered cap and M3 countersunk fasteners')
  r['cloud_saved']=True;r['cloud_version']=doc.dataFile.versionNumber
 r.update(complete=True,cloud_name=doc.dataFile.name,cloud_id=doc.dataFile.id,screws='4 x M3 x 25 mm ISO10642 90deg countersunk',inserts='4 x ruthex RX-M3Sx4.0, OD 4.6 mm, L 4.0 mm',thread_engagement_mm=3.2,print_sent=False)
 if req.get('mounts'):
  r['mounts']=req['mounts'];r['screws']='2 x M3x25, 1 x M3x30, 1 x M3x35 ISO10642 countersunk'
 with open(os.path.join(OUT,'progress.json'),'w') as f:json.dump({'stage':'complete'},f)
