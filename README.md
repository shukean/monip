monip
=====
17mon.cn 的PHP扩展版本

#Linux下编译
/path/to/phpize
./configure --with-php-config=/path/to/php-config
make && make install

#方法
bool monip_init(string file);
file为17monipdb.dat的文件路径,返回false表示失败，true表示初始化成功，NULL表示不需要再次初始化。
20140520-现在该方法如果传入的文件名不同，最后一次的数据将会替换之前的数据。

bool monip_clear(bool clear = 0);
该方法将会释放在查找IP过程中产生的缓存数据。
20140521-现在该方法接受一个参数，为假时只清除ip对应的缓存数据，为真时还将清除monip_init初始化的信息。

array monip_find(string ip);
该方法将会返回IP的对应的地址信息（现已实现gethostbyname）。

#测试结果
monip_find(‘www.baidu.com’);
输出:
Array
(
    [0] => 中国
    [1] => 浙江
)
