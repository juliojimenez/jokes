FROM ubuntu:latest
RUN apt-get update -yq && apt-get upgrade -yq
COPY public /app
COPY jokes /app
EXPOSE 8080
ENTRYPOINT [ "/app/jokes" ]
