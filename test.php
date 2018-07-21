<?php

$t = microtime(true);
$i = 10000;
while ($i > 0) {
    $ip = [
        mt_rand(1,  255),
        mt_rand(1,  255),
        mt_rand(1,  255),
        mt_rand(1,  255)
    ];
    $ipstr = implode('.', $ip);
    #echo $ipstr, PHP_EOL;
    #var_dump(ipip_find($ipstr));
    ipip_find($ipstr);
    $i--;
}
echo microtime(true) - $t;
