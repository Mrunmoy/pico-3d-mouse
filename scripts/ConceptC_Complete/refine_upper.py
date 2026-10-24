"""Inspect exact upper-shell geometry and re-tessellate; never sends to printer."""
import adsk.core, adsk.fusion, os, json, math, traceback, shutil
ROOT=r'D:\github\fusion360\space-mouse'
OUT=os.path.join(ROOT,'output','complete')
def run(context):
    app=adsk.core.Application.get()
    try:
        d=adsk.fusion.Design.cast(app.activeProduct)
        if not d or not d.rootComponent.occurrences.count:
            doc=app.importManager.importToNewDocument(app.importManager.createFusionArchiveImportOptions(os.path.join(OUT,'ConceptC_Complete.f3d')))
            d=adsk.fusion.Design.cast(app.activeProduct)
        root=d.rootComponent;upper=root.occurrences.item(0).component
        assert upper.name.startswith('01 Sculpted'),upper.name
        b=upper.bRepBodies.item(0)
        d.computeAll()
        health=[]
        for t in d.timeline:
            e=t.entity
            if hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:
                health.append({'name':e.name,'message':e.errorOrWarningMessage})
        assert not health,health
        assert b.isSolid
        surfaces={}
        for f in b.faces:
            k=f.geometry.objectType;surfaces[k]=surfaces.get(k,0)+1
        path=os.path.join(OUT,'Upper_Shell_assembly.stl')
        backup=os.path.join(OUT,'previous_upper_export')
        os.makedirs(backup,exist_ok=True)
        for name in ('Upper_Shell_assembly.stl','Upper_Shell_print.stl'):
            if not os.path.exists(os.path.join(backup,name)):
                shutil.copy2(os.path.join(OUT,name),os.path.join(backup,name))
        exp=d.exportManager
        opts=exp.createSTLExportOptions(b,path)
        opts.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh
        old={'surface_deviation_mm':opts.surfaceDeviation*10,'normal_deviation_degrees':math.degrees(opts.normalDeviation),'maximum_edge_mm':opts.maximumEdgeLength*10}
        opts.surfaceDeviation=0.0005 # cm = 0.005 mm
        opts.normalDeviation=math.radians(1)
        opts.maximumEdgeLength=0.1 # cm = 1 mm
        opts.sendToPrintUtility=False
        assert exp.execute(opts)
        report={'geometry_changed':False,'reason':'Retain exact editable loft and 10 mm fillet; refine tessellation only.',
                'solid':b.isSolid,'feature_warnings':health,'surface_types':surfaces,
                'fillets':[f.name for f in upper.features.filletFeatures],
                'previous_high_preset':old,'new_export':{'surface_deviation_mm':opts.surfaceDeviation*10,
                'normal_deviation_degrees':math.degrees(opts.normalDeviation),'maximum_edge_mm':opts.maximumEdgeLength*10},
                'sent_to_printer':False}
        with open(os.path.join(OUT,'upper_surface_audit.json'),'w') as f:json.dump(report,f,indent=2)
        visible=[o.isLightBulbOn for o in root.occurrences]
        for i,o in enumerate(root.occurrences):o.isLightBulbOn=(i==0)
        app.userInterface.activeSelections.clear()
        cam=app.activeViewport.camera;cam.isSmoothTransition=False
        cam.eye=adsk.core.Point3D.create(18,-25,15);cam.target=adsk.core.Point3D.create(0,0,2.5)
        cam.upVector=adsk.core.Vector3D.create(0,0,1)
        app.activeViewport.camera=cam;app.activeViewport.fit();app.activeViewport.refresh();adsk.doEvents()
        app.activeViewport.saveAsImageFile(os.path.join(OUT,'Upper_Shell_Curvature_Review.png'),2000,1400)
        for o,v in zip(root.occurrences,visible):o.isLightBulbOn=v
        with open(os.path.join(OUT,'smooth.ok'),'w') as f:f.write('Exact surface audit and refined STL export complete')
    except:
        with open(os.path.join(OUT,'smooth_error.txt'),'w') as f:f.write(traceback.format_exc())
