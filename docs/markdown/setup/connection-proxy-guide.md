# Connection Proxy Setup Guide

Timestream ODBC driver supports connecting to Amazon Timestream through a proxy. To use this feauture the following environment variables should be configured based on your proxy setting.

|  Environment Variable       |   Comment                                                              |
|-----------------------------|------------------------------------------------------------------------|
|  TS_PROXY_HOST              |  Proxy host                                                            |
|  TS_PROXY_PORT              |  Proxy port                                                            |
|  TS_PROXY_SCHEME            |  Proxy scheme, `http` or `https`                                       |
|  TS_PROXY_USER              |  User name for proxy authentication                                    |
|  TS_PROXY_PASSWORD          |  User password for proxy authentication                                |
|  TS_PROXY_SSL_CERT_PATH     |  SSL Certificate file to use for connecting to an HTTPS proxy          |
|  TS_PROXY_SSL_CERT_TYPE     |  Type of proxy client SSL certificate                                  |
|  TS_PROXY_SSL_KEY_PATH      |  Private key file to use for connecting to an HTTPS proxy              |
|  TS_PROXY_SSL_KEY_TYPE      |  Type of private key file used to connect to an HTTPS proxy            |
|  TS_PROXY_SSL_KEY_PASSWORD  |  Passphrase to the private key file used to connect to an HTTPS proxy  |
