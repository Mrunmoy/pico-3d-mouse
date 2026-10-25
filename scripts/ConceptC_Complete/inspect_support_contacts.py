"""Map sliced support-interface extrusion back into assembly coordinates."""
from pathlib import Path
import numpy as np,re,json,math
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'output'/'two-piece'
raw=(OUT/'Palm_Cap_assembly.stl').read_bytes()
t=np.frombuffer(raw,dtype=np.dtype([('n','<f4',3),('v','<f4',(3,3)),('a','u2')]),offset=84)['v'].astype(float)
n=np.array([.14,-.04,1]);n/=np.linalg.norm(n);ex=np.array([1,0,-.14]);ex/=np.linalg.norm(ex);ey=np.cross(n,ex)
a=math.pi/4;A=np.array([[math.cos(a),-math.sin(a),0],[math.sin(a),math.cos(a),0],[0,0,1]])@np.array([ex,ey,n])
tt=t@A.T;lo=tt.min((0,1));hi=tt.max((0,1));off=np.r_[(lo[:2]+hi[:2])/2,lo[2]]
role='';xyz=np.zeros(3);pts=[]
for line in (OUT/'slicer-audit'/'Palm_Cap_local_support'/'plate_1.gcode').open():
 if line.startswith('; FEATURE:'):role=line.split(':',1)[1].strip()
 if line.startswith(('G0 ','G1 ','G2 ','G3 ')):
  vals={k:float(v) for k,v in re.findall(r'([XYZE])(-?(?:\d*\.)?\d+)',line.split(';')[0])}
  for i,k in enumerate('XYZ'):
   if k in vals:xyz[i]=vals[k]
  if role=='Support interface' and vals.get('E',0)>0 and ('X' in vals or 'Y' in vals):pts.append((xyz-np.array([90,90,0])+off)@A)
pts=np.array(pts);joy=np.linalg.norm(pts[:,:2]-[-44.5,8],axis=1)
mounts=np.array([[-28,-26],[-23.5,41],[37,30],[37,-10]])
dist=np.linalg.norm(pts[:,:2,None]-mounts.T[None,:,:],axis=1).min(1)
inside_joystick=(pts[:,0]>=-71)&(pts[:,0]<=-18)&(pts[:,1]>=-18.5)&(pts[:,1]<=34.5)&(pts[:,2]<=34.6)
unclassified=~inside_joystick&(dist>6)
r=dict(interface_points=len(pts),assembly_bounds_mm=[pts.min(0).tolist(),pts.max(0).tolist()],joystick_cavity_points=int(inside_joystick.sum()),fixing_pocket_points=int((~inside_joystick&(dist<=6)).sum()),unclassified_points=int(unclassified.sum()),unclassified_bounds_mm=[pts[unclassified].min(0).tolist(),pts[unclassified].max(0).tolist()] if unclassified.any() else None)
(OUT/'support_contacts.json').write_text(json.dumps(r,indent=2));print(json.dumps(r,indent=2))
