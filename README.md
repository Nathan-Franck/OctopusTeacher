
# Octopus Teacher

![image](Content/logo_small.png)
## Forked From [Wicked Engine](https://github.com/turanszkij/WickedEngine)

## Objectives

* Provide myself a crash-course in modern C++ programming techniques and the latest and greatest in rendering
* Make a cool little demo about an octopus!

## Progress Milestones
### Day 1
* Created a simple octopus model in Blender rigged with some tentacle bones

![image](https://user-images.githubusercontent.com/6273324/130271616-8b6ed30d-3f4a-4283-8c9b-d2e0404bc35c.png)
* Exported as .GLB and am able to view in the existing Wicked Engine - Editor

![image](https://user-images.githubusercontent.com/6273324/130271543-e111600e-a2ee-484c-8df6-cd88fdb88ca5.png)
* Went through simple tutorial for setting up 2D and 3D rendering, modify somewhat to allow for strobe lights

![image](https://user-images.githubusercontent.com/6273324/130272111-189e4126-d647-4fa1-b0df-ac5619f25680.png)

### Day 2
* Adding new `components` member to existing wiScene to allow for generic component retrievable functions

![image](https://user-images.githubusercontent.com/6273324/130273976-b9fa3ab8-6bcb-4e90-967d-258d374e9328.png)
* Using more ergonomic query system (c++20 is fun!) to get the octopus tentacles out into new vectors for use in procedural animation

![image](https://user-images.githubusercontent.com/6273324/130274069-193aa50b-d103-4c21-a4af-37d3a37f825b.png)
* Simple animation to move newly gathered octopus appendages!

![image](https://user-images.githubusercontent.com/6273324/130274958-e57cef68-d94f-478e-9b24-3753e04f83e7.png)
* Import and integrate existing `Translator` tool from the Editor project to allow for playing with objects during game execution

![image](https://user-images.githubusercontent.com/6273324/130274422-d26b43f4-71d7-430f-9683-d2e43f13c2aa.png)
* Now with new translator inside the executable, I can successfully have the octopus wear the teapot. Finally.

![image](https://user-images.githubusercontent.com/6273324/130273215-d7f5281d-2368-4986-b4e3-e8ed6440ceca.png)


