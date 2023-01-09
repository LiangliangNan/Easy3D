# Multiple Point Lights - Tutorial 25

In this tutorial we add support for multiple point light objects in the scene. Lights will still be stored in the GlobalUbo, however for rendering the light objects we will use push constants.

With the simple implementation shown in the video, each light in the scene contributes to every fragment's lighting calculation. For this reason, I set a limit on the total number of lights possible. With a more complex implementation this limit can be increased significantly, but is beyond the scope of the material we have covered up to this point.

#### Paste Bin Links

[Light Colors](https://pastebin.com/HJZXyBTi)

#### Tutorial Links

[View branch](https://github.com/blurrypiano/littleVulkanEngine/tree/tut25)

[View Files Changed](https://github.com/blurrypiano/littleVulkanEngine/pull/26/files)

[Video Tutorial](https://youtu.be/1olS6ayckKM)
