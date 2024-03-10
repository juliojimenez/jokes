FROM ubuntu:latest
RUN apt-get update -yq && \
	apt-get upgrade -yq && \
	apt-get install -yq ca-certificates && \
	apt-get install -yq build-essential && \
	apt-get install -yq clang
COPY public /app/public
COPY main.c /app
COPY Makefile /app
RUN make -C /app
EXPOSE 8080
ENTRYPOINT [ "/app/jokes" ]
