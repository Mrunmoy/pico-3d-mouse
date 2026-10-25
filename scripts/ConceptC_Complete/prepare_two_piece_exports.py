"""Orient split pieces on their real print faces; package no-support slicer audits."""
from pathlib import Path
import numpy as np,struct,json,zipfile,collections,math
from xml.etree import ElementTree as ET
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'output'/'two-piece'
report=json.loads((OUT/'report.json').read_text());assert report.get('complete')
template=ROOT/'output'/'asymmetric'/'Top_Cover_A1mini_0p12.3mf'
results=[]
for part in report['parts']:
 name=part['name'];data=bytearray((OUT/(name+'_assembly.stl')).read_bytes());count=struct.unpack_from('<I',data,80)[0]
 dtype=np.dtype([('normal','<f4',3),('vertices','<f4',(3,3)),('attr','<u2')]);records=np.frombuffer(data,dtype=dtype,offset=84);tri=records['vertices'].astype(float)
 if name=='Palm_Cap':
  normal=np.array([.14,-.04,1]);normal/=np.linalg.norm(normal);ex=np.array([1,0,-.14]);ex/=np.linalg.norm(ex);ey=np.cross(normal,ex)
  tri=tri@np.array([ex,ey,normal]).T
 a=math.pi/4;tri[:,:,:2]=tri[:,:,:2]@np.array([[math.cos(a),-math.sin(a)],[math.sin(a),math.cos(a)]]).T
 lo=tri.min((0,1));hi=tri.max((0,1));size=hi-lo;assert max(size[:2])<170,size
 tri[:,:,:2]-=(lo[:2]+hi[:2])/2;tri[:,:,2]-=lo[2]
 norm=np.cross(tri[:,1]-tri[:,0],tri[:,2]-tri[:,0]);areas=np.linalg.norm(norm,axis=1);assert min(areas)>1e-10;norm/=areas[:,None]
 edges=collections.Counter(tuple(sorted((tuple(a),tuple(b)))) for t in tri.round(5) for a,b in [(t[0],t[1]),(t[1],t[2]),(t[2],t[0])]);assert all(v==2 for v in edges.values()),name
 volume=np.einsum('ij,ij->i',tri[:,0],np.cross(tri[:,1],tri[:,2])).sum()/6;assert volume>0 and abs(volume-part['volume_cm3']*1000)<volume*.01
 records['normal']=norm;records['vertices']=tri;(OUT/(name+'_print.stl')).write_bytes(data)
 # Flat contact area and steep downward faces expose residual bridges/overhangs honestly.
 centres=tri.mean(1);down=norm[:,2]<-.707
 results.append(dict(name=name,watertight=True,triangles=count,size_mm=size.tolist(),volume_mm3=volume,bed_contact_area_mm2=float(areas[tri[:,:,2].max(1)<.03].sum()/2),downward_overhang_area_above_first_layer_mm2=float(areas[down&(centres[:,2]>.3)].sum()/2)))
 with zipfile.ZipFile(template) as z:
  settings=json.loads(z.read('Metadata/project_settings.config'))
  settings['enable_support']='0';settings['support_type']='tree(auto)';settings['brim_type']='outer_only';settings['brim_width']='5'
  settings['print_settings_id']='Two-piece - no support audit'
  # A compact valid 3MF with one centred object; Bambu reads its native settings.
  ns='http://schemas.microsoft.com/3dmanufacturing/core/2015/02'
  model=ET.Element('model',unit='millimeter',xmlns=ns);ET.SubElement(model,'metadata',name='Application').text='BambuStudio-02.08.02.61';ET.SubElement(model,'metadata',name='BambuStudio:3mfVersion').text='1';resources=ET.SubElement(model,'resources');obj=ET.SubElement(resources,'object',id='1',type='model',name=name);mesh=ET.SubElement(obj,'mesh');verts=ET.SubElement(mesh,'vertices');tris=ET.SubElement(mesh,'triangles')
  unique,indices=np.unique(tri.reshape(-1,3).round(6),axis=0,return_inverse=True)
  for x,y,zv in unique:ET.SubElement(verts,'vertex',x=str(x),y=str(y),z=str(zv))
  for i,j,k in indices.reshape(-1,3):ET.SubElement(tris,'triangle',v1=str(i),v2=str(j),v3=str(k))
  build=ET.SubElement(model,'build');ET.SubElement(build,'item',objectid='1',transform='1 0 0 0 1 0 0 0 1 90 90 0')
  rels='<?xml version="1.0"?><Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships"><Relationship Type="http://schemas.microsoft.com/3dmanufacturing/2013/01/3dmodel" Target="/3D/3dmodel.model" Id="rel0"/></Relationships>'
  content='<?xml version="1.0"?><Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types"><Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/><Default Extension="model" ContentType="application/vnd.ms-package.3dmanufacturing-3dmodel+xml"/><Default Extension="config" ContentType="text/plain"/></Types>'
  with zipfile.ZipFile(OUT/(name+'_NoSupports_Audit.3mf'),'w',zipfile.ZIP_DEFLATED) as target:
   target.writestr('[Content_Types].xml',content);target.writestr('_rels/.rels',rels);target.writestr('3D/3dmodel.model',ET.tostring(model,encoding='utf-8',xml_declaration=True));target.writestr('Metadata/project_settings.config',json.dumps(settings))
(OUT/'mesh_audit.json').write_text(json.dumps(results,indent=2));print(json.dumps(results,indent=2))
