#!/bin/sh
res=`ps | grep -c newcir`
if [ "$res" == "1" ];then
	echo "newcir is not running"
else
	echo "newcir is running"
	pid=`ps | grep newcir | head -n1 | cut -d" " -f2`
	echo newcir pid is $pid
	kill $pid
	echo "have kill newcir"
fi
rm /newcir
echo "rm done..."
tftp-hpa 192.168.0.137 -m binary -c get newcir
echo "tftp-hpa..."
chmod 777 newcir
echo "chmod ..."
echo "run newcir..."
/newcir