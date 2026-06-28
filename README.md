# SNHU CS-330: Computational Graphics and Visualization Portfolio

This is my academic portfolio for **CS-330: Computational Graphics and Visualization** at Southern New Hampshire University. The main skills this repo showcases are my work in computer graphics, spanning modern real-time 3D rendering (using modern OpenGL, C++, and custom GLSL shaders) and interactive 2D physics simulations (using legacy OpenGL).

The highlights of this portfolio are:
1. **Modern 3D Desk Scene (Final Project)**: A fully textured, multi-light, interactive 3D scene utilizing custom shaders, Phong illumination, and advanced planar shadow projections.
2. **Interactive 2D Brick Simulation (Module 8)**: An interactive 2D breakout-style physics demonstration featuring collision detection and dynamic object spawning.

---

## Project 1: Real-Time 3D Desk Scene (Final Project)

The core deliverable of this course is a real-time interactive 3D application that replicates a 2D photograph of a desk setup. The application is built in C++ using **OpenGL 3.3/4.6 Core Profile** (managed by GLEW and GLFW) and utilizes GLSL shaders for custom rendering, lighting, and shadow generation.

### 1. 3D Object Representation and Mesh Hierarchy
The scene represents a realistic desk layout constructed from low-polygon geometric primitives (connecting vertices into triangles and indexing them for performance):
*   **The Desk (Plane)**: A large horizontal plane textured with a repeating rustic wood material.
*   **The Coffee Mug (Cylinder & Torus)**: A complex object composed of two combined primitive shapes:
    *   A cylinder representing the body of the mug.
    *   A half-torus rotated -90 degrees on the Z-axis and translated to form the handle.
*   **The Coaster (Box)**: A flat box rotated slightly on the Y-axis to replicate organic placement.
*   **The Marble Ball (Sphere)**: A sphere resting on top of the coaster, simulating glass material properties.
*   **The Notebook (Box)**: A larger rectangular volume rotated at a -25 degree angle on the Y-axis.
*   **The Pencil (Cylinder & Cone)**: A compound model combining:
    *   A long, thin cylinder for the pencil body.
    *   A cone rotated and placed at the end to represent the sharpened tip.

### 2. Phong Shading Model and Custom Materials
All rendering is performed using the **Phong Shading Model**, implemented in the custom GLSL fragment shader. It calculates color based on three light components:
1.  **Ambient**: Simulates indirect background light scattering ($I_{ambient} = \text{light.ambient} \times \text{material.ambient}$).
2.  **Diffuse**: Simulates directional light reflection on rough surfaces using the dot product of the surface normal and the light direction vector ($I_{diffuse} = \max(\vec{N} \cdot \vec{L}, 0.0) \times \text{light.diffuse} \times \text{material.diffuse}$).
3.  **Specular**: Simulates the shiny highlights on reflective surfaces using the dot product of the camera view direction and the light reflection vector ($I_{specular} = \text{intensity} \times (\max(\vec{V} \cdot \vec{R}, 0.0))^{\text{shininess}} \times \text{light.specular} \times \text{material.specular}$).

#### Object Materials Configuration
The application defines a custom struct for object materials. Key material characteristics include:
*   **Stainless Steel (Mug)**: High specularity, low diffuse absorption, and a moderate shininess rating to simulate brushed metal.
*   **Glass (Marble Ball)**: High shininess ($85.0$), high specularity, and soft ambient strength.
*   **Wood (Desk)**: Low shininess ($0.3$), low specularity, and strong diffuse response.
*   **Tile (Coaster)**: High shininess ($25.0$) with specular highlights.
*   **Clay (Notebook & Pencil Tip)**: Matte finish with extremely low specularity ($0.3$ - $0.5$).
*   **Gold (Pencil Body)**: Balanced metallic yellow/gold color coefficients with high reflectivity.

### 3. Triple Light Source Setup
To achieve depth, color contrast, and realism, the scene utilizes a three-point lighting system:
*   **Light 0 (Key Light - Front Right)**: A strong white/gray light source positioned at `(5.0f, 4.0f, 5.0f)` that acts as the primary light source and drives the planar shadows.
*   **Light 1 (Fill Light - Front Left)**: A cool blue light source located at `(-5.0f, 3.0f, 4.0f)` with a low specular intensity, providing ambient color and filling in dark areas with a subtle blue hue.
*   **Light 2 (Rim/Back Light - Behind)**: A bright purple light located at `(0.0f, 4.0f, -4.0f)` that highlights the silhouettes of the objects, separating them from the black background.

### 4. Planar Shadow Projections
To render shadows dynamically without the overhead of shadow maps, the project implements a **parallel projection shadow matrix**.
*   **Shadow Matrix Formulation**: The shadow matrix projects the vertices of each model along the key light's direction vector onto the flat desk plane ($y = 0.0$).
    $$\text{shadowMat} = \begin{bmatrix} 1.0 & 0.0 & 0.0 & 0.0 \\ -l_x/l_y & 0.0 & -l_z/l_y & 0.0 \\ 0.0 & 0.0 & 1.0 & 0.0 \\ 0.0 & 0.0 & 0.0 & 1.0 \end{bmatrix}$$
*   **Z-Fighting Mitigation**: To prevent the projected shadow geometry from clipping through the desk geometry (which causes flickering artifacts), the shadow coordinate is offset slightly upward by translating it by $y = 0.005$.
*   **Depth Testing Bypass**: Depth testing is temporarily disabled (`glDisable(GL_DEPTH_TEST)`) while drawing the semi-transparent black shadow overlays (45% opacity) and re-enabled immediately after to ensure shadow geometry renders cleanly without depth-sorting errors.

### 5. 3D Flight Camera and View Projections
The camera system allows the user to explore the scene with full six-degrees-of-freedom flight camera physics:
*   **WASD Translation**: `W` (forward), `S` (backward), `A` (left), and `D` (right) move the camera along the local camera axes.
*   **QE Vertical Control**: `Q` moves the camera straight up, and `E` moves it straight down.
*   **Mouse Look (Pitch & Yaw)**: Moving the mouse rotates the camera. Pitch (looking up/down) is capped at $-89.0^\circ$ to $89.0^\circ$ to prevent gimbal lock.
*   **Mouse Scroll Speed Controller**: Scrolling the mouse wheel increases or decreases the translation speed multiplier of the camera.
*   **Projection Toggle**:
    *   Pressing **`P`** activates **Perspective Projection** (calculating the 3D depth perspective matrix with a dynamic Field of View).
    *   Pressing **`O`** activates **Orthographic Projection** (producing a flat, 2D technical layout rendering of the objects, scaled proportionally to the window aspect ratio to prevent stretching).

---

## Project 2: 2D Interactive Physics Simulation (Module 8)

The Module 8 assignment demonstrates legacy OpenGL 2.0 graphics, basic physics, and dynamic collision handling.

### Key Features
*   **Dynamic Spawn Engine**: Pressing the `Spacebar` instantiates a new ball entity at the origin `(0.0, 0.0)` with a randomized RGB color and a randomized initial heading vector.
*   **Brick Classification**:
    *   *Reflective Bricks (Yellow/Orange)*: Behave as static physical boundaries. When a ball overlaps with them, it rebounds in a new random direction.
    *   *Destructible Bricks (Green/Cyan)*: Act as breakable targets. Upon collision, the brick's visibility flag is toggled to `OFF`, and it is removed from the active simulation space.
*   **Collision System**: Uses Axis-Aligned Bounding Box (AABB) checking to detect overlapping boundaries between circular ball coordinates and rectangular brick bounds.

---

## Build and Run Instructions

### Prerequisites
*   **Operating System**: Windows 10/11
*   **IDE**: Microsoft Visual Studio 2022
*   **C++ Compiler**: MSVC (C++17 or later recommended)
*   **Required Libraries**:
    *   **GLFW** (Windowing and Input events handling)
    *   **GLEW** (OpenGL extension loading)
    *   **GLM** (OpenGL Mathematics for vectors and matrices)
    *   **stb_image.h** (Single-header library for texture loading)

### Setup in Visual Studio
1.  Open the Visual Studio Solution file (`.sln`) for the desired project (e.g., [7-1_FinalProjectMilestones.sln](file:///c:/Users/thera/Documents/SNHU/CS-330/CS330Content/Projects/7-1_FinalProjectMilestones/7-1_FinalProjectMilestones.sln)).
2.  Ensure that the project configurations are set to **Debug** or **Release** and targeted to the **x64** platform.
3.  Ensure the Include Directories and Library Directories are pointed correctly to the `Libraries` folder within `CS330Content`:
    *   *Additional Include Directories*: `$(SolutionDir)..\..\Libraries\includes`
    *   *Additional Library Directories*: `$(SolutionDir)..\..\Libraries\lib`
    *   *Linker Input Dependencies*: Add `opengl32.lib`, `glfw3.lib`, and `glew32s.lib` (or `glew32.lib`).
4.  Copy the `glew32.dll` to the directory containing the compiled executable (`Debug` or `Release` directory) if dynamic linking is used.
5.  Build the solution (`Ctrl + Shift + B`) and run the application (`F5`).

---

## SNHU CS-330 Reflections

### 1. How do you approach designing 3D scenes?
Designing a 3D scene starts with an analysis of reference material. I break down real-world elements into basic geometric shapes (primitives). For example, a coffee mug is simplified into a cylinder for the liquid container and a torus for the handle. 
After establishing the geometry, I organize the transformation hierarchy:
1.  **Scaling**: Sizing primitives relative to each other so objects maintain realistic proportions.
2.  **Rotation**: Aligning meshes (e.g., tipping a cone horizontally to form a pencil tip or flipping a half-torus upright for a mug handle).
3.  **Translation**: Placing the objects at coordinate offsets ($X, Y, Z$) relative to the center origin.

Next, I select texture maps with appropriate resolutions (1024x1024 or higher) to avoid pixelation and define material properties to determine light interactions. Highly specular textures represent metals and glass, while rough, matte textures are used for wood and clay.

### 2. How can a user navigate your 3D scene?
The camera system uses a first-person fly camera paradigm:
*   **Movement Keys (`W`, `S`, `A`, `D`, `Q`, `E`)**: Move the camera forward, backward, left, right, up, and down.
*   **Mouse Interaction**: Controls look orientation (yaw and pitch). The mouse movement calculates angular offsets to reorient the camera's front vector.
*   **Mouse Scroll**: Scrolling controls the camera speed, allowing for faster navigation or slower, more precise inspections.
*   **Projection Toggle**: The user can toggle between **Perspective View (`P`)** for immersive 3D and **Orthographic View (`O`)** for flat, technical 2D projections.

### 3. Explain the custom functions in your program that promote modularity.
The project isolates rendering and window management into dedicated classes, exposing clean APIs:
*   `CreateGLTexture(const char* filename, std::string tag)`: Standardizes image loading, binds the texture unit, defines filtering parameters (`GL_LINEAR`), sets coordinate wrapping (`GL_REPEAT`), and generates mipmaps. The loaded texture is stored in a map-like structure with an associated string `tag` for easy referencing.
*   `SetTransformations(glm::vec3 scaleXYZ, float Xrot, float Yrot, float Zrot, glm::vec3 pos)`: Encapsulates matrix algebra. It creates the model matrix ($M = T \times R_x \times R_y \times R_z \times S$), and sends it directly to the active GLSL shader program.
*   `SetupSceneLights()`: Configures light uniforms in the shader, passing arrays of struct variables containing positions, diffuse colors, and specular parameters.
*   `ProcessKeyboardEvents()` & `Mouse_Position_Callback()`: Isolate user inputs from the main loop, updating camera coordinates using delta time to ensure consistent camera speeds regardless of system framerates.

### 4. How has computer graphics changed your view of software development?
Working with OpenGL has changed my perspective on software development in several ways:
*   **Hardware and Driver Interaction**: Unlike typical high-level programming where hardware is abstracted, graphics development requires direct communication with the GPU. I must structure data in VBOs (Vertex Buffer Objects) and VAOs (Vertex Array Objects) to minimize CPU-to-GPU bandwidth bottlenecks.
*   **High Performance and Real-Time Loop**: Game engines and graphics applications must render frames within 16.67 milliseconds to maintain 60 frames per second. This places optimization at the forefront of code design, prompting a shift toward data-oriented design.
*   **Applied Mathematics**: Concepts in linear algebra (quaternions, dot products for diffuse light calculation, and cross products to compute camera vectors) are no longer theoretical; they are directly responsible for the visuals on the screen. Writing shader code (GLSL) highlights how hardware-parallelized programming can perform millions of vector operations per second.
