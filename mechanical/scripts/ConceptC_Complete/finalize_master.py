"""Verify the saved R2 cloud master, refresh local archive, close extra tabs."""
import adsk.core, adsk.fusion, json, os, traceback
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT = os.path.join(ROOT, 'output', 'revision2')

def run(context):
    app = adsk.core.Application.get()
    report = {}
    try:
        doc = app.activeDocument
        data = doc.dataFile
        assert data and 'R2' in data.name, 'Active document must be saved R2'
        folder = data.parentFolder
        project = data.parentProject
        report.update(name=data.name, project=project.name, folder=folder.name,
                      data_file_id=data.id, version=data.versionNumber)
        assert project.name.lower().replace(' ', '') == 'spacemouse', report
        design = adsk.fusion.Design.cast(app.activeProduct)
        archive = os.path.join(OUT, 'ConceptC_Complete.f3d')
        assert design.exportManager.execute(design.exportManager.createFusionArchiveExportOptions(archive))
        report['local_archive'] = archive
        report['closed_tabs'] = []
        report['preserved_tabs'] = []
        for other in list(app.documents):
            if other == doc:
                continue
            # Only prior generated SpaceMouse documents and blank scratch tabs.
            if other.name.startswith(('SpaceMouse Concept C', 'Untitled', 'ConceptC_')):
                report['closed_tabs'].append(other.name)
                other.close(False)
            else:
                report['preserved_tabs'].append(other.name)
        report['remaining_tabs'] = [d.name for d in app.documents]
        report['verified'] = True
    except Exception:
        report['error'] = traceback.format_exc()
    with open(os.path.join(OUT, 'master_save_record.json'), 'w') as f:
        json.dump(report, f, indent=2)
