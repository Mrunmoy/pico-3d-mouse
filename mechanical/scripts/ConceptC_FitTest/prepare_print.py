"""Rigidly rotate exported Fusion STL, then verify manifold edges and dimensions."""
import struct, json, collections
from pathlib import Path
import numpy as np

folder=Path(__file__).resolve().parents[2]/'output'/'fit-test'
source=folder/'ConceptC_GripTest_upright.stl'
data=bytearray(source.read_bytes())
count=struct.unpack_from('<I',data,80)[0]
assert len(data)==84+50*count
triangles=[]
for i in range(count):
    values=struct.unpack_from('<12f',data,84+50*i)
    triangles.append(np.array(values[3:]).reshape(3,3))
mesh=np.array(triangles,dtype=float)
before=np.array([mesh.min(axis=(0,1)),mesh.max(axis=(0,1))])
mesh[:,:,1]*=-1
mesh[:,:,2]=before[1,2]-mesh[:,:,2]
edges=collections.Counter()
volume=0.0
for i,tri in enumerate(mesh):
    normal=np.cross(tri[1]-tri[0],tri[2]-tri[0])
    length=np.linalg.norm(normal)
    assert length>1e-10, 'Degenerate triangle'
    normal/=length
    struct.pack_into('<12f',data,84+50*i,*normal,*tri.flatten())
    vs=[tuple(np.round(v,5)) for v in tri]
    for a,b in [(0,1),(1,2),(2,0)]: edges[tuple(sorted((vs[a],vs[b])))]+=1
    volume+=np.dot(tri[0],np.cross(tri[1],tri[2]))/6
assert all(n==2 for n in edges.values()), 'Non-manifold mesh'
after=np.array([mesh.min(axis=(0,1)),mesh.max(axis=(0,1))])
assert np.allclose(after[1]-after[0],before[1]-before[0])
assert np.all(after[1]-after[0]<180)
target=folder/'ConceptC_GripTest_print.stl'
target.write_bytes(data)
report={'triangles':count,'watertight_edges':True,'bounds_mm':after.tolist(),'extent_mm':(after[1]-after[0]).tolist(),'volume_mm3':volume,'orientation':'180 degrees about X; palm plateau on bed, hollow underside upward','output':str(target)}
(folder/'mesh_check.json').write_text(json.dumps(report,indent=2))
print(json.dumps(report,indent=2))
