# FlashNess

Application of Outdated NPAPI Technology.

使用NPAPI模拟类Flash技术实现，封装NAPI for FF/Chrome和ActiveX for IE。

# 目录说明

|目录|说明|备注|
|---|----|----|
|mozilla|火狐组织代码，开源SDK|请勿修改|
|npapi|NPAPI SDK|请勿修改|
|npplugin|NPAPI插件的实现|已经实现自动化接口处理及数据转调，默认可以不做修改即可使用|
|regs|注册表操作，包含32位/64位，及相关安全性处理|使用者根据程序生成私有化的CLSID，修改对应的ID值即可|
|flashness|FlashNess插件实现代码|用户根据需要生成对应的业务代码和CLSID等信息，接口实现参考样例|
|release|编译生成目录，保护生成的npFlashNess.dll和注册批处理、测试页面等|用户可以注册后打开test.html进行简单测试|

# 开发文档

1. 工程创建

    使用Visual Studio向导创建活动模板库(使用ATL)，作为开始编写动态链接库(DLL)的起点。

2. 文件说明

    |文件名|说明|备注|
    |-----|----|---|
    |FlashNess.vcxproj|vs向导生成vc++项目的主项目文件，包含VC++的版本信息以及平台、配置和项目功能的信息||
    |FlashNess.vcxproj.filters|向导生成的项目筛选器文件，包含筛选器及对应的文件信息||
    |FlashNess.idl|项目定义的类型库、接口、组件类的IDL定义，由MIDL编译器进行处理生成C++接口定义和GUID声明(FlashNess.h)、GUID定义(FlashNess_i.c)、类型库(FlashNess.tlb)、封送处理代码(FlashNess_p.c和dlldata.c)|工程核心文件，接口定义|
    |FlashNess.h|包含FlashNess.idl中定义的项目的C++接口定义和GUID声明，在编译过程中由MIDL重新生成|不用修改，修改idl文件后自动生成|
    |FlashNess.cpp|包含对象映射和DLL导出的接口实现|核心功能和业务逻辑实现|
    |FlashNess.rc|程序资源列表|可以在资源管理器中修改，尽量不要手动修改以免出现错误|
    |FlashNess.def|定义文件为链接器提供的有关DLL所要求导出的信息，例如想要导出某一个接口供外部直接调用|对于插件来说基本上不用修改，固定导出DllGetClassObject、DllCanUnloadNow、DllRegisterServer、DllUnregisterServer、DllInstall即可|
    |stdAfx.h/cpp|预编译文件|无需修改，已包含基本库|
    |resource.h|定义例如按钮ID、图片ID等|无需修改|

3. 接口实现


4. NPAPI剖析


# 如何开启新的征程？

使用如上demo如何开发一个新的基于NPAPI和ActiveX的插件体系？

1. 代码拉取

    ```
    git clone https://github.com/walklang/FlashNess.git
    ```

2. 使用向导新建ATL接口


3. NPAPI接口扩展(可选)




