<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('ipip')) {
	dl('ipip.' . PHP_SHLIB_SUFFIX);
}
$module = 'ipip';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
if (class_exists("ipip")) {
	echo "Class ipip Methods:\n";
	print_r(get_class_methods('ipip'));
}

print_r(ipip_find("127.0.0.1"));
print_r(ipip::find("127.0.0.1"));
$myip = new ipip;
print_r($myip->find("168.0.0.1"));

?>
