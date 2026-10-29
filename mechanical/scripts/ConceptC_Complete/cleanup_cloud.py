"""Inventory cloud documents; delete only explicitly approved obsolete mouse IDs."""
import adsk.core, os, json, traceback
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
OUT = os.path.join(ROOT, 'output', 'revision2')

def run(context):
    report = {'files': [], 'deleted': [], 'errors': []}
    try:
        app = adsk.core.Application.get()
        master = json.load(open(os.path.join(OUT, 'master_save_record.json')))['data_file_id']
        approval_path = os.path.join(OUT, 'cleanup_approved.json')
        approved = json.load(open(approval_path, encoding='utf-8-sig')) if os.path.exists(approval_path) else []
        def visit(folder, project):
            for f in list(folder.dataFiles):
                entry = dict(id=f.id, name=f.name, project=project, folder=folder.name)
                report['files'].append(entry)
                if f.id in approved:
                    assert f.id != master and 'deskmate' not in f.name.lower()
                    assert f.name.startswith(('SpaceMouse Concept C', 'ConceptC_', 'Concept C - Palm Support'))
                    try:
                        if f.deleteMe(): report['deleted'].append(entry)
                        else: report['errors'].append(entry)
                    except Exception: report['errors'].append(dict(entry, error=traceback.format_exc()))
            for sub in list(folder.dataFolders): visit(sub, project)
        for project in app.data.dataProjects: visit(project.rootFolder, project.name)
        report['master_preserved'] = master
    except Exception: report['errors'].append(traceback.format_exc())
    with open(os.path.join(OUT, 'cloud_cleanup_record.json'), 'w') as f:
        json.dump(report, f, indent=2)
