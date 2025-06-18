# Watercolor Shader Project
Hello! This is a project final for my CS216 class with Kevin Smith. I wrote an implementation for a real-time watercolor style post-processing effect, mostly within GLSL shaders. Right now, the shaders are tied to working in OpenFrameworks, but with slight tweaks they could be applied to many different rendering pipelines. 

## Introduction
In ["Art-directed watercolor stylization of 3D animations in real-time"](https://www.sciencedirect.com/science/article/abs/pii/S0097849317300316) by Montesdeoca et al., they describe a post-processing pipelinebased on the physical attributes of the paint to render a scene/image in a watercolor style. The various stages of the pipeline that replicate various physical properties such as color bleeding, edge darkening, and pigment turbulence can be controlled using the vertex colors of the geometry - giving individual artists the ability to express unique styles within this single pipeline.

## Implementation
Ideally, I wanted to implement this system within a game engine such as Unreal Engine so that I could stress test its effectiveness inside scenes with complex geometry. However, in the interest of submitting my project on time, I opted to use a framework that was more familiar to me so that I could focus on writing the shaders: [OpenFrameworks](https://openframeworks.cc/).  

### Summary
For a high level overview of the system, the paper provides the following diagram:   
![Pipeline Diagram](WatercolorPipeline.png)  
#### First Pass
The final style consists of a series of shaders that emulate different traits of the physical paint medium. In our first pass, we physically offset the position of our vertices to simulate errors from hand tremors (Vertex Shader) and perform basic shading calculations. In "MRT Pixel Shader" is our first example of using our vertex colors to direct the output of the pipeline, where we use the blue channel to direct the *turbulence* of the pigment, which can be thought of as noise. 
![Result of First Pass](./exampleimages/base_scene.png) ![Vertex Color Image](./exampleimages/vertex_colors.png)  
On the left is the scene output, and on the right is an image containing the vertex colors of that output. The alpha channel contains the values of a noise texture, which is used to alter the pass output.   
#### Intermediate Image Creation
In our intermediate phase, we aim to create three images that we use to create our final image. 
First, there is a simple gaussian blur of our scene output using a 5x5 kernel. More interestingly, the second image is a bleed algorithm as described by the paper. Simple bleeding doesn't accurately portray the properties of paint when wet & dry paint interact with one another in different layers. For example, in a normal bleed algorithm, a "dry" background will end up bleeding into the newer "wet" foreground. The "4D joint-bilateral bleed algorithm" amends this to allow artists to designate colors should be bled according to their depth in the image. This means that the designated bleed areas (vertex color blue channel) should be allowed to bleed, but the ground plane located in the background should not bleed into the spheres.   
Our final image is a texture that compresses a paper's diffuse and normal maps into a single image. The xy vectors of the normal are stored in the rg channels, and the diffuse is used as a height value stored in the b channel. 
![Result of gaussian pass](./exampleimages/gauss_blur.png)![Result of bleed pass](./exampleimages/intermediate_bleed.png)  ![Result of paper pass](./exampleimages/paper_output.png)
