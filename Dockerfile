FROM ubuntu:14.04
RUN \
	apt-get update && \
	apt-get install -y make clang
RUN make
