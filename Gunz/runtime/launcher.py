import urllib.request
import xml.etree.ElementTree as ET
import hashlib
import os
import time
import sys
from subprocess import Popen
from urllib.error import HTTPError

if os.path.isfile("launcher_old.exe"):
    time.sleep(0.1)
    os.remove("launcher_old.exe")

with urllib.request.urlopen("http://51.254.130.130/patch/patch.xml") as url:
    s = url.read()

e = ET.XML(s)

for atype in e.findall('file'):
    filename = atype.get('name')
    wantedmd5 = atype.get('md5')

    updating = True

    try:
        md5 = hashlib.md5(open(filename, 'rb').read()).digest();
        md5str = ""
        for b in md5:
            md5str += "%x" % b
        updating = md5str.lower() != wantedmd5.lower()
        print(filename + ": " + md5str + " - " + wantedmd5)
    except (FileNotFoundError) as e:
        print("Exception: ", e)

    if(updating):
        print("Downloading " + filename)
        try:
            if filename == "launcher.exe":
                urllib.request.urlretrieve("http://51.254.130.130/patch/" + filename, "launcher_new.exe")
                os.rename("launcher.exe", "launcher_old.exe")
                os.rename("launcher_new.exe", "launcher.exe")
                Popen('"launcher.exe"')
                sys.exit()
            else:
                urllib.request.urlretrieve("http://51.254.130.130/patch/" + filename, filename)
        except (HTTPError) as e:
            print("Exception: ", e)
            continue

print("Starting Gunz...")
Popen('"theduel.exe"')
