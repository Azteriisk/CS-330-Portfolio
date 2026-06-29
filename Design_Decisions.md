# CS-330: 3D Scene Design Decisions Document
**Author:** Alec Brandt  
**Project:** 3D Desktop Workspace Scene Replication  
**Course:** SNHU CS-330 Computer Graphics and Visualization  

---

## 1. Scene Overview & Design Proposal
The goal of this project is to replicate a modern, minimalist desktop workspace in a fully realized 3D scene. Replicating a workspace is a good graphics development challenge in my opinion because it incorporates a diverse blend of primitive geometries (planes, boxes, cylinders, and tori) arranged in a structured layout. 

The objects chosen for replication include:
- **The Desk (Plane)**: Serves as the ground surface and boundary for the objects.
- **The Coffee Mug (Cylinder & Torus)**: A cylindrical container with a half-torus handle.
- **The Coaster (Box)**: A flat box rotated slightly to simulate natural placement.
- **The Marble Ball (Sphere)**: A glass sphere resting on the coaster.
- **The Notebook (Box)**: A rectangular volume acting as a book.
- **The Pencil (Cylinder & Cone)**: A long cylinder body with a conical tip.

---

## 2. Shape Breakdown & Primitive Selections
To build the 3D models, complex real-world objects were decomposed into basic geometric primitives:

1. **Desk**: 
   - *Primitive:* 2D Plane.
   - *Rationale:* A single horizontal plane is mathematically ideal for simulating a flat, non-deformable desk surface.
2. **Coffee Mug**:
   - *Primitives:* 3D Cylinder (body) and 3D Half-Torus (handle).
   - *Rationale:* The liquid container is perfectly represented by a cylinder. The handle requires curved geometry which is best approximated by a torus rotated and translated to attach to the cylinder's side.
3. **Coaster**:
   - *Primitive:* 3D Box (rectangular prism).
   - *Rationale:* A low-profile rectangular box provides the flat supporting base for the marble ball.
4. **Marble Ball**:
   - *Primitive:* 3D Sphere.
   - *Rationale:* A perfect sphere is used to simulate a glass marble, allowing light to diffuse and reflect equally in all directions.
5. **Notebook**:
   - *Primitive:* 3D Box.
   - *Rationale:* A rectangular prism represents the thickness, width, and length of a closed notebook.
6. **Pencil**:
   - *Primitives:* 3D Cylinder (shaft) and 3D Cone (tip).
   - *Rationale:* The lead/wooden shaft is represented by a thin cylinder. The sharpened point is represented by a cone rotated 90 degrees and joined to the cylinder's end.

---

## 3. Coordinate System & Transformation Hierarchy
Objects are transformed relative to the world coordinate system ($X, Y, Z$) to establish realistic proportions and placement:

- **Scaling**: Sizing is defined proportionally. The desk plane is scaled to $20 \times 1 \times 10$, establishing a large workspace. The mug is scaled to $1.5 \times 3 \times 1.5$ to sit naturally. The pencil is extremely thin ($0.06 \times 1.5 \times 0.06$).
- **Rotation**: Rotations align objects to look organic. The coaster is rotated $15.0^\circ$ on the Y-axis. The notebook is rotated $-25.0^\circ$ on the Y-axis. The pencil is rotated $90.0^\circ$ on the X-axis to lie flat on the desk, and its tip is rotated $-90.0^\circ$ to point forward.
- **Translation**: Positions place objects on top of the desk ($y = 0.0$). The mug sits at the origin `(0.0, 0.0, 0.0)`. The coaster is translated to `(-3.0, 0.0, -1.0)`. The marble ball sits on top of the coaster at `(-3.0, 0.8, -1.0)`.

---

## 4. Lighting & Material Configurations
The scene implements the **Phong Shading Model** using custom GLSL shaders. Three distinct light sources (a three-point lighting system) are configured:

1. **Light 0 (Key Light - Front Right)**:
   - *Position:* `(5.0, 4.0, 5.0)`
   - *Color:* Strong white/gray `(0.7, 0.7, 0.7)`
   - *Role:* Acts as the primary light source to illuminate the front-right of the objects and cast planar shadows.
2. **Light 1 (Fill Light - Front Left)**:
   - *Position:* `(-5.0, 3.0, 4.0)`
   - *Color:* Subtle blue `(0.1, 0.2, 0.6)`
   - *Role:* Fills in the shadows on the left side of the objects with a soft, cool color, reducing harsh contrast.
3. **Light 2 (Rim/Back Light - Behind)**:
   - *Position:* `(0.0, 4.0, -4.0)`
   - *Color:* Bright purple `(0.9, 0.0, 0.9)`
   - *Role:* Highlights the silhouettes of the objects, separating them from the dark background.

### Material Properties:
Each object has custom material coefficients configured to simulate realistic textures:
- **Stainless Steel (Mug)**: High specular reflectivity and moderate shininess.
- **Glass (Marble)**: Extremely high shininess ($85.0$) and high specularity to produce a glass look.
- **Wood (Desk)**: High diffuse reflection with low specularity ($0.3$) and low shininess to look rough.
- **Tile (Coaster)**: Balanced specular highlights ($25.0$) for a semi-polished tile surface.
- **Clay (Notebook & Pencil)**: Low specularity ($0.3$ - $0.5$) with a matte finish.

---

## 5. Texturing Strategy
High-resolution images are mapped to the vertices using UV coordinates:
- **Wrapping:** `GL_REPEAT` is configured on both S and T coordinates to tile the wood grain on the desk.
- **Filtering:** `GL_LINEAR` is used for minification and magnification to ensure smooth interpolation when the camera zooms in or out.
- **Mipmaps:** Mipmapping (`glGenerateMipmap`) is enabled to prevent aliasing artifacts at steep viewing angles or far distances.

---

## 6. Dynamic Shadows Implementation
To render shadows dynamically without the overhead of shadow maps, a **parallel planar shadow projection matrix** is calculated based on Key Light 0:
- The vertices of the objects are projected along the light vector onto the desk plane ($y = 0.0$).
- **Z-Fighting Mitigation:** The shadow models are translated slightly upward by $y = 0.005$ to prevent rendering overlapping pixels with the desk plane (avoiding flickering).
- **Depth Testing Bypass:** `glDisable(GL_DEPTH_TEST)` is called while drawing the semi-transparent black shadow overlays ($45\%$ opacity) to ensure the shadows render cleanly, and re-enabled immediately after.
