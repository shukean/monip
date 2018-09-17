## ipip  
A php 7.x c module extension for search ip address.  
**warning**  
Only support datx format data file, because dat format is give up by [IPIP](https://en.ipip.net/?origin=CN)  

## design 
Load datx file data in memory on this module started (PHP_MINIT_FUNCTION),  
lookup ip address base on memory cache buf, so very quickly.  
### random test 10000 times  
Used low than 25ms on php-fpm mode (data is loaded)  
Used low than 40ms on php cli mode  

## update in php.ini
You need add this config items to you php.ini file, like this:  
```
extension="ipip.so"

[ipip]
ipip.datx_file = "path to ipip datx file"
```

## install
```
git clone https://github.com/shukean/monip.git  
cd monip  
/you install path/phpize  
./configue --with-php-config=/you install path/php-config  
make && make install  
```

## methods list
If find failed return FALSE, else return Array  
```
$ret = ipip_find("127.0.0.1");
if (!$ret) {
  print_r($ret);
}
```

```
$ret = ipip::find("127.0.0.1");
if (!$ret) {
  print_r($ret);
}
```

```
$myip = new ipip();
$ret = $myip->find("127.0.0.1");
if (!$ret) {
  print_r($ret);
}
```

output:  
```
Array
(
    [0] => 本机地址
    [1] => 本机地址
)
```
