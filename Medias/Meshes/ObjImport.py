# -*- coding: latin-1 -*-
import os

fileName = input("*.obj converter : the obj file you want to convert must be placed in the same folder as this program.\nWhat *.obj file do you want to use ? (don't type file extension) : ")
nomProjet = fileName + ".aso"
while not os.path.exists(fileName + ".obj") :
    print(fileName + ".obj doesn't exist !")
    fileName = input("What *.obj file do you want to use ? : ")

fileOrig= open(fileName + ".obj","r")
vertices= list()
normals = list()
UVs     = list()
face    = list()
dico    = dict()
faces   = list()

for line in fileOrig :
    if "v " in line :
        vertex  = (line.replace("\n", '').split(" ")[1:])
        vertices.append(vertex)
    if "vn " in line :
        normal  = (line.replace("\n", '').split(" ")[1:])
        normals.append(normal)
    if "vt " in line :
        UV      = (line.replace("\n", '').split(" ")[1:])
        UVs.append(UV)
    if "f " in line :
        temp    = (line.replace("\n", '').split(" ")[1:])
        for element in temp :
            face.append(str(element).split("/"))
        for i in range(0, 3) :
            dico[str(face[i][0])] = (face[i][1], face[i][2])
        faces.append(face)
        face = list()
fileOrig.close()

fileDest = open(fileName + ".aso", 'w')
if len(vertices) :
    vType = 0
    if len(normals) :
        vType += 1
        if len(UVs) :
            vType += 2
    elif len(UVs) :
        vType += 2
fileDest.write("vtype %i\n" % (vType))        
fileDest.write("vertices %i\n\n" % (len(vertices)))
index = 1
for vertex in vertices :
    if vType == 0 :
        fileDest.write("v %s %s %s\n" % (vertex[0], vertex[1], vertex[2]))
    elif vType == 1 :
        fileDest.write("v %s %s %s, %s %s %s\n" % (vertex[0], vertex[1], vertex[2], normals[int(dico[str(index)][1]) - 1][0], normals[int(dico[str(index)][1]) - 1][1], normals[int(dico[str(index)][1]) - 1][2]))
    elif vType == 2 :    
        fileDest.write("v %s %s %s, %s %s\n" % (vertex[0], vertex[1], vertex[2], UVs[int(dico[str(index)][0]) - 1][0], UVs[int(dico[str(index)][0]) - 1][1]))
    elif vType == 3 :
        fileDest.write("v %s %s %s, %s %s %s, %s %s\n" % (vertex[0], vertex[1], vertex[2], normals[int(dico[str(index)][1]) - 1][0], normals[int(dico[str(index)][1]) - 1][1], normals[int(dico[str(index)][1]) - 1][2], UVs[int(dico[str(index)][0]) - 1][0], UVs[int(dico[str(index)][0]) - 1][1]))
    index += 1

fileDest.write("\ntriangles %i\n" % (len(faces) * 3))
for f in faces :
    fileDest.write("t %s %s %s\n" % (str(int(f[0][0]) - 1), str(int(f[1][0]) - 1), str(int(f[2][0]) - 1)))
fileDest.close()

print("Success ! new file : " + fileName + ".aso")
