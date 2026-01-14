# Switch to CentOS vault repositories.
/^mirrorlist=/d
s|^# *baseurl=http://mirror.centos.org|baseurl=http://vault.centos.org|
