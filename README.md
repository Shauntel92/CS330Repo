# CS330Repo
Git repository for CS-330 projects.
1. How do I approach designing software?
What new design skills has your work on the project helped you to craft?
What design process did you follow for your project work?
How could tactics from your design approach be applied in future work?

This project strengthened my ability to design from a reference and translate it into a modular, reproducible 3D scene. I began with a clear visual target, a cozy winter house, and decomposed it into primitives (boxes for the body, frames, and porch; prisms/boxes for the roof planes; a cylinder/box for the chimney; planes for backdrop and ground). From there, I defined a scene palette and material tags so that aesthetic decisions could be tuned independently of geometry. The design process followed an outside-in loop: start with the final look, sketch the composition, then formalize the data flow between CPU and shaders. This deliberately separated responsibilities, e.g. DefineObjectMaterials, SetupSceneLights, PrepareScene, and RenderScene, so I could iterate on lighting without touching geometry, or adjust UV tiling without touching shader math. These tactics generalize well: on future projects I will continue to specify interfaces first, then fill in implementation details, which scales to larger codebases and teams.

2. How do I approach developing programs?
What new development strategies did you use while working on your 3D scene?
How did iteration factor into your development?
How has your approach to developing code evolved throughout the milestones, which led you to the project’s completion?

On the development side, I leaned on disciplined iteration and targeted debugging. Early rendering issues—flat white output, then a black screen—were traced to mismatched uniform names and toggles between C++ and GLSL. I resolved these by auditing the full data path and writing small “probes” via temporarily forcing bUseLighting=false, using solid colors, and varying one parameter at a time, to confirm each stage. I adopted practical texture strategies—mipmaps and linear mipmap filtering for stability in motion, explicit UV tiling per object to avoid stretching—and framed movement with frame-rate independence. Camera navigation was implemented with WASD/QE translation, mouse yaw/pitch, scroll-controlled speed, and a keyboard toggle between perspective and orthographic projections, which improved both usability and evaluation. Over the milestones my approach evolved from getting shapes on screen to building a maintainable mini-engine: reusable helpers for transforms and materials, a consistent lighting API, and clear separation of scene preparation versus per-frame rendering.

3. How can computer science help me in reaching my goals?
How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future educational pathway?
How do computational graphics and visualizations give you new knowledge and skills that can be applied in your future professional pathway?

Working through computational graphics deepened skills that are directly transferable to my educational and professional path. I gained practical knowledge of real-time rendering pipelines—coordinate transforms, camera models, material systems, and Phong lighting—that I can extend with shadows, normal maps, or PBR when needed. The emphasis on modularity and clean interfaces maps to software-engineering best practices in any domain, while the interactivity requirements  mirror the constraints of modern applications. In my future coursework and in professional settings such as Healthcare IT, these capabilities translate to building interactive viewers and simulations where clarity, responsiveness, and reproducibility are critical, e.g. explorable visualizations of data or devices. This project provides a strong foundation to expand into more advanced rendering techniques and more robust architectures, and to port concepts to other APIs or platforms when requirements grow.
