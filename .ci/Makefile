build:
	sudo docker build . -t patwie/platformio

run:
	sudo docker run --rm -it --mount type=bind,source="$(pwd)",target=/project -u `id -u $USER`:`id -g $USER` patwie/platformio:latest bash
