"""Compose labeled reference sheets from Fusion exports and package for sharing."""
from pathlib import Path
from PIL import Image, ImageDraw, ImageFont
import zipfile

ROOT = Path(__file__).resolve().parents[2]
OUT = ROOT / 'output' / 'NanoBanana_References'
font = ImageFont.truetype('C:/Windows/Fonts/arial.ttf', 28)
titlefont = ImageFont.truetype('C:/Windows/Fonts/arialbd.ttf', 38)
SUBTITLE='Current CAD reference | Dome to be redesigned | Views fitted individually'

def sheet(files, name, title):
    rows = (len(files) + 2) // 3
    canvas = Image.new('RGB', (2400, rows * 650 + 140), 'white')
    draw = ImageDraw.Draw(canvas)
    draw.text((35, 20), title, font=titlefont, fill='#202a35')
    draw.text((35, 75), SUBTITLE, font=font, fill='#56616c')
    for i, path in enumerate(files):
        im = Image.open(path).convert('RGBA')
        bbox = im.getchannel('A').getbbox()
        assert bbox, path
        im = im.crop(bbox)
        im.thumbnail((740, 550), Image.Resampling.LANCZOS)
        x, y = (i % 3) * 800, (i // 3) * 650 + 140
        canvas.paste(im, (x + (800 - im.width)//2, y + (560 - im.height)//2), im)
        label = path.stem.replace('Body_', '').split('_', 1)[1].replace('_', ' ')
        draw.text((x + 30, y + 580), label, font=font, fill='#202a35')
    canvas.save(OUT / name)

if __name__ == '__main__':
    full = sorted(OUT.glob('[0-9][0-9]_*.png'))
    body = sorted(OUT.glob('Body_*.png'))
    assert len(full) == 9 and len(body) == 5
    for path in full + body:
        with Image.open(path) as im:
            assert im.size == (2400, 1800)
    sheet(full, 'Contact_Sheet_All_Views.png', 'SpaceMouse | Assembly views')
    sheet(body, 'Contact_Sheet_Body.png', 'SpaceMouse | Shell views without joystick')
    archive = OUT.parent / 'SpaceMouse_NanoBanana_References.zip'
    with zipfile.ZipFile(archive, 'w', zipfile.ZIP_DEFLATED) as z:
        for path in sorted(OUT.iterdir()):
            if path.is_file(): z.write(path, path.name)
    print(f'{len(full) + len(body)} verified reference images; {archive} ({archive.stat().st_size:,} bytes)')
