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
*   **Glass (Marble Ball)**: High shininess (85.0), high specularity, and soft ambient strength.
*   **Wood (Desk)**: Low shininess (0.3), low specularity, and strong diffuse response.
*   **Tile (Coaster)**: High shininess (25.0) with specular highlights.
*   **Clay (Notebook & Pencil Tip)**: Matte finish with extremely low specularity (0.3 - 0.5).
*   **Gold (Pencil Body)**: Balanced metallic yellow/gold color coefficients with high reflectivity.

### 3. Triple Light Source Setup
To achieve depth, color contrast, and realism, the scene utilizes a three-point lighting system:
*   **Light 0 (Key Light - Front Right)**: A strong white/gray light source positioned at `(5.0f, 4.0f, 5.0f)` that acts as the primary light source and drives the planar shadows.
*   **Light 1 (Fill Light - Front Left)**: A cool blue light source located at `(-5.0f, 3.0f, 4.0f)` with a low specular intensity, providing ambient color and filling in dark areas with a subtle blue hue.
*   **Light 2 (Rim/Back Light - Behind)**: A bright purple light located at `(0.0f, 4.0f, -4.0f)` that highlights the silhouettes of the objects, separating them from the black background.

### 4. Planar Shadow Projections
To render shadows dynamically without the overhead of shadow maps, the project implements a **parallel projection shadow matrix**.
*   **Shadow Matrix Formulation**: The shadow matrix projects the vertices of each model along the key light's direction vector ($L = (l_x, l_y, l_z)$) onto the flat desk plane ($y = 0.0$). The projection is defined by the matrix:
    ```
    shadowMat = [
      1.0       0.0   0.0       0.0
     -lx / ly   0.0  -lz / ly   0.0
      0.0       0.0   1.0       0.0
      0.0       0.0   0.0       1.0
    ]
    ```
*   **Z-Fighting Mitigation**: To prevent the projected shadow geometry from clipping through the desk geometry (which causes flickering artifacts), the shadow coordinate is offset slightly upward by translating it by y = 0.005.
*   **Depth Testing Bypass**: Depth testing is temporarily disabled (`glDisable(GL_DEPTH_TEST)`) while drawing the semi-transparent black shadow overlays (45% opacity) and re-enabled immediately after to ensure shadow geometry renders cleanly without depth-sorting errors.

### 5. 3D Flight Camera and View Projections
The camera system allows the user to explore the scene with full six-degrees-of-freedom flight camera physics:
*   **WASD Translation**: `W` (forward), `S` (backward), `A` (left), and `D` (right) move the camera along the local camera axes.
*   **QE Vertical Control**: `Q` moves the camera straight up, and `E` moves it straight down.
*   **Mouse Look (Pitch & Yaw)**: Moving the mouse rotates the camera. Pitch (looking up/down) is capped at -89.0 to 89.0 degrees to prevent gimbal lock.
*   **Mouse Scroll Speed Controller**: Scrolling the mouse wheel increases or decreases the translation speed multiplier of the camera.
*   **Projection Toggle**:
    *   Pressing **`P`** activates **Perspective Projection** (calculating the 3D depth perspective matrix with a dynamic Field of View).
    *   Pressing **`O`** activates **Orthographic Projection** (producing a flat, 2D technical layout rendering of the objects, scaled proportionally to the window aspect ratio to prevent stretching).

---

## Project 2: 2D Interactive Physics Simulation (Module 8)

The Module 8 assignment demonstrates legacy OpenGL 2.0 graphics, basic physics, dynamic collision handling, and interactive player input.

### Key Features
*   **Dynamic Spawn Engine (Debounced)**: Pressing the `Spacebar` instantiates a new ball entity at `(0.0, -0.5)` moving upwards at a random angle (between 45 and 135 degrees) with randomized RGB colors. The input is debounced to ensure only one ball spawns per press.
*   **Interactive Player Paddle**: Added a keyboard-controlled paddle at the bottom of the screen (moves left and right using `A`/`D` or the Left/Right Arrow keys). The paddle is bounded to prevent leaving the screen.
*   **Brick Classification & State Machine**:
    *   *Reflective Bumpers (Blue/Gray)*: Static boundaries placed on the sides to redirect balls upon collision.
    *   *Destructible Bricks (Green/Yellow/Red)*: Multi-hit targets positioned at the top in rows. Each destructible brick has a durability of 3 hits. They change color based on their health (Green for 3 hits, Yellow for 2 hits, Red for 1 hit) and display dynamic crack overlays as they take damage before being removed (`OFF`).
    *   *Player Paddle (Gray)*: A dynamic reflector controlled by the user. Bounces are calculated based on where the ball strikes the paddle relative to its center, allowing the user to direct the angle of rebound.
*   **Advanced Physics & Collision Systems**:
    *   *Ball-to-Brick Collisions*: Resolves overlap by pushing the ball outside the brick's boundary and reflects the velocity vector mathematically (`V' = V - 2 * dot(V, N) * N`) upon hitting a surface.
    *   *Ball-to-Ball Elastic Collisions*: Simulates real-time momentum exchange between active balls. Resolves overlaps and performs elastic collision bounces, updating colors on impact.
    *   *Boundary Cleanup*: Any ball that falls past the bottom boundary (missed by the paddle) is automatically erased from the simulation to manage memory.

---

## Build and Run Instructions

### Prerequisites
*   **Operating System**: Windows 10/11
*   **IDE**: Microsoft Visual Studio 2022
*   **C++ Compiler**: MSVC (C++17 or later recommended)
*   **Required Libraries** (All included locally in the project):
    *   **GLFW** (Windowing and Input events handling)
    *   **GLEW** (OpenGL extension loading)
    *   **GLM** (OpenGL Mathematics for vectors and matrices)
    *   **stb_image.h** (Single-header library for texture loading)

### Setup in Visual Studio
1.  Open the Visual Studio Solution file (`.sln`) for the desired project (e.g., [3D_Scene.sln](file:///c:/Users/thera/Documents/SNHU/CS-330/CS-330-Portfolio/3D_Scene/3D_Scene.sln) or [2D_Simulation.sln](file:///c:/Users/thera/Documents/SNHU/CS-330/CS-330-Portfolio/2D_Simulation/2D_Simulation.sln)).
2.  Ensure that the configuration is set to **Debug** or **Release** and targeted to the **Win32** (x86) platform for the 3D Scene.
3.  The project is pre-configured to be fully self-contained using relative paths to the local `Libraries` folder, so no manual include or library directory setups are required.
4.  Build the solution (`Ctrl + Shift + B`) and run the application (`F5`).

---

## SNHU CS-330 Reflections (Portfolio Journal)

### 1. How do I approach designing software?
- **New Design Skills Crafted**: Working on these graphics projects helped me build advanced spatial decomposition skills. Instead of treating a complex 3D object as a single mesh, I learned to break it down into compound primitive geometries (such as a cylinder body and a torus handle for a mug, or a cylinder shaft and conical tip for a pencil). Additionally, it enhanced my ability to design object-oriented graphics systems with clean separation of concerns, dividing tasks among specialized components like `ViewManager`, `SceneManager`, and `ShaderManager`.
- **Design Process Followed**: I followed a structured, iterative design process. I began by analyzing 2D reference images, identifying and scaling geometric primitives, defining relative translation and rotation offsets, and establishing a transformation hierarchy (Scale $\rightarrow$ Rotate $\rightarrow$ Translate). I then designed Phong illumination models (ambient, diffuse, and specular light behaviors) and aligned texture maps using appropriate filtering and wrapping parameters.
- **Applying Tactics in Future Work**: Decomposing complex requirements into small, modular primitives is directly applicable to general software architecture. Breaking large software programs into independent, single-responsibility units makes them more maintainable, easier to test, and simpler to refactor.

### 2. How do I approach developing programs?
- **New Development Strategies**: I utilized incremental implementation and proactive debugging strategies. I tested each 3D primitive individually to verify geometry and texture coordinates before layering lighting variables or planar shadows. I also implemented static type verification to resolve compiler type-truncation and comparison warnings, ensuring clean code execution.
- **Role of Iteration in Development**: Iteration was vital in refining the physics and visuals. For example, rendering realistic shadows required multiple test-and-adjust cycles to tweak translation offsets ($y = 0.005$) and prevent z-fighting clipping. Similarly, the 2D physics simulation evolved iteratively from simple discrete movements to continuous velocity calculations and elastic ball-to-ball momentum exchanges.
- **Evolution of Coding Approach**: Throughout the milestones, my coding approach evolved from writing tightly coupled monolithic code to producing modular, decoupled structures. Abstracting the camera physics and window events into dedicated classes simplified the main game loop and established a professional, scaleable architecture.

### 3. How can computer science help me in reaching my goals?
- **Educational Pathway**: Understanding computational graphics and real-time simulations bridges the gap between high-level language abstractions and hardware-level performance. It cements concepts in linear algebra (vector math, dot products for lighting, cross products for camera vectors, and transformation matrices) that are essential for future coursework in computer graphics, game engine development, and scientific computing.
- **Professional Pathway**: Graphics programming requires writing code optimized for strict frame-rate windows (e.g., rendering within 16.67ms for 60 FPS). This focus on resource efficiency, memory layouts, CPU-to-GPU bandwidth, and cache locality is directly transferable to high-performance software engineering, low-latency applications, and systems programming, helping me write optimized, professional software.
