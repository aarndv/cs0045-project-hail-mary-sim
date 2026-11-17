# OpenGL, GLUT, GLEW & VBO Complete API Syntax Reference Guide (Modules 01 – 08)

This reference guide provides a categorized API syntax list extracted from Modules 01 through 08 of **Introduction to Computer Graphics and Visual Computing**.

---

## 1. Program Initialization & Event Loop (GLUT & GLEW)

Functions for window lifecycle management, event callbacks, and library initialization.

| Function / Macro Syntax | Description / Parameters | Module |
| :--- | :--- | :--- |
| `glutInit(&argc, argv)` | Initializes the GLUT library. Should be called before any other GLUT routines. | Mod 01, 03 |
| `glutInitDisplayMode(unsigned int mode)` | Configures display buffer modes (`GLUT_RGBA`, `GLUT_DOUBLE`, `GLUT_SINGLE`, `GLUT_DEPTH`). | Mod 03, 07, 08 |
| `glutInitWindowSize(int width, int height)` | Sets the initial window width and height in pixels. | Mod 03, 04, 05 |
| `glutInitWindowPosition(int x, int y)` | Sets the initial top-left window position in screen pixels. | Mod 03, 04, 05 |
| `glutCreateWindow(const char *title)` | Creates a top-level window with the specified title string. | Mod 01, 03 |
| `glewInit()` | Initializes extension function pointers via GLEW. Returns `GLEW_OK` on success. | Mod 05, 06 |
| `glutDisplayFunc(void (*func)(void))` | Sets the display callback for redrawing the active window. | Mod 01, 03 |
| `glutKeyboardFunc(void (*func)(unsigned char key, int x, int y))` | Sets the keyboard event callback function. | Mod 03, 05 |
| `glutTimerFunc(unsigned int msecs, void (*func)(int value), int value)` | Registers a timer callback triggered after specified milliseconds. | Mod 05, 06 |
| `glutPostRedisplay()` | Marks the current window as needing to be re-rendered. | Mod 03, 05 |
| `glutMainLoop()` | Enters the GLUT event-processing loop. Should be the final main statement. | Mod 01, 03 |

---

## 2. Immediate Mode Drawing & Primitives

Legacy CPU-bound pipeline functions for defining geometric primitives step-by-step.

| Function / Syntax | Purpose & Usage | Module |
| :--- | :--- | :--- |
| `glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)` | Sets the RGBA clear color values used by `glClear`. | Mod 01, 02 |
| `glClear(GLbitfield mask)` | Clears specified buffers (`GL_COLOR_BUFFER_BIT`, `GL_DEPTH_BUFFER_BIT`). | Mod 01, 07, 08 |
| `glBegin(GLenum mode)` | Delimits the vertices of a primitive or group of primitives. | Mod 02, 04 |
| `glEnd()` | Ends vertex list specification started by `glBegin()`. | Mod 02, 04 |
| `glFlush()` | Forces execution of OpenGL commands in finite time. | Mod 01, 02, 04 |
| `glVertex2f(GLfloat x, GLfloat y)` | Specifies 2D vertex coordinates in floating-point format. | Mod 02, 04 |
| `glVertex3f(GLfloat x, GLfloat y, GLfloat z)` | Specifies 3D vertex coordinates in floating-point format. | Mod 02, 04, 07 |

### Immediate Mode Primitive Constants (`glBegin` Modes)
- `GL_POINTS`: Individual points.
- `GL_LINES`: Disjoint line segments.
- `GL_LINE_STRIP`: Connected group of line segments.
- `GL_LINE_LOOP`: Connected group of line segments with closure to start.
- `GL_TRIANGLES`: Triples of vertices treated as individual triangles.
- `GL_TRIANGLE_STRIP`: Linked strip of triangles.
- `GL_TRIANGLE_FAN`: Fan of triangles connected to a central vertex.
- `GL_QUADS`: Groups of four vertices treated as individual quadrilaterals.
- `GL_QUAD_STRIP`: Linked strip of quadrilaterals.
- `GL_POLYGON`: Simple, convex polygon.

---

## 3. Styling, Attributes & Text Utilities

Rasterization controls for line widths, patterns, color modes, and text rendering.

| Function / Constant Syntax | Description | Module |
| :--- | :--- | :--- |
| `glPointSize(GLfloat size)` | Sets the rasterized point diameter in pixels. | Mod 02 |
| `glLineWidth(GLfloat width)` | Sets the rasterized line segment width in pixels. | Mod 02 |
| `glEnable(GL_LINE_STIPPLE)` | Enables line stippling (patterned dashed lines). | Mod 02 |
| `glLineStipple(GLint factor, GLushort pattern)` | Sets 16-bit bitmask pattern and stretch factor for stippled lines. | Mod 02 |
| `glColor3f(GLfloat r, GLfloat g, GLfloat b)` | Sets current color using RGB floating-point values `[0.0, 1.0]`. | Mod 02, 04 |
| `glColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a)` | Sets current color using RGBA floating-point values including alpha. | Mod 02, 06 |
| `glColor3ub(GLubyte r, GLubyte g, GLubyte b)` | Sets current color using unsigned byte values `[0, 255]`. | Mod 02 |
| `glRasterPos2f(GLfloat x, GLfloat y)` | Sets 2D raster position for bitmap text / image operations. | Mod 03 |
| `glutBitmapCharacter(void *font, int character)` | Renders a bitmap character at the current raster position. | Mod 03 |

### GLUT Bitmap Font Constants
- `GLUT_BITMAP_8_BY_13`, `GLUT_BITMAP_9_BY_15`
- `GLUT_BITMAP_TIMES_ROMAN_10`, `GLUT_BITMAP_TIMES_ROMAN_24`
- `GLUT_BITMAP_HELVETICA_10`, `GLUT_BITMAP_HELVETICA_12`, `GLUT_BITMAP_HELVETICA_18`

---

## 4. Vertex Arrays (Client-Side Memory Management)

Batch rendering utilities using contiguous CPU memory buffers.

| Function Syntax | Parameter & Behavior Description | Module |
| :--- | :--- | :--- |
| `glEnableClientState(GLenum cap)` | Enables client-side capability (`GL_VERTEX_ARRAY`, `GL_COLOR_ARRAY`, `GL_NORMAL_ARRAY`, `GL_TEXTURE_COORD_ARRAY`). | Mod 04, 05, 08 |
| `glDisableClientState(GLenum cap)` | Disables specified client-side state capability. | Mod 04, 05, 08 |
| `glVertexPointer(size, type, stride, pointer)` | Defines an array of vertex coordinates (size: 2, 3, 4; type: `GL_FLOAT`). | Mod 04, 05, 06 |
| `glColorPointer(size, type, stride, pointer)` | Defines an array of color attributes. | Mod 04, 05, 06 |
| `glNormalPointer(type, stride, pointer)` | Defines an array of surface normal vectors. | Mod 08 |
| `glTexCoordPointer(size, type, stride, pointer)` | Defines an array of texture coordinates. | Mod 08 |
| `glDrawArrays(GLenum mode, GLint first, GLsizei count)` | Renders primitives directly from enabled array pointers. | Mod 04, 05, 06 |
| `glArrayElement(GLint i)` | Transfers a single element from enabled array index to vertex pipeline. | Mod 04, 06, 07 |

---

## 5. Vertex Buffer Objects (VBOs - Server-Side GPU Memory)

High-performance GPU video memory allocation and indexed rendering calls.

| Function / Macro Syntax | Usage & Parameter Details | Module |
| :--- | :--- | :--- |
| `glGenBuffers(GLsizei n, GLuint *buffers)` | Generates `n` buffer object names into the buffer array. | Mod 05, 06 |
| `glBindBuffer(GLenum target, GLuint buffer)` | Binds buffer object (`GL_ARRAY_BUFFER`, `GL_ELEMENT_ARRAY_BUFFER`). | Mod 05, 06 |
| `glBufferData(target, size, data, usage)` | Allocates GPU memory and transfers data (`GL_STATIC_DRAW`, `GL_DYNAMIC_DRAW`). | Mod 05, 06, 08 |
| `glBufferSubData(target, offset, size, data)` | Updates a subset of a bound buffer object's data. | Mod 06, 07, 08 |
| `glMapBuffer(GLenum target, GLenum access)` | Maps buffer data into client address space (`GL_READ_WRITE`, `GL_WRITE_ONLY`). | Mod 05, 06 |
| `glUnmapBuffer(GLenum target)` | Relinquishes client mapping of buffer data back to GPU memory. | Mod 05, 06 |
| `glDeleteBuffers(GLsizei n, const GLuint *buffers)` | Deletes `n` named buffer objects. | Mod 05, 06, 08 |
| `glDrawElements(mode, count, type, indices)` | Renders primitives using array indices from an element buffer. | Mod 05 |
| `#define BUFFER_OFFSET(i) ((void*)(i))` | Preprocessor macro converting integer byte offsets into pointer offsets. | Mod 05, 06, 08 |

---

## 6. Coordinate Systems, Transformations & Camera Viewing

Matrix stack management, geometric modifications, and 3D camera projection configurations.

| Function Syntax | Parameters & Purpose | Module |
| :--- | :--- | :--- |
| `glMatrixMode(GLenum mode)` | Specifies target matrix stack (`GL_MODELVIEW`, `GL_PROJECTION`). | Mod 07, 08 |
| `glLoadIdentity()` | Replaces the current transformation matrix with the identity matrix. | Mod 07, 08 |
| `glPushMatrix()` | Pushes the current matrix stack down by one copy. | Mod 07 |
| `glPopMatrix()` | Pops the current matrix stack, restoring prior transformation state. | Mod 07 |
| `glTranslatef(x, y, z)` / `glTranslated(x, y, z)` | Multiplies current matrix by a translation matrix along (x, y, z). | Mod 07 |
| `glRotatef(angle, x, y, z)` / `glRotated(...)` | Multiplies current matrix by a rotation matrix of `angle` degrees around (x, y, z). | Mod 07 |
| `glScalef(x, y, z)` / `glScaled(x, y, z)` | Multiplies current matrix by a general scaling matrix along (x, y, z). | Mod 07 |
| `glViewport(x, y, width, height)` | Sets the viewport rectangular region in screen coordinates. | Mod 07 |
| `gluPerspective(fovy, aspect, zNear, zFar)` | Creates a perspective projection matrix (vertical FOV angle, aspect ratio, clipping planes). | Mod 07, 08 |
| `glFrustum(left, right, bottom, top, zNear, zFar)` | Defines a 3D perspective viewing frustum explicitly. | Mod 07 |
| `glOrtho(left, right, bottom, top, zNear, zFar)` | Creates a 3D orthographic (parallel) cuboid projection matrix. | Mod 07 |
| `gluOrtho2D(left, right, bottom, top)` | Defines a 2D orthographic viewing region. | Mod 07 |
| `gluLookAt(eyeX..Z, centerX..Z, upX..Z)` | Defines a viewing transform matrix based on camera position, target center, and up vector. | Mod 07, 08 |

---

## 7. Surface Realism: Lighting, Materials & Textures

Configuring depth testing, material reflections, light source parameters, and 2D image textures.

| Function / Header Syntax | Description & Parameters | Module |
| :--- | :--- | :--- |
| `glEnable(GL_DEPTH_TEST)` | Enables depth buffer comparison testing for hidden surface removal. | Mod 07, 08 |
| `glEnable(GL_LIGHTING)` | Enables global OpenGL lighting calculations. | Mod 08 |
| `glEnable(GL_LIGHTn)` | Enables specific light index (`GL_LIGHT0`, `GL_LIGHT1`, ...). | Mod 08 |
| `glLightfv(light, pname, params)` | Sets light source array parameters (`GL_POSITION`, `GL_AMBIENT`, `GL_DIFFUSE`, `GL_SPECULAR`, `GL_SPOT_DIRECTION`). | Mod 08 |
| `glLightf(light, pname, param)` | Sets scalar light source parameters (e.g., `GL_SPOT_CUTOFF`). | Mod 08 |
| `glLightModelfv(pname, params)` | Configures global light model parameters (`GL_LIGHT_MODEL_AMBIENT`). | Mod 08 |
| `glMaterialfv(face, pname, params)` | Sets material surface properties (`GL_FRONT`, `GL_BACK`, `GL_FRONT_AND_BACK`; properties: `GL_AMBIENT`, `GL_DIFFUSE`, `GL_SPECULAR`, `GL_EMISSION`, `GL_SHININESS`). | Mod 08 |
| `glEnable(GL_TEXTURE_2D)` | Enables 2D texturing. | Mod 08 |
| `glGenTextures(GLsizei n, GLuint *textures)` | Generates texture names/handles. | Mod 08 |
| `glBindTexture(GLenum target, GLuint texture)` | Binds named texture to target (`GL_TEXTURE_2D`). | Mod 08 |
| `glTexParameteri(target, pname, param)` | Sets texture wrapping (`GL_TEXTURE_WRAP_S`, `GL_TEXTURE_WRAP_T`, `GL_REPEAT`) and filtering (`GL_TEXTURE_MIN_FILTER`, `GL_TEXTURE_MAG_FILTER`, `GL_LINEAR`, `GL_NEAREST`). | Mod 08 |
| `glTexImage2D(...)` | Specifies a 2D texture image (target, level, internalFormat, width, height, border, format, type, data). | Mod 08 |
| `glGenerateMipmap(GLenum target)` | Generates mipmaps for specified texture target. | Mod 08 |
| `glTexEnvf(target, pname, param)` | Configures texture environment parameters (`GL_TEXTURE_ENV_MODE`, `GL_MODULATE`). | Mod 08 |
| `stbi_load(filename, &w, &h, &comp, req_comp)` | Single-header image loader function from `stb_image.h`. | Mod 08 |
| `stbi_image_free(data)` | Frees memory allocated by `stbi_load`. | Mod 08 |
| `#define STB_IMAGE_IMPLEMENTATION` | Preprocessor definition required before including `"stb_image.h"`. | Mod 08 |

---

## 8. Debugging, Information & GLUT Shape Helpers

Error callback setup, system queries, and GLUT 3D solid/wireframe shape generators.

| Function / Constant Syntax | Description | Module |
| :--- | :--- | :--- |
| `glEnable(GL_DEBUG_OUTPUT)` | Enables synchronous OpenGL error and debug message reporting. | Mod 06, 07, 08 |
| `glDebugMessageCallback(func, userParam)` | Registers user error callback for diagnostic log handling. | Mod 06, 07, 08 |
| `glGetString(GLenum name)` | Queries system strings (`GL_VERSION`, `GL_VENDOR`, `GL_RENDERER`). | Mod 06, 07, 08 |
| `glutSolidSphere` / `glutWireSphere` | Renders a solid or wireframe sphere. | Mod 08 |
| `glutSolidCube` / `glutWireCube` | Renders a solid or wireframe cube. | Mod 08 |
| `glutSolidCone` / `glutWireCone` | Renders a solid or wireframe cone. | Mod 08 |
| `glutSolidTorus` / `glutWireTorus` | Renders a solid or wireframe torus. | Mod 08 |
| `glutSolidTeapot` / `glutWireTeapot` | Renders the classic Utah Teapot solid or wireframe. | Mod 08 |
| `glutSolidDodecahedron` / `glutWireDodecahedron` | Renders a solid or wireframe 12-sided polyhedron. | Mod 08 |
| `glutSolidOctahedron` / `glutWireOctahedron` | Renders a solid or wireframe 8-sided polyhedron. | Mod 08 |
| `glutSolidTetrahedron` / `glutWireTetrahedron` | Renders a solid or wireframe 4-sided polyhedron. | Mod 08 |
| `glutSolidIcosahedron` / `glutWireIcosahedron` | Renders a solid or wireframe 20-sided polyhedron. | Mod 08 |

---

## 9. OpenGL Data Types Quick Reference

Mapping of standard C/C++ fundamental types to internal OpenGL type definitions.

| OpenGL Type | Internal C / C++ Definition | Bits & Format |
| :--- | :--- | :--- |
| `GLboolean` | `unsigned char` | 8-bit boolean |
| `GLbyte` | `signed char` | 8-bit signed integer |
| `GLubyte` | `unsigned char` | 8-bit unsigned integer |
| `GLshort` | `short` | 16-bit signed integer |
| `GLushort` | `unsigned short` | 16-bit unsigned integer |
| `GLint` | `int` | 32-bit signed integer |
| `GLuint` | `unsigned int` | 32-bit unsigned integer |
| `GLsizei` | `int` | 32-bit non-negative integer size |
| `GLfloat` | `float` | 32-bit single-precision float |
| `GLclampf` | `float` | 32-bit float clamped to `[0.0, 1.0]` |
| `GLdouble` | `double` | 64-bit double-precision float |
| `GLclampd` | `double` | 64-bit double clamped to `[0.0, 1.0]` |
| `GLenum` | `unsigned int` | 32-bit enumerated binary token |
| `GLbitfield` | `unsigned int` | 32-bit bitfield mask |
| `GLintptr` | `ptrdiff_t` / `intptr_t` | Signed integer for memory offsets |
| `GLsizeiptr` | `ptrdiff_t` / `ssize_t` | Non-negative integer for memory sizes |
