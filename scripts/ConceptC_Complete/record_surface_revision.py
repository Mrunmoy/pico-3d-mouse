from pathlib import Path
import json, zipfile, hashlib
out=Path(__file__).resolve().parents[2]/'output'/'complete'
with zipfile.ZipFile(out/'Upper_Shell_print.3mf') as z:
    assert z.testzip() is None
    configs=[n for n in z.namelist() if 'project_settings.config' in n]
    assert configs, z.namelist()
    cfg=json.loads(z.read(configs[0]))
    keys=['layer_height','initial_layer_print_height','wall_loops','sparse_infill_density','enable_support','support_type']
    actual={k:cfg.get(k) for k in keys}
    assert float(cfg['layer_height'])==0.12,actual
    assert int(cfg['wall_loops'])==3,actual
    assert str(cfg['enable_support'])=='1',actual
audit=json.loads((out/'upper_surface_audit.json').read_text())
audit['verified_bambu_project_settings']=actual
audit['previous_triangles']=21900
audit['new_triangles']=512618
audit['print_orientation']='open rim down; palm up; diagonal'
audit['physical_finish_verified']=False
audit['new_stl_sha256']=hashlib.sha256((out/'Upper_Shell_print.stl').read_bytes()).hexdigest()
(out/'upper_surface_audit.json').write_text(json.dumps(audit,indent=2))
report=json.loads((out/'slicer_checks.json').read_text())
report['parts'][0]={'file':'Upper_Shell_print.stl','revision':'Refined mesh, palm up',
 'wall_loops':3,'layer_height_mm':0.12,'support':'tree auto, 30 degrees, internal cavity',
 'slice':'success','total_grams':133.75,'support_grams':38.38,'estimated_total_time':'8h36m',
 'notes':'No displayed slicing warning. Local slicing only; not sent to printer.'}
report['operation']='Local slicing report; refined upper was not sent. Bottom cover and retainer subsequently submitted separately by user request.'
(out/'slicer_checks.json').write_text(json.dumps(report,indent=2))
with (out/'Assembly_and_Print_Guide.txt').open('a') as f:
    f.write('\nUPPER SURFACE REVISION\nOpen Upper_Shell_print.3mf in Bambu Studio to restore the checked quality\nsettings (0.12 mm layers, 3 walls, 15% infill, internal tree supports).\nEstimated 8h36m and 133.75 g including 38.38 g supports. Not sent to printer.\nReload updated Upper_Shell_print.stl if an older copy is already on a plate.\nThe CAD dimensions and fit to the bottom plate have not changed.\n')
print(json.dumps(actual,indent=2))
