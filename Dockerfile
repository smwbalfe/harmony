FROM ubuntu

# Set the working directory
WORKDIR /usr/src/app

# Copy the current directory contents into the container
COPY . /usr/src/app

# Install necessary packages
RUN apt-get update && apt-get install -y \
    gcc \
    cmake \
    ninja-build \
    python3-pip \
    build-essential \
    git \ 
    curl \ 
    zip \ 
    pkg-config \             
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

ENV PATH="${PATH}:/usr/bin/ninja"

RUN git clone https://github.com/microsoft/vcpkg.git

RUN ./vcpkg/bootstrap-vcpkg.sh

RUN mkdir build

WORKDIR ./build

RUN cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=/usr/src/app/vcpkg/scripts/buildsystems/vcpkg.cmake && ninja

RUN ninja

RUN mkdir index

CMD ["./harmony"]
