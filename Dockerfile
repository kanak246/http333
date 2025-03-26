FROM ubuntu:22.04

# Install compilers and boost
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    libboost-all-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /usr/src/app

COPY . .

RUN make

EXPOSE 8000

CMD ["./http333d", "8000"]