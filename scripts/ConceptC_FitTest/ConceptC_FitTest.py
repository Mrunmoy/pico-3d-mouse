import adsk.core, adsk.fusion, os, json, traceback

ROOT=r'D:\github\fusion360\space-mouse'
OUT=os.path.join(ROOT,'output','fit-test')
V=adsk.core.ValueInput.createByString
P=adsk.core.Point3D.create

def run(context):
    complete_request=os.path.join(ROOT,'scripts','ConceptC_Complete','queued.json')
    if os.path.exists(complete_request):
        import importlib.util
        with open(complete_request,encoding='utf-8-sig') as f: request=json.load(f)
        os.replace(complete_request,complete_request+'.started')
        if request.get('action')=='tpu_feet':
            path=os.path.join(ROOT,'scripts','ConceptC_Complete','tpu_feet.py')
            spec=importlib.util.spec_from_file_location('tpu_feet',path)
            module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            module.run(request)
            return
        if request.get('action')=='reference_views':
            path=os.path.join(ROOT,'scripts','ConceptC_Complete','reference_views.py')
            spec=importlib.util.spec_from_file_location('reference_views',path)
            module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
            module.run(request)
            return
        if request.get('action')=='asymmetric':
            path=os.path.join(ROOT,'scripts','ConceptC_Complete','asymmetric_revision.py')
            spec=importlib.util.spec_from_file_location('asymmetric_revision',path)
            module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
            module.run(request)
            return
        if request.get('action')=='rounded':
            path=os.path.join(ROOT,'scripts','ConceptC_Complete','rounded_revision.py')
            spec=importlib.util.spec_from_file_location('rounded_revision',path)
            module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
            module.run(request)
            return
        if request.get('action')=='continuous':
            path=os.path.join(ROOT,'scripts','ConceptC_Complete','continuous_revision.py')
            spec=importlib.util.spec_from_file_location('continuous_revision',path)
            module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
            module.run(request)
            return
        if request.get('action')=='ergonomic':
            path=os.path.join(ROOT,'scripts','ConceptC_Complete','ergonomic_revision.py')
            spec=importlib.util.spec_from_file_location('ergonomic_revision',path)
            module=importlib.util.module_from_spec(spec); spec.loader.exec_module(module)
            module.run(request)
            return
        filename='cleanup_cloud.py' if request.get('action')=='cleanup_cloud' else ('finalize_master.py' if request.get('action')=='finalize_master' else ('ConceptC_R2.py' if request.get('revision')=='R2' else 'ConceptC_Complete.py'))
        path=os.path.join(ROOT,'scripts','ConceptC_Complete',filename)
        spec=importlib.util.spec_from_file_location('ConceptC_Complete',path)
        module=importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        module.run(context)
        return
    # Dispatch an explicitly queued fit revision through the registered script.
    queued=os.path.join(ROOT,'scripts','ConceptC_Palm25','queued.json')
    if os.path.exists(queued):
        import importlib.util
        os.replace(queued,queued+'.started')
        path=os.path.join(ROOT,'scripts','ConceptC_Palm25','ConceptC_Palm25.py')
        spec=importlib.util.spec_from_file_location('ConceptC_Palm25',path)
        module=importlib.util.module_from_spec(spec)
        spec.loader.exec_module(module)
        module.run(context)
        return
    app=adsk.core.Application.get()
    try:
        os.makedirs(OUT,exist_ok=True)
        imp=app.importManager
        doc=imp.importToNewDocument(imp.createFusionArchiveImportOptions(os.path.join(ROOT,'output','SpaceMouse_ConceptC_Exterior.f3d')))
        doc.name='SpaceMouse Concept C - Grip Fit Test'
        design=adsk.fusion.Design.cast(app.activeProduct)
        root=design.rootComponent
        upper=root.occurrences.item(0).component
        root.occurrences.item(1).isLightBulbOn=False
        # Preserve reviewed height while extending the test shell to the desktop.
        design.userParameters.itemByName('body_height').expression='39.5 mm'
        design.userParameters.itemByName('bottom_thickness').expression='0 mm'
        for name,exp,comment in [
            ('fit_aperture','40.5 mm','TRIAL boot/knob opening; not a manufacturer panel-cutout specification'),
            ('fit_screw_clearance','2.8 mm','M2.5 trial clearance; verify actual screw fit'),
        ]: design.userParameters.add(name,V(exp),'mm',comment)
        def value(e): return design.unitsManager.evaluateExpression(e,'mm')
        def plane(exp,name):
            i=upper.constructionPlanes.createInput(); i.setByOffset(upper.xYConstructionPlane,V(exp))
            p=upper.constructionPlanes.add(i); p.name=name; p.isLightBulbOn=False
            return p
        sk=upper.sketches.add(plane('palm_height','Fit test mounting plane'))
        sk.name='Trial opening and verified M2.5 flange pitch'
        def circle(xe,ye,de):
            ci=sk.sketchCurves.sketchCircles.addByCenterRadius(P(value(xe),value(ye),0),value(de)/2)
            for expr,ori in [(xe,adsk.fusion.DimensionOrientations.HorizontalDimensionOrientation),(ye,adsk.fusion.DimensionOrientations.VerticalDimensionOrientation)]:
                d=sk.sketchDimensions.addDistanceDimension(sk.originPoint,ci.centerSketchPoint,ori,P(ci.centerSketchPoint.geometry.x+0.3,ci.centerSketchPoint.geometry.y+0.2,0))
                d.parameter.expression=expr if value(expr)>0 else f'-({expr})'
            sk.sketchDimensions.addDiameterDimension(ci,P(value(xe)+1,value(ye)+1,0)).parameter.expression=de
        circle('joystick_x','joystick_y','fit_aperture')
        for sx in [-1,1]:
            for sy in [-1,1]: circle(f'joystick_x+({sx})*flange_pitch_x/2',f'joystick_y+({sy})*flange_pitch_y/2','fit_screw_clearance')
        profiles=adsk.core.ObjectCollection.create()
        for profile in sk.profiles: profiles.add(profile)
        ei=upper.features.extrudeFeatures.createInput(profiles,adsk.fusion.FeatureOperations.CutFeatureOperation)
        ei.setDistanceExtent(False,V('-wall'))
        ei.participantBodies=[upper.bRepBodies.item(0)]
        cut=upper.features.extrudeFeatures.add(ei); cut.name='Grip fit test opening and flange clearance holes'
        sk.isVisible=False
        design.computeAll()
        b=upper.bRepBodies.item(0)
        warnings=[]
        for t in design.timeline:
            e=t.entity
            if hasattr(e,'healthState') and e.healthState!=adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:
                warnings.append({'name':e.name,'message':e.errorOrWarningMessage})
        if warnings: raise RuntimeError(str(warnings))
        b.name='Concept C - grip test only'
        exp=design.exportManager
        stl=exp.createSTLExportOptions(b,os.path.join(OUT,'ConceptC_GripTest_upright.stl'))
        stl.meshRefinement=adsk.fusion.MeshRefinementSettings.MeshRefinementHigh
        stl.sendToPrintUtility=False
        exp.execute(stl)
        exp.execute(exp.createFusionArchiveExportOptions(os.path.join(OUT,'ConceptC_GripTest.f3d')))
        cam=app.activeViewport.camera; cam.isSmoothTransition=False
        cam.eye=P(18,-26,24); cam.target=P(0,0,3); cam.upVector=adsk.core.Vector3D.create(0,0,1)
        app.activeViewport.camera=cam; app.activeViewport.fit(); app.activeViewport.refresh()
        app.activeViewport.saveAsImageFile(os.path.join(OUT,'ConceptC_GripTest.png'),1400,1000)
        bb=b.boundingBox
        report={'status':'ergonomic_fit_test_only','body_solid':b.isSolid,'body_volume_cm3':b.volume,'bbox_mm':[(bb.maxPoint.x-bb.minPoint.x)*10,(bb.maxPoint.y-bb.minPoint.y)*10,(bb.maxPoint.z-bb.minPoint.z)*10],'parameters':{p.name:p.expression for p in design.userParameters},'warnings':warnings,'closed_temporary_documents':[]}
        # Close only our prior generated review attempts and empty new-document tab.
        for other in list(app.documents):
            if other==doc: continue
            name=other.name
            eligible=name.startswith('SpaceMouse Concept C - Exterior Review')
            if name.startswith('Untitled'):
                od=adsk.fusion.Design.cast(other.products.itemByProductType('DesignProductType'))
                eligible=bool(od and od.timeline.count==0 and od.rootComponent.bRepBodies.count==0 and od.rootComponent.occurrences.count==0)
            if eligible:
                other.close(False); report['closed_temporary_documents'].append(name)
        doc.activate()
        with open(os.path.join(OUT,'fit_test_report.json'),'w') as f: json.dump(report,f,indent=2)
    except:
        with open(os.path.join(OUT,'fit_test_error.txt'),'w') as f:f.write(traceback.format_exc())
        app.userInterface.messageBox(traceback.format_exc())
