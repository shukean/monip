monip
=====

17mon.cn 的PHP扩展版本


提供了三个方法：

bool monip_init(string file);
file为17monipdb.dat的文件路径，该方法会做一些准备工作，大体同php的版本的init()方法。
这些数据将会随PHP一起长存在内存中，或者直到手动调用monip_clear()方法释放。

bool monip_clear();
该方法将会释放由monip_init()以及在查找过程中产生的缓存数据。

array monip_find(string ip);
该方法将会返回IP的对应的地址信息。但没有实现原PHP版中的gethostbyname()。
