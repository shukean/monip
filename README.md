monip  
=====  

17mon.cn 的PHP扩展版本  

采用了monip内置类支持IP与位置的对应.  

##install
git clone https://github.com/shukean/monip.git  
./phpize   
./configue --with-php-config=your_php_config_path  
make && make install  

##php.ini
1. monip.cache_enable  
开启php进程缓存功能，request间可以重复利用。默认为request缓存。 设置On ／ 1 开启该功能。  

2. monip.cache_expire_time  
开启cache_enable后，设置数据的有效期，单位为秒。默认为0， 永久有效。  

3. monip.default_ipdata_file  
设置默认IP数据文件的位置。  当构造函数的参数为空时，使用此值。   


##class
```$ip = new Yk\Ip();```
or  
```$ip = new monip();```  


##func

```$ret_arr = $ip->find('beequick.cn');```



