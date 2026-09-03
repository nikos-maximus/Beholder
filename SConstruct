import os

libPath = os.environ['HOME'] + '/lib'

env = Environment( 
    CPPPATH=[
        libPath + '/SDL/include', 
        libPath + '/VulkanSDK/1.4.350.1/x86_64/include', 
    #     os.environ['HOME'] + '/lib/VulkanSDK/1.4.350.1/x86_64/include', 
        libPath + '/volk', 
        libPath + '/VulkanMemoryAllocator/include', 
    #     './thirdparty/glm', 
        './include' 
    ] 
)

env.Program(target='Beholder_SDL', srcdir = 'src', source='bhMain.cpp')

#print(os.environ['HOME'])
