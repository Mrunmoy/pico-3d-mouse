"""Compare vertical surface samples of the former and continuous upper shells."""
import json, struct
from pathlib import Path
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
samples=[(-44.5,-42),(-44.5,-35),(-44.5,-28),(-20,-35),(0,-35),(29,-35),(29,-12),(60,-25)]
rows=[]
for revision in ('ergonomic','continuous'):
    data=(ROOT/'output'/revision/'Upper_Shell_assembly.stl').read_bytes()
    count=struct.unpack_from('<I',data,80)[0]
    dtype=np.dtype([('n','<f4',(3,)),('v','<f4',(3,3)),('attr','<u2')])
    a=np.frombuffer(data,dtype=dtype,offset=84,count=count)['v'].astype(float)
    v0=a[:,0,:2];v1=a[:,1,:2]-v0;v2=a[:,2,:2]-v0
    den=v1[:,0]*v2[:,1]-v2[:,0]*v1[:,1]
    for x,y in samples:
        q=np.array([x,y])-v0
        with np.errstate(divide='ignore',invalid='ignore'):
            u=(q[:,0]*v2[:,1]-v2[:,0]*q[:,1])/den
            v=(v1[:,0]*q[:,1]-q[:,0]*v1[:,1])/den
            mask=(u>=0)&(v>=0)&(u+v<=1)&(abs(den)>1e-10)
            z=a[:,0,2]+u*(a[:,1,2]-a[:,0,2])+v*(a[:,2,2]-a[:,0,2])
        hits=sorted(set(round(float(t),4) for t in z[mask]))
        rows.append(dict(revision=revision,x_mm=x,y_mm=y,intersections_mm=hits,
                         surface_mm=hits[-1] if hits else None,
                         vertical_roof_mm=round(hits[-1]-hits[-2],3) if len(hits)>=2 else None))
target=ROOT/'output'/'continuous'/'surface_comparison.json'
target.write_text(json.dumps(rows,indent=2))
print(json.dumps(rows,indent=2))
