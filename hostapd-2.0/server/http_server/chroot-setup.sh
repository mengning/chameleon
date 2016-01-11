#!/bin/sh -x
if id | grep -qv uid=0; then
    echo "Must run setup as root"
    exit 1
fi

create_socket_dir() {
    local dirname="$1"
    local ownergroup="$2"
    local perms="$3"

    mkdir -p $dirname
    chown $ownergroup $dirname
    chmod $perms $dirname
}

set_perms() {
    local ownergroup="$1"
    local perms="$2"
    local pn="$3"

    chown $ownergroup $pn
    chmod $perms $pn
}

rm -rf /jail
mkdir -p /jail
cp -p index.html /jail

#zookd user from httpd to root
./chroot-copy.sh touchstone /jail

./chroot-copy.sh httpd /jail
#./chroot-copy.sh zookfs /jail

./chroot-copy.sh filesv /jail
./chroot-copy.sh banksv /jail

./chroot-copy.sh /bin/bash /jail

./chroot-copy.sh /usr/bin/env /jail
./chroot-copy.sh /usr/bin/python /jail

# to bring in the crypto libraries
./chroot-copy.sh /usr/bin/openssl /jail
#
#dev      etc         lib         tmp  zoobar  zookfsdyn
#echosvc  index.html  profilesvc  usr  zookd   zookfssta

mkdir -p /jail/usr/lib /jail/usr/lib/i386-linux-gnu /jail/lib /jail/lib/i386-linux-gnu
#extra_files : usr
cp -r /usr/lib/python2.6 /jail/usr/lib
cp -r /usr/lib/pymodules /jail/usr/lib
#cp /usr/lib/i386-linux-gnu/libsqlite3.so.0 /jail/usr/lib/i386-linux-gnu
cp /usr/lib/libxslt.so.1 /jail/usr/lib
cp /usr/lib/libexslt.so.0 /jail/usr/lib
cp /usr/lib/libxml2.so.2 /jail/usr/lib

mkdir -p /jail/usr/local/lib
cp -r /usr/local/lib/python2.6 /jail/usr/local/lib

#extra_files : lib
cp /lib/libgcrypt.so.11 /jail/lib
cp /lib/libgpg-error.so.0 /jail/lib
cp /lib/i386-linux-gnu/libnss_dns.so.2 /jail/lib/i386-linux-gnu
cp /lib/i386-linux-gnu/libresolv.so.2 /jail/lib/i386-linux-gnu

#/lib/resolvconf not exist..
#cp -r /lib/resolvconf /jail/lib


#extra_files :  etc
mkdir -p /jail/etc
#extra_files
cp /etc/localtime /jail/etc/
cp /etc/timezone /jail/etc/
cp /etc/resolv.conf /jail/etc/

mkdir -p /jail/usr/share/zoneinfo
#extra_files
cp -r /usr/share/zoneinfo/America /jail/usr/share/zoneinfo/


#extra_files : tmp
mkdir -p /jail/tmp
chmod a+rwxt /jail/tmp

#extra_files : dev 
mkdir -p /jail/dev
mknod /jail/dev/urandom c 1 9

#copy directory: sql-lite3 , db
cp -r sql-lite3 /jail/
cp -r db        /jail/
#rm -rf /jail/db/users.db
#touch  /jail/db/users.db

#copy some static files , a.txt b.txt
cp a.txt /jail/
cp b.txt /jail/

#fill in your commands here
#using chgrp, chmod, chown and etc..
chown 1100 /jail/index.html
chgrp 1200 /jail/index.html
chmod 740 /jail/index.html

chown 2100 /jail/a.txt
chgrp 1100 /jail/a.txt
chmod 740 /jail/a.txt

chown 3100 /jail/b.txt
chgrp 3200 /jail/b.txt
chmod 740 /jail/b.txt


chown -R 1200 /jail/db/
chgrp -R 1200 /jail/db/
chmod 700 /jail/db/
chown  1200 /jail/db/users.db
chgrp  1200 /jail/db/users.db
chmod 700 /jail/db/users.db
