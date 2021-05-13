#!/bin/bash

# https://github.com/apache/thrift/blob/077b5fce825e79d84592fff893639b92b637eec7/test/keys/README.md

# generate server key
openssl ecparam -genkey -name secp384r1 -noout -out "server.key"

# generate server certificate
openssl req -new -key "server.key" -days 3000 -x509 -sha384 -subj "/C=US/ST=Maryland/L=Forest Hill/O=The Apache Software Foundation/OU=Apache Thrift/CN=localhost/emailAddress=dev@thrift.apache.org" -out "server.cert"

# export ca pem
openssl x509 -in "server.cert" -text -out "ca.pem"

# generate client key
# openssl ecparam -genkey -name secp384r1 -noout -out "client.key"

# create a signing request
# openssl req -new -key "client.key" -subj "/C=US/ST=Maryland/L=Forest Hill/O=The Apache Software Foundation/OU=Apache Thrift/CN=localhost/emailAddress=dev@thrift.apache.org" -out "client.csr"

# sign the client certificate with the server.key
# openssl x509 -sha384 -req -days 3000 -in "client.csr" -CA "ca.pem" -CAkey "server.key" -set_serial 01 -out "client.cert"

# pub key from priv
# openssl ec -in server.key -pubout -out server_pub.key

# print certificate
# openssl x509 -in server.cert -text -noout
