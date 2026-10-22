import os

libPath = os.environ['HOME'] + '/lib'

env = Environment( 
    
    CPPPATH=[
        libPath + '/glm', 
        libPath + '/imgui', 
        # libPath + '/KTX-Software-4.4.2-Linux-x86_64/include', 
        libPath + '/SDL/include', 
        libPath + '/volk', 
        libPath + '/VulkanMemoryAllocator/include', 
        libPath + '/VulkanSDK/1.4.350.1/x86_64/include', 
        
        './include', 
    ], 

    LIBS=[
        'libSDL3', 
    ], 

    LIBPATH=[
        libPath + '/SDL/build', 
        # libPath + '/VulkanSDK/1.4.350.1/x86_64/lib', 
        # libPath + '/VulkanSDK/1.4.350.1/x86_64/lib/VulkanLoader/lib', 
    ]
)

# CacheDir('build_cache')

env.Program(
    target='bin/Beholder_SDL', 
    srcdir = 'src', 
    source=[
        'bhVMAImpl.cpp', 
        'bhVolkImpl.cpp', 
        'bhUtil.cpp', 
        'bhPlatform.cpp', 
        'bhLog.cpp',
        'bhConfig.cpp', 
        'bhVk.cpp', 
        'bhVkPipeline.cpp', 
        'bhMain.cpp', 
    ]
)

#print(os.environ['HOME'])
