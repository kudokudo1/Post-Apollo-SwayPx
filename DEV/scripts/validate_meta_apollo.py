#!/usr/bin/env python3
from pathlib import Path
import re
ROOT=Path(__file__).resolve().parents[2]
ROOMS=("ATLAS","MODEL","BUILD","DEV","OPERATE","EVIDENCE","ARCHIVE")
L="✦︎✦︎✦︎ Meta Apollo Logos //"
e=[]
m=(ROOT/".meta-apollo.yml").read_text() if (ROOT/".meta-apollo.yml").exists() else ""
if not re.search(r"(?m)^design_language:\s*1\s*$",m): e.append("missing design_language: 1")
r=(ROOT/"README.md").read_text() if (ROOT/"README.md").exists() else ""
if not r.startswith(L): e.append("README missing lineage")
if "> **STATE //**" not in r: e.append("README missing metadata")
for room in ROOMS:
 p=ROOT/room/"README.md"
 if not p.exists(): e.append(f"missing {room}/README.md"); continue
 t=p.read_text()
 if not t.startswith(L): e.append(f"{room} missing lineage")
 if f"MAP // {room}" not in t: e.append(f"{room} missing map identity")
 if "> **STATE //**" not in t: e.append(f"{room} missing metadata")
 for target in ROOMS:
  if target not in t: e.append(f"{room} nav missing {target}")
for x in ("focus","nav","model","agency","system","scope","warning","neutral"):
 if not (ROOT/"BUILD"/"assets"/"design"/"chassis"/f"{x}-rail.svg").exists(): e.append(f"missing {x} rail")
if e:
 print("META APOLLO REPOSITORY GRAMMAR // v1 // FAIL"); [print(" - "+x) for x in e]; raise SystemExit(1)
print("META APOLLO REPOSITORY GRAMMAR // v1 // PASS")
