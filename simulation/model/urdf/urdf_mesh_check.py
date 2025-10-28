import os
import xml.etree.ElementTree as ET
from lxml import etree

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BRUTUS_PATH = os.path.join(SCRIPT_DIR, "brutus.urdf")

print("Usando URDF:", BRUTUS_PATH)

tree = etree.parse(BRUTUS_PATH)

def check_empty_attrs(tag, attr):
    for node in tree.xpath(f"//{tag}"):
        val = node.get(attr)
        if val is None or val.strip() == "":
            print(f"⚠️  {tag} sin atributo '{attr}' válido en línea {node.sourceline}")

check_empty_attrs("link", "name")
check_empty_attrs("joint", "name")
check_empty_attrs("joint", "type")
check_empty_attrs("parent", "link")
check_empty_attrs("child", "link")

urdf_dir = os.path.dirname(BRUTUS_PATH)

for mesh in tree.xpath("//mesh"):
    filename = mesh.get("filename")
    if not filename:
        print(f"⚠️ Mesh sin filename en línea {mesh.sourceline}")
        continue

    # Convertir ruta relativa a absoluta según el directorio del URDF
    abs_path = os.path.normpath(os.path.join(urdf_dir, filename))
    if not os.path.exists(abs_path):
        print(f"Mesh missing: {filename}")

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
URDF_PATH = os.path.join(SCRIPT_DIR, "brutus.urdf")

print("🔍 Validando URDF:", URDF_PATH)
tree = etree.parse(URDF_PATH)
root = tree.getroot()
urdf_dir = os.path.dirname(URDF_PATH)

links = set()
for link in root.xpath("//link"):
    name = link.get("name")
    if not name or name.strip() == "":
        print(f"❌ Link sin nombre en línea {link.sourceline}")
    else:
        links.add(name)

for joint in root.xpath("//joint"):
    jname = joint.get("name")
    jtype = joint.get("type")
    if not jname or jname.strip() == "":
        print(f"❌ Joint sin nombre en línea {joint.sourceline}")
    if not jtype or jtype.strip() == "":
        print(f"❌ Joint sin tipo en línea {joint.sourceline}")

    parent = joint.find("parent")
    child = joint.find("child")
    if parent is None or not parent.get("link"):
        print(f"❌ Joint '{jname}' sin <parent link='...'> en línea {joint.sourceline}")
    elif parent.get("link") not in links:
        print(f"⚠️ Joint '{jname}' con parent desconocido: {parent.get('link')} (línea {joint.sourceline})")

    if child is None or not child.get("link"):
        print(f"❌ Joint '{jname}' sin <child link='...'> en línea {joint.sourceline}")
    elif child.get("link") not in links:
        print(f"⚠️ Joint '{jname}' con child desconocido: {child.get('link')} (línea {joint.sourceline})")

for viscol in root.xpath("//visual | //collision"):
    geom = viscol.find("geometry")
    if geom is None:
        print(f"⚠️ {viscol.tag} sin <geometry> en línea {viscol.sourceline}")

for mesh in root.xpath("//mesh"):
    filename = mesh.get("filename")
    if not filename or filename.strip() == "":
        print(f"⚠️ Mesh sin filename en línea {mesh.sourceline}")
        continue
    abs_path = os.path.normpath(os.path.join(urdf_dir, filename))
    if not os.path.exists(abs_path):
        print(f"⚠️ Mesh faltante: {filename} (línea {mesh.sourceline})")

for mat in root.xpath("//material"):
    mname = mat.get("name")
    if not mname or mname.strip() == "":
        print(f"⚠️ Material sin nombre en línea {mat.sourceline}")
    tex = mat.find("texture")
    if tex is not None:
        tex_file = tex.get("filename")
        if tex_file is None or tex_file.strip() == "":
            print(f"⚠️ Material '{mname}' con textura vacía en línea {mat.sourceline}")
        else:
            abs_path = os.path.normpath(os.path.join(urdf_dir, tex_file))
            if not os.path.exists(abs_path):
                print(f"⚠️ Textura faltante '{tex_file}' en material '{mname}' (línea {mat.sourceline})")

for link in root.xpath("//link[not(inertial)]"):
    name = link.get("name")
    print(f"ℹ️ Link sin <inertial>: {name} (línea {link.sourceline})")

print("\n✅ Revisión estructural completada.")