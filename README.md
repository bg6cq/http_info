## http_info 显示HTTP协议的服务器端信息

````
usage:
    http_info [ -d ] [ -w wait_time ] [ -s -t table_name ] [ -i url_file | url ]
          -d              debug
          -w wait_time    wait_time when do curl
          -s              ouput sql replace into statement
          -t table_name   sql replace table_name
          -i url_file     read url from url_file
````
http_info 执行 curl 去获取网站的HTTP 头信息，因此需要安装curl才可以正常使用。

## 输出可以直接插入数据库
   数据库表结构
````
CREATE TABLE `http_info` (
  `url` varchar(255) NOT NULL,
  `server` varchar(255) NOT NULL DEFAULT '',
  `soft` varchar(255) NOT NULL DEFAULT '',
  `tag` varchar(255) NOT NULL DEFAULT '',
  `lastcheck` datetime NOT NULL DEFAULT '0000-00-00 00:00:00',
  PRIMARY KEY (`url`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
````

## 例子输出
````
./http_info http://202.38.64.8
"http://202.38.64.8" "Apache-Coyote/1.1" "" "java"
./http_info https://114.214.166.119
"https://114.214.166.119" "Apache/2.2.15 (CentOS)" "PHP/5.3.3" "php"
./http_info -s https://114.214.166.119
replace into http_info (url,server,soft,tag,lastcheck) values("https://114.214.166.119","Apache/2.2.15 (CentOS)","PHP/5.3.3","php",now());

````

## 我的使用
* 为避免扫描对核心交换机的冲击，预先生成近10分钟在线的IP，文件为 ustcip.txt
* 使用masscan得到开放80,8080,443端口的IP，文件为web.txt
````
masscan -p 80,8080,443 -iL ustcip.txt --wait 2 --max-rate 2000 -oL web.txt --excludefile exclude.txt
````
*  使用http_info获取服务器信息
````
opt="-d"
grep "open tcp" web.txt |while read a b port ip; do
	echo $ip $port
	if [ $port == 443 ]; then
        	./http_info $opt https://$ip
	elif [ $port == 80 ]; then
        	./http_info $opt http://$ip
	else
        	./http_info $opt http://$ip:$port
	fi
done
````


