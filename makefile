driver: driver/makefile
	cd driver
	make
server: server/package.json
	cd server
	npm start
app: app/monitor.html
	open app/monitor.html