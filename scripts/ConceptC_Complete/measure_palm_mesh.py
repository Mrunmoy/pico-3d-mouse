"""Independent vertical-ray samples of the exported upper in assembly mm coordinates."""
import json, struct
from pathlib import Path
import numpy as np

folder=Path(__file__).resolve().parents[2]/'output'/'ergonomic'
data=(folder/'Upper_Shell_assembly.stl').read_bytes()
n=struct.unpack_from('<I',data,80)[0]
a=np.array([struct.unpack_from('<12f',data,84+50*i)[3:] for i in range(n)]).reshape(-1,3,3)
v0=a[:,0,:2];v1=a[:,1,:2]-v0;v2=a[:,2,:2]-v0
den=v1[:,0]*v2[:,1]-v2[:,0]*v1[:,1]
rows=[]
for x,y in [(29,-40),(29,-35),(29,-25),(29,-12),(29,0),(29,30),(-44.5,8),(-44.5,-20),(0,-35),(60,-35)]:
    q=np.array([x,y])-v0
    with np.errstate(divide='ignore',invalid='ignore'):
        u=(q[:,0]*v2[:,1]-v2[:,0]*q[:,1])/den
        v=(v1[:,0]*q[:,1]-q[:,0]*v1[:,1])/den
        mask=(u>=0)&(v>=0)&(u+v<=1)&(abs(den)>1e-10)
        z=a[:,0,2]+u*(a[:,1,2]-a[:,0,2])+v*(a[:,2,2]-a[:,0,2])
    zs=sorted(set(round(float(t),4) for t in z[mask]))
    rows.append(dict(x_mm=x,y_mm=y,z_intersections_mm=zs,
                     roof_vertical_mm=round(zs[-1]-zs[-2],3) if len(zs)>=2 else None))
assert rows[3]['roof_vertical_mm']>=3
assert rows[0]['z_intersections_mm'][-1]<25
(folder/'height_samples.json').write_text(json.dumps(rows,indent=2))
print('Palm heights and sampled cradle roof thickness verified')
