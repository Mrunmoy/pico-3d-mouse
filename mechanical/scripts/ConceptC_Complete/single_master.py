"""Keep the current two-piece cloud document; remove the explicitly obsolete R2."""
import adsk.core, adsk.fusion, json, os, traceback
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
KEEP='urn:adsk.wipprod:dm.lineage:Oa1GpTwQR8iVvi6D4SRQIg'
OLD='urn:adsk.wipprod:dm.lineage:BfuyUQ4XTJGKQyfE52KnBw'

def run(req):
    app=adsk.core.Application.get(); report={'preserved_id':KEEP,'deleted':[]}
    try:
        current=next(d for d in app.documents if d.dataFile and d.dataFile.id==KEEP)
        current.activate()
        assert current.dataFile.parentProject.name.lower().replace(' ','')=='spacemouse'
        if current.isModified: assert current.save('Save current two-piece master before obsolete R2 cleanup')
        design=adsk.fusion.Design.cast(app.activeProduct)
        path=os.path.join(ROOT,'output','two-piece','SpaceMouse_TwoPiece.f3d')
        assert design.exportManager.execute(design.exportManager.createFusionArchiveExportOptions(path))
        report.update(name=current.dataFile.name,version=current.dataFile.versionNumber,local_archive=path)
        for doc in list(app.documents):
            if doc.dataFile and doc.dataFile.id==OLD: doc.close(False)
        project=current.dataFile.parentProject
        def files(folder):
            result=list(folder.dataFiles)
            for sub in folder.dataFolders: result+=files(sub)
            return result
        before=files(project.rootFolder)
        obsolete=next((f for f in before if f.id==OLD),None)
        if obsolete:
            assert obsolete.name=='SpaceMouse Concept C - Left Hand Recessed Mount R2'
            old_name=obsolete.name
            assert obsolete.deleteMe(), 'Fusion did not delete obsolete R2'
            report['deleted'].append(dict(id=OLD,name=old_name))
        remaining=files(project.rootFolder)
        report['remaining']=[dict(id=f.id,name=f.name) for f in remaining]
        assert any(f.id==KEEP for f in remaining)
        assert not any(f.id==OLD for f in remaining)
        report['complete']=True
    except Exception: report['error']=traceback.format_exc()
    with open(os.path.join(ROOT,'output','two-piece','single_master_record.json'),'w') as f: json.dump(report,f,indent=2)
