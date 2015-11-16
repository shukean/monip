monip  
=====  

17mon.cn 的PHP扩展版本  

采用了monip内置类支持IP与位置的对应.  

#install
git clone https://github.com/shukean/monip.git  
./configue --with-php-config=your_php_config_path  
make && make install  

#demo

$monip = new monip('ip_data_file');  
$location = $monip->find('www.baidu.com');  
if($location){  
  var_dump($location);      //array  
}else{  
  var_dump($location);      // NULL  
}  

Array  
(  
    [0] => 中国  
    [1] => 北京  
    [2] => 北京  
)  
