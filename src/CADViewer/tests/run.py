"""Build and run CADViewer UI/editor integration checks using the installed Qt/MSVC toolchain.
First build CADViewer's dependencies in Debug|x64. Run from any directory.
"""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import xml.etree.ElementTree as ET

parser=argparse.ArgumentParser()
parser.add_argument('--msbuild',default='C:/Program Files/Microsoft Visual Studio/18/Community/MSBuild/Current/Bin/MSBuild.exe')
parser.add_argument('--qt',default='D:/DevTools/Qt/6.8.3/msvc2022_64')
args=parser.parse_args()
root=Path(__file__).resolve().parents[3]
source=root/'src/CADViewer'
work=root/'build-dwg/verification/viewer-smoke'
work.mkdir(parents=True,exist_ok=True)
ns='http://schemas.microsoft.com/developer/msbuild/2003'
ET.register_namespace('',ns)
tag=lambda n:f'{{{ns}}}{n}'
tree=ET.parse(source/'CADViewer.vcxproj');project=tree.getroot()
for group in project.findall(tag('ItemGroup')):
    for item in list(group):
        include=item.get('Include')
        if not include or item.tag==tag('ProjectConfiguration'):continue
        if item.tag==tag('ClCompile') and include=='MainWnd.cpp':group.remove(item);continue
        path=source/'tests/Smoke.cpp' if include=='App.cpp' else source/Path(include.replace('\\','/'))
        item.set('Include',str(path.resolve()))
for item in project.iter():
    if item.tag==tag('AdditionalIncludeDirectories'):item.text=str(source)+';'+(item.text or '')
    if item.tag==tag('SubSystem'):item.text='Console'
    if item.tag==tag('ProjectGuid'):item.text='{7C409193-4EB7-4879-81DC-1E72F2B37B89}'
# This property group must precede targets but follow inherited properties.
group=ET.Element(tag('PropertyGroup'))
for key,value in {'TargetName':'CADViewerSmoke','OutDir':str(root/'x64/Debug')+'\\','IntDir':str(work/'integration-obj')+'\\'}.items():ET.SubElement(group,tag(key)).text=value
project.insert(len(project)-1,group)
path=work/'CADViewerSmoke.vcxproj';tree.write(path,encoding='utf-8',xml_declaration=True)
shutil.copyfile(source/'tests/sample.dxf',work/'sample.dxf')
# MSBuild cannot tolerate environment keys duplicated only by case.
env={};seen=set()
for k,v in os.environ.items():
    if k.upper() not in seen:env[k]=v;seen.add(k.upper())
command=[args.msbuild,str(path),'/p:Configuration=Debug','/p:Platform=x64',f'/p:SolutionDir={root}\\','/p:BuildProjectReferences=false','/p:LinkLibraryDependencies=false','/m','/v:minimal','/nologo']
with (work/'build.log').open('w') as log:
    result=subprocess.run(command,cwd=root,env=env,stdout=log,stderr=subprocess.STDOUT)
if result.returncode:
    print((work/'build.log').read_text(errors='replace'));raise SystemExit(result.returncode)
for k in list(env):
    if k.upper()=='PATH':env.pop(k)
env['PATH']=str(Path(args.qt)/'bin')+';'+str(root/'bin')+';'+os.environ.get('PATH','')
env['QT_QPA_PLATFORM']='offscreen';env['QT_PLUGIN_PATH']=str(Path(args.qt)/'plugins')
env['QT_QPA_FONTDIR']='C:/Windows/Fonts'
# Reproducible layout; this is the test's private settings directory only.
settings=work/'settings'
if settings.exists():
    for f in settings.rglob('*.ini'):f.unlink()
failure=work/'failure.txt'
if failure.exists():failure.unlink()
result=subprocess.run([str(root/'x64/Debug/CADViewerSmoke.exe')],cwd=root,env=env,capture_output=True,timeout=60)
(work/'run.log').write_bytes(result.stdout+result.stderr)
if result.returncode:
    print(failure.read_text() if failure.exists() else result.stderr.decode(errors='replace'))
    raise SystemExit(result.returncode)
print('PASS: DXF/DWG, tree/view selection, visibility, fit, editing, history, Working Set, filters, .shape round-trip and corruption rejection.')
print('Screenshots and logs:',work)
