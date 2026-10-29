"""Package completed CAD and reviewed exports; never connects to a printer."""
from pathlib import Path
import zipfile
root = Path(__file__).resolve().parents[2]
out = root / 'output' / 'complete'
names = ['ConceptC_Complete.f3d', 'ConceptC_Exploded.f3d',
         'ConceptC_Complete_Assembly.step', 'Upper_Shell_print.stl',
         'Bottom_Cover_print.stl', 'PCB_Retainer_print.stl',
         'ConceptC_Full_Assembly.png', 'ConceptC_Exploded.png',
         'ConceptC_Internal_Layout.png', 'ConceptC_Upper_Underside.png',
         'Assembly_and_Print_Guide.txt', 'validation_report.json',
         'mesh_checks.json', 'slicer_checks.json', 'upper_surface_audit.json',
         'Upper_Shell_Curvature_Review.png', 'Upper_Shell_print.3mf']
target = root / 'output' / 'ConceptC_Complete_R1.zip'
with zipfile.ZipFile(target, 'w', zipfile.ZIP_DEFLATED) as z:
    for name in names:
        p = out / name
        assert p.exists() and p.stat().st_size > 0, p
        z.write(p, name)
    for name in ['ConceptC_Complete.py', 'ConceptC_Complete.manifest', 'prepare_exports.py', 'refine_upper.py']:
        z.write(Path(__file__).parent / name, 'source/' + name)
    z.write(root / 'output' / 'palm25' / 'ConceptC_Palm25.f3d', 'source/ConceptC_Palm25.f3d')
with zipfile.ZipFile(target) as z:
    assert z.testzip() is None
    print(str(target), target.stat().st_size, 'bytes;', len(z.namelist()), 'files; CRC passed')
