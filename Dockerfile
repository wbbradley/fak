FROM ubuntu:14.04
RUN \
	apt-get update \
	&& apt-get install -y make clang

COPY . src

RUN \
	cd src \
	&& make
