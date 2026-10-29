"""Measure sidewall reversal and transverse palm height from actual STL triangles."""
import json
from pathlib import Path
import numpy as np

ROOT=Path(__file__).resolve().parents[2]
DT=np.dtype([('normal','<f4',(3,)),('vertices','<f4',(3,3)),('attribute','<u2')])

def triangles(path):
    return np.frombuffer(path.read_bytes(),dtype=DT,offset=84)['vertices'].astype(float)

def silhouette(t):
    rows=[]
    for z in np.arange(3.5,41.6,.5):
        a=t[(t[:,:,2].min(axis=1)<=z)&(t[:,:,2].max(axis=1)>=z)]
        hits=[]
        for i,j in ((0,1),(1,2),(2,0)):
            p,q=a[:,i],a[:,j];dz=q[:,2]-p[:,2]
            mask=(abs(dz)>1e-9)&((p[:,2]-z)*(q[:,2]-z)<=0)
            p,q,dz=p[mask],q[mask],dz[mask]
            hits.append(p+(q-p)*((z-p[:,2])/dz)[:,None])
        pts=np.concatenate(hits)
        rows.append([float(z),float(pts[:,1].min()),float(pts[:,1].max())])
    a=np.array(rows)
    return {'sections_z_minY_maxY_mm':rows,
            'front_outward_reversal_mm':float(np.maximum(-np.diff(a[:,1]),0).sum()),
            'rear_outward_reversal_mm':float(np.maximum(np.diff(a[:,2]),0).sum())}

def roof(t,x,y):
    a,b,c=t[:,0],t[:,1],t[:,2]
    den=(b[:,1]-c[:,1])*(a[:,0]-c[:,0])+(c[:,0]-b[:,0])*(a[:,1]-c[:,1])
    mask=abs(den)>1e-10;a,b,c,den=a[mask],b[mask],c[mask],den[mask]
    u=((b[:,1]-c[:,1])*(x-c[:,0])+(c[:,0]-b[:,0])*(y-c[:,1]))/den
    v=((c[:,1]-a[:,1])*(x-c[:,0])+(a[:,0]-c[:,0])*(y-c[:,1]))/den
    w=1-u-v;mask=(u>=-1e-7)&(v>=-1e-7)&(w>=-1e-7)
    z=u*a[:,2]+v*b[:,2]+w*c[:,2]
    return float(z[mask].max())

def topology(t):
    verts,idx=np.unique(t.reshape(-1,3).round(5),axis=0,return_inverse=True)
    faces=idx.reshape(-1,3)
    edges=np.concatenate([faces[:,[0,1]],faces[:,[1,2]],faces[:,[2,0]]])
    edges.sort(axis=1)
    edges,counts=np.unique(edges,axis=0,return_counts=True)
    chi=len(verts)-len(edges)+len(faces)
    return {'euler_characteristic':int(chi),'genus_if_connected':float((2-chi)/2),'all_edges_shared_twice':bool((counts==2).all())}

report={}
for rev in ('rounded','asymmetric'):
    t=triangles(ROOT/'output'/rev/'Upper_Shell_assembly.stl')
    report[rev]=silhouette(t)
    report[rev]['topology']=topology(t)
    report[rev]['palm_height_samples_mm']=[{'x':60,'y':y,'z':roof(t,60,y)} for y in (-20,0,20)]
    samples=report[rev]['palm_height_samples_mm']
    report[rev]['rear_minus_front_palm_height_mm']=samples[-1]['z']-samples[0]['z']
out=ROOT/'output'/'asymmetric'/'shape_audit.json'
out.write_text(json.dumps(report,indent=2)+'\n')
print(json.dumps({r:{k:v for k,v in a.items() if k!='sections_z_minY_maxY_mm'} for r,a in report.items()},indent=2))
