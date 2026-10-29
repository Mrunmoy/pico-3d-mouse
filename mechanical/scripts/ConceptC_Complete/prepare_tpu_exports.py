"""Orient curved foot exports and independently verify each closed STL."""
import json, struct, collections, math
from pathlib import Path
import numpy as np
folder=Path(__file__).resolve().parents[2]/'output'/'tpu-feet'
report=json.loads((folder/'validation_report.json').read_text())
assert report.get('complete'),report
checks=[]
for part in report['print_parts']:
    name=part['name'];data=bytearray((folder/(name+'_assembly.stl')).read_bytes())
    n=struct.unpack_from('<I',data,80)[0];assert len(data)==84+n*50
    values=np.array([struct.unpack_from('<12f',data,84+50*i) for i in range(n)])
    tri=values[:,3:].reshape(-1,3,3)
    if name=='Bottom_Cover':
        a=math.pi/4;tri[:,:,:2]=tri[:,:,:2]@np.array([[math.cos(a),-math.sin(a)],[math.sin(a),math.cos(a)]]).T
    low=tri.min(axis=(0,1));high=tri.max(axis=(0,1));extent=high-low
    tri[:,:,:2]-=(low[:2]+high[:2])/2;tri[:,:,2]-=low[2]
    assert max(extent)<176
    normals=np.cross(tri[:,1]-tri[:,0],tri[:,2]-tri[:,0]);areas=np.linalg.norm(normals,axis=1)
    assert areas.min()>1e-10;normals/=areas[:,None]
    edges=collections.Counter(tuple(sorted((tuple(a),tuple(b)))) for t in tri.round(5) for a,b in [(t[0],t[1]),(t[1],t[2]),(t[2],t[0])])
    assert all(v==2 for v in edges.values()),name
    volume=np.einsum('ij,ij->i',tri[:,0],np.cross(tri[:,1],tri[:,2])).sum()/6
    assert volume>0 and abs(volume/1000-part['volume_cm3'])/part['volume_cm3']<.01
    for i in range(n):struct.pack_into('<12f',data,84+i*50,*normals[i],*tri[i].flatten())
    target=folder/(name+'_print.stl');target.write_bytes(data)
    checks.append(dict(part=name,watertight=True,triangles=n,extent_mm=extent.tolist(),volume_mm3=volume,file=target.name))
(folder/'mesh_checks.json').write_text(json.dumps(checks,indent=2))
print(json.dumps(checks,indent=2))
