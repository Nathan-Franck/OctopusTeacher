
# Octopus Teacher

![image](CustomContent/OctopusTeacherIcon_Small.png)
![image](Content/logo_small.png)
### Forked From [Wicked Engine](https://github.com/turanszkij/WickedEngine)

## Objectives

* Provide myself a crash-course in modern C++ programming techniques and the latest and greatest in rendering
* Make a cool little octopus demo

## Progress Milestones
### 8/28/2021
* Due to how models were imported, the main transform of the octopus model was being given a `local_scale.z` of -1, which aggrevated the quaterion math for deriving the orientation of the tentacle needed to reach the target. Now fixed using matrix math fully, works when octopus is in any orientation!

![image](https://user-images.githubusercontent.com/6273324/131238935-c8d9c76f-a207-475b-b547-c7f78abf6093.png)

### 8/21/2021
* Updated model with subsurface reference material, more tentacle bones, and a bit of webbing

![image](https://user-images.githubusercontent.com/6273324/130657867-c3d8be71-6cbc-47b4-861f-7032d899d6ae.png)
* Naive tentacle script to curl bones around target point - doesn't yet aim towards target, just matches distance

![image](https://user-images.githubusercontent.com/6273324/130656113-66a68f39-882b-4d18-8804-36f24afb3e81.png)
* Code necessary to aim tentale's root bone towards the target, from global to local-bone-space, works great --- except it's requiring using a magic value to reverse the z-axis? Something smells...

![image](https://user-images.githubusercontent.com/6273324/130660353-75822f06-e806-43cf-b39b-5ee00bde4df2.png)
* Single tentacle targetting point great! Easily generalize to all tentacles

![image](https://user-images.githubusercontent.com/6273324/130658093-4a0218f2-5001-47e1-bd0d-2ecd77db237d.png)

### 8/19/2021
* Adding new `components` member to existing wiScene to allow for generic component retrievable functions

![image](https://user-images.githubusercontent.com/6273324/130273976-b9fa3ab8-6bcb-4e90-967d-258d374e9328.png)
* Using more ergonomic query system (c++20 is fun!) to get the octopus tentacles out into new vectors for use in procedural animation

![image](https://user-images.githubusercontent.com/6273324/130282790-41121cb6-1b99-475b-8799-3a803df7bdf8.png)
* Simple animation to move newly gathered octopus appendages!

![image](https://user-images.githubusercontent.com/6273324/130274958-e57cef68-d94f-478e-9b24-3753e04f83e7.png)
* Import and integrate existing `Translator` tool from the Editor project to allow for playing with objects during game execution

![image](https://user-images.githubusercontent.com/6273324/130274422-d26b43f4-71d7-430f-9683-d2e43f13c2aa.png)
* Now with new translator inside the executable, I can successfully have the octopus wear the teapot. Finally.

![image](https://user-images.githubusercontent.com/6273324/130273215-d7f5281d-2368-4986-b4e3-e8ed6440ceca.png)

### 8/18/2021
* Created a simple octopus model in Blender rigged with some tentacle bones

![image](https://user-images.githubusercontent.com/6273324/130271616-8b6ed30d-3f4a-4283-8c9b-d2e0404bc35c.png)
* Exported as .GLB and am able to view in the existing Wicked Engine - Editor

![image](https://user-images.githubusercontent.com/6273324/130271543-e111600e-a2ee-484c-8df6-cd88fdb88ca5.png)
* Went through simple tutorial for setting up 2D and 3D rendering, modify somewhat to allow for strobe lights

![image](https://user-images.githubusercontent.com/6273324/130272111-189e4126-d647-4fa1-b0df-ac5619f25680.png)


