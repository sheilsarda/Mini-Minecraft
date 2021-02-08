# CIS460/560
## Mini Minecraft Milestone 1

### Sheil Sarda

#### Efficient Terrain Rendering and Chunking (Milestone 1)

I implemented efficient terrain rendering and chunking by writing the `Chunk::create()` function after making `Chunk` extend from drawable, and modifying `ShaderProgram`, `MyGL` and the Lambert shader to accomodate the interleaved VBO.

#### Multithreaded Terrain Generation (Milestone 2)

I implemented multithreaded terrain generation by disaggregating the terrain generation into 3 steps: (1) determining the `BlockType` for every block in the chunk, (2) generating VBO data (index, position, normal, UV, etc.) for each chunk, and (3) pushing chunk data into the VBO buffers. The first step is implemented in the `BlockTypeWorker` thread. Since the individual thread worker classes do not have access to the VBO, we cannot combine step (2) and (3). Step (2) is performed in the `VBOWorker` class, which takes in a pointer to a chunk, and calls the `Chunk` class `createVBOData()` function, passing in as arguments the data structures where the VBO data should get stored. The last step is performed inside the `terrainUpdate()` function of the `Terrain` class, which is called every game tick. This function copies over the chunks which have been generated into the VBO.

These operations require 2 mutexes: `chunkMutex`, which controls which thread writes their chunks after `BlockTypeWorker` finishes executing, and `vboMutex` which controls which thread writes their populated VBO data back to a common data structure to get copied onto the GPU.

#### Additional Biomes (Milestone 3)

For MS3, I implemented the following biomes:
- Desert (Sand)
- Tundra (Snow)
- Grassland (Grass and Dirt)
- Mountain (Stone, Grass and Snow)

This implementation of biomes assigns each chunk a biome randomly, and uses noise functions to interpolate the blocks on biome boundaries using distance between neighboring biomes as probabilities. There is an equal likelihood that a block located on the boundary of a biome is assigned to either side.

These biomes are generated using the multithreaded implementation I developed in the previous milestone, with some additional data structures such as a biome hash map which maps chunks to biome types (using the provided toKey method), and noise functions.

### Yichao Wang

#### Game Engine Tick Function and Player Physics (Milestone 1)

I implemented character controller by using the tick function to calculate physics (i.e, acceleration, velocity, position, and etc.). I also using the polar coordinates to implement the camera system. For the collision detection and adding/removing block, I used grid march algorithm along with the ray casting of character's box bounding.

#### Texturing and Texture Animation (Milestone 2)

I implemented texturing on blocks through openGL shaders. I splitted the interleaved VBO data in the Chunk class into two separate VBOs: one for data for all opaque blocks and one for data for all transparent blocks. I also add a time variable to animate textures such as WATER and LAVA by set UV offset.

#### Day and night cycle (Milestone 3)

I implemented raycast to create a skybox. The sky has different sunset and dusk color based on a hard-code palette. The sky have cloud effect based on Worley and fbm noise. The position of the sun and sky color will change over time. The light effect also applies to the lambert shader.


### Hanzhang Wu

#### Procedural Terrain (Milestone 1)

I implemented the procedural terrain part, and the code is mostly in terrain.cpp. For the first noise function, I wrote a function to generate Perlin noise and another function to get a fractal of it. I used this function to generate grassland. For the second noise function, I wrote a function to get fbm and another one to generate varonoi-like effect. I combined them with 2:1 ratio. I used the second noise function to generate mountains. Then, I used smoothstep on a Perlin noise to interpolate between the two biomes(grassland and mountain). Based on the third noise, I drew grassland in some areas, and mountains in some other areas which are surrounded by grassland. Lastly, I added a new blocktype SNOW in chunk, which is shown at the top of the mountain.

#### L-System Rivers (Milestone 2)
I implemented l-system rivers in river.cpp/.h, terrain.cpp/.h and blocktypeworker.cpp/.h. The turtle has probability of 0.9 to create a new branch, and the new orientation is randomized. Instead of using a stack of turtles I used a stack of glm::vec4 to store the information of turtles. For the second part, when a player is in WATER, we can hold the space bar to make him/her move upwards very slowly. When a player is in WATER, the sky becomes blue.
I still had difficulties carving out terrains and implementing the post-process pipelines.

#### Caves (Milestone 3)
I implemented caves in terrain.cpp/.h and noisefunctions.cpp/.h. I used 3D Perlin noise functions to generate a cave in the mountain, which can be seen as the player moves up and sees the snow. It is obvious that there are stalactites, and as the player goes deep into the cave there is a pool full of lava. You can also see the ore blocks in the walls that surround the cave.
Also, now the terrain gradually slopes down to the l-system river. However, I am still unable to fully implement the post-process pipeline.
