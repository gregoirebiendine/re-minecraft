FILE = "zombie"

with open(f"{FILE}.obj", "r") as f:
    lines = f.readlines()

    vertices: list[str] = []
    uvs: list[str] = []
    normals: list[str] = []
    res: list[str] = []

    for line in lines:
        if line.startswith("v "):
            vertices.append(line.replace("v ", "").strip().replace(" ", ","))

        elif line.startswith("vt "):
            parts = line.replace("vt ", "").strip().split(" ")
            # Flip V: OBJ has V=0 at bottom, engine expects V=0 at top
            uvs.append(f"{parts[0]},{1.0 - float(parts[1])}")

        elif line.startswith("vn "):
            normals.append(line.replace("vn ", "").strip().replace(" ", ","))

        elif line.startswith("f "):
            faces = line.replace("f ", "").strip().split(" ")
            indicesList = [list(map(int, i.split("/"))) for i in faces]
            for indices in indicesList:
                res.append("{" + "{{{v}}}, {{{n}}}, {{{uv}}}, texId".format(v = vertices[indices[0] - 1], uv = uvs[indices[1] - 1], n = normals[indices[2] - 1]) + "}")
        
    with open(f"{FILE}.ffm", "w") as out:
        out.write(",\n".join(res))