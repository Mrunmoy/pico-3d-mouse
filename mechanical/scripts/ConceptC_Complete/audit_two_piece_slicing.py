"""Local Bambu CLI audit only. Never connects to or sends a job to a printer."""
from pathlib import Path
import json,zipfile,subprocess,os,time,sys
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'output'/'two-piece'
EXE=Path(r'C:\Program Files\Bambu Studio\bambu-studio.exe')
results=[]
for name in ('Base_Tray','Palm_Cap'):
 if len(sys.argv)>1 and name not in sys.argv[1:]:continue
 for enabled in (False,True):
  mode='local_support' if enabled else 'no_support'
  folder=OUT/'slicer-audit'/(name+'_'+mode);folder.mkdir(parents=True,exist_ok=True)
  source=OUT/'slicer-audit'/(name+'_NoSupports_Audit.3mf');project=folder/'input.3mf'
  with zipfile.ZipFile(source) as src,zipfile.ZipFile(project,'w',zipfile.ZIP_DEFLATED) as dst:
   for item in src.infolist():
    data=src.read(item.filename)
    if item.filename=='Metadata/project_settings.config':
     settings=json.loads(data);settings.update(enable_support='1' if enabled else '0',support_type='tree(auto)',print_settings_id='SpaceMouse '+mode)
     data=json.dumps(settings).encode()
    dst.writestr(item,data)
  cmd=[str(EXE),'--slice','0','--debug','2','--outputdir',str(folder),'--export-3mf',name+'_'+mode+'.gcode.3mf',str(project)]
  with open(folder/'stdout.txt','wb') as stdout,open(folder/'stderr.txt','wb') as stderr:
   proc=subprocess.run(cmd,stdout=stdout,stderr=stderr,creationflags=subprocess.CREATE_NO_WINDOW,timeout=180)
  raw=json.loads((folder/'result.json').read_text()) if (folder/'result.json').exists() else {}
  assert proc.returncode==0 and raw.get('return_code')==0,(name,mode,proc.returncode,raw)
  plate=raw['sliced_plates'][0];filaments=plate.get('filaments',[])
  summary=dict(part=name,mode=mode,return_code=proc.returncode,total_minutes=round(plate['total_predication']/60,2),model_minutes=round(plate['main_predication']/60,2),total_grams=round(sum(f['total_used_g'] for f in filaments),2),warnings=plate.get('warning_message',''),feature_seconds=plate['feature_type_times'],project=str((folder/(name+'_'+mode+'.gcode.3mf')).relative_to(ROOT)))
  results.append(summary);print(json.dumps(summary),flush=True)
if len(sys.argv)>1 and (OUT/'slicer_audit.json').exists():
 old=json.loads((OUT/'slicer_audit.json').read_text())
 results=[v for v in old['results'] if v['part'] not in sys.argv[1:]]+results
(OUT/'slicer_audit.json').write_text(json.dumps(dict(bambu_version='02.08.02.61',printer='A1 mini 0.4 mm',layer_height_mm=.12,walls=3,infill_percent=15,brim_mm=5,results=results,print_sent=False),indent=2))
