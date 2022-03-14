FROM ubuntu:20.04

RUN apt-get update && apt-get install -y build-essential wget --no-install-recommends

RUN ln -sf /usr/share/zoneinfo/Asia/Seoul /etc/localtime

RUN rm -rf /var/lib/apt/lists/*

WORKDIR /cmake
RUN wget https://github.com/Kitware/CMake/releases/download/v3.23.0-rc3/cmake-3.23.0-rc3-linux-x86_64.tar.gz --no-check-certificate
RUN tar -xvf cmake-3.23.0-rc3-linux-x86_64.tar.gz
ENV PATH="/cmake/cmake-3.23.0-rc3-linux-x86_64/bin:${PATH}"

COPY . /app

WORKDIR /app/build
RUN cmake .. &&\
    make -j "$(nproc)"

CMD ["/app/bin/UnitTest"]