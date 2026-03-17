# Physica

physica is a high performance framework for building GUI interfaces primarely focussed on interfaces for scientific instruments

## Build

**currently physica is only buildable on windows!**

first make sure to download the [VulkanSDK](https://vulkan.lunarg.com/sdk/home) and install it on your computer, premake will figure out automatically where it is located!

Then clone the repository recursively to include all submodules
```
git clone --recursive https://github.com/ThijsJansen02/Physica.git <target directory>
```

To build a configuration for your visual studio edition open a command window in the root directory and run:

```
buildconfig\\premake5 vs20**
```

Your should now have a buildable visual studio solution!
