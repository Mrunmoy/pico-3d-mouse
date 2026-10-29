"""Restricted dispatcher for current SpaceMouse operations only."""
import os,json,importlib.util
ROOT=os.path.abspath(os.path.join(os.path.dirname(__file__),'..','..'))
def run(context):
    queue=os.path.join(ROOT,'scripts','ConceptC_Complete','queued.json')
    if not os.path.exists(queue):
        raise RuntimeError('No queued operation. See mechanical/README.md.')
    with open(queue,encoding='utf-8-sig') as f:request=json.load(f)
    modules={'project_gallery':'project_gallery.py','amazon_inserts':'amazon_inserts.py'}
    action=request.get('action')
    if action not in modules:raise ValueError('Unsupported current operation: '+str(action))
    os.replace(queue,queue+'.started')
    spec=importlib.util.spec_from_file_location(action,os.path.join(ROOT,'scripts','ConceptC_Complete',modules[action]))
    mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod);mod.run(request)
