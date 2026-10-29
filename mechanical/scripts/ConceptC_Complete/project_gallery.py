"""Export current native CAD orthographic views locally, without cloud access."""
import adsk.core, adsk.fusion, os, json, traceback
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT=os.path.join(ROOT,'docs','images')
def run(req):
 app=adsk.core.Application.get();os.makedirs(OUT,exist_ok=True);report={}
 try:
  doc=app.importManager.importToNewDocument(app.importManager.createFusionArchiveImportOptions(os.path.join(ROOT,'output','two-piece','SpaceMouse_TwoPiece.f3d')))
  doc.name='SpaceMouse mechanical - local gallery'
  d=adsk.fusion.Design.cast(app.activeProduct);root=d.rootComponent;vp=app.activeViewport
  for o in root.occurrences:
   o.isLightBulbOn=not(o.component.name.startswith('SOURCE') or o.component.name.startswith('OM300B_Envelope') or o.component.name.startswith('REF USB-C'))
  app.userInterface.activeSelections.clear();vp.visualStyle=adsk.core.VisualStyles.ShadedVisualStyle
  target=adsk.core.Point3D.create(0,0,4)
  views=[('front',(0,-45,4),(0,0,1)),('rear',(0,45,4),(0,0,1)),('left',(-45,0,4),(0,0,1)),('right',(45,0,4),(0,0,1)),('top',(0,0,50),(0,1,0)),('bottom',(0,0,-50),(0,-1,0)),('assembled',(-25,-32,25),(0,0,1))]
  for name,eye,up in views:
   cam=vp.camera;cam.isSmoothTransition=False;cam.cameraType=adsk.core.CameraTypes.OrthographicCameraType
   cam.eye=adsk.core.Point3D.create(*eye);cam.target=target;cam.upVector=adsk.core.Vector3D.create(*up)
   vp.camera=cam;vp.fit();vp.refresh();adsk.doEvents()
   assert vp.saveAsImageFile(os.path.join(OUT,name+'.png'),1800,1300)
  report=dict(complete=True,source='output/two-piece/SpaceMouse_TwoPiece.f3d',source_cloud_version=7,views=[v[0]+'.png' for v in views],geometry_changed=False)
 except Exception:report=dict(complete=False,error=traceback.format_exc())
 with open(os.path.join(OUT,'gallery_record.json'),'w') as f:json.dump(report,f,indent=2)
