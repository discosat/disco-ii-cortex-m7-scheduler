FROM debian:buster-slim

RUN apt-get update && apt-get install -y \
    curl \
    tar \
    bzip2 \
    bash \
    make \
    cmake \
    ca-certificates \
    --no-install-recommends && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /usr/local/arm
RUN curl -L "https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-x86_64-linux.tar.bz2?rev=78196d3461ba4c9089a67b5f33edf82a&hash=D484B37FF37D6FC3597EBE2877FB666A41D5253B" -o gcc-arm-none-eabi.tar.bz2 \
    && tar -xjf gcc-arm-none-eabi.tar.bz2 -C /usr/local/arm \
    && rm gcc-arm-none-eabi.tar.bz2
ENV ARMGCC_DIR=/usr/local/arm/gcc-arm-none-eabi-10.3-2021.10

COPY fsimx8mp-m7-sdk /fsimx8mp-m7-sdk

WORKDIR /fsimx8mp-m7-sdk

RUN /bin/bash -c "./prepare.sh" <<EOF
1
d
EOF

RUN mkdir /tmp-src
CMD cp -r /src/* /tmp-src \
    && cd /tmp-src/armgcc \
    && ./build.sh \
    && if [ -d "/output/bin/debug" ]; then rm -r /output/bin/debug; fi \
    && if [ -d "/output/bin/release" ]; then rm -r /output/bin/release; fi \
    && mv debug /output/bin \
    && mv release /output/bin
