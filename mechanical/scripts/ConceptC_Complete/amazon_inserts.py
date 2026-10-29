"""Update the single master for 3DZWMAN M3 x L4 x D4.2 heat-set inserts."""
import adsk.core, adsk.fusion, os, json, traceback, importlib.util, math
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT=os.path.join(ROOT,'output','two-piece')

def run(req):
 r=json.load(open(os.path.join(OUT,'report.json')));r.pop('error',None)
 try:
  app=adsk.core.Application.get();doc=app.activeDocument
  assert doc.dataFile.id==r['cloud_id']
  d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent;d.timeline.moveToEnd()
  assert d.attributes.itemByName('SpaceMouse','left_hand_corrected')
  tray=next(o.component for o in root.occurrences if o.component.name=='01 Integrated base tray')
  cap=next(o.component for o in root.occurrences if o.component.name=='02 Registered curved palm cap')
  before=tray.bRepBodies.item(0).volume
  ps=d.userParameters
  ps.itemByName('case_insert_bore').expression='3.8 mm'
  ps.itemByName('case_insert_bore').comment='Engineering starting pilot for 3DZWMAN M3 x L4 x D4.2; validate actual heat-set retention'
  ps.itemByName('case_insert_depth').expression='4.8 mm'
  d.computeAll()
  assert abs(tray.bRepBodies.item(0).volume-before)<1e-8
  # Check the physical, mirrored cap at all four pocket axes.
  b=cap.bRepBodies.item(0);checks=[]
  inside=adsk.fusion.PointContainment.PointInsidePointContainment
  for m in r['mounts']:
   x,y=m['x'],m['y'];face=m['seat']+.8
   for a in range(8):
    ang=a*math.pi/4
    assert b.pointContainment(adsk.core.Point3D.create((x+1.95*math.cos(ang))/10,(y+1.95*math.sin(ang))/10,(face+2)/10))==inside
   assert b.pointContainment(adsk.core.Point3D.create(x/10,y/10,(face+2)/10))!=inside
   assert b.pointContainment(adsk.core.Point3D.create(x/10,y/10,(face+4.85)/10))==inside
   checks.append(dict(x=x,y=y,pilot_mm=3.8,depth_mm=4.8,closed_end=True))
  for o in list(root.occurrences):
   if o.component.name in ('REF Case screws - do not print','REF Brass inserts - do not print'):o.deleteMe()
  spec=importlib.util.spec_from_file_location('two_piece',os.path.join(ROOT,'scripts','ConceptC_Complete','two_piece.py'))
  mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
  mod.POINTS=[(m['x'],m['y']) for m in r['mounts']]
  mod.finish(app,doc,d,root,tray,cap,r,dict(mounts=r['mounts'],post_clearance_mm=3,insert_length_mm=4,insert_od_mm=4.2,save_cloud=True))
  r.update(inserts='4 x 3DZWMAN M3 x L4 x D4.2, Amazon AU B0BBSLL6G7',insert_pilot_diameter_mm=3.8,insert_pilot_depth_mm=4.8,insert_checks=checks,printed_base_unchanged=True,physical_insert_retention='Not yet tested; 3.8 mm engineering starting pilot')
 except Exception:r['error']=traceback.format_exc();r['complete']=False
 with open(os.path.join(OUT,'report.json'),'w') as f:json.dump(r,f,indent=2)
