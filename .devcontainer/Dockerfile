ARG ARCH="linux/amd64"
FROM --platform=${ARCH} cross_clang16:latest

COPY ./certs/forge.escad.de.crt /root/forge.escad.de.crt
RUN git config --global http.sslCAInfo "/root/forge.escad.de.crt"


RUN apt-get update && apt-get install -y lcov doxygen graphviz