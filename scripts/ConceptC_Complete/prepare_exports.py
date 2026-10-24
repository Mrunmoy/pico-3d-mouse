"""Rigid print orientation and independent manifold checks for Fusion exports."""
import json,struct,collections,math,zipfile,sys
from pathlib import Path
import numpy as np

folder=Path(sys.argv[1]).resolve() if len(sys.argv)>1 else Path(__file__).resolve().parents[2]/'output'/'complete'
report=json.loads((folder/'validation_report.json').read_text())
results=[]
for part in report['print_parts']:
    name=part['name']
    data=bytearray((folder/(name+'_assembly.stl')).read_bytes())
    n=struct.unpack_from('<I',data,80)[0]
    assert len(data)==84+50*n
    values=np.array([struct.unpack_from('<12f',data,84+50*i) for i in range(n)])
    tri=values[:,3:].reshape(-1,3,3)
    # Upper prints open rim down, palm up for exterior quality. Retainer bridge down.
    if name == 'PCB_Retainer':
        tri[:,:,1:]*=-1
    if name in ('Upper_Shell','Bottom_Cover'):
        a=math.pi/4;rot=np.array([[math.cos(a),-math.sin(a)],[math.sin(a),math.cos(a)]])
        tri[:,:,:2]=tri[:,:,:2]@rot.T
    low=tri.min(axis=(0,1));high=tri.max(axis=(0,1))
    tri[:,:,:2]-=(low[:2]+high[:2])/2
    tri[:,:,2]-=low[2]
    extents=high-low
    assert max(extents)<176,(name,extents)
    normals=np.cross(tri[:,1]-tri[:,0],tri[:,2]-tri[:,0])
    areas=np.linalg.norm(normals,axis=1)
    assert areas.min()>1e-10
    normals/=areas[:,None]
    edges=collections.Counter(tuple(sorted((tuple(a),tuple(b)))) for t in tri.round(5) for a,b in [(t[0],t[1]),(t[1],t[2]),(t[2],t[0])])
    assert all(n==2 for n in edges.values()),name
    volume=np.einsum('ij,ij->i',tri[:,0],np.cross(tri[:,1],tri[:,2])).sum()/6
    assert volume>0
    assert abs(volume/1000-part['volume_cm3'])/part['volume_cm3']<.015
    for i in range(n):struct.pack_into('<12f',data,84+50*i,*normals[i],*tri[i].flatten())
    target=folder/(name+'_print.stl');target.write_bytes(data)
    results.append({'part':name,'triangles':n,'watertight':True,'extent_mm':extents.tolist(),
                    'volume_mm3':volume,'orientation':'open rim down / palm up / diagonal; internal supports required' if name=='Upper_Shell' else ('flat underside down / diagonal' if name=='Bottom_Cover' else 'flat bridge face down'),
                    'file':str(target)})
(folder/'mesh_checks.json').write_text(json.dumps(results,indent=2))
print(json.dumps(results,indent=2))
