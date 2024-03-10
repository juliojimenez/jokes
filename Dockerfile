FROM ubuntu:latest
RUN apt-get update -yq && \
	apt-get upgrade -yq && \
	apt-get install -yq ca-certificates && \
	apt-get install -yq build-essential && \
	apt-get install -yq clang
WORKDIR /app
COPY public public/
COPY main.c .
COPY Makefile .
RUN make
RUN ls -l .
EXPOSE 8080
ENTRYPOINT [ "/app/jokes" ]
