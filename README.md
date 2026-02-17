# gpsHttpServer

This a simple HTTP server (Port 8080) which are handling :
1. /api/send_devinfo
2. /api/send_location
3. /api/get_location
4. /api/get_newid
All data format has application/json type.

## Dependencies 

		apt-get update && apt-get install -y build-essential cmake git curl openssl libssl-dev
		apt-get install -y qt6-base-dev qt6-base-dev-tools qt6-httpserver-dev qt6-websockets-dev


## Build using cmake

		git clone https://github.com/aks-arise1600/gpsHttpServer.git
		cd gpsHttpServer
		mkdir cBuild && cd cBuild
		cmake ..
		make
		
## Build using qmake

		git clone https://github.com/aks-arise1600/gpsHttpServer.git
		cd gpsHttpServer
		mkdir qBuild && cd qBuild
		qmake ..
		make
		
## Docker build 

		git clone https://github.com/aks-arise1600/gpsHttpServer.git
		cd gpsHttpServer
		docker build -t gsp-httpserver .
		docker run -p 8080:8080 gps-httpserver


