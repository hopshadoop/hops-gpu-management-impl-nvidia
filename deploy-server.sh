#!/bin/bash

set -e

if [ $# -ne 2 ] ; then
  echo "usage: <prog> hadoop_version ndb_version (e.g., 2.4.0 7.4.6)"
  exit 1
fi

VERSION=`grep -o -a -m 1 -h -r "version>.*</version" pom.xml | head -1 | sed "s/version//g" | sed "s/>//" | sed "s/<\///g"`


server=glassfish@snurran.sics.se:/var/www/hops/

mvn clean install assembly:assembly -DskipTests

echo "Deploying Hops - NVIDIA connector...."
scp target/hops-gpu-management-impl-nvidia-${VERSION}-jar-with-dependencies.jar $server/nvidia-management-$1-$2.jar

