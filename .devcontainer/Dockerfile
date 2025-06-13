ARG ARCH="linux/amd64"
FROM --platform=${ARCH} cross_clang16:iot-bookworm

COPY ./certs/forge.escad.de.crt /root/forge.escad.de.crt
RUN git config --global http.sslCAInfo "/root/forge.escad.de.crt"
