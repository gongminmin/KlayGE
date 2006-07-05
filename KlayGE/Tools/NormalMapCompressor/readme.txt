NormalMapCompressor是KlayGE的normal map压缩工具，可以把normal压缩成AL8或DXT5格式，分别由2:1和4:1的压缩率。

使用方法 NormalMapCompressor normal_map.dds new_normal_map.dds format
normal_map是源normal map文件名。
new_normal_map是压缩后的normal map文件名。
format可以使用"AL8"或"DXT5"。AL8方法的压缩率是2:1，无损压缩。DXT5方法的压缩率是4:1，有一定的损失。

龚敏敏, 2006
