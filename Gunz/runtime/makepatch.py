import os
import hashlib
import xml.etree.cElementTree as ET

root = ET.Element('xml')

for rootdir, dirs, files in os.walk(os.getcwd()):
    for file in files:
        if file == "makepatch.exe" or file == "makepatch.py" or file == "patch.xml":
            continue
        
        filename = os.path.join(rootdir, file).replace(os.getcwd() + "\\", "")
        md5 = hashlib.md5(open(filename, 'rb').read()).digest();
        md5str = ""
        for b in md5:
            md5str += "%.2x" % b

        root.append(ET.Element('file', name = filename, md5 = md5str))
    
ET.ElementTree(root).write("patch.xml")
