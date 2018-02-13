const WindowsTryicon = require("..");

const myTrayApp = new WindowsTryicon({
	icon: "c:\\tmp\\icon.ico",
	menu: [
		{
			id: "http://www.google.com",
			caption: "Google"
		},
		{
			id: "http://www.cnn.com",
			caption: "cnn"
		},
		{
			id: "exit",
			caption: "Exit"
		}
	]
});

myTrayApp.item((id) => {
	switch (id) {
		case "item-1-id": {
			// do something
			break;
		}
		case "item-2-id": {
			myTrayApp.balloon("Hello There!", "This is my message to you").then(() => {
				console.log("Balloon dismissed");
			})
			break;
		}
		case "item-3-id-exit": {
			myTrayApp.exit();
			break;
		}
	}
});

console.log("test end");