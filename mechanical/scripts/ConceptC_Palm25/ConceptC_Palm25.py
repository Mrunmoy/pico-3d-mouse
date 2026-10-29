"""Run in Fusion: enlarge palm support relative to unchanged joystick interface."""
import adsk.core, adsk.fusion, os, json, traceback, struct, math

ROOT = r'D:\github\fusion360\space-mouse'
OUT = os.path.join(ROOT, 'output', 'palm25')

def run(context):
    app = adsk.core.Application.get()
    os.makedirs(OUT, exist_ok=True)
    try:
        previous = app.activeDocument
        folder = previous.dataFile.parentFolder if previous and previous.dataFile else None
        doc = app.importManager.importToNewDocument(app.importManager.createFusionArchiveImportOptions(
            os.path.join(ROOT, 'output', 'fit-test', 'ConceptC_GripTest.f3d')))
        doc.name = 'Concept C - Palm Support Plus 25 mm'
        design = adsk.fusion.Design.cast(app.activeProduct)
        params = design.userParameters
        params.add('palm_extension', adsk.core.ValueInput.createByString('25 mm'), 'mm',
                   'User requested extra palm support after first physical fit test')
        params.itemByName('body_width').expression = '155 mm + palm_extension'
        params.itemByName('joystick_x').expression = '32 mm + palm_extension / 2'
        design.computeAll()
        warnings = []
        for item in design.timeline:
            e = item.entity
            if hasattr(e, 'healthState') and e.healthState != adsk.fusion.FeatureHealthStates.HealthyFeatureHealthState:
                warnings.append({'name': e.name, 'message': e.errorOrWarningMessage})
        if warnings:
            raise RuntimeError(str(warnings))
        upper = design.rootComponent.occurrences.item(0).component
        body = upper.bRepBodies.item(0)
        assert body.isSolid
        body.name = 'Palm support +25 mm - ergonomic fit test'
        export = design.exportManager
        archive = os.path.join(OUT, 'ConceptC_Palm25.f3d')
        assert export.execute(export.createFusionArchiveExportOptions(archive))
        upright = os.path.join(OUT, 'ConceptC_Palm25_upright.stl')
        stl = export.createSTLExportOptions(body, upright)
        stl.meshRefinement = adsk.fusion.MeshRefinementSettings.MeshRefinementHigh
        stl.sendToPrintUtility = False
        assert export.execute(stl)
        # Rigid rotations only: palm down, then 45 degrees in XY to fit A1 Mini.
        data = bytearray(open(upright, 'rb').read())
        count = struct.unpack_from('<I', data, 80)[0]
        assert len(data) == 84 + 50 * count
        vertices = []
        c = math.sqrt(0.5)
        for i in range(count):
            values = struct.unpack_from('<12f', data, 84 + 50*i)
            n = values[:3]
            normal = (c*(n[0]+n[1]), c*(n[0]-n[1]), -n[2])
            tri = []
            for j in (3,6,9):
                x,y,z = values[j:j+3]
                tri.extend((c*(x+y), c*(x-y), -z))
            vertices.extend([tri[j:j+3] for j in (0,3,6)])
            struct.pack_into('<12f', data, 84+50*i, *normal, *tri)
        low = [min(v[k] for v in vertices) for k in range(3)]
        high = [max(v[k] for v in vertices) for k in range(3)]
        extent = [high[k]-low[k] for k in range(3)]
        assert max(extent[:2]) < 176, str(extent)
        shift = [-(high[0]+low[0])/2, -(high[1]+low[1])/2, -low[2]]
        for i in range(count):
            values = list(struct.unpack_from('<12f', data, 84+50*i))
            for j in (3,6,9):
                for k in range(3): values[j+k] += shift[k]
            struct.pack_into('<12f', data, 84+50*i, *values)
        target = os.path.join(OUT, 'ConceptC_Palm25_print.stl')
        open(target, 'wb').write(data)
        cam = app.activeViewport.camera
        cam.isSmoothTransition = False
        cam.eye = adsk.core.Point3D.create(18,-26,24)
        cam.target = adsk.core.Point3D.create(0,0,3)
        cam.upVector = adsk.core.Vector3D.create(0,0,1)
        app.activeViewport.camera = cam
        app.activeViewport.fit()
        app.activeViewport.refresh()
        app.activeViewport.saveAsImageFile(os.path.join(OUT,'ConceptC_Palm25.png'),1400,1000)
        report = {'parameters':{p.name:p.expression for p in params}, 'warnings':warnings,
                  'print_extent_mm':extent, 'triangles':count, 'archive':archive,
                  'stl':target, 'cloud_saved':False,
                  'notes':'25 mm more span from joystick toward left edge. Height, depth, opening and screw pitch preserved. Trial opening still unverified against boot.'}
        if folder:
            try: report['cloud_saved'] = doc.saveAs(doc.name, folder, 'Physical fit revision', 'Palm support extended 25 mm')
            except: report['cloud_error'] = traceback.format_exc()
        with open(os.path.join(OUT,'revision_report.json'),'w') as f: json.dump(report,f,indent=2)
    except:
        with open(os.path.join(OUT,'error.txt'),'w') as f: f.write(traceback.format_exc())
        app.userInterface.messageBox(traceback.format_exc())
